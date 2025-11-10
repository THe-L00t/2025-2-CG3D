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
	glm::vec3 pos{ 5.f,5.f,5.f };
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
GLvoid Mouse(int, int, int, int);
GLvoid loop(int);

GLuint CompileShaders(const std::string_view&, const std::string_view&);
bool ReadFile(const std::string_view&, std::string*);
void AddShader(GLuint, const std::string_view&, GLenum);

// Shader
GLuint bs{};

// Axis
GLuint axisVBO{};
void createAxis();
void drawAxis();

// Floor
GLuint floorVBO{};
void createFloor();
void drawFloor();

// Tank
GLuint cubeVBO{};
void createCube();
void drawCube(const glm::mat4& parentTransform, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec4& color);
void drawTank();

GameTimer gt;
Color bg = beginConfig::bg;

// Camera control variables
float cameraMoveSpeed = 0.2f;
float cameraRotateSpeed = 5.0f;

// Tank control variables
glm::vec3 tankPosition(0.0f, 0.0f, 0.0f);
float tankMoveSpeed = 0.2f;
float middleBodyRotation = 0.0f;
float cannon1Rotation = 0.0f;
float cannon2Rotation = 0.0f;
float pole1Rotation = 0.0f;
float pole2Rotation = 0.0f;
float rotationSpeed = 5.0f;

// Swap animation variables
bool isSwapping = false;
float swapStartRotation = 0.0f;
float swapTargetRotation = 0.0f;
float swapAnimSpeed = 90.0f; // degrees per second

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(beginConfig::width, beginConfig::height);
	glutCreateWindow("CG1-20");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}

	bs = CompileShaders("CG1-15.vs", "CG1-15.fs");

	createAxis();
	createFloor();
	createCube();

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

	drawAxis();
	drawFloor();
	drawTank();

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'z':
		// Move camera forward along Z axis (negative direction)
		CameraConfig::pos.z -= cameraMoveSpeed;
		break;
	case 'Z':
		// Move camera backward along Z axis (positive direction)
		CameraConfig::pos.z += cameraMoveSpeed;
		break;
	case 'x':
		// Move camera right along X axis (positive direction)
		CameraConfig::pos.x += cameraMoveSpeed;
		break;
	case 'X':
		// Move camera left along X axis (negative direction)
		CameraConfig::pos.x -= cameraMoveSpeed;
		break;
	case 'y':
		// Rotate camera around its own Y axis (self-rotation, clockwise)
		{
			glm::vec3 viewDir = glm::normalize(CameraConfig::dir - CameraConfig::pos);
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 newDir = rotation * glm::vec4(viewDir, 0.0f);
			CameraConfig::dir = CameraConfig::pos + glm::vec3(newDir);
		}
		break;
	case 'Y':
		// Rotate camera around its own Y axis (self-rotation, counter-clockwise)
		{
			glm::vec3 viewDir = glm::normalize(CameraConfig::dir - CameraConfig::pos);
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotateSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 newDir = rotation * glm::vec4(viewDir, 0.0f);
			CameraConfig::dir = CameraConfig::pos + glm::vec3(newDir);
		}
		break;
	case 'r':
		// Rotate camera around screen center (dir point) Y axis (revolution, clockwise)
		{
			glm::vec3 center = CameraConfig::dir;
			glm::vec3 relative = CameraConfig::pos - center;
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotateSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 newRelative = rotation * glm::vec4(relative, 1.0f);
			CameraConfig::pos = center + glm::vec3(newRelative);
		}
		break;
	case 'R':
		// Rotate camera around screen center (dir point) Y axis (revolution, counter-clockwise)
		{
			glm::vec3 center = CameraConfig::dir;
			glm::vec3 relative = CameraConfig::pos - center;
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotateSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec4 newRelative = rotation * glm::vec4(relative, 1.0f);
			CameraConfig::pos = center + glm::vec3(newRelative);
		}
		break;
	case 't':
	case 'T':
		// Rotate middle body around Y axis
		middleBodyRotation += rotationSpeed;
		break;
	case 'l':
	case 'L':
		// Start swap animation if not already swapping
		if (!isSwapping) {
			isSwapping = true;
			swapStartRotation = middleBodyRotation;
			swapTargetRotation = middleBodyRotation + 180.0f;
			std::cout << "Swapping upper bodies..." << std::endl;
		}
		break;
	case 'g':
	case 'G':
		// Rotate cannons in opposite directions around Y axis
		cannon1Rotation += rotationSpeed;
		cannon2Rotation -= rotationSpeed;
		break;
	case 'p':
	case 'P':
		// Rotate poles in opposite directions around X axis
		pole1Rotation += rotationSpeed;
		pole2Rotation -= rotationSpeed;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid SKeyboard(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		// Move tank left (negative X)
		tankPosition.x -= tankMoveSpeed;
		break;
	case GLUT_KEY_RIGHT:
		// Move tank right (positive X)
		tankPosition.x += tankMoveSpeed;
		break;
	case GLUT_KEY_UP:
		// Move tank forward (positive Z)
		tankPosition.z += tankMoveSpeed;
		break;
	case GLUT_KEY_DOWN:
		// Move tank backward (negative Z)
		tankPosition.z -= tankMoveSpeed;
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

	// Update swap animation
	if (isSwapping) {
		float step = swapAnimSpeed * gt.deltaTime;
		if (middleBodyRotation < swapTargetRotation) {
			middleBodyRotation += step;
			if (middleBodyRotation >= swapTargetRotation) {
				middleBodyRotation = swapTargetRotation;
				isSwapping = false;
				std::cout << "Swap complete" << std::endl;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(1, loop, 1);
}

void createAxis()
{
	static std::array<float, 42> axisVertexs = {
		-10,0,0,	1,0,0,1,
		10,0,0,		1,0,0,1,
		0,-10,0,	0,1,0,1,
		0,10,0,		0,1,0,1,
		0,0,-10,	0,0,1,1,
		0,0,10,		0,0,1,1
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

void createFloor()
{
	static std::array<float, 42> floorVertexs = {
		// Green floor plane on XZ plane (Y = 0)
		// First triangle
		-10.0f, 0.0f, -10.0f,	0.0f, 1.0f, 0.0f, 1.0f,
		 10.0f, 0.0f, -10.0f,	0.0f, 1.0f, 0.0f, 1.0f,
		 10.0f, 0.0f,  10.0f,	0.0f, 1.0f, 0.0f, 1.0f,
		// Second triangle
		-10.0f, 0.0f, -10.0f,	0.0f, 1.0f, 0.0f, 1.0f,
		 10.0f, 0.0f,  10.0f,	0.0f, 1.0f, 0.0f, 1.0f,
		-10.0f, 0.0f,  10.0f,	0.0f, 1.0f, 0.0f, 1.0f
	};

	glGenBuffers(1, &floorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertexs), floorVertexs.data(), GL_STATIC_DRAW);
}

void drawFloor()
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
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(aPosition);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createCube()
{
	std::vector<float> cubeVertices;

	// Using cube data from cheat.h
	for (int face = 0; face < 6; face++) {
		for (int vertex = 0; vertex < 6; vertex++) {
			// Position
			cubeVertices.push_back(cube[face][vertex * 3]);
			cubeVertices.push_back(cube[face][vertex * 3 + 1]);
			cubeVertices.push_back(cube[face][vertex * 3 + 2]);
			// Color (will be set per draw call)
			cubeVertices.push_back(1.0f);
			cubeVertices.push_back(1.0f);
			cubeVertices.push_back(1.0f);
			cubeVertices.push_back(1.0f);
		}
	}

	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);
}

void drawCube(const glm::mat4& parentTransform, const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation, const glm::vec4& color)
{
	glm::mat4 worldTransMatrix = parentTransform;
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	// Apply transformations
	worldTransMatrix = glm::translate(worldTransMatrix, position);
	worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	worldTransMatrix = glm::scale(worldTransMatrix, scale);

	// Create colored vertex data
	std::vector<float> coloredVertices;
	for (int face = 0; face < 6; face++) {
		for (int vertex = 0; vertex < 6; vertex++) {
			// Position
			coloredVertices.push_back(cube[face][vertex * 3]);
			coloredVertices.push_back(cube[face][vertex * 3 + 1]);
			coloredVertices.push_back(cube[face][vertex * 3 + 2]);
			// Color
			coloredVertices.push_back(color.r);
			coloredVertices.push_back(color.g);
			coloredVertices.push_back(color.b);
			coloredVertices.push_back(color.a);
		}
	}

	GLuint tempVBO;
	glGenBuffers(1, &tempVBO);
	glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
	glBufferData(GL_ARRAY_BUFFER, coloredVertices.size() * sizeof(float), coloredVertices.data(), GL_STATIC_DRAW);

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
	glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);

	glEnableVertexAttribArray(aColor);
	glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
	glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableVertexAttribArray(aPosition);
	glDisableVertexAttribArray(aColor);

	glDeleteBuffers(1, &tempVBO);
}

void drawTank()
{
	// Tank base at world position
	glm::mat4 tankBase(1.0f);
	tankBase = glm::translate(tankBase, tankPosition);

	// 1. Base body (bottom) - Gray
	glm::vec4 baseColor(0.5f, 0.5f, 0.5f, 1.0f);
	drawCube(tankBase, glm::vec3(0.0f, 0.3f, 0.0f), glm::vec3(2.0f, 0.6f, 1.5f), glm::vec3(0.0f), baseColor);

	// 2. Middle body - White (with rotation)
	glm::mat4 middleTransform = tankBase;
	middleTransform = glm::translate(middleTransform, glm::vec3(0.0f, 0.6f, 0.0f));
	middleTransform = glm::rotate(middleTransform, glm::radians(middleBodyRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 middleColor(1.0f, 1.0f, 1.0f, 1.0f);
	drawCube(middleTransform, glm::vec3(0.0f, 0.4f, 0.0f), glm::vec3(1.5f, 0.8f, 1.2f), glm::vec3(0.0f), middleColor);

	// Upper body positions (fixed left and right)
	float leftX = -0.4f;
	float rightX = 0.4f;

	glm::vec4 upperColor(0.0f, 0.5f, 0.5f, 1.0f);
	glm::vec4 cannonColor(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 poleColor(1.0f, 1.0f, 0.0f, 1.0f);

	// 3. Upper body 1 - Dark cyan
	// Upper bodies revolve with middle body but do not rotate themselves
	glm::mat4 upper1Transform = middleTransform;
	upper1Transform = glm::translate(upper1Transform, glm::vec3(0.0f, 0.8f, 0.0f));
	glm::vec3 upper1Pos(leftX, 0.3f, 0.0f);

	glm::mat4 upper1Base = upper1Transform;
	upper1Base = glm::translate(upper1Base, upper1Pos);
	// Counter-rotate to prevent self-rotation
	upper1Base = glm::rotate(upper1Base, glm::radians(-middleBodyRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	drawCube(upper1Transform, upper1Pos, glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.0f, -middleBodyRotation, 0.0f), upperColor);

	// Upper body 1 - Cannon - Black (with rotation around Y axis)
	glm::mat4 cannon1Transform = upper1Base;
	cannon1Transform = glm::rotate(cannon1Transform, glm::radians(cannon1Rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	drawCube(cannon1Transform, glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.2f, 0.2f, 1.2f), glm::vec3(0.0f), cannonColor);

	// Upper body 1 - Flag pole - Yellow (with rotation around X axis)
	// Pivot at bottom of pole (top of upper body)
	glm::mat4 pole1Transform = upper1Base;
	pole1Transform = glm::translate(pole1Transform, glm::vec3(0.0f, 0.3f, 0.0f)); // Move to top of upper body
	pole1Transform = glm::rotate(pole1Transform, glm::radians(pole1Rotation), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X
	drawCube(pole1Transform, glm::vec3(0.0f, 0.4f, 0.0f), glm::vec3(0.1f, 0.8f, 0.1f), glm::vec3(0.0f), poleColor); // Draw pole above pivot

	// 4. Upper body 2 - Dark cyan
	glm::vec3 upper2Pos(rightX, 0.3f, 0.0f);

	glm::mat4 upper2Base = upper1Transform;
	upper2Base = glm::translate(upper2Base, upper2Pos);
	// Counter-rotate to prevent self-rotation
	upper2Base = glm::rotate(upper2Base, glm::radians(-middleBodyRotation), glm::vec3(0.0f, 1.0f, 0.0f));
	drawCube(upper1Transform, upper2Pos, glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.0f, -middleBodyRotation, 0.0f), upperColor);

	// Upper body 2 - Cannon - Black (with rotation around Y axis)
	glm::mat4 cannon2Transform = upper2Base;
	cannon2Transform = glm::rotate(cannon2Transform, glm::radians(cannon2Rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	drawCube(cannon2Transform, glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.2f, 0.2f, 1.2f), glm::vec3(0.0f), cannonColor);

	// Upper body 2 - Flag pole - Yellow (with rotation around X axis)
	// Pivot at bottom of pole (top of upper body)
	glm::mat4 pole2Transform = upper2Base;
	pole2Transform = glm::translate(pole2Transform, glm::vec3(0.0f, 0.3f, 0.0f)); // Move to top of upper body
	pole2Transform = glm::rotate(pole2Transform, glm::radians(pole2Rotation), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X
	drawCube(pole2Transform, glm::vec3(0.0f, 0.4f, 0.0f), glm::vec3(0.1f, 0.8f, 0.1f), glm::vec3(0.0f), poleColor); // Draw pole above pivot
}
