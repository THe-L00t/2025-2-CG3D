#include "loadShader.hpp"
#include "cheat.h"

namespace beginConfig {
	int width{ 800 };
	int height{ 800 };
	Color bg{ 1, 1, 1, 1 };
}

namespace CameraConfig {
	glm::vec3 pos{ 0.f, 0.f, 3.f };
	glm::vec3 dir{ 0.f, 0.f, 0.f };
	glm::vec3 up{ 0.f, 1.f, 0.f };
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
GLvoid Mouse(int, int, int, int);
GLvoid loop(int);

// Shader
GLuint bs{};

// Cube
GLuint cubeVBO{};
void createCube();
void drawCube();

GameTimer gt;
Color bg = beginConfig::bg;

// Cube transformation variables
glm::vec3 cubePosition(0.0f, 0.0f, 0.0f);
float moveSpeed = 0.1f;

// Camera revolution variables
float cameraRevolutionAngle = 0.0f;
float cameraRevolutionSpeed = 5.0f;
float cameraDistance = 3.0f;

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(beginConfig::width, beginConfig::height);
	glutCreateWindow("CG1-21");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}

	bs = CompileShaders("CG1-15.vs", "CG1-15.fs");

	createCube();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

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

	drawCube();

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	ViewfConfig::aspect = static_cast<float>(w) / static_cast<float>(h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'z':
		// Move cube in +Z direction
		cubePosition.z += moveSpeed;
		std::cout << "Cube position Z: " << cubePosition.z << std::endl;
		break;
	case 'Z':
		// Move cube in -Z direction
		cubePosition.z -= moveSpeed;
		std::cout << "Cube position Z: " << cubePosition.z << std::endl;
		break;
	case 'y':
		// Revolve camera around Y axis (left)
		cameraRevolutionAngle -= cameraRevolutionSpeed;
		CameraConfig::pos.x = cameraDistance * sin(glm::radians(cameraRevolutionAngle));
		CameraConfig::pos.z = cameraDistance * cos(glm::radians(cameraRevolutionAngle));
		std::cout << "Camera revolution angle: " << cameraRevolutionAngle << std::endl;
		break;
	case 'Y':
		// Revolve camera around Y axis (right)
		cameraRevolutionAngle += cameraRevolutionSpeed;
		CameraConfig::pos.x = cameraDistance * sin(glm::radians(cameraRevolutionAngle));
		CameraConfig::pos.z = cameraDistance * cos(glm::radians(cameraRevolutionAngle));
		std::cout << "Camera revolution angle: " << cameraRevolutionAngle << std::endl;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
}

GLvoid loop(int v)
{
	gt.Update();

	glutPostRedisplay();
	glutTimerFunc(1, loop, 1);
}

void createCube()
{
	std::vector<float> cubeVertices;

	// Front face (z = 0.5) - clockwise from outside
	cubeVertices.insert(cubeVertices.end(), {
		-0.5f, -0.5f,  0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	1.0f, 0.0f, 0.0f, 1.0f
	});

	// Back face (z = -0.5) - clockwise from outside
	cubeVertices.insert(cubeVertices.end(), {
		 0.5f, -0.5f, -0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f, 0.0f, 1.0f
	});

	// Left face (x = -0.5) - clockwise from outside
	cubeVertices.insert(cubeVertices.end(), {
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f, 1.0f, 1.0f
	});

	// Right face (x = 0.5) - clockwise from outside
	cubeVertices.insert(cubeVertices.end(), {
		 0.5f, -0.5f,  0.5f,	1.0f, 1.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	1.0f, 1.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	1.0f, 1.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	1.0f, 1.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	1.0f, 1.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f, 1.0f, 0.0f, 1.0f
	});

	// Top face (y = 0.5) - clockwise from outside
	cubeVertices.insert(cubeVertices.end(), {
		-0.5f,  0.5f,  0.5f,	1.0f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	1.0f, 0.0f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	1.0f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	1.0f, 0.0f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	1.0f, 0.0f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	1.0f, 0.0f, 1.0f, 1.0f
	});

	// Bottom face (y = -0.5) - clockwise from outside
	cubeVertices.insert(cubeVertices.end(), {
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, 1.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, 1.0f, 1.0f, 1.0f
	});

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
}

void drawCube()
{
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	// Apply transformations
	worldTransMatrix = glm::translate(worldTransMatrix, cubePosition);

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
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableVertexAttribArray(aPosition);
	glDisableVertexAttribArray(aColor);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
