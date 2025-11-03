#include "loadShader.hpp"
#include "OBJLoader.h"
#include "cheat.h"

namespace beginConfig {
	int width{ 800 };
	int height{ 800 };
	Color bg{ 1,1,1,1 };
}
namespace CameraConfig {
	glm::vec3 pos{ 0.f, 1.f, 3.f };
	glm::vec3 dir{ 0.f, 0.f, 0.f };  // Looking at origin
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
GLvoid SpecialKeyboard(int key, int x, int y);
GLvoid RanColor(Color&);
GLvoid Mouse(int, int, int, int);
GLvoid loop(int);


GLuint CompileShaders(const std::string_view&, const std::string_view&);
bool ReadFile(const std::string_view&, std::string*);
void AddShader(GLuint, const std::string_view&, GLenum);
void GenerateSphere(std::vector<float>&, std::vector<unsigned int>&, float, int, int, float, float, float, float);
void GenerateCircle(std::vector<float>&, float, int, float, float, float, float);

// Shader
GLuint bs{};
GLuint VBO{};
GLuint EBO{};
GLuint VAO{};
GLuint VBO_blue{};
GLuint VAO_blue{};
GLuint VAO_orbit{};
GLuint VBO_orbit{};
GLuint VAO_green{};
GLuint VBO_green{};
GLuint VAO_green_orbit{};
GLuint VBO_green_orbit{};
std::vector<float> sphereVertices{};
std::vector<float> blueSphereVertices{};
std::vector<float> greenSphereVertices{};
std::vector<unsigned int> sphereIndices{};
std::vector<float> orbitCircleVertices{};
std::vector<float> greenOrbitCircleVertices{};

GameTimer gt;

int num{};
int method{};
Color bg = beginConfig::bg;

// Camera movement variables
float cameraSpeed = 0.1f;
float cameraAngle = 0.0f;  // Y-axis rotation angle
float cameraDistance = 3.0f;  // Distance from origin

// Rendering mode toggles
bool isPerspective = true;  // true: perspective, false: orthographic
bool isSolid = true;  // true: solid, false: wireframe

// Orbit variables
float orbitRadius = 1.5f;
float orbitSpeed = 0.01f;
float orbitPlaneTilts[3] = { glm::radians(30.0f), glm::radians(0.0f), glm::radians(-30.0f) };
float orbitRotation = 0.0f;

// Green sphere orbit variables
float greenOrbitRadius = 0.5f;
float greenOrbitSpeed = 0.02f;
float greenOrbitRotation = 0.0f;


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

	// Generate red sphere vertices (center)
	GenerateSphere(sphereVertices, sphereIndices, 0.5f, 30, 30, 1.0f, 0.0f, 0.0f, 1.0f);

	// Generate blue sphere vertices (orbiting)
	std::vector<unsigned int> tempIndices;
	GenerateSphere(blueSphereVertices, tempIndices, 0.25f, 30, 30, 0.0f, 0.0f, 1.0f, 1.0f);

	// Generate green sphere vertices (orbiting blue spheres)
	GenerateSphere(greenSphereVertices, tempIndices, 0.125f, 30, 30, 0.0f, 1.0f, 0.0f, 1.0f);

	// Generate orbit circle vertices (black color)
	GenerateCircle(orbitCircleVertices, orbitRadius, 100, 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate green orbit circle vertices (black color)
	GenerateCircle(greenOrbitCircleVertices, greenOrbitRadius, 50, 0.0f, 0.0f, 0.0f, 1.0f);

	// Setup VAO, VBO, EBO for red center sphere
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

	// Position attribute (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute (r, g, b, a)
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Setup VAO, VBO for blue orbiting spheres
	glGenVertexArrays(1, &VAO_blue);
	glGenBuffers(1, &VBO_blue);

	glBindVertexArray(VAO_blue);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_blue);
	glBufferData(GL_ARRAY_BUFFER, blueSphereVertices.size() * sizeof(float), blueSphereVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Position attribute (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute (r, g, b, a)
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Setup VAO, VBO for orbit circles
	glGenVertexArrays(1, &VAO_orbit);
	glGenBuffers(1, &VBO_orbit);

	glBindVertexArray(VAO_orbit);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_orbit);
	glBufferData(GL_ARRAY_BUFFER, orbitCircleVertices.size() * sizeof(float), orbitCircleVertices.data(), GL_STATIC_DRAW);

	// Position attribute (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute (r, g, b, a)
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Setup VAO, VBO for green spheres
	glGenVertexArrays(1, &VAO_green);
	glGenBuffers(1, &VBO_green);

	glBindVertexArray(VAO_green);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_green);
	glBufferData(GL_ARRAY_BUFFER, greenSphereVertices.size() * sizeof(float), greenSphereVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// Position attribute (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute (r, g, b, a)
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Setup VAO, VBO for green orbit circles
	glGenVertexArrays(1, &VAO_green_orbit);
	glGenBuffers(1, &VBO_green_orbit);

	glBindVertexArray(VAO_green_orbit);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_green_orbit);
	glBufferData(GL_ARRAY_BUFFER, greenOrbitCircleVertices.size() * sizeof(float), greenOrbitCircleVertices.data(), GL_STATIC_DRAW);

	// Position attribute (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute (r, g, b, a)
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutMouseFunc(Mouse);
	glutTimerFunc(1, loop, 1);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(bg.r, bg.g, bg.b, bg.al);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use shader
	GLuint shader = bs;
	glUseProgram(shader);

	// Set up transformation matrices
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Center position

	glm::mat4 view = glm::lookAt(
		CameraConfig::pos,
		CameraConfig::dir,
		CameraConfig::up
	);

	// Set projection matrix based on current mode
	glm::mat4 projection;
	if (isPerspective) {
		projection = glm::perspective(
			ViewfConfig::fovy,
			ViewfConfig::aspect,
			ViewfConfig::n,
			ViewfConfig::f
		);
	}
	else {
		// Orthographic projection
		float orthoSize = 3.0f;
		projection = glm::ortho(
			-orthoSize * ViewfConfig::aspect,
			orthoSize * ViewfConfig::aspect,
			-orthoSize,
			orthoSize,
			ViewfConfig::n,
			ViewfConfig::f
		);
	}

	// Send matrices to shader (using correct uniform names)
	GLuint modelLoc = glGetUniformLocation(shader, "worldT");
	GLuint viewLoc = glGetUniformLocation(shader, "viewT");
	GLuint projectionLoc = glGetUniformLocation(shader, "projectionT");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Set polygon mode based on current mode
	if (isSolid) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Draw center sphere
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

	// Draw 3 orbit circles with tilted planes
	glBindVertexArray(VAO_orbit);
	int circleVertexCount = orbitCircleVertices.size() / 7;
	for (int i = 0; i < 3; i++)
	{
		glm::mat4 orbitCircleModel = glm::mat4(1.0f);

		// Rotate orbit plane around X-axis (tilt the plane)
		orbitCircleModel = glm::rotate(orbitCircleModel, orbitPlaneTilts[i], glm::vec3(1.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(orbitCircleModel));
		glDrawArrays(GL_LINE_LOOP, 0, circleVertexCount);
	}

	// Draw 3 orbiting blue spheres with tilted orbit planes
	glBindVertexArray(VAO_blue);
	glm::vec3 blueSpherePositions[3];
	for (int i = 0; i < 3; i++)
	{
		glm::mat4 orbitModel = glm::mat4(1.0f);

		// Rotate orbit plane around X-axis (tilt the plane)
		orbitModel = glm::rotate(orbitModel, orbitPlaneTilts[i], glm::vec3(1.0f, 0.0f, 0.0f));

		// Position on the tilted orbit circle
		float x = orbitRadius * cosf(orbitRotation);
		float z = orbitRadius * sinf(orbitRotation);
		orbitModel = glm::translate(orbitModel, glm::vec3(x, 0.0f, z));

		// Store blue sphere position for green sphere orbits
		glm::vec4 worldPos = orbitModel * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		blueSpherePositions[i] = glm::vec3(worldPos);

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(orbitModel));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
	}

	// Draw 3 green orbit circles around blue spheres (parallel to blue sphere orbit planes)
	glBindVertexArray(VAO_green_orbit);
	int greenCircleVertexCount = greenOrbitCircleVertices.size() / 7;
	for (int i = 0; i < 3; i++)
	{
		glm::mat4 greenOrbitCircleModel = glm::mat4(1.0f);

		// Rotate orbit plane around X-axis (same tilt as parent blue sphere)
		greenOrbitCircleModel = glm::rotate(greenOrbitCircleModel, orbitPlaneTilts[i], glm::vec3(1.0f, 0.0f, 0.0f));

		// Position on the blue sphere's orbit
		float x = orbitRadius * cosf(orbitRotation);
		float z = orbitRadius * sinf(orbitRotation);
		greenOrbitCircleModel = glm::translate(greenOrbitCircleModel, glm::vec3(x, 0.0f, z));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(greenOrbitCircleModel));
		glDrawArrays(GL_LINE_LOOP, 0, greenCircleVertexCount);
	}

	// Draw 3 green spheres orbiting around blue spheres
	glBindVertexArray(VAO_green);
	for (int i = 0; i < 3; i++)
	{
		glm::mat4 greenModel = glm::mat4(1.0f);

		// Rotate orbit plane around X-axis (same tilt as parent blue sphere)
		greenModel = glm::rotate(greenModel, orbitPlaneTilts[i], glm::vec3(1.0f, 0.0f, 0.0f));

		// Position on the blue sphere's orbit
		float blueX = orbitRadius * cosf(orbitRotation);
		float blueZ = orbitRadius * sinf(orbitRotation);
		greenModel = glm::translate(greenModel, glm::vec3(blueX, 0.0f, blueZ));

		// Position green sphere on its orbit around blue sphere
		float greenX = greenOrbitRadius * cosf(greenOrbitRotation);
		float greenZ = greenOrbitRadius * sinf(greenOrbitRotation);
		greenModel = glm::translate(greenModel, glm::vec3(greenX, 0.0f, greenZ));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(greenModel));
		glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);

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
	case 'p':
	case 'P':
		// Toggle projection mode
		isPerspective = !isPerspective;
		std::cout << (isPerspective ? "Perspective" : "Orthographic") << " projection" << std::endl;
		break;
	case 'm':
	case 'M':
		// Toggle wireframe mode
		isSolid = !isSolid;
		std::cout << (isSolid ? "Solid" : "Wireframe") << " mode" << std::endl;
		break;
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

GLvoid SpecialKeyboard(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		// Rotate camera left around Y-axis
		cameraAngle += glm::radians(5.0f);
		break;
	case GLUT_KEY_RIGHT:
		// Rotate camera right around Y-axis
		cameraAngle -= glm::radians(5.0f);
		break;
	case GLUT_KEY_UP:
		// Move camera closer (decrease distance)
		cameraDistance -= cameraSpeed;
		if (cameraDistance < 0.5f) cameraDistance = 0.5f;  // Minimum distance
		break;
	case GLUT_KEY_DOWN:
		// Move camera farther (increase distance)
		cameraDistance += cameraSpeed;
		if (cameraDistance > 10.0f) cameraDistance = 10.0f;  // Maximum distance
		break;
	}

	// Update camera position based on angle and distance
	CameraConfig::pos.x = cameraDistance * sinf(cameraAngle);
	CameraConfig::pos.z = cameraDistance * cosf(cameraAngle);

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

	// Update orbit rotation
	orbitRotation += orbitSpeed;
	if (orbitRotation > 2 * glm::pi<float>()) {
		orbitRotation -= 2 * glm::pi<float>();
	}

	// Update green sphere orbit rotation
	greenOrbitRotation += greenOrbitSpeed;
	if (greenOrbitRotation > 2 * glm::pi<float>()) {
		greenOrbitRotation -= 2 * glm::pi<float>();
	}

	glutPostRedisplay();
	glutTimerFunc(1, loop, 1);
}

void GenerateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, int sectorCount, int stackCount, float r, float g, float b, float a)
{
	vertices.clear();
	indices.clear();

	float x, y, z, xy;
	float sectorStep = 2 * glm::pi<float>() / sectorCount;
	float stackStep = glm::pi<float>() / stackCount;
	float sectorAngle, stackAngle;

	// Generate vertices
	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = glm::pi<float>() / 2 - i * stackStep;
		xy = radius * cosf(stackAngle);
		z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;

			x = xy * cosf(sectorAngle);
			y = xy * sinf(sectorAngle);

			// Position
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			// Color
			vertices.push_back(r);
			vertices.push_back(g);
			vertices.push_back(b);
			vertices.push_back(a);
		}
	}

	// Generate indices
	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);
		k2 = k1 + sectorCount + 1;

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}
}

void GenerateCircle(std::vector<float>& vertices, float radius, int segmentCount, float r, float g, float b, float a)
{
	vertices.clear();

	float angleStep = 2 * glm::pi<float>() / segmentCount;

	for (int i = 0; i < segmentCount; ++i)
	{
		float angle = i * angleStep;
		float x = radius * cosf(angle);
		float z = radius * sinf(angle);

		// Position (circle on XZ plane, Y = 0)
		vertices.push_back(x);
		vertices.push_back(0.0f);
		vertices.push_back(z);

		// Color
		vertices.push_back(r);
		vertices.push_back(g);
		vertices.push_back(b);
		vertices.push_back(a);
	}
}
