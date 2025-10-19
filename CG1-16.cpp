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

GameTimer* GameTimer::Instance;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
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

GameTimer gt;
int ci = colorUid(dre);

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
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(30.f), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(-30.f), glm::vec3(0., 1., 0.));
		projectTransMatrix = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);



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

		glDrawArrays(GL_TRIANGLES, 0, Vertex.size() / 7);

		glDisableVertexAttribArray(aPosition);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glutSwapBuffers();
	}
};

class PyramidO : public Object{
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
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(30.f), glm::vec3(1., 0., 0.));
		worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(-30.f), glm::vec3(0., 1., 0.));
		projectTransMatrix = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);



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

		glDrawArrays(GL_TRIANGLES, 0, Vertex.size() / 7);

		glDisableVertexAttribArray(aPosition);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glutSwapBuffers();
	}
};

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
	glClear(GL_COLOR_BUFFER_BIT);

	// 셰이더 사용하여 그리기
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
