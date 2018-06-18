#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <list>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;
GLuint seaTexture;

GLuint ocean;
GLuint FISH;
GLuint CRAB;
GLuint BUBBLE;

//camerapos w momencie wystrzelenia, cameradir, time w momencie
std::list<std::tuple<glm::vec3, glm::vec3, float>> bubbleattack = {};


Core::Shader_Loader shaderLoader;

obj::Model sphereModel;
obj::Model fish;
obj::Model crab;

float timefour = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;
glm::vec3 cameraSide;
float cameraAngle = 0;
glm::mat4 cameraMatrix, perspectiveMatrix;

float cameraX, cameraY, lastX, lastY;
bool firstMouse = true;
glm::quat rotation = glm::quat(1, 0, 0, 0);

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

float angleSpeed = 0.1f;
float moveSpeed = 0.1f;

std::vector<glm::vec3> fishPositions;
std::vector<float> FishSize;

void drawObjectTextureSea(obj::Model * model, glm::mat4 modelMatrix, GLuint textureID)
{
	GLuint program = seaTexture;

	glUseProgram(program);
	Core::SetActiveTexture(textureID, "sampler2dtype", 1, 0);
	/*glUniform3f(glGetUniformLocation(program, "objectColor"), textureColor.x, textureColor.y, textureColor.z); */
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}



glm::mat4 createCameraMatrix()
{
	// Obliczanie kierunku patrzenia kamery (w plaszczyznie x-z) przy uzyciu zmiennej cameraAngle kontrolowanej przez klawisze.
	//cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	//glm::vec3 up = glm::vec3(0,1,0);

	//return Core::createViewMatrix(cameraPos, cameraDir, up);

	glm::quat rotationChange = glm::angleAxis(glm::radians(cameraY), glm::vec3(0.f, 1.f, 0.f)) * glm::angleAxis(glm::radians(cameraX), glm::vec3(1.f, 0.f, 0.f));
	rotation = glm::normalize(rotationChange * rotation);

	cameraDir = glm::inverse(rotation) * glm::vec3(0, 0, -1);
	cameraSide = glm::inverse(rotation) * glm::vec3(1, 0, 0);
	cameraX = 0; cameraY = 0;

	return Core::createViewMatrixQuat(cameraPos, rotation);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	/*
	glm::mat4 rotation;
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	rotation[0][0] = cos(time);
	rotation[2][0] = sin(time);
	rotation[0][2] = -sin(time);
	rotation[2][2] = cos(time);
	
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix * rotation;
	*/
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}


void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureID)
{
	GLuint program = programTexture;
	
	glUseProgram(program);
	Core::SetActiveTexture(textureID, "sampler2dtype", 1, 0);
	//glUniform3f(glGetUniformLocation(program, "objectColor"), textureColor.x, textureColor.y, textureColor.z); 
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	
	
	glm::mat4 rotation;
	/*
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	rotation[0][0] = cos(time);
	rotation[2][0] = sin(time);
	rotation[0][2] = -sin(time);
	rotation[2][2] = cos(time);
	*/
	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	

//	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix ;
	
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}


void mouse(int y, int x)
{
	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	cameraX = x - lastX;
	cameraY = y - lastY;
	lastX = x;
	lastY = y;

	float sensitivity = 0.2f;
	cameraX *= sensitivity;
	cameraY *= sensitivity;
}

void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania. Macierze sa przechowywane w zmiennych globalnych, bo uzywa ich funkcja drawObject.
	// (Bardziej elegancko byloby przekazac je jako argumenty do funkcji, ale robimy tak dla uproszczenia kodu.
	//  Jest to mozliwe dzieki temu, ze macierze widoku i rzutowania sa takie same dla wszystkich obiektow!)
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	// Macierz statku "przyczepia" go do kamery. Warto przeanalizowac te linijke i zrozumiec jak to dziala.
	//glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0,-0.25f,0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	//glm::mat4 turretMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0, -0.25f, 0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));


	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	//Ocean to wielka kula
	drawObjectTextureSea(&sphereModel, glm::translate(glm::vec3(0, 0, 0))* glm::scale(glm::vec3(70.0f)), ocean);


	//renderowanie babelkow
	timefour = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	float moveSpeedBUBBLE = 0.4f + moveSpeed;
	for (std::tuple<glm::vec3,glm::vec3, float> i : bubbleattack)
	{
		//do pozycji babelka dodaj predkosc pocisku w kierunku camerapos w momencie wystrzelania razy miniony czas
		glm::vec3 currentposition = std::get<0>(i) + std::get<1>(i) * (time - std::get<2>(i)) * moveSpeedBUBBLE;
		drawObjectTexture(&sphereModel, glm::translate(currentposition) * glm::scale(glm::vec3(0.006)), BUBBLE);
		
		
	}

	for (int i = 0; i < fishPositions.size(); i++) {

		//rysowanie kraba
		if (i % 50 == 0)
		{
			drawObjectTexture(&crab, glm::scale(glm::translate(fishPositions[i]), glm::vec3(FishSize[i], FishSize[i], FishSize[i])), CRAB);
			fishPositions[i] = fishPositions[i] + glm::vec3(0.0f, 0.0f, 1.0f - FishSize[i] / 50.0f);
			if (fishPositions[i].z > 275) fishPositions[i].z = -275;
		}
		else
		{
			drawObjectTexture(&fish, glm::scale(glm::translate(fishPositions[i]), glm::vec3(FishSize[i], FishSize[i], FishSize[i])), FISH);
			fishPositions[i] = fishPositions[i] + glm::vec3(0.0f, 0.0f, 1.0f - FishSize[i] / 50.0f);
			if (fishPositions[i].z > 275) fishPositions[i].z = -275;
		}
	}

	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	seaTexture = shaderLoader.CreateProgram("shaders/shader_tex_sea.vert", "shaders/shader_tex_sea.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	fish = obj::loadModelFromFile("models/fish.obj");
	crab = obj::loadModelFromFile("models/crab.obj");

	ocean = Core::LoadTexture("textures/ocean.png");
	FISH = Core::LoadTexture("textures/fish.png");
	CRAB = Core::LoadTexture("textures/crab.png");
	BUBBLE = Core::LoadTexture("textures/bubble.png");

	for (int i = 0; i < 720; i++) {
		fishPositions.push_back(glm::ballRand(150.0));

		if (i % 50 == 0)
			FishSize.push_back(3);
		//FishSize.push_back(4.5);
		else if (i % 20 == 0)
			FishSize.push_back(2.5);
		else if (i % 35 == 0)
			FishSize.push_back(0.5);
		else
			FishSize.push_back(1);
	}


}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(seaTexture);
}

void idle()
{
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;
	case 't': cameraPos += glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;
	case 'g': cameraPos -= glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeed; break;
		//Strzelanie babelkami
	case 'f':
		bubbleattack.push_back(std::make_tuple(glm::vec3(cameraPos)-glm::vec3(0,0.05f,0), glm::vec3(cameraDir), float(timefour)));
		break;

	}
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);


	glutInitWindowPosition(0, 0); //gdzie na ekranie wyrzuci program
	glutInitWindowSize(1200, 800); //rozmiar ekranu
	glutCreateWindow("Anna Krysiak & Artur Pieniazek"); //podpis
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}

