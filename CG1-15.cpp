#include "loadShader.hpp"
#include "cheat.h"

namespace beginConfig {
	int width{ 800 };
	int height{ 800 };
	Color bg{ 1,1,1,1 };
}

namespace matConfig {
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);
}

std::array<std::array<float, 18>, 6> cube = { {
		// 면 1: 앞면 (z = 0.5)
	   {-0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f},

		// 면 2: 뒷면 (z = -0.5)
		{-0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,
		 -0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f},

		 // 면 3: 왼쪽면 (x = -0.5)
		 {-0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
		  -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f},

		  // 면 4: 오른쪽면 (x = 0.5)
		 { 0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f},

		 // 면 5: 윗면 (y = 0.5)
	 {-0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
	  -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f},

	  // 면 6: 아랫면 (y = -0.5)
 {-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
  -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f}
} };

std::array<std::array<float, 18>, 5> pyramid = { {
		// 면 1: 바닥면 (y = -0.5, 사각형 = 삼각형 2개)
		{-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
		 -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f},

		 // 면 2: 앞면 삼각형 (z = 0.5 쪽)
		 {-0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f,
		  -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f},

		  // 면 3: 오른쪽면 삼각형 (x = 0.5 쪽)
		  { 0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f,
			0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f},

			// 면 4: 뒷면 삼각형 (z = -0.5 쪽)
			{ 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f,
			  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.0f,  0.5f,  0.0f},

			  // 면 5: 왼쪽면 삼각형 (x = -0.5 쪽)
			  {-0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f,
			   -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,  0.0f,  0.5f,  0.0f}
} };

std::array<std::array<float, 4>, 6> col = { {
	{1,0,0,1},	// R
	{0,1,0,1},	// G
	{0,0,1,1},	// B
	{1,1,0,1},	// Y
	{1,0,1,1},	// M
	{0,1,1,1}	// c
} };

GameTimer* GameTimer::Instance;

// basic callback
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid RanColor(Color&);
GLvoid Mouse(int, int, int, int);
GLvoid loop(int);

// Shader 
GLuint bs{};
GLuint VBO{};
std::vector<float> Vertex{};	// 점 float 3, 색 float 4

// custom function
// - Axis
GLuint axisVBO{};
void createAxis();
void drawAxis();

GameTimer gt;

int num{};
int method{};
Color bg = beginConfig::bg;


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


	matConfig::worldTransMatrix = glm::rotate(matConfig::worldTransMatrix, glm::radians(30.f), glm::vec3(1., 0., 0.));
	matConfig::worldTransMatrix = glm::rotate(matConfig::worldTransMatrix, glm::radians(-30.f), glm::vec3(0., 1., 0.));
	bs = CompileShaders("CG1-15.vs", "CG1-15.fs");





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
	drawAxis();

	GLuint shader = bs;
	glUseProgram(shader);


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

		break;
	case '2':

		break;
	case '3':

		break;
	case '4':

		break;
	case '5':

		break;
	case '6':

		break;
	case '7':

		break;
	case '8':

		break;
	case '9':

		break;
	case '0':

		break;
	case 'c':

		break;
	case 't':

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
	glBufferData(GL_ARRAY_BUFFER, axisVertexs.size(), axisVertexs.data(), GL_STATIC_DRAW);


}

void drawAxis()
{
	GLuint shader = bs;
	glUseProgram(shader);

	int worldTLoc = glGetUniformLocation(shader, "worldT");
	int viewTLoc = glGetUniformLocation(shader, "viewT");
	int projectTLoc = glGetUniformLocation(shader, "projectionT");
	glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(matConfig::worldTransMatrix));
	glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(matConfig::viewTransMatrix));
	glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(matConfig::projectTransMatrix));
	
	int aPosition = glGetAttribLocation(shader, "a_Position");
	int aColor = glGetAttribLocation(shader, "a_Color");

	glEnableVertexAttribArray(aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glVertexAttribPointer(aColor, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_LINE, 0, 6);

	glDisableVertexAttribArray(aPosition);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
