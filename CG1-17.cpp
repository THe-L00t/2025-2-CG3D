#include <memory>
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
	glm::vec3 pos{ 5.f,5.f,-2.f };
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
GLvoid SKeyboard(int key, int x, int y);
GLvoid RanColor(Color&);
GLvoid Mouse(int, int, int, int);
GLvoid loop(int);


GLuint CompileShaders(const std::string_view&, const std::string_view&);
bool ReadFile(const std::string_view&, std::string*);
void AddShader(GLuint, const std::string_view&, GLenum);

// Shader 
GLuint bs{};
GLuint VBO{};
std::vector<float> Vertex{};


GLuint axisVBO{};
void createAxis();
void drawAxis();

GameTimer gt;
int ci = colorUid(dre);

bool w{ true };

class Object {
public:
	virtual void draw() = 0;
	virtual void init()
	{
		pos = { 0.f,0.f,0.f };
		ang = { 0.f,0.f,0.f };
	}
	void move(const glm::vec3& dir) {
		pos += dir;
	}
	void rotate(const glm::vec3& tor) {
		ang += tor;
	}
protected:
	glm::vec3 pos{};
	glm::vec3 ang{};
};

class CubeO : public Object {
public:
	CubeO()
	{
		init();
	}
	virtual void init() override
	{
		Vertex.clear();
		for (size_t p = 0; p < 6; p++)
		{
			ci = colorUid(dre);
			for (size_t i = 0; i < 6; i++)
			{
				Vertex.push_back(cube[p][3 * i]);
				Vertex.push_back(cube[p][3 * i + 1]);
				Vertex.push_back(cube[p][3 * i + 2]);

				for (size_t j = 0; j < 4; j++)
				{
					Vertex.push_back(col[ci][j]);
				}
			}
		}
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Vertex.size() * sizeof(float), Vertex.data(), GL_STATIC_DRAW);
	}
	virtual void draw() final
	{
		glm::mat4 worldTransMatrix(1.0f);
		glm::mat4 viewTransMatrix(1.0f);
		glm::mat4 projectTransMatrix(1.0f);

		GLuint shader = bs;
		glUseProgram(shader);

		int worldTLoc = glGetUniformLocation(shader, "worldT");
		int viewTLoc = glGetUniformLocation(shader, "viewT");
		int projectTLoc = glGetUniformLocation(shader, "projectionT");

		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(ang.x), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(ang.y), glm::vec3(0., 1., 0.));
		worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(pos.x, pos.y, pos.z));

		viewTransMatrix = glm::lookAt(CameraConfig::pos, CameraConfig::dir, CameraConfig::up);
		projectTransMatrix = glm::perspective(ViewfConfig::fovy, ViewfConfig::aspect, ViewfConfig::n, ViewfConfig::f);



		glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(worldTransMatrix));
		glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(viewTransMatrix));
		glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(projectTransMatrix));

		int aPosition = glGetAttribLocation(shader, "a_Pos");
		int aColor = glGetAttribLocation(shader, "a_Color");

		glEnableVertexAttribArray(aPosition);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

		glEnableVertexAttribArray(aColor);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

		if (w) glDrawArrays(GL_TRIANGLES, 0, Vertex.size() / 7);
		else glDrawArrays(GL_LINE_LOOP, 0, Vertex.size() / 7);

		glDisableVertexAttribArray(aPosition);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
};

class PyramidO : public Object {
public:
	PyramidO()
	{
		init();
	}
	virtual void init() override
	{
		Vertex.clear();
		for (size_t p = 0; p < 5; p++)
		{
			ci = colorUid(dre);
			for (size_t i = 0; i < 6; i++)
			{
				Vertex.push_back(pyramid[p][3 * i]);
				Vertex.push_back(pyramid[p][3 * i + 1]);
				Vertex.push_back(pyramid[p][3 * i + 2]);

				for (size_t j = 0; j < 4; j++)
				{
					Vertex.push_back(col[ci][j]);
				}
			}
		}
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, Vertex.size() * sizeof(float), Vertex.data(), GL_STATIC_DRAW);
	}
	virtual void draw() final
	{
		glm::mat4 worldTransMatrix(1.0f);
		glm::mat4 viewTransMatrix(1.0f);
		glm::mat4 projectTransMatrix(1.0f);

		GLuint shader = bs;
		glUseProgram(shader);

		int worldTLoc = glGetUniformLocation(shader, "worldT");
		int viewTLoc = glGetUniformLocation(shader, "viewT");
		int projectTLoc = glGetUniformLocation(shader, "projectionT");

		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(ang.x), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(ang.y), glm::vec3(0., 1., 0.));
		worldTransMatrix = glm::translate(worldTransMatrix, glm::vec3(pos.x, pos.y, pos.z));


		viewTransMatrix = glm::lookAt(CameraConfig::pos, CameraConfig::dir, CameraConfig::up);
		projectTransMatrix = glm::perspective(ViewfConfig::fovy, ViewfConfig::aspect, ViewfConfig::n, ViewfConfig::f);


		glUniformMatrix4fv(worldTLoc, 1, GL_FALSE, glm::value_ptr(worldTransMatrix));
		glUniformMatrix4fv(viewTLoc, 1, GL_FALSE, glm::value_ptr(viewTransMatrix));
		glUniformMatrix4fv(projectTLoc, 1, GL_FALSE, glm::value_ptr(projectTransMatrix));

		int aPosition = glGetAttribLocation(shader, "a_Pos");
		int aColor = glGetAttribLocation(shader, "a_Color");

		glEnableVertexAttribArray(aPosition);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

		glEnableVertexAttribArray(aColor);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

		if (w) glDrawArrays(GL_TRIANGLES, 0, Vertex.size() / 7);
		else glDrawArrays(GL_LINE_LOOP, 0, Vertex.size() / 7);

		glDisableVertexAttribArray(aPosition);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
};

int num{};
int method{};
Color bg = beginConfig::bg;

Object* curObj{};

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
	curObj = new CubeO();
	createAxis();
	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SKeyboard);
	glutMouseFunc(Mouse);
	glutTimerFunc(1, loop, 1);
	glutMainLoop();
}

GLvoid drawScene()
{

	glClearColor(bg.r, bg.g, bg.b, bg.al);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 셰이더 사용하여 그리기
	if (curObj) curObj->draw();
	drawAxis();

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

bool h{ true }, u{ true };
GLvoid Keyboard(unsigned char key, int x, int y)
{

	switch (key) {
	case 'c':
		delete curObj;
		curObj = new CubeO();
		break;
	case 'p':
		delete curObj;
		curObj = new PyramidO();
		break;
	case 'h':
		if (h) {
			h = false;
			glEnable(GL_CULL_FACE);
		}
		else {
			h = true;
			glDisable(GL_CULL_FACE);
		}
		break;
	case 'u':
		if (u) {
			u = false;
			glEnable(GL_DEPTH_TEST);
		}
		else {
			u = true;
			glDisable(GL_DEPTH_TEST);
		}
		break;
	case 'w': case 'W':
		if (w) w = false;
		else w = true;
		break;
	case 'z':	//x축 음회전
		curObj->rotate(glm::vec3(-10, 0, 0));
		break;
	case 'x':	//x축 양회전
		curObj->rotate(glm::vec3(10, 0, 0));
		break;
	case 't':	//y축 음회전
		curObj->rotate(glm::vec3(0, -10, 0));
		break;
	case 'y':	//y축 양회전
		curObj->rotate(glm::vec3(0, 10, 0));
		break;

	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid SKeyboard(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		curObj->move(glm::vec3(-0.05, 0, 0));
		break;
	case GLUT_KEY_RIGHT:
		curObj->move(glm::vec3(0.05, 0, 0));
		break;
	case GLUT_KEY_UP:
		curObj->move(glm::vec3(0, 0.05, 0));
		break;
	case GLUT_KEY_DOWN:
		curObj->move(glm::vec3(0, -0.05, 0));
		break;
	}
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
		std::cout << "up???" << std::endl;
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