#include "loadShader.hpp"
#include "cheat.h"

std::random_device rd{};
std::default_random_engine dre{ rd() };
std::uniform_int_distribution colorUid(0, 5);

namespace beginConfig {
	int width{ 800 };
	int height{ 800 };
	Color bg{ 1,1,1,1 };
}
namespace CameraConfig {
	glm::vec3 pos{ -5.f,5.f,-2.f };
	glm::vec3 dir{ 0,0,0 };
	glm::vec3 up{ 0,1,0 };
}

namespace ViewfConfig {
	float fovy{ glm::radians(45.f) };
	float aspect{ 1.f };
	float n{ 1.f };
	float f{ 100.f };
}

GameTimer* GameTimer::Instance;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid RanColor(Color&);
GLvoid Mouse(int, int, int, int);
GLvoid loop(int);


void createAxis();
void drawAxis();
void createCylinder();
void createSphere();
void createCone();
void createCube();
void drawCylinder();
void drawSphere();
void drawCone();
void drawCube();
void drawObject1();  // 첫 번째 객체 (원기둥 또는 원뿔)
void drawObject2();  // 두 번째 객체 (구 또는 육면체)
void resetToInitialState();

// Shader
GLuint bs{};
GLuint VBO{};
GLuint axisVBO{};
GLuint cylinderVBO{};
GLuint sphereVBO{};
GLuint coneVBO{};
GLuint cubeVBO{};
std::vector<float> Vertex{};
std::vector<float> cylinderVertex{};
std::vector<float> sphereVertex{};
std::vector<float> coneVertex{};
std::vector<float> cubeVertex{};

GameTimer gt;
int ci = colorUid(dre);

int num{};
int method{};
Color bg = beginConfig::bg;

// 선택 모드 (1: 원기둥만, 2: 구만, 3: 둘 다)
int mode{ 3 };

// 객체 타입 (0: 원기둥/구, 1: 원뿔/육면체)
int shapeType{ 0 };

// 애니메이션 상태
enum AnimationType { ANIM_NONE, ANIM_T, ANIM_U, ANIM_V };
AnimationType currentAnim{ ANIM_NONE };
float animTime{ 0.0f };
bool animating{ false };
bool vAnimToggle{ false }; // V 애니메이션 토글 상태
const float ANIM_DURATION = 2.0f; // 애니메이션 지속 시간 (초)

// 애니메이션용 임시 변수
struct {
	float cylinderX{}, cylinderY{}, cylinderZ{};
	float sphereX{}, sphereY{}, sphereZ{};
	float scale{ 1.0f };
	float rotationX{ 0.0f };
	float rotationY{ 0.0f };
	float rotationZ{ 0.0f };
	float revolution{ 0.0f };
} anim;

// 원기둥 변환 변수
struct {
	float rotationX{};
	float rotationY{};
	float revolutionAngle{};
	float localScale{ 1.0f };
	float globalScale{ 1.0f };
	float positionX{};
	float positionY{};
	float baseX{ 2.0f };  // 기본 위치 X
} cylinder;

// 구 변환 변수
struct {
	float rotationX{};
	float rotationY{};
	float revolutionAngle{};
	float localScale{ 1.0f };
	float globalScale{ 1.0f };
	float positionX{};
	float positionY{};
	float baseX{ -2.0f };  // 기본 위치 X
} sphere;


void main(int argc, char** argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(beginConfig::width, beginConfig::height);
	glutCreateWindow("Example1");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl
			;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}
	bs = CompileShaders("CG1-15.vs", "CG1-15.fs");

	glEnable(GL_DEPTH_TEST);

	createAxis();
	createCylinder();
	createSphere();
	createCone();
	createCube();

	//std::cout << "x/X : 객체의 x축에 대하여 각각 양/음 방향으로 회전하기(자전)" << std::endl;
	//std::cout << "y/Y: 객체의 y축에 대하여 각각 양/음 방향으로 회전하기 (자전)" << std::endl;
	//std::cout << "r/R: 중앙의 y축에 대하여 양/음 방향으로 회전하기 (공전)" << std::endl;
	//std::cout << "a/A: 도형이 제자리에서 확대/축소" << std::endl;
	//std::cout << "b/B: 도형이 원점에 대해서 확대/축소 (위치도 바뀐다.)" << std::endl;
	//std::cout << "d/D: 도형이 x축에서 좌/우로 이동" << std::endl;
	//std::cout << "e/E: 도형이 y축에서 위/아래로 이동" << std::endl;
	//std::cout << "t: 두 도형이 원점을 통과하며 상대방의 자리로 이동하는 애니메이션" << std::endl;
	//std::cout << "u: 두 도형이 한 개는 위로, 다른 도형은 아래로 이동하면서 상대방의 자리로 이동하는 애니메이션" << std::endl;
	//std::cout << "v: 키보드 5: 두 도형이 한 개는 확대, 다른 한 개는 축소되며 자전과 공전하기" << std::endl;
	//std::cout << "c: 두 도형을 다른 도형으로 바꾼다" << std::endl;
	//std::cout << "s: 초기화 하기" << std::endl <<std::endl;

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutTimerFunc(1, loop, 1);
	glutMainLoop();
}

GLvoid drawScene()
{

	glClearColor(bg.r, bg.g, bg.b, bg.al);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 셰이더 사용하여 그리기
	GLuint shader = bs;
	glUseProgram(shader);

	drawAxis();

	if (mode == 1 || mode == 3) {
		drawObject1();
	}
	if (mode == 2 || mode == 3) {
		drawObject2();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}


GLvoid Keyboard(unsigned char key, int x, int y)
{

	switch (key) {
	case '1':
		// 원기둥만 선택
		mode = 1;
		std::cout << "Mode: Cylinder only" << std::endl;
		break;
	case '2':
		// 구만 선택
		mode = 2;
		std::cout << "Mode: Sphere only" << std::endl;
		break;
	case '3':
		// 둘 다 선택
		mode = 3;
		std::cout << "Mode: Both objects" << std::endl;
		break;
	case 'x':
		// x축에 대해 +방향으로 자전
		if (mode == 1 || mode == 3) cylinder.rotationX += 5.0f;
		if (mode == 2 || mode == 3) sphere.rotationX += 5.0f;
		break;
	case 'X':
		// x축에 대해 -방향으로 자전
		if (mode == 1 || mode == 3) cylinder.rotationX -= 5.0f;
		if (mode == 2 || mode == 3) sphere.rotationX -= 5.0f;
		break;
	case 'y':
		// y축에 대해 +방향으로 자전
		if (mode == 1 || mode == 3) cylinder.rotationY += 5.0f;
		if (mode == 2 || mode == 3) sphere.rotationY += 5.0f;
		break;
	case 'Y':
		// y축에 대해 -방향으로 자전
		if (mode == 1 || mode == 3) cylinder.rotationY -= 5.0f;
		if (mode == 2 || mode == 3) sphere.rotationY -= 5.0f;
		break;
	case 'r':
		// y축에 대해 +방향으로 공전
		if (mode == 1 || mode == 3) cylinder.revolutionAngle += 5.0f;
		if (mode == 2 || mode == 3) sphere.revolutionAngle += 5.0f;
		break;
	case 'R':
		// y축에 대해 -방향으로 공전
		if (mode == 1 || mode == 3) cylinder.revolutionAngle -= 5.0f;
		if (mode == 2 || mode == 3) sphere.revolutionAngle -= 5.0f;
		break;
	case 'a':
		// 제자리에서 확대
		if (mode == 1 || mode == 3) cylinder.localScale += 0.1f;
		if (mode == 2 || mode == 3) sphere.localScale += 0.1f;
		break;
	case 'A':
		// 제자리에서 축소
		if (mode == 1 || mode == 3) {
			cylinder.localScale -= 0.1f;
			if (cylinder.localScale < 0.1f) cylinder.localScale = 0.1f;
		}
		if (mode == 2 || mode == 3) {
			sphere.localScale -= 0.1f;
			if (sphere.localScale < 0.1f) sphere.localScale = 0.1f;
		}
		break;
	case 'b':
		// 원점에 대해 확대
		if (mode == 1 || mode == 3) cylinder.globalScale += 0.1f;
		if (mode == 2 || mode == 3) sphere.globalScale += 0.1f;
		break;
	case 'B':
		// 원점에 대해 축소
		if (mode == 1 || mode == 3) {
			cylinder.globalScale -= 0.1f;
			if (cylinder.globalScale < 0.1f) cylinder.globalScale = 0.1f;
		}
		if (mode == 2 || mode == 3) {
			sphere.globalScale -= 0.1f;
			if (sphere.globalScale < 0.1f) sphere.globalScale = 0.1f;
		}
		break;
	case 'd':
		// x축에서 우측으로 이동
		if (mode == 1 || mode == 3) cylinder.positionX += 0.2f;
		if (mode == 2 || mode == 3) sphere.positionX += 0.2f;
		break;
	case 'D':
		// x축에서 좌측으로 이동
		if (mode == 1 || mode == 3) cylinder.positionX -= 0.2f;
		if (mode == 2 || mode == 3) sphere.positionX -= 0.2f;
		break;
	case 'e':
		// y축에서 위로 이동
		if (mode == 1 || mode == 3) cylinder.positionY += 0.2f;
		if (mode == 2 || mode == 3) sphere.positionY += 0.2f;
		break;
	case 'E':
		// y축에서 아래로 이동
		if (mode == 1 || mode == 3) cylinder.positionY -= 0.2f;
		if (mode == 2 || mode == 3) sphere.positionY -= 0.2f;
		break;
	case 't':
		// 원점을 통과하여 자리 바꾸기 애니메이션
		currentAnim = ANIM_T;
		animating = true;
		animTime = 0.0f;
		std::cout << "Animation T: Linear swap through origin" << std::endl;
		break;
	case 'u':
		// 위/아래로 이동하며 자리 바꾸기 애니메이션
		currentAnim = ANIM_U;
		animating = true;
		animTime = 0.0f;
		std::cout << "Animation U: Arc swap (up/down)" << std::endl;
		break;
	case 'v':
		// 확대/축소 + 자전 + 공전 애니메이션 토글
		vAnimToggle = !vAnimToggle;
		if (vAnimToggle) {
			currentAnim = ANIM_V;
			animating = true;
			animTime = 0.0f;
			std::cout << "Animation V: ON (Scale + Rotate + Revolve)" << std::endl;
		}
		else {
			animating = false;
			currentAnim = ANIM_NONE;
			animTime = 0.0f;
			// 애니메이션 초기화
			anim.cylinderX = anim.cylinderY = anim.cylinderZ = 0.0f;
			anim.sphereX = anim.sphereY = anim.sphereZ = 0.0f;
			anim.scale = 1.0f;
			anim.rotationX = anim.rotationY = anim.rotationZ = 0.0f;
			anim.revolution = 0.0f;
			std::cout << "Animation V: OFF" << std::endl;
		}
		break;
	case 'c':
		// 두 객체를 다른 3D 도형으로 변경
		shapeType = (shapeType + 1) % 2;
		if (shapeType == 0) {
			std::cout << "Shapes changed to: Cylinder and Sphere" << std::endl;
		}
		else {
			std::cout << "Shapes changed to: Cone and Cube" << std::endl;
		}
		break;
	case 's':
		// 완전 초기 상태로 돌아가기
		resetToInitialState();
		std::cout << "Reset to initial state" << std::endl;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

//GLvoid RanColor(Color& c)
//{
//	float r = urgb(dre);
//	float g = urgb(dre);
//	float b = urgb(dre);
//	c = { r / 255,g / 255,b / 255,1 };
//}

GLvoid Mouse(int button, int state, int x, int y)
{
	int once{ 0 };
	float mpx{ (static_cast<float>(x) - 400) / 400 }, mpy{ (static_cast<float>(-y) + 400) / 400 };
	if (state == GLUT_DOWN) {
		if (once == 0) {


			once = 1;
		}
	}
	if (state == GLUT_UP) {
		std::cout << "up" << std::endl;
		once = 0;
	}
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (once == 1) {
			once = 2;
		}
		break;
	case GLUT_MIDDLE_BUTTON:
		break;
	case GLUT_RIGHT_BUTTON:

		break;
	}


}

GLvoid loop(int v)
{
	gt.Update();

	if (animating) {
		animTime += gt.deltaTime;
		float t = animTime / ANIM_DURATION; // 0.0 ~ 1.0

		if (t >= 1.0f) {
			// V 애니메이션은 반복, 나머지는 종료
			if (currentAnim == ANIM_V && vAnimToggle) {
				// V 애니메이션 반복
				animTime = 0.0f;
			}
			else {
				// 애니메이션 종료
				// t, u 애니메이션의 경우 실제 기본 위치 교환
				if (currentAnim == ANIM_T || currentAnim == ANIM_U) {
					// 원기둥과 구의 기본 위치를 교환
					std::swap(cylinder.baseX, sphere.baseX);
				}

				animating = false;
				currentAnim = ANIM_NONE;
				animTime = 0.0f;
				vAnimToggle = false;

				// 애니메이션 초기화
				anim.cylinderX = anim.cylinderY = anim.cylinderZ = 0.0f;
				anim.sphereX = anim.sphereY = anim.sphereZ = 0.0f;
				anim.scale = 1.0f;
				anim.rotationX = anim.rotationY = anim.rotationZ = 0.0f;
				anim.revolution = 0.0f;
			}
		}
		else {
			switch (currentAnim) {
			case ANIM_T:
				// 원점을 통과하여 자리 바꾸기 (선형 보간)
				// 원기둥: 현재위치 -> (0,0,0) -> 상대방위치
				// 구: 현재위치 -> (0,0,0) -> 상대방위치
				anim.cylinderX = cylinder.baseX + (sphere.baseX - cylinder.baseX) * t;
				anim.sphereX = sphere.baseX + (cylinder.baseX - sphere.baseX) * t;
				break;

			case ANIM_U:
				// 위/아래로 이동하며 자리 바꾸기
				// 원기둥: 위로 포물선, 구: 아래로 포물선
				anim.cylinderX = cylinder.baseX + (sphere.baseX - cylinder.baseX) * t;
				anim.sphereX = sphere.baseX + (cylinder.baseX - sphere.baseX) * t;

				// 포물선 궤적 (sin 곡선)
				anim.cylinderY = 3.0f * sin(t * glm::pi<float>());
				anim.sphereY = -3.0f * sin(t * glm::pi<float>());
				break;

			case ANIM_V:
				// 확대/축소 반복 + 자전 + 공전
				// 스케일: 1.0 ~ 1.5 ~ 1.0 반복
				anim.scale = 1.0f + 0.5f * sin(t * glm::pi<float>() * 4.0f);

				// 모든 축에 대해 자전 (각 축마다 다른 속도로 회전)
				anim.rotationX = t * 720.0f;  // X축: 2회전
				anim.rotationY = t * 900.0f;  // Y축: 2.5회전
				anim.rotationZ = t * 540.0f;  // Z축: 1.5회전

				// 공전 (360도 * 1회전)
				anim.revolution = t * 360.0f;
				break;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(1, loop, 1);
}

void createAxis()
{
	static std::array<float, 42> axisVertexs = {
		-1,0,0,		1,0,0,1,
		1,0,0,		1,0,0,1,
		0,-1,0,		0,1,0,1,
		0,1,0,		0,1,0,1,
		0,0,-1,		0,0,1,1,
		0,0,1,		0,0,1,1
	};

	glGenBuffers(1, &axisVBO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertexs), axisVertexs.data(), GL_STATIC_DRAW);
}

void drawAxis()
{
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	GLuint shader = bs;
	glUseProgram(shader);

	viewTransMatrix = glm::lookAt(CameraConfig::pos, CameraConfig::dir, CameraConfig::up);
	projectTransMatrix = glm::perspective(ViewfConfig::fovy, ViewfConfig::aspect, ViewfConfig::n, ViewfConfig::f);

	int worldTLoc = glGetUniformLocation(shader, "worldT");
	int viewTLoc = glGetUniformLocation(shader, "viewT");
	int projectTLoc = glGetUniformLocation(shader, "projectionT");
	glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(worldTransMatrix));
	glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(viewTransMatrix));
	glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(projectTransMatrix));

	int aPosition = glGetAttribLocation(shader, "a_Pos");
	int aColor = glGetAttribLocation(shader, "a_Color");

	glEnableVertexAttribArray(aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glLineWidth(2.0f);

	glDrawArrays(GL_LINES, 0, 6);

	glDisableVertexAttribArray(aPosition);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createCylinder()
{
	cylinderVertex.clear();

	// 원기둥 생성
	float radius = 0.5f;
	float height = 2.0f;
	int segments = 50;
	int stacks = 20;

	// 옆면 생성
	ci = colorUid(dre);
	for (int i = 0; i < stacks; ++i) {
		float y1 = -height / 2.0f + (height * i / stacks);
		float y2 = -height / 2.0f + (height * (i + 1) / stacks);

		for (int j = 0; j < segments; ++j) {
			float theta1 = 2.0f * glm::pi<float>() * j / segments;
			float theta2 = 2.0f * glm::pi<float>() * (j + 1) / segments;

			float x1 = radius * cos(theta1);
			float z1 = radius * sin(theta1);
			float x2 = radius * cos(theta2);
			float z2 = radius * sin(theta2);

			// 첫 번째 삼각형
			cylinderVertex.push_back(x1); cylinderVertex.push_back(y1); cylinderVertex.push_back(z1);
			cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

			cylinderVertex.push_back(x2); cylinderVertex.push_back(y1); cylinderVertex.push_back(z2);
			cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

			cylinderVertex.push_back(x1); cylinderVertex.push_back(y2); cylinderVertex.push_back(z1);
			cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

			// 두 번째 삼각형
			cylinderVertex.push_back(x2); cylinderVertex.push_back(y1); cylinderVertex.push_back(z2);
			cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

			cylinderVertex.push_back(x2); cylinderVertex.push_back(y2); cylinderVertex.push_back(z2);
			cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

			cylinderVertex.push_back(x1); cylinderVertex.push_back(y2); cylinderVertex.push_back(z1);
			cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);
		}
	}

	// 윗면
	ci = colorUid(dre);
	for (int j = 0; j < segments; ++j) {
		float theta1 = 2.0f * glm::pi<float>() * j / segments;
		float theta2 = 2.0f * glm::pi<float>() * (j + 1) / segments;

		float x1 = radius * cos(theta1);
		float z1 = radius * sin(theta1);
		float x2 = radius * cos(theta2);
		float z2 = radius * sin(theta2);

		cylinderVertex.push_back(0.0f); cylinderVertex.push_back(height / 2.0f); cylinderVertex.push_back(0.0f);
		cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

		cylinderVertex.push_back(x1); cylinderVertex.push_back(height / 2.0f); cylinderVertex.push_back(z1);
		cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

		cylinderVertex.push_back(x2); cylinderVertex.push_back(height / 2.0f); cylinderVertex.push_back(z2);
		cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);
	}

	// 아랫면
	ci = colorUid(dre);
	for (int j = 0; j < segments; ++j) {
		float theta1 = 2.0f * glm::pi<float>() * j / segments;
		float theta2 = 2.0f * glm::pi<float>() * (j + 1) / segments;

		float x1 = radius * cos(theta1);
		float z1 = radius * sin(theta1);
		float x2 = radius * cos(theta2);
		float z2 = radius * sin(theta2);

		cylinderVertex.push_back(0.0f); cylinderVertex.push_back(-height / 2.0f); cylinderVertex.push_back(0.0f);
		cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

		cylinderVertex.push_back(x2); cylinderVertex.push_back(-height / 2.0f); cylinderVertex.push_back(z2);
		cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);

		cylinderVertex.push_back(x1); cylinderVertex.push_back(-height / 2.0f); cylinderVertex.push_back(z1);
		cylinderVertex.push_back(col[ci][0]); cylinderVertex.push_back(col[ci][1]); cylinderVertex.push_back(col[ci][2]); cylinderVertex.push_back(col[ci][3]);
	}

	glGenBuffers(1, &cylinderVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
	glBufferData(GL_ARRAY_BUFFER, cylinderVertex.size() * sizeof(float), cylinderVertex.data(), GL_STATIC_DRAW);
}

void createSphere()
{
	sphereVertex.clear();

	// 구 생성
	float radius = 0.5f;
	int stacks = 30;
	int sectors = 30;

	ci = colorUid(dre);
	for (int i = 0; i < stacks; ++i) {
		float phi1 = glm::pi<float>() * i / stacks;
		float phi2 = glm::pi<float>() * (i + 1) / stacks;

		for (int j = 0; j < sectors; ++j) {
			float theta1 = 2.0f * glm::pi<float>() * j / sectors;
			float theta2 = 2.0f * glm::pi<float>() * (j + 1) / sectors;

			// 구의 좌표 계산 (구면 좌표계)
			float x1 = radius * sin(phi1) * cos(theta1);
			float y1 = radius * cos(phi1);
			float z1 = radius * sin(phi1) * sin(theta1);

			float x2 = radius * sin(phi1) * cos(theta2);
			float y2 = radius * cos(phi1);
			float z2 = radius * sin(phi1) * sin(theta2);

			float x3 = radius * sin(phi2) * cos(theta1);
			float y3 = radius * cos(phi2);
			float z3 = radius * sin(phi2) * sin(theta1);

			float x4 = radius * sin(phi2) * cos(theta2);
			float y4 = radius * cos(phi2);
			float z4 = radius * sin(phi2) * sin(theta2);

			// 첫 번째 삼각형
			sphereVertex.push_back(x1); sphereVertex.push_back(y1); sphereVertex.push_back(z1);
			sphereVertex.push_back(col[ci][0]); sphereVertex.push_back(col[ci][1]); sphereVertex.push_back(col[ci][2]); sphereVertex.push_back(col[ci][3]);

			sphereVertex.push_back(x2); sphereVertex.push_back(y2); sphereVertex.push_back(z2);
			sphereVertex.push_back(col[ci][0]); sphereVertex.push_back(col[ci][1]); sphereVertex.push_back(col[ci][2]); sphereVertex.push_back(col[ci][3]);

			sphereVertex.push_back(x3); sphereVertex.push_back(y3); sphereVertex.push_back(z3);
			sphereVertex.push_back(col[ci][0]); sphereVertex.push_back(col[ci][1]); sphereVertex.push_back(col[ci][2]); sphereVertex.push_back(col[ci][3]);

			// 두 번째 삼각형
			sphereVertex.push_back(x2); sphereVertex.push_back(y2); sphereVertex.push_back(z2);
			sphereVertex.push_back(col[ci][0]); sphereVertex.push_back(col[ci][1]); sphereVertex.push_back(col[ci][2]); sphereVertex.push_back(col[ci][3]);

			sphereVertex.push_back(x4); sphereVertex.push_back(y4); sphereVertex.push_back(z4);
			sphereVertex.push_back(col[ci][0]); sphereVertex.push_back(col[ci][1]); sphereVertex.push_back(col[ci][2]); sphereVertex.push_back(col[ci][3]);

			sphereVertex.push_back(x3); sphereVertex.push_back(y3); sphereVertex.push_back(z3);
			sphereVertex.push_back(col[ci][0]); sphereVertex.push_back(col[ci][1]); sphereVertex.push_back(col[ci][2]); sphereVertex.push_back(col[ci][3]);
		}
	}

	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertex.size() * sizeof(float), sphereVertex.data(), GL_STATIC_DRAW);
}

void drawCylinder()
{
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	GLuint shader = bs;
	glUseProgram(shader);

	if (animating) {
		// 애니메이션 중일 때
		if (currentAnim == ANIM_V) {
			// V 애니메이션: 공전 + 자전 + 스케일
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.revolution), glm::vec3(0.0f, 1.0f, 0.0f));
			worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(cylinder.baseX, 0.0f, 0.0f));
			// 모든 축에 대해 자전
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
			worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(anim.scale, anim.scale, anim.scale));
		}
		else {
			// T, U 애니메이션: 위치 이동
			worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(anim.cylinderX, anim.cylinderY, anim.cylinderZ));
		}
	}
	else {
		// 일반 변환
		// 원점 기준 확대/축소 (b/B)
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(cylinder.globalScale, cylinder.globalScale, cylinder.globalScale));

		// 공전 변환 적용 (r/R)
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cylinder.revolutionAngle), glm::vec3(0.0f, 1.0f, 0.0f));

		// 원기둥을 baseX 위치에 배치
		worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(cylinder.baseX, 0.0f, 0.0f));

		// 이동 변환 적용 (d/D, e/E)
		worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(cylinder.positionX, cylinder.positionY, 0.0f));

		// 자전 변환 적용 (x/X, y/Y)
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cylinder.rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cylinder.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

		// 제자리 확대/축소 (a/A)
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(cylinder.localScale, cylinder.localScale, cylinder.localScale));
	}

	viewTransMatrix = glm::lookAt(CameraConfig::pos, CameraConfig::dir, CameraConfig::up);
	projectTransMatrix = glm::perspective(ViewfConfig::fovy, ViewfConfig::aspect, ViewfConfig::n, ViewfConfig::f);

	int worldTLoc = glGetUniformLocation(shader, "worldT");
	int viewTLoc = glGetUniformLocation(shader, "viewT");
	int projectTLoc = glGetUniformLocation(shader, "projectionT");
	glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(worldTransMatrix));
	glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(viewTransMatrix));
	glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(projectTransMatrix));

	int aPosition = glGetAttribLocation(shader, "a_Pos");
	int aColor = glGetAttribLocation(shader, "a_Color");

	glEnableVertexAttribArray(aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, cylinderVertex.size() / 7);

	glDisableVertexAttribArray(aPosition);
	glDisableVertexAttribArray(aColor);
}

void drawSphere()
{
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	GLuint shader = bs;
	glUseProgram(shader);

	if (animating) {
		// 애니메이션 중일 때
		if (currentAnim == ANIM_V) {
			// V 애니메이션: 공전 + 자전 + 스케일
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.revolution), glm::vec3(0.0f, 1.0f, 0.0f));
			worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(sphere.baseX, 0.0f, 0.0f));
			// 모든 축에 대해 자전
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
			worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
			worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(anim.scale, anim.scale, anim.scale));
		}
		else {
			// T, U 애니메이션: 위치 이동
			worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(anim.sphereX, anim.sphereY, anim.sphereZ));
		}
	}
	else {
		// 일반 변환
		// 원점 기준 확대/축소 (b/B)
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(sphere.globalScale, sphere.globalScale, sphere.globalScale));

		// 공전 변환 적용 (r/R)
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(sphere.revolutionAngle), glm::vec3(0.0f, 1.0f, 0.0f));

		// 구를 baseX 위치에 배치
		worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(sphere.baseX, 0.0f, 0.0f));

		// 이동 변환 적용 (d/D, e/E)
		worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(sphere.positionX, sphere.positionY, 0.0f));

		// 자전 변환 적용 (x/X, y/Y)
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(sphere.rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(sphere.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

		// 제자리 확대/축소 (a/A)
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(sphere.localScale, sphere.localScale, sphere.localScale));
	}

	viewTransMatrix = glm::lookAt(CameraConfig::pos, CameraConfig::dir, CameraConfig::up);
	projectTransMatrix = glm::perspective(ViewfConfig::fovy, ViewfConfig::aspect, ViewfConfig::n, ViewfConfig::f);

	int worldTLoc = glGetUniformLocation(shader, "worldT");
	int viewTLoc = glGetUniformLocation(shader, "viewT");
	int projectTLoc = glGetUniformLocation(shader, "projectionT");
	glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(worldTransMatrix));
	glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(viewTransMatrix));
	glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(projectTransMatrix));

	int aPosition = glGetAttribLocation(shader, "a_Pos");
	int aColor = glGetAttribLocation(shader, "a_Color");

	glEnableVertexAttribArray(aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, sphereVertex.size() / 7);

	glDisableVertexAttribArray(aPosition);
	glDisableVertexAttribArray(aColor);
}

void createCone()
{
	coneVertex.clear();

	const int segments = 50;
	const float radius = 0.5f;
	const float height = 2.0f;

	// 원뿔 밑면 중심점
	float tipX = 0.0f, tipY = height / 2.0f, tipZ = 0.0f;
	float baseY = -height / 2.0f;

	// 측면 삼각형들
	ci = colorUid(dre);
	for (int i = 0; i < segments; i++)
	{
		float theta1 = (float)i / segments * 2.0f * glm::pi<float>();
		float theta2 = (float)(i + 1) / segments * 2.0f * glm::pi<float>();

		float x1 = radius * cos(theta1);
		float z1 = radius * sin(theta1);
		float x2 = radius * cos(theta2);
		float z2 = radius * sin(theta2);

		// 측면 삼각형
		coneVertex.push_back(tipX); coneVertex.push_back(tipY); coneVertex.push_back(tipZ);
		coneVertex.push_back(col[ci][0]); coneVertex.push_back(col[ci][1]); coneVertex.push_back(col[ci][2]); coneVertex.push_back(col[ci][3]);

		coneVertex.push_back(x1); coneVertex.push_back(baseY); coneVertex.push_back(z1);
		coneVertex.push_back(col[ci][0]); coneVertex.push_back(col[ci][1]); coneVertex.push_back(col[ci][2]); coneVertex.push_back(col[ci][3]);

		coneVertex.push_back(x2); coneVertex.push_back(baseY); coneVertex.push_back(z2);
		coneVertex.push_back(col[ci][0]); coneVertex.push_back(col[ci][1]); coneVertex.push_back(col[ci][2]); coneVertex.push_back(col[ci][3]);
	}

	// 밑면 삼각형들
	ci = colorUid(dre);
	for (int i = 0; i < segments; i++)
	{
		float theta1 = (float)i / segments * 2.0f * glm::pi<float>();
		float theta2 = (float)(i + 1) / segments * 2.0f * glm::pi<float>();

		float x1 = radius * cos(theta1);
		float z1 = radius * sin(theta1);
		float x2 = radius * cos(theta2);
		float z2 = radius * sin(theta2);

		// 밑면 중심
		coneVertex.push_back(0.0f); coneVertex.push_back(baseY); coneVertex.push_back(0.0f);
		coneVertex.push_back(col[ci][0]); coneVertex.push_back(col[ci][1]); coneVertex.push_back(col[ci][2]); coneVertex.push_back(col[ci][3]);

		coneVertex.push_back(x2); coneVertex.push_back(baseY); coneVertex.push_back(z2);
		coneVertex.push_back(col[ci][0]); coneVertex.push_back(col[ci][1]); coneVertex.push_back(col[ci][2]); coneVertex.push_back(col[ci][3]);

		coneVertex.push_back(x1); coneVertex.push_back(baseY); coneVertex.push_back(z1);
		coneVertex.push_back(col[ci][0]); coneVertex.push_back(col[ci][1]); coneVertex.push_back(col[ci][2]); coneVertex.push_back(col[ci][3]);
	}

	glGenBuffers(1, &coneVBO);
	glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
	glBufferData(GL_ARRAY_BUFFER, coneVertex.size() * sizeof(float), coneVertex.data(), GL_STATIC_DRAW);
}

void createCube()
{
	cubeVertex.clear();

	const float size = 1.0f;
	const float half = size / 2.0f;

	// 정육면체 8개 꼭짓점
	glm::vec3 vertices[8] = {
		{-half, -half, -half}, // 0
		{ half, -half, -half}, // 1
		{ half,  half, -half}, // 2
		{-half,  half, -half}, // 3
		{-half, -half,  half}, // 4
		{ half, -half,  half}, // 5
		{ half,  half,  half}, // 6
		{-half,  half,  half}  // 7
	};

	// 6개 면, 각 면당 2개 삼각형
	int indices[36] = {
		// 앞면
		0, 1, 2,  0, 2, 3,
		// 뒷면
		4, 6, 5,  4, 7, 6,
		// 왼쪽면
		0, 3, 7,  0, 7, 4,
		// 오른쪽면
		1, 5, 6,  1, 6, 2,
		// 윗면
		3, 2, 6,  3, 6, 7,
		// 아랫면
		0, 4, 5,  0, 5, 1
	};

	for (int i = 0; i < 36; i++)
	{
		if (i % 6 == 0) ci = colorUid(dre); // 각 면마다 색상 변경

		glm::vec3 v = vertices[indices[i]];
		cubeVertex.push_back(v.x);
		cubeVertex.push_back(v.y);
		cubeVertex.push_back(v.z);
		cubeVertex.push_back(col[ci][0]);
		cubeVertex.push_back(col[ci][1]);
		cubeVertex.push_back(col[ci][2]);
		cubeVertex.push_back(col[ci][3]);
	}

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeVertex.size() * sizeof(float), cubeVertex.data(), GL_STATIC_DRAW);
}

void drawCone()
{
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	GLuint shader = bs;
	glUseProgram(shader);

	int worldTLoc = glGetUniformLocation(shader, "worldT");
	int viewTLoc = glGetUniformLocation(shader, "viewT");
	int projectTLoc = glGetUniformLocation(shader, "projectionT");

	// 전역 스케일
	worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(cylinder.globalScale));

	// 공전 (원점 기준)
	if (animating && currentAnim == ANIM_V) {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.revolution), glm::vec3(0., 1., 0.));
	}
	else {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cylinder.revolutionAngle), glm::vec3(0., 1., 0.));
	}

	// 기본 위치 + 이동 변환
	float finalX = cylinder.baseX + cylinder.positionX;
	float finalY = cylinder.positionY;
	float finalZ = 0.0f;

	// 애니메이션 위치 적용
	if (animating) {
		if (currentAnim == ANIM_T || currentAnim == ANIM_U) {
			finalX = anim.cylinderX;
			finalY = anim.cylinderY;
		}
		else if (currentAnim == ANIM_V) {
			finalX = cylinder.baseX + anim.cylinderX;
			finalY = anim.cylinderY;
		}
	}

	worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(finalX, finalY, finalZ));

	// 자전
	if (animating && currentAnim == ANIM_V) {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationX), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationY), glm::vec3(0., 1., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationZ), glm::vec3(0., 0., 1.));
	}
	else {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cylinder.rotationX), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cylinder.rotationY), glm::vec3(0., 1., 0.));
	}

	// 지역 스케일
	if (animating && currentAnim == ANIM_V) {
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(anim.scale));
	}
	else {
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(cylinder.localScale));
	}

	viewTransMatrix = glm::lookAt(CameraConfig::pos, CameraConfig::dir, CameraConfig::up);
	projectTransMatrix = glm::perspective(ViewfConfig::fovy, ViewfConfig::aspect, ViewfConfig::n, ViewfConfig::f);

	glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(worldTransMatrix));
	glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(viewTransMatrix));
	glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(projectTransMatrix));

	int aPosition = glGetAttribLocation(shader, "a_Pos");
	int aColor = glGetAttribLocation(shader, "a_Color");

	glEnableVertexAttribArray(aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, coneVertex.size() / 7);

	glDisableVertexAttribArray(aPosition);
	glDisableVertexAttribArray(aColor);
}

void drawCube()
{
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	GLuint shader = bs;
	glUseProgram(shader);

	int worldTLoc = glGetUniformLocation(shader, "worldT");
	int viewTLoc = glGetUniformLocation(shader, "viewT");
	int projectTLoc = glGetUniformLocation(shader, "projectionT");

	// 전역 스케일
	worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(sphere.globalScale));

	// 공전 (원점 기준)
	if (animating && currentAnim == ANIM_V) {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.revolution), glm::vec3(0., 1., 0.));
	}
	else {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(sphere.revolutionAngle), glm::vec3(0., 1., 0.));
	}

	// 기본 위치 + 이동 변환
	float finalX = sphere.baseX + sphere.positionX;
	float finalY = sphere.positionY;
	float finalZ = 0.0f;

	// 애니메이션 위치 적용
	if (animating) {
		if (currentAnim == ANIM_T || currentAnim == ANIM_U) {
			finalX = anim.sphereX;
			finalY = anim.sphereY;
		}
		else if (currentAnim == ANIM_V) {
			finalX = sphere.baseX + anim.sphereX;
			finalY = anim.sphereY;
		}
	}

	worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(finalX, finalY, finalZ));

	// 자전
	if (animating && currentAnim == ANIM_V) {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationX), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationY), glm::vec3(0., 1., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(anim.rotationZ), glm::vec3(0., 0., 1.));
	}
	else {
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(sphere.rotationX), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(sphere.rotationY), glm::vec3(0., 1., 0.));
	}

	// 지역 스케일
	if (animating && currentAnim == ANIM_V) {
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(anim.scale));
	}
	else {
		worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(sphere.localScale));
	}

	viewTransMatrix = glm::lookAt(CameraConfig::pos, CameraConfig::dir, CameraConfig::up);
	projectTransMatrix = glm::perspective(ViewfConfig::fovy, ViewfConfig::aspect, ViewfConfig::n, ViewfConfig::f);

	glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(worldTransMatrix));
	glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(viewTransMatrix));
	glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(projectTransMatrix));

	int aPosition = glGetAttribLocation(shader, "a_Pos");
	int aColor = glGetAttribLocation(shader, "a_Color");

	glEnableVertexAttribArray(aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, cubeVertex.size() / 7);

	glDisableVertexAttribArray(aPosition);
	glDisableVertexAttribArray(aColor);
}

void drawObject1()
{
	if (shapeType == 0) {
		drawCylinder();
	}
	else {
		drawCone();
	}
}

void drawObject2()
{
	if (shapeType == 0) {
		drawSphere();
	}
	else {
		drawCube();
	}
}

void resetToInitialState()
{
	// 도형 타입 초기화
	shapeType = 0;

	// 원기둥 상태 초기화
	cylinder.rotationX = 0.0f;
	cylinder.rotationY = 0.0f;
	cylinder.revolutionAngle = 0.0f;
	cylinder.localScale = 1.0f;
	cylinder.globalScale = 1.0f;
	cylinder.positionX = 0.0f;
	cylinder.positionY = 0.0f;
	cylinder.baseX = 2.0f;

	// 구 상태 초기화
	sphere.rotationX = 0.0f;
	sphere.rotationY = 0.0f;
	sphere.revolutionAngle = 0.0f;
	sphere.localScale = 1.0f;
	sphere.globalScale = 1.0f;
	sphere.positionX = 0.0f;
	sphere.positionY = 0.0f;
	sphere.baseX = -2.0f;

	// 애니메이션 상태 초기화
	animating = false;
	vAnimToggle = false;
	currentAnim = ANIM_NONE;
	animTime = 0.0f;

	// 애니메이션 변수 초기화
	anim.cylinderX = 0.0f;
	anim.cylinderY = 0.0f;
	anim.cylinderZ = 0.0f;
	anim.sphereX = 0.0f;
	anim.sphereY = 0.0f;
	anim.sphereZ = 0.0f;
	anim.scale = 1.0f;
	anim.rotationX = 0.0f;
	anim.rotationY = 0.0f;
	anim.rotationZ = 0.0f;
	anim.revolution = 0.0f;

	// 모드 초기화
	mode = 3;

	// 도형 재생성
	createCylinder();
	createSphere();
	createCone();
	createCube();
}
