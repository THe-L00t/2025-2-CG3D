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

// Small cubes structure
struct SmallCube {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 size;
	Color color;
	bool isActive;
};

// Sphere structure
struct Sphere {
	glm::vec3 position;
	glm::vec3 velocity;
	float radius;
	Color color;
	bool isActive;
};

GameTimer* GameTimer::Instance;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int, int, int, int);
GLvoid PassiveMotion(int x, int y);
GLvoid loop(int);

// Shader
GLuint bs{};

// Cube
GLuint cubeVBO{};
void createCube();
void drawCube();

// Small cubes functions
void initializeSmallCubes();
void updatePhysics(float deltaTime);
void drawSmallCube(const SmallCube& cube);
bool checkCollision(const SmallCube& cube1, const SmallCube& cube2);

// Sphere functions
void createSphere();
void drawSphere(const Sphere& sphere);
void updateSpherePhysics(float deltaTime);

GameTimer gt;
Color bg = beginConfig::bg;

// Cube transformation variables
glm::vec3 cubePosition(0.0f, 0.0f, 0.0f);
float cubeRotationZ = 0.0f;
float moveSpeed = 0.1f;
int prevMouseX = -1;
bool bottomOpen = false; // Track if bottom face is open

// Camera revolution variables
float cameraRevolutionAngle = 0.0f;
float cameraRevolutionSpeed = 5.0f;
float cameraDistance = 3.0f;



std::vector<SmallCube> smallCubes;
std::vector<Sphere> spheres;
const int maxSpheres = 5;
const float gravity = -9.8f;
const float containerSize = 1.0f; // The main cube is from -0.5 to 0.5

// Sphere rendering data
GLuint sphereVBO;
int sphereVertexCount;

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
	createSphere();
	initializeSmallCubes();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(PassiveMotion);
	glutTimerFunc(1, loop, 1);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(bg.r, bg.g, bg.b, bg.al);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawCube();

	// Draw all small cubes
	for (const auto& cube : smallCubes) {
		if (cube.isActive) {
			drawSmallCube(cube);
		}
	}

	// Draw all spheres
	for (const auto& sphere : spheres) {
		if (sphere.isActive) {
			drawSphere(sphere);
		}
	}

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
	case 'a':
	case 'A':
		// Toggle bottom face open/closed
		bottomOpen = !bottomOpen;
		if (bottomOpen) {
			std::cout << "Bottom face opened" << std::endl;
		}
		else {
			std::cout << "Bottom face closed" << std::endl;
		}
		break;
	case 'b':
	case 'B':
		// Create a new sphere (max 5)
		if (spheres.size() < maxSpheres) {
			Sphere newSphere;
			newSphere.radius = 0.08f;
			newSphere.position = glm::vec3(0.0f, 0.0f, 0.0f);

			// Random initial velocity
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> velDistr(-0.5f, 0.5f);
			newSphere.velocity = glm::vec3(velDistr(gen), velDistr(gen), velDistr(gen));

			// Normalize and set speed
			if (glm::length(newSphere.velocity) > 0.0f) {
				newSphere.velocity = glm::normalize(newSphere.velocity) * 0.8f;
			}

			// Random color
			std::uniform_real_distribution<float> colorDistr(0.3f, 1.0f);
			newSphere.color = { colorDistr(gen), colorDistr(gen), colorDistr(gen), 1.0f };
			newSphere.isActive = true;

			spheres.push_back(newSphere);
			std::cout << "Sphere created. Total: " << spheres.size() << "/" << maxSpheres << std::endl;
		}
		else {
			std::cout << "Maximum number of spheres reached!" << std::endl;
		}
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

GLvoid PassiveMotion(int x, int y)
{
	if (prevMouseX == -1) {
		prevMouseX = x;
		return;
	}

	int deltaX = x - prevMouseX;

	// 마우스가 왼쪽으로 이동: 시계방향 회전 (각도 감소)
	// 마우스가 오른쪽으로 이동: 시계반대방향 회전 (각도 증가)
	cubeRotationZ += deltaX * 0.5f;

	prevMouseX = x;
	glutPostRedisplay();
}

GLvoid loop(int v)
{
	gt.Update();
	updatePhysics(gt.deltaTime);
	updateSpherePhysics(gt.deltaTime);

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
	worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cubeRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));

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

	// Draw 5 or 6 faces depending on if bottom is open
	int vertexCount = bottomOpen ? 30 : 36;
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);

	glDisableVertexAttribArray(aPosition);
	glDisableVertexAttribArray(aColor);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// AABB collision detection
bool checkCollision(const SmallCube& cube1, const SmallCube& cube2)
{
	glm::vec3 min1 = cube1.position - cube1.size * 0.5f;
	glm::vec3 max1 = cube1.position + cube1.size * 0.5f;
	glm::vec3 min2 = cube2.position - cube2.size * 0.5f;
	glm::vec3 max2 = cube2.position + cube2.size * 0.5f;

	return (min1.x <= max2.x && max1.x >= min2.x) &&
		   (min1.y <= max2.y && max1.y >= min2.y) &&
		   (min1.z <= max2.z && max1.z >= min2.z);
}

// Initialize small cubes with random sizes and positions
void initializeSmallCubes()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> sizeDistr(0.05f, 0.15f);
	std::uniform_real_distribution<float> colorDistr(0.3f, 1.0f);

	const int numCubes = 3;
	smallCubes.clear();

	// Fixed positions with different z values
	std::vector<glm::vec3> positions = {
		glm::vec3(0.0f, 0.3f, -0.25f),
		glm::vec3(0.0f, 0.3f, 0.0f),
		glm::vec3(0.0f, 0.3f, 0.25f)
	};

	for (int i = 0; i < numCubes; ++i) {
		SmallCube cube;
		cube.size = glm::vec3(sizeDistr(gen));
		cube.position = positions[i];
		cube.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		cube.color = { colorDistr(gen), colorDistr(gen), colorDistr(gen), 1.0f };
		cube.isActive = true;
		smallCubes.push_back(cube);
	}
}

// Update physics for all small cubes
void updatePhysics(float deltaTime)
{
	const float damping = 0.0f; // No elasticity - cubes don't bounce
	const float epsilon = 0.001f; // Small value to prevent sinking

	// Create rotation matrix for the container (inverse rotation to get local space)
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(cubeRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 inverseRotation = glm::inverse(rotationMatrix);

	// Transform gravity to local space
	glm::vec3 worldGravity(0.0f, gravity, 0.0f);
	glm::vec3 localGravity = glm::vec3(inverseRotation * glm::vec4(worldGravity, 0.0f));

	for (auto& cube : smallCubes) {
		if (!cube.isActive) continue;

		// Apply gravity in local space
		cube.velocity += localGravity * deltaTime;

		// Update position
		cube.position += cube.velocity * deltaTime;

		// Collision with container walls (in local space)
		float halfSize = containerSize * 0.5f;

		// X axis
		if (cube.position.x - cube.size.x * 0.5f < -halfSize) {
			cube.position.x = -halfSize + cube.size.x * 0.5f + epsilon;
			cube.velocity.x = 0.0f;
		}
		if (cube.position.x + cube.size.x * 0.5f > halfSize) {
			cube.position.x = halfSize - cube.size.x * 0.5f - epsilon;
			cube.velocity.x = 0.0f;
		}

		// Y axis (floor) - only if bottom is closed
		if (!bottomOpen && cube.position.y - cube.size.y * 0.5f < -halfSize) {
			cube.position.y = -halfSize + cube.size.y * 0.5f + epsilon;
			cube.velocity.y = 0.0f;
		}
		// If bottom is open and cube falls below, deactivate it
		if (bottomOpen && cube.position.y < -halfSize - 0.5f) {
			cube.isActive = false;
		}
		// Y axis (ceiling)
		if (cube.position.y + cube.size.y * 0.5f > halfSize) {
			cube.position.y = halfSize - cube.size.y * 0.5f - epsilon;
			cube.velocity.y = 0.0f;
		}

		// Z axis
		if (cube.position.z - cube.size.z * 0.5f < -halfSize) {
			cube.position.z = -halfSize + cube.size.z * 0.5f + epsilon;
			cube.velocity.z = 0.0f;
		}
		if (cube.position.z + cube.size.z * 0.5f > halfSize) {
			cube.position.z = halfSize - cube.size.z * 0.5f - epsilon;
			cube.velocity.z = 0.0f;
		}
	}

	// Cube-to-cube collision
	for (size_t i = 0; i < smallCubes.size(); ++i) {
		for (size_t j = i + 1; j < smallCubes.size(); ++j) {
			if (!smallCubes[i].isActive || !smallCubes[j].isActive) continue;

			if (checkCollision(smallCubes[i], smallCubes[j])) {
				// Simple collision response: separate and bounce
				glm::vec3 direction = smallCubes[j].position - smallCubes[i].position;
				float distance = glm::length(direction);

				if (distance > 0.0f) {
					direction = glm::normalize(direction);

					// Separate the cubes
					float overlap = (smallCubes[i].size.x + smallCubes[j].size.x) * 0.5f - distance;
					if (overlap > 0) {
						smallCubes[i].position -= direction * (overlap * 0.5f);
						smallCubes[j].position += direction * (overlap * 0.5f);
					}

					// Stop velocities on collision (no elasticity)
					smallCubes[i].velocity = glm::vec3(0.0f);
					smallCubes[j].velocity = glm::vec3(0.0f);
				}
			}
		}
	}
}

// Draw a small cube with specific size and color
void drawSmallCube(const SmallCube& cube)
{
	std::vector<float> cubeVertices;
	float sx = cube.size.x * 0.5f;
	float sy = cube.size.y * 0.5f;
	float sz = cube.size.z * 0.5f;

	// Front face
	cubeVertices.insert(cubeVertices.end(), {
		-sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al
	});

	// Back face
	cubeVertices.insert(cubeVertices.end(), {
		 sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al
	});

	// Left face
	cubeVertices.insert(cubeVertices.end(), {
		-sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al
	});

	// Right face
	cubeVertices.insert(cubeVertices.end(), {
		 sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al
	});

	// Top face
	cubeVertices.insert(cubeVertices.end(), {
		-sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx,  sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al
	});

	// Bottom face
	cubeVertices.insert(cubeVertices.end(), {
		-sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		-sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx, -sy,  sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al,
		 sx, -sy, -sz,	cube.color.r, cube.color.g, cube.color.b, cube.color.al
	});

	// Create temporary VBO
	GLuint tempVBO;
	glGenBuffers(1, &tempVBO);
	glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_DYNAMIC_DRAW);

	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	// Apply container transformation first, then cube's local position
	worldTransMatrix = glm::translate(worldTransMatrix, cubePosition);
	worldTransMatrix = glm::rotate(worldTransMatrix, glm::radians(cubeRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	worldTransMatrix = glm::translate(worldTransMatrix, cube.position);

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

	// Clean up
	glDeleteBuffers(1, &tempVBO);
}

// Create sphere geometry (icosphere approximation)
void createSphere()
{
	std::vector<float> vertices;
	const int latitudeBands = 16;
	const int longitudeBands = 16;
	const float radius = 1.0f;

	for (int lat = 0; lat <= latitudeBands; ++lat) {
		float theta = lat * glm::pi<float>() / latitudeBands;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);

		for (int lon = 0; lon <= longitudeBands; ++lon) {
			float phi = lon * 2.0f * glm::pi<float>() / longitudeBands;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);

			float x = cosPhi * sinTheta;
			float y = cosTheta;
			float z = sinPhi * sinTheta;

			vertices.push_back(radius * x);
			vertices.push_back(radius * y);
			vertices.push_back(radius * z);
		}
	}

	// Create indices for triangles
	std::vector<float> sphereVertices;
	for (int lat = 0; lat < latitudeBands; ++lat) {
		for (int lon = 0; lon < longitudeBands; ++lon) {
			int first = (lat * (longitudeBands + 1)) + lon;
			int second = first + longitudeBands + 1;

			// First triangle
			sphereVertices.push_back(vertices[first * 3]);
			sphereVertices.push_back(vertices[first * 3 + 1]);
			sphereVertices.push_back(vertices[first * 3 + 2]);

			sphereVertices.push_back(vertices[second * 3]);
			sphereVertices.push_back(vertices[second * 3 + 1]);
			sphereVertices.push_back(vertices[second * 3 + 2]);

			sphereVertices.push_back(vertices[(first + 1) * 3]);
			sphereVertices.push_back(vertices[(first + 1) * 3 + 1]);
			sphereVertices.push_back(vertices[(first + 1) * 3 + 2]);

			// Second triangle
			sphereVertices.push_back(vertices[second * 3]);
			sphereVertices.push_back(vertices[second * 3 + 1]);
			sphereVertices.push_back(vertices[second * 3 + 2]);

			sphereVertices.push_back(vertices[(second + 1) * 3]);
			sphereVertices.push_back(vertices[(second + 1) * 3 + 1]);
			sphereVertices.push_back(vertices[(second + 1) * 3 + 2]);

			sphereVertices.push_back(vertices[(first + 1) * 3]);
			sphereVertices.push_back(vertices[(first + 1) * 3 + 1]);
			sphereVertices.push_back(vertices[(first + 1) * 3 + 2]);
		}
	}

	sphereVertexCount = sphereVertices.size() / 3;

	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
}

// Draw a sphere
void drawSphere(const Sphere& sphere)
{
	glm::mat4 worldTransMatrix(1.0f);
	glm::mat4 viewTransMatrix(1.0f);
	glm::mat4 projectTransMatrix(1.0f);

	// Sphere is in world space, so just translate to its position
	worldTransMatrix = glm::translate(worldTransMatrix, cubePosition + sphere.position);
	worldTransMatrix = glm::scale(worldTransMatrix, glm::vec3(sphere.radius));

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
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Set color as vertex attribute (constant for all vertices)
	glDisableVertexAttribArray(aColor);
	glVertexAttrib4f(aColor, sphere.color.r, sphere.color.g, sphere.color.b, sphere.color.al);

	glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);

	glDisableVertexAttribArray(aPosition);
}

// Update sphere physics with wall collisions
void updateSpherePhysics(float deltaTime)
{
	const float epsilon = 0.001f;
	float halfSize = containerSize * 0.5f;

	// Create rotation matrix for the container
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(cubeRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 inverseRotation = glm::inverse(rotationMatrix);

	for (auto& sphere : spheres) {
		if (!sphere.isActive) continue;

		// Update position in world space (straight line motion)
		sphere.position += sphere.velocity * deltaTime;

		// Transform sphere position to container's local space for collision detection
		glm::vec3 localPos = glm::vec3(inverseRotation * glm::vec4(sphere.position, 1.0f));

		// Check collision with container walls in local space
		bool collided = false;
		glm::vec3 localNormal(0.0f);

		// X axis
		if (localPos.x - sphere.radius < -halfSize) {
			localPos.x = -halfSize + sphere.radius + epsilon;
			localNormal = glm::vec3(1.0f, 0.0f, 0.0f);
			collided = true;
		}
		else if (localPos.x + sphere.radius > halfSize) {
			localPos.x = halfSize - sphere.radius - epsilon;
			localNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
			collided = true;
		}

		// Y axis (floor) - only if bottom is closed
		if (!bottomOpen && localPos.y - sphere.radius < -halfSize) {
			localPos.y = -halfSize + sphere.radius + epsilon;
			localNormal = glm::vec3(0.0f, 1.0f, 0.0f);
			collided = true;
		}
		// If bottom is open and sphere falls below, deactivate it
		if (bottomOpen && localPos.y < -halfSize - 0.5f) {
			sphere.isActive = false;
			continue;
		}
		// Y axis (ceiling)
		if (localPos.y + sphere.radius > halfSize) {
			localPos.y = halfSize - sphere.radius - epsilon;
			localNormal = glm::vec3(0.0f, -1.0f, 0.0f);
			collided = true;
		}

		// Z axis
		if (localPos.z - sphere.radius < -halfSize) {
			localPos.z = -halfSize + sphere.radius + epsilon;
			localNormal = glm::vec3(0.0f, 0.0f, 1.0f);
			collided = true;
		}
		else if (localPos.z + sphere.radius > halfSize) {
			localPos.z = halfSize - sphere.radius - epsilon;
			localNormal = glm::vec3(0.0f, 0.0f, -1.0f);
			collided = true;
		}

		if (collided) {
			// Transform corrected position back to world space
			sphere.position = glm::vec3(rotationMatrix * glm::vec4(localPos, 1.0f));

			// Transform local normal to world space
			glm::vec3 worldNormal = glm::vec3(rotationMatrix * glm::vec4(localNormal, 0.0f));
			worldNormal = glm::normalize(worldNormal);

			// Reflect velocity in world space
			sphere.velocity = sphere.velocity - 2.0f * glm::dot(sphere.velocity, worldNormal) * worldNormal;
		}
	}
}
