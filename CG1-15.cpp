#include "loadShader.hpp"

namespace beginConfig {
	int width{ 800 };
	int height{ 800 };
	Color bg{ 1,1,1,1 };
}

GameTimer* GameTimer::Instance;

std::uniform_int_distribution urgb(0, 255);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid RanColor(Color&);
GLvoid Mouse(int, int, int, int);
GLvoid loop(int);
void makeRect(float, float);
void initShapes();
void randomizeShapes();
Point getQuadrantCenter(int);
Color getShapeColor(int);
std::vector<float> createShape(int, int);

GLuint CompileShaders(const std::string_view&, const std::string_view&);
bool ReadFile(const std::string_view&, std::string*);
void AddShader(GLuint, const std::string_view&, GLenum);

// Shader 
GLuint bs{};
GLuint pVBO{};
GLuint lVBO{};
std::vector<float> polyVertex{};
std::vector<float> lineVertex{};

GameTimer gt;

int num{};
int method{};
Color bg = beginConfig::bg;

// 각 도형의 정보를 저장
struct Shape {
	int vertexCount{};
	int type{}; // 0: line, 1: triangle, 2: rect, 3: pentagon
	int quadrant{}; // 0: 좌상, 1: 우상, 2: 좌하, 3: 우하
	std::vector<float> vertices{}; // 도형의 정점 데이터
};

std::vector<Shape> shapes{};

// 애니메이션 상태 관리
struct AnimState {
	bool isAnimating{};
	float animTime{};
	int currentType{};   // 현재 도형 타입
	int targetType{};    // 변환될 도형 타입
};

AnimState animStates[4]{}; // 각 사분면별 애니메이션 상태
float animSpeed = 2.0f;     // 애니메이션 속도 (0.5초 = 1 / 0.5 = 2.0)

// 화면 중앙 단일 도형 모드
bool centerMode = false;
int centerShapeType = 0;
AnimState centerAnimState{};

std::random_device rd;
std::mt19937 dre(rd());


// 사분면 중앙 좌표 계산
Point getQuadrantCenter(int quadrant) {
	switch (quadrant) {
	case 0: return { -0.5f, 0.5f };  // 좌상
	case 1: return { 0.5f, 0.5f };   // 우상
	case 2: return { -0.5f, -0.5f }; // 좌하
	case 3: return { 0.5f, -0.5f };  // 우하
	}
	return { 0, 0 };
}

// 색상 가져오기
Color getShapeColor(int quadrant) {
	switch (quadrant) {
	case 0: return { 1.0f, 0.0f, 0.0f, 1.0f };  // 빨강
	case 1: return { 0.0f, 1.0f, 0.0f, 1.0f };  // 초록
	case 2: return { 0.0f, 0.0f, 1.0f, 1.0f };  // 파랑
	case 3: return { 1.0f, 1.0f, 0.0f, 1.0f };  // 노랑
	}
	return { 1.0f, 1.0f, 1.0f, 1.0f };
}

// 도형 생성 (모든 도형을 5개 점으로 통일)
std::vector<float> createShape(int type, int quadrant) {
	Point center;
	Color color;
	float size = 0.2f;

	if (centerMode) {
		center = { 0.0f, 0.0f };
		color = { 1.0f, 0.5f, 0.0f, 1.0f }; // 주황색
		size = 0.3f; // 중앙 도형은 더 크게
	}
	else {
		center = getQuadrantCenter(quadrant);
		color = getShapeColor(quadrant);
	}
	std::vector<float> vertices;

	if (type == 0) {
		// 선: 5개 점으로 대각선 표현 (시작점, 1/4지점, 중간점, 3/4지점, 끝점)
		vertices = {
			center.x - size, center.y - size, 0.0f, color.r, color.g, color.b, color.al,        // 시작점
			center.x - size * 0.5f, center.y - size * 0.5f, 0.0f, color.r, color.g, color.b, color.al,  // 1/4지점
			center.x, center.y, 0.0f, color.r, color.g, color.b, color.al,                    // 중간점
			center.x + size * 0.5f, center.y + size * 0.5f, 0.0f, color.r, color.g, color.b, color.al,  // 3/4지점
			center.x + size, center.y + size, 0.0f, color.r, color.g, color.b, color.al         // 끝점
		};
	}
	else if (type == 1) {
		// 삼각형: 3개 꼭짓점 + 2개 중간점
		vertices = {
			center.x, center.y + size, 0.0f, color.r, color.g, color.b, color.al,              // 위 꼭짓점
			center.x - size * 0.5f, center.y, 0.0f, color.r, color.g, color.b, color.al,        // 왼쪽 중간점
			center.x - size, center.y - size, 0.0f, color.r, color.g, color.b, color.al,      // 좌하 꼭짓점
			center.x, center.y - size, 0.0f, color.r, color.g, color.b, color.al,             // 아래 중간점
			center.x + size, center.y - size, 0.0f, color.r, color.g, color.b, color.al       // 우하 꼭짓점
		};
	}
	else if (type == 2) {
		// 사각형: 4개 꼭짓점 + 1개 중앙점
		vertices = {
			center.x - size, center.y + size, 0.0f, color.r, color.g, color.b, color.al,      // 좌상
			center.x + size, center.y + size, 0.0f, color.r, color.g, color.b, color.al,      // 우상
			center.x + size, center.y - size, 0.0f, color.r, color.g, color.b, color.al,      // 우하
			center.x - size, center.y - size, 0.0f, color.r, color.g, color.b, color.al,      // 좌하
			center.x, center.y, 0.0f, color.r, color.g, color.b, color.al                     // 중앙점
		};
	}
	else if (type == 3) {
		// 오각형: 5개 꼭짓점
		float angle = 3.14159f / 2.0f; // 90도부터 시작
		for (int i = 0; i < 5; ++i) {
			float x = center.x + size * cos(angle);
			float y = center.y + size * sin(angle);
			vertices.insert(vertices.end(), { x, y, 0.0f, color.r, color.g, color.b, color.al });
			angle += 2.0f * 3.14159f / 5.0f;
		}
	}

	return vertices;
}

void initShapes() {
	shapes.clear();
	lineVertex.clear();
	polyVertex.clear();

	// 초기 도형 설정: 좌상(선), 우상(삼각형), 좌하(사각형), 우하(오각형)
	int initialTypes[4] = { 0, 1, 2, 3 };

	for (int i = 0; i < 4; ++i) {
		auto verts = createShape(initialTypes[i], i);
		Shape shape;
		shape.type = initialTypes[i];
		shape.quadrant = i;
		shape.vertices = verts;
		shape.vertexCount = verts.size() / 7;

		if (initialTypes[i] == 0) {
			lineVertex.insert(lineVertex.end(), verts.begin(), verts.end());
		}
		else {
			polyVertex.insert(polyVertex.end(), verts.begin(), verts.end());
		}

		shapes.push_back(shape);
		animStates[i].currentType = initialTypes[i];

		// 무한 애니메이션 시작
		animStates[i].isAnimating = true;
		animStates[i].animTime = 0.0f;
		animStates[i].targetType = (initialTypes[i] + 1) % 4; // 다음 도형으로
	}
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv); //--- glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); //--- ���÷��� ��� ����
	glutInitWindowPosition(0, 0); //--- �������� ��ġ ����
	glutInitWindowSize(beginConfig::width, beginConfig::height); //--- �������� ũ�� ����
	glutCreateWindow("Example1"); //--- ������ ����(������ �̸�	)
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { //--- glew �ʱ�ȭ
		std::cerr << "Unable to initialize GLEW" << std::endl
			;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}
	bs = CompileShaders("shader1-12.vs", "shader1-8.fs");

	initShapes();

	glutDisplayFunc(drawScene); //--- ��� �ݹ��Լ��� ����
	glutReshapeFunc(Reshape); //--- �ٽ� �׸��� �ݹ��Լ� ����
	glutKeyboardFunc(Keyboard); //--- Ű���� �Է� �ݹ��Լ� ����
	glutMouseFunc(Mouse);
	glutTimerFunc(1, loop, 1);
	glutMainLoop(); //--- �̺�Ʈ ó�� ����
}

GLvoid drawScene()
{

	glClearColor(bg.r, bg.g, bg.b, bg.al);
	glClear(GL_COLOR_BUFFER_BIT);

	// 셰이더 사용하여 그리기
	GLuint shader = bs;
	glUseProgram(shader);

	int aPosition = glGetAttribLocation(shader, "a_Position");
	int aColor = glGetAttribLocation(shader, "a_Color");
	int uShapeType = glGetUniformLocation(shader, "u_ShapeType");
	int uTargetType = glGetUniformLocation(shader, "u_TargetType");
	int uAnimTime = glGetUniformLocation(shader, "u_AnimTime");
	int uCenter = glGetUniformLocation(shader, "u_Center");

	if (centerMode) {
		// 중앙 단일 도형 모드 - 중앙에 하나의 도형만 그리기
		auto verts = createShape(centerShapeType, 0); // quadrant는 무시됨
		Point center = { 0.0f, 0.0f };

		// 모든 도형에 5개 정점 사용
		int drawVertexCount = 5;

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(aPosition);
		glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 0));

		glEnableVertexAttribArray(aColor);
		glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

		glUniform1i(uShapeType, centerAnimState.currentType);
		glUniform1i(uTargetType, centerAnimState.targetType);
		glUniform1f(uAnimTime, centerAnimState.isAnimating ? centerAnimState.animTime : 0.0f);
		glUniform3f(uCenter, center.x, center.y, 0.0f);

		int currentType = centerAnimState.isAnimating ? centerAnimState.targetType : centerShapeType;

		if (currentType == 0) {
			// 선: 연결된 선분으로 그리기
			glDrawArrays(GL_LINE_STRIP, 0, drawVertexCount);
		}
		else {
			// 나머지 도형: 삼각형 팬으로 그리기
			glDrawArrays(GL_TRIANGLE_FAN, 0, drawVertexCount);
		}

		glDisableVertexAttribArray(aPosition);
		glDisableVertexAttribArray(aColor);
		glDeleteBuffers(1, &vbo);

		// 중앙 모드에서는 4분면 도형들을 그리지 않고 여기서 끝
	}
	else {
		// 4분면 모드
		for (const auto& shape : shapes) {
			if (shape.vertices.empty()) continue;

			int quadrant = shape.quadrant;
			Point center = getQuadrantCenter(quadrant);

			// 모든 도형에 5개 정점 사용
			int drawVertexCount = 5;

			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, shape.vertices.size() * sizeof(float), shape.vertices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(aPosition);
			glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 0));

			glEnableVertexAttribArray(aColor);
			glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

			glUniform1i(uShapeType, animStates[quadrant].currentType);
			glUniform1i(uTargetType, animStates[quadrant].targetType);
			glUniform1f(uAnimTime, animStates[quadrant].isAnimating ? animStates[quadrant].animTime : 0.0f);
			glUniform3f(uCenter, center.x, center.y, 0.0f);

			int currentType = animStates[quadrant].isAnimating ? animStates[quadrant].targetType : shape.type;

			if (currentType == 0) {
				// 선: 연결된 선분으로 그리기
				glDrawArrays(GL_LINE_STRIP, 0, drawVertexCount);
			}
			else {
				// 나머지 도형: 삼각형 팬으로 그리기
				glDrawArrays(GL_TRIANGLE_FAN, 0, drawVertexCount);
			}

			glDisableVertexAttribArray(aPosition);
			glDisableVertexAttribArray(aColor);
			glDeleteBuffers(1, &vbo);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void randomizeShapes() {
	std::uniform_int_distribution<int> shapeTypeDist(0, 3);

	shapes.clear();
	lineVertex.clear();
	polyVertex.clear();

	for (int i = 0; i < 4; ++i) {
		int randomType = shapeTypeDist(dre);
		auto verts = createShape(randomType, i);

		Shape shape;
		shape.type = randomType;
		shape.quadrant = i;
		shape.vertices = verts;
		shape.vertexCount = verts.size() / 7;

		if (randomType == 0) {
			lineVertex.insert(lineVertex.end(), verts.begin(), verts.end());
		}
		else {
			polyVertex.insert(polyVertex.end(), verts.begin(), verts.end());
		}

		shapes.push_back(shape);
		animStates[i].currentType = randomType;
		animStates[i].isAnimating = true;  // 무한 애니메이션 시작
		animStates[i].animTime = 0.0f;
		animStates[i].targetType = (randomType + 1) % 4; // 다음 도형으로 변환
	}
}

GLvoid Keyboard(unsigned char key, int x, int y)
{

	switch (key) {
	case '1':
		// 중앙에 선 그리기
		centerMode = true;
		centerShapeType = 0;
		centerAnimState.currentType = 0;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 1; // 선 -> 삼각형으로 변환
		break;
	case '2':
		// 중앙에 삼각형 그리기
		centerMode = true;
		centerShapeType = 1;
		centerAnimState.currentType = 1;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 2; // 삼각형 -> 사각형으로 변환
		break;
	case '3':
		// 중앙에 사각형 그리기
		centerMode = true;
		centerShapeType = 2;
		centerAnimState.currentType = 2;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 3; // 사각형 -> 오각형으로 변환
		break;
	case '4':
		// 중앙에 오각형 그리기
		centerMode = true;
		centerShapeType = 3;
		centerAnimState.currentType = 3;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 0; // 오각형 -> 선으로 변환
		break;
	case 'l':
		// 중앙에 선 -> 삼각형 애니메이션
		centerMode = true;
		centerShapeType = 0;
		centerAnimState.currentType = 0;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 1;
		break;
	case 't':
		// 중앙에 삼각형 -> 사각형 애니메이션
		centerMode = true;
		centerShapeType = 1;
		centerAnimState.currentType = 1;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 2;
		break;
	case 'r':
		// 중앙에 사각형 -> 오각형 애니메이션
		centerMode = true;
		centerShapeType = 2;
		centerAnimState.currentType = 2;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 3;
		break;
	case 'p':
		// 중앙에 오각형 -> 선 애니메이션
		centerMode = true;
		centerShapeType = 3;
		centerAnimState.currentType = 3;
		centerAnimState.isAnimating = true;
		centerAnimState.animTime = 0.0f;
		centerAnimState.targetType = 0;
		break;
	case 'a':
		// 4분면 모드로 돌아가서 랜덤 도형들이 무한 변환
		centerMode = false;
		randomizeShapes();
		break;
	case 's':
		// 4분면 모드로 복귀
		centerMode = false;
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
		std::cout << "upȣ��" << std::endl;
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

	if (centerMode) {
		// 중앙 도형 애니메이션 업데이트
		if (centerAnimState.isAnimating) {
			centerAnimState.animTime += gt.deltaTime * animSpeed;
			if (centerAnimState.animTime >= 1.0f) {
				centerAnimState.animTime = 1.0f;
				centerAnimState.isAnimating = false; // 애니메이션 중지

				// 최종 도형으로 업데이트
				centerAnimState.currentType = centerAnimState.targetType;
				centerShapeType = centerAnimState.targetType;
			}
		}
	}
	else {
		// 각 사분면의 애니메이션 업데이트
		for (int i = 0; i < 4; ++i) {
			if (animStates[i].isAnimating) {
				animStates[i].animTime += gt.deltaTime * animSpeed;
				if (animStates[i].animTime >= 1.0f) {
					animStates[i].animTime = 0.0f; // 무한 반복을 위해 0으로 리셋

					// 애니메이션 완료 시 도형 업데이트
					int targetType = animStates[i].targetType;
					shapes[i].vertices = createShape(targetType, i);
					shapes[i].type = targetType;
					shapes[i].vertexCount = shapes[i].vertices.size() / 7;
					animStates[i].currentType = targetType;

					// 다음 도형으로 계속 변환
					animStates[i].targetType = (targetType + 1) % 4;
				}
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(1, loop, 1);
}

void makeRect(float tx, float ty)
{

}