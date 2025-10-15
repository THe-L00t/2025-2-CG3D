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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); 
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

	createAxis();

	matConfig::worldTransMatrix = glm::rotate(matConfig::worldTransMatrix, glm::radians(30.f), glm::vec3(1., 0., 0.));
	matConfig::worldTransMatrix = glm::rotate(matConfig::worldTransMatrix, glm::radians(-30.f), glm::vec3(0., 1., 0.));
	//matConfig::viewTransMatrix = glm::translate(matConfig::viewTransMatrix, glm::vec3(0.0f, 0.0f, -2.0f));
	//matConfig::projectTransMatrix = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	matConfig::projectTransMatrix = glm::ortho(-1.f,1.f,-1.f,1.f,-1.f,1.f);
	bs = CompileShaders("CG1-15.vs", "CG1-15.fs");



	glEnable(GL_DEPTH_TEST);

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

	int worldTLoc = glGetUniformLocation(shader, "worldT");
	int viewTLoc = glGetUniformLocation(shader, "viewT");
	int projectTLoc = glGetUniformLocation(shader, "projectionT");
	glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(matConfig::worldTransMatrix));
	glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(matConfig::viewTransMatrix));
	glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(matConfig::projectTransMatrix));

	int aPosition = glGetAttribLocation(shader, "a_Pos");
	int aColor = glGetAttribLocation(shader, "a_Color");

	glEnableVertexAttribArray(aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, Vertex.size()/7);

	glDisableVertexAttribArray(aPosition);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}


GLvoid Keyboard(unsigned char key, int x, int y)
{
	int ci = colorUid(dre);
	switch (key) {
	case '1':	// 앞
		Vertex.clear();
		for (size_t i = 0; i < 6; i++)
		{
			Vertex.push_back(cube[0][3 * i]);
			Vertex.push_back(cube[0][3 * i+1]);
			Vertex.push_back(cube[0][3 * i+2]);

			for (size_t j = 0; j < 4; j++)
			{
				Vertex.push_back(col[ci][j]);
			}
		}
		break;
	case '2':	// 뒤 
		Vertex.clear();
		for (size_t i = 0; i < 6; i++)
		{
			Vertex.push_back(cube[1][3 * i]);
			Vertex.push_back(cube[1][3 * i + 1]);
			Vertex.push_back(cube[1][3 * i + 2]);

			for (size_t j = 0; j < 4; j++)
			{
				Vertex.push_back(col[ci][j]);
			}
		}

		break;
	case '3':	// 왼
		Vertex.clear();
		for (size_t i = 0; i < 6; i++)
		{
			Vertex.push_back(cube[2][3 * i]);
			Vertex.push_back(cube[2][3 * i + 1]);
			Vertex.push_back(cube[2][3 * i + 2]);

			for (size_t j = 0; j < 4; j++)
			{
				Vertex.push_back(col[ci][j]);
			}
		}
		break;
	case '4':	// 오
		Vertex.clear();
		for (size_t i = 0; i < 6; i++)
		{
			Vertex.push_back(cube[3][3 * i]);
			Vertex.push_back(cube[3][3 * i + 1]);
			Vertex.push_back(cube[3][3 * i + 2]);

			for (size_t j = 0; j < 4; j++)
			{
				Vertex.push_back(col[ci][j]);
			}
		}
		break;	
	case '5':	// 위
		Vertex.clear();
		for (size_t i = 0; i < 6; i++)
		{
			Vertex.push_back(cube[4][3 * i]);
			Vertex.push_back(cube[4][3 * i + 1]);
			Vertex.push_back(cube[4][3 * i + 2]);

			for (size_t j = 0; j < 4; j++)
			{
				Vertex.push_back(col[ci][j]);
			}
		}
		break;
	case '6':	//아래
		Vertex.clear();
		for (size_t i = 0; i < 6; i++)
		{
			Vertex.push_back(cube[5][3 * i]);
			Vertex.push_back(cube[5][3 * i + 1]);
			Vertex.push_back(cube[5][3 * i + 2]);

			for (size_t j = 0; j < 4; j++)
			{
				Vertex.push_back(col[ci][j]);
			}
		}
		break;
	case '7':	// 앞

		break;
	case '8':	// 뒤

		break;
	case '9':	// 왼

		break;
	case '0':	// 오

		break;
	case 'c':

		break;
	case 't':

		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	if (Vertex.size() not_eq 0) {
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Vertex.size(), Vertex.data(), GL_STATIC_DRAW);
	}
	glutPostRedisplay();
}



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
	glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertexs), axisVertexs.data(), GL_STATIC_DRAW);


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
