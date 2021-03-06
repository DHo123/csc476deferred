#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

/**
 * Mercenary Manager
 * @author Brandon Clark
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include "GLSL.h"
#include "Camera.h"
#include "Shape.h"
#include "Terrain.h"
#include "hud.h"
#include "menu.h"
#include "MatrixStack.h"
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "RenderingHelper.h"
#include "TextureLoader.h"
#include "obj3d.h"
#include "obj3dcontainer.h"
#include "tavern.h"
#include "Wagon.h"
#include "manager.h"
#include "TavernTerrain.h"
#include "Materials.h"
#include "FrustumCull.h"
#include <string>
#include "splineTest.cpp"
#include "TerrainEvent.h"
#include "text2D.hpp"
#include "SoundPlayer.h"
#include "Skybox.h"
#include "GBuffer.h"

using namespace std;
using namespace glm;

void idleGL();

float rF(float l, float h)
{
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}

TextureLoader texLoader;

//The Window & window size
GLFWwindow* window;
int g_width;
int g_height;

int points = 0;

Terrain terrain;
//Plane toggle for coloring
GLint terrainToggleID;

Wagon wagon;

int NUMOBJ = 5;
Camera camera;
bool gamePaused = false;
bool cull = false;
glm::vec2 mouse;
int shapeCount = 1;
std::vector<Shape> shapes;
//pid used for glUseProgram(pid);
GLuint pid;
GLuint light_pid;
GLuint geom_pid;
GLuint stencil_pid;
GLuint dir_pid;
GLint h_vertPos[5];
GLint h_vertNor[5];
GLint h_aTexCoord[1];
//Handles to the shader data
GLint h_uTexUnit;
GLint h_ProjMatrix[3];
GLint h_ViewMatrix[3];
GLint h_ModelMatrix[3];

GLint h_lightPos1;
GLint h_lightPos2;
GLint h_ka;
GLint h_kd;
GLint h_ks;
GLint h_s;
GLint h_option;
GLint h_flag;

GLint h_diff;
GLint h_norm;

bool keyToggles[256] = {false};
float t = 0.0f;
float h = 0.1f;
glm::vec3 location(0.0f,0.0f,0.0f);
glm::vec3 g(-10.0f, -5.0f, 0.0f);

// Display time to control fps
float timeOldDraw = 0.0f;
float timeNew = 0.0f;
float timeOldSpawn = 0.0f;

typedef struct{
	float x;
	float y;
	float z;
}threeFloat;

typedef struct{
	threeFloat ka;
	threeFloat kd;
	threeFloat ks;
	float s;
}Material;

int matCount = 2;
float optionS = 0.0f;
// int g_GiboLen;
GLuint NumBufObj, NumIndBufObj, NumTexBufObj;

//Rendering Helper
RenderingHelper ModelTrans;
Tavern tavern;
TerrainEvent terrEv;
Manager manager("The Dude");
TavernTerrain tavTerr;
Materials matSetter;
FrustumCull fCuller;
HUD hud(&manager);
Menu menu;
double dtDraw;
SoundPlayer audio;
GBuffer gbuf(1024, 768);

//The skybox
Skybox skybox;

/**
 * For now, this just initializes the Shape object.
 * Later, we'll updated to initialize all objects moving.
 * (This is very specific right now to one object).....
 */
void initShape(char * filename)
{
	t = 0.0f;
	h = 0.001f;

	//Initialize shapes here
}



/**
 * Generalized approach to intialization.
 */
void spinOffNewShape(char * filename, float x, float z){
	Shape temp;
	temp.load(filename);
	temp.init(x, z);
	shapes.push_back(temp);
}

void initModels()
{
	//Initialize Terrain object
	terrain.init(&texLoader, &matSetter, &fCuller);
	tavTerr.init(&texLoader);

	//Initalize Wagon
	wagon.init(&texLoader, &terrain, &menu, &gamePaused, &manager);

	//Initialize skybox
	skybox.init(&texLoader);

	//initialize the modeltrans matrix stack
   ModelTrans.useModelViewMatrix();
   ModelTrans.loadIdentity();
}

void initGL()
{	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);

	/* texture specific settings */
    glEnable(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gbuf.FBO_init();
}

/**
 * Initialize the shaders passed to the function
 */
bool installShaders(const string &vShaderName, const string &fShaderName)
{		
	GLint rc;

	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Read shader sources
	const char *vshader = GLSL::textFileRead(vShaderName.c_str());
	const char *fshader = GLSL::textFileRead(fShaderName.c_str());
	glShaderSource(VS, 1, &vshader, NULL);
	glShaderSource(FS, 1, &fshader, NULL);
	
	// Compile vertex shader
	glCompileShader(VS);
	GLSL::printError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(VS);
	if(!rc) {
		printf("Error compiling vertex shader %s\n", vShaderName.c_str());
	}
	
	// Compile fragment shader
	glCompileShader(FS);
	GLSL::printError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(FS);
	if(!rc) {
		printf("Error compiling fragment shader %s\n", fShaderName.c_str());
	}
	
	// Create the program and link
	pid = glCreateProgram();
	glAttachShader(pid, VS);
	glAttachShader(pid, FS);
	glLinkProgram(pid);
	GLSL::printError();
	glGetProgramiv(pid, GL_LINK_STATUS, &rc);
	GLSL::printProgramInfoLog(pid);
	if(!rc) {
		printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
	}

	// Check GLSL
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
	return true;
}

void initUniforms() {
	
	matSetter.init(&geom_pid, &h_ka, &h_kd, &h_ks, &h_s);
	h_vertPos[0] = GLSL::getAttribLocation(geom_pid, "vertPos");
	h_vertNor[0] = GLSL::getAttribLocation(geom_pid, "vertNor");
	h_aTexCoord[0] = GLSL::getAttribLocation(geom_pid, "aTexCoord");

	h_vertPos[1] = GLSL::getAttribLocation(stencil_pid, "vertPos");
	h_vertNor[1] = GLSL::getAttribLocation(stencil_pid, "vertNor");
	//h_aTexCoord[1] = GLSL::getAttribLocation(stencil_pid, "aTexCoord");

	assert(glGetError() == GL_NO_ERROR);
	h_vertPos[2] = GLSL::getAttribLocation(light_pid, "vertPos");
	assert(glGetError() == GL_NO_ERROR);
	h_vertNor[2] = GLSL::getAttribLocation(light_pid, "vertNor");

	assert(glGetError() == GL_NO_ERROR);
	h_vertPos[3] = GLSL::getAttribLocation(dir_pid, "vertPos");
	h_vertNor[3] = GLSL::getAttribLocation(dir_pid, "vertNor");

	h_ProjMatrix[0] = GLSL::getUniformLocation(geom_pid, "uProjMatrix");
	h_ViewMatrix[0] = GLSL::getUniformLocation(geom_pid, "uViewMatrix");
	h_ModelMatrix[0] = GLSL::getUniformLocation(geom_pid, "uModelMatrix");

	h_ProjMatrix[1] = GLSL::getUniformLocation(stencil_pid, "uProjMatrix");
	h_ViewMatrix[1] = GLSL::getUniformLocation(stencil_pid, "uViewMatrix");
	h_ModelMatrix[1] = GLSL::getUniformLocation(stencil_pid, "uModelMatrix");

	h_ProjMatrix[2] = GLSL::getUniformLocation(light_pid, "uProjMatrix");
	h_ViewMatrix[2] = GLSL::getUniformLocation(light_pid, "uViewMatrix");
	h_ModelMatrix[2] = GLSL::getUniformLocation(light_pid, "uModelMatrix");

	h_uTexUnit = GLSL::getUniformLocation(geom_pid, "uTexUnit");
	h_flag = GLSL::getUniformLocation(geom_pid, "flag");

	h_ka = GLSL::getUniformLocation(geom_pid, "ka");
	h_kd = GLSL::getUniformLocation(geom_pid, "kd");
	h_ks = GLSL::getUniformLocation(geom_pid, "ks");
	h_s = GLSL::getUniformLocation(geom_pid, "s");

	assert(glGetError() == GL_NO_ERROR);
	/*Toggle for plane coloring*/
	terrainToggleID = GLSL::getUniformLocation(geom_pid, "terrainToggle");

	//h_diff = GLSL::getUniformLocation(light_pid, "diffuse_texture");
	//h_norm = GLSL::getUniformLocation(light_pid, "normal_texture");
}

void test()
{
	cout << "test funct pointer" << endl;
}


void drawScene() {
}

void drawGL()
{
	gbuf.start();
	//Update Camera
	// Get mouse position
  	double xpos, ypos;

  	glfwGetCursorPos(window, &xpos, &ypos);
	camera.update(xpos, ypos, wagon.getPosition());

	//glUniform3fv(h_lightPos1, 1, glm::value_ptr(glm::vec3(23.05f, 4.0f, -23.5f)));
	//glUniform3fv(h_lightPos2, 1, glm::value_ptr(glm::vec3(-125.0f, 4.0f, 25.0f)));
	//glUniform1f(h_option, optionS);
	
	// Bind the program
	
	//Set projection matrix
	MatrixStack proj, view;
	proj.pushMatrix();
	camera.applyProjectionMatrix(&proj);
	proj.pushMatrix();
	camera.applyViewMatrix(&view, wagon.getPosition());
	
	// Set up geometry pass & Draw
	glUseProgram(geom_pid);
	gbuf.geometryPass();
	glUniform1i(h_flag, 0);
	glUniformMatrix4fv( h_ProjMatrix[0], 1, GL_FALSE, glm::value_ptr( proj.topMatrix()));
	glUniformMatrix4fv(h_ViewMatrix[0], 1, GL_FALSE, glm::value_ptr(view.topMatrix()));

	fCuller.setProjMat(proj.topMatrix(), view.topMatrix());
	
	matSetter.setMaterial(2);

   assert(glGetError() == GL_NO_ERROR);
	//========================== DRAW OUTSIDE SCENE ====================
	if (!camera.isTavernView() || camera.isFreeRoam())
	{
		glUniform1i(terrainToggleID, 1);
		glUniform1i(h_uTexUnit, 0);
   assert(glGetError() == GL_NO_ERROR);
		ModelTrans.loadIdentity();
		ModelTrans.pushMatrix();
			ModelTrans.translate(glm::vec3(-100.0, 0.0, 0.0));
			glUniformMatrix4fv(h_ModelMatrix[0], 1, GL_FALSE,
					glm::value_ptr(ModelTrans.modelViewMatrix));

   assert(glGetError() == GL_NO_ERROR);
			ModelTrans.pushMatrix();
				terrain.draw(h_vertPos[0], h_vertNor[0], h_aTexCoord[0], h_ModelMatrix[0], &camera, wagon.getPosition(), &geom_pid);
            //glUseProgram(geom_pid);
   assert(glGetError() == GL_NO_ERROR);
				wagon.draw(h_vertPos[0], h_vertNor[0], h_aTexCoord[0], h_ModelMatrix[0], &ModelTrans);
   assert(glGetError() == GL_NO_ERROR);
			ModelTrans.popMatrix();
		ModelTrans.popMatrix();
		
   assert(glGetError() == GL_NO_ERROR);
		ModelTrans.loadIdentity();
		ModelTrans.pushMatrix();
		ModelTrans.popMatrix();

		/*
		terrEv.drawTerrainEvents(h_ModelMatrix, h_vertPos, 
						h_vertNor, h_aTexCoord);
		terrEv.drawTerrainEvents(h_ModelMatrix, h_vertPos, h_vertNor,
						h_aTexCoord, dtDraw);
		*/

		glUniform1i(terrainToggleID, 0);
	// Draw the skybox
	skybox.draw(&camera, wagon.getPosition());
	}
	glUseProgram(geom_pid);

	//========================= END OUTSIDE SCENE =======================

   assert(glGetError() == GL_NO_ERROR);
	if (camera.isTavernView() || camera.isFreeRoam())
	{
		//Draw TAVERN
   assert(glGetError() == GL_NO_ERROR);
		glUniform1i(terrainToggleID, 1);
		//glUniform1i(h_uTexUnit, 0);

   assert(glGetError() == GL_NO_ERROR);
		ModelTrans.loadIdentity();
   assert(glGetError() == GL_NO_ERROR);
		ModelTrans.pushMatrix();
   assert(glGetError() == GL_NO_ERROR);
		tavTerr.draw(h_vertPos[0], h_vertNor[0], h_aTexCoord[0], h_ModelMatrix[0], &ModelTrans);
   assert(glGetError() == GL_NO_ERROR);

		matSetter.setMaterial(4);
   assert(glGetError() == GL_NO_ERROR);
		ModelTrans.popMatrix();
		matSetter.setMaterial(3);

   assert(glGetError() == GL_NO_ERROR);
		tavern.drawTavern(h_ModelMatrix[0], h_vertPos[0], h_vertNor[0], h_aTexCoord[0], dtDraw);

   assert(glGetError() == GL_NO_ERROR);
		glUniform1i(terrainToggleID, 0);
	}

   assert(glGetError() == GL_NO_ERROR);
	//**************Draw HUD START*********************
	if(hud.on)
	{
		glUseProgram(geom_pid);
		glUniform1i(h_flag, 1);
   assert(glGetError() == GL_NO_ERROR);
		if(menu.inMenu)
		{
   assert(glGetError() == GL_NO_ERROR);
			glUseProgram(geom_pid);
			menu.drawMenu();
			glUseProgram(geom_pid);
		}
   assert(glGetError() == GL_NO_ERROR);
		hud.drawHud(h_ModelMatrix[0], h_vertPos[0], g_width, g_height, h_aTexCoord[0]);
		//glUniform1i(h_flag, 0);

		if(!hud.homeScreenOn)
		{
			char info[64];
			sprintf(info,"x %d", manager.getGold());
			printText2D(info, 50, 566, 18);

			sprintf(info,"x %d", manager.getFood());
			printText2D(info, 220, 566, 18);

			sprintf(info,"x %d", manager.getBeer());
			printText2D(info, 430, 566, 18);

			sprintf(info,"x %d", manager.getMercs());
			printText2D(info, 620, 566, 18);
		}
		else
		{
			printText2D("Press Enter to Continue", 75, 75, 24);
		}
	}

   assert(glGetError() == GL_NO_ERROR);
	//**************Draw HUD FINISH********************
	glUseProgram(0);
	proj.popMatrix();
	proj.popMatrix();
	gbuf.geometryFinish();

   assert(glGetError() == GL_NO_ERROR);
	//--------------------Stencil Pass-------------------------
	glEnable(GL_STENCIL_TEST);

   assert(glGetError() == GL_NO_ERROR);
	//For Every Light
	gbuf.stencil(stencil_pid);

   assert(glGetError() == GL_NO_ERROR);
	// Reset matrix stacks
	proj.loadIdentity();
	view.loadIdentity();
	ModelTrans.loadIdentity();

   assert(glGetError() == GL_NO_ERROR);
	// Get Camera pos, and perspective stuff
	// Uniform get World View Proj Transform

	// Matrices for spheres
	proj.pushMatrix();
		camera.applyProjectionMatrix(&proj);
		proj.pushMatrix();
		camera.applyViewMatrix(&view, wagon.getPosition());
			// Set location of light sphere
			ModelTrans.pushMatrix();
			ModelTrans.scale(3.0);
	
			// Position the lights
			//ModelTrans.translate(glm::vec3(23.05f, 4.0f, -23.5f));

			// Uniforms
			glUniformMatrix4fv(h_ViewMatrix[1], 1, GL_FALSE, 						glm::value_ptr(view.topMatrix()));
			glUniformMatrix4fv(h_ProjMatrix[1], 1, GL_FALSE, 						glm::value_ptr(proj.topMatrix()));
			glUniformMatrix4fv(h_ModelMatrix[1], 1, GL_FALSE, 						glm::value_ptr(ModelTrans.modelViewMatrix));

   assert(glGetError() == GL_NO_ERROR);
			// Render Spheres
			Shape sphere;
			sphere.load("assets/sphere.obj");
   assert(glGetError() == GL_NO_ERROR);
			sphere.init(23.0,0.-23.5);
			sphere.norBufID = 0;
   assert(glGetError() == GL_NO_ERROR);
			sphere.draw(h_vertPos[1], h_vertNor[1]);

   assert(glGetError() == GL_NO_ERROR);
			ModelTrans.popMatrix();
		proj.popMatrix();
	proj.popMatrix();
	glUseProgram(0);
	printf("Stencil OK\n");

	//---------------------------------------------------------

	//------------------Point Lights--------------------------
	// Set Eye World Position

	gbuf.lightPass(light_pid);

	// Reset matrix stacks
	proj.loadIdentity();
	view.loadIdentity();
	ModelTrans.loadIdentity();

	// Get Camera pos, and perspective stuff
	// Uniform get World View Proj Transform

   assert(glGetError() == GL_NO_ERROR);
	// Matrices for spheres
	proj.pushMatrix();
		camera.applyProjectionMatrix(&proj);
		proj.pushMatrix();
		camera.applyViewMatrix(&view, wagon.getPosition());
			// Set location of light sphere
			ModelTrans.pushMatrix();
			ModelTrans.scale(3.0);
	
			// Position the lights
			//ModelTrans.translate(glm::vec3(23.05f, 4.0f, -23.5f));

			// Uniforms
			glUniformMatrix4fv(h_ViewMatrix[2], 1, GL_FALSE, 						glm::value_ptr(view.topMatrix()));
			glUniformMatrix4fv(h_ProjMatrix[2], 1, GL_FALSE, 						glm::value_ptr(proj.topMatrix()));
			glUniformMatrix4fv(h_ModelMatrix[2], 1, GL_FALSE, 						glm::value_ptr(ModelTrans.modelViewMatrix));

			// Render Spheres
			//Shape sphere;
			//sphere.load("assets/sphere.obj");
			sphere.init(23.05,-23.5);
			sphere.norBufID = 0;
			sphere.draw(h_vertPos[2], h_vertNor[2]);

   assert(glGetError() == GL_NO_ERROR);
			ModelTrans.popMatrix();
		proj.popMatrix();
	proj.popMatrix();

	gbuf.lightFinish();
	glDisable(GL_STENCIL_TEST);

	//------------------Directional Light----------------------

	gbuf.dirLightPass(dir_pid);

	Shape square;
	square.load("assets/square.obj");
	square.init(0.0,0.0);
	square.norBufID = 0;
	square.draw(h_vertPos[3], h_vertNor[3]);
	glUseProgram(0);
   assert(glGetError() == GL_NO_ERROR);
	glDisable(GL_BLEND);
	//---------------------------------------------------------
	gbuf.moveToScreen();
}

bool hasCollided(glm::vec3 incr)
{
	vector<Obj3d> temp = tavern.tavernItems;
	glm::vec3 camPos = camera.getPosition() + incr;

	float curCam[6] = {
    camera.bound.minX + camPos.x,
    camera.bound.maxX + camPos.x,
    camera.bound.minY + 1,
    camera.bound.maxY + 1,
    camera.bound.minZ + camPos.z,
    camera.bound.maxZ + camPos.z};

	bool validMove = (curCam[0] < 6.75 || curCam[1] > 39.5 || curCam[4] < -36.0 || curCam[5] > -11.4);

	for (std::vector<Obj3d>::iterator it1 = temp.begin(); it1 != temp.end(); ++it1)
	{
		glm::vec3 pos1 = it1 ->getCurSpot();
		if(it1->bound.checkCollision(curCam, it1->scale, pos1))
		{
			validMove = true;
		}
	}

	if (camera.isFreeRoam())
	{
		return false;
	}
	else
	{
		return validMove;
	}
}

/**
 * This will get called when any button on keyboard is pressed.
 */
void checkUserInput()
{
   float attemptX = numeric_limits<int>::min();
   float attemptZ = numeric_limits<int>::min();
   vec3 view = camera.getLookAtPoint() - camera.getTheEye();
   vec3 strafe = glm::cross(view, vec3(0.0, 1.0, 0.0));
   if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS && !hasCollided(-strafe))
   {
      camera.updateStrafe(-strafe);
   }
   if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !hasCollided(strafe))
   {
      camera.updateStrafe(strafe);
   }
   if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !hasCollided(view*1.2f))
   {
      camera.updateZoom(view*1.2f);
   }
   else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !hasCollided(view))
   {
      camera.updateZoom(view);
   }
   if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !hasCollided(-view*1.2f))
   {
      camera.updateZoom(-view*1.2f);
   }
   else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !hasCollided(-view))
   {
      camera.updateZoom(-view);
   }

}

void mouseScrollCB(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!camera.isTavernView())
	{
		camera.updateWagonZoom(yoffset);
	}
}

/**
 * Use this for debugging purposes for right now.
 */

 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	timeNew = glfwGetTime();
	double dtKey = timeNew - timeOldDraw;

	//This time step is causing issues for key inputs right now.
	// Update every 60Hz
	//if(dtKey >= (1.0 / 60.0) ) {
	//Free roam camera
	if (key == GLFW_KEY_0 && action == GLFW_PRESS)
   	{
   		camera.toggleFreeRoam();
   	}

   	//Create a new trail
   	if (key == GLFW_KEY_9 && action == GLFW_PRESS)
   	{
   		terrain.createTrail();
   		wagon.resetWagon();
   	}

	//Print Manager status
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		manager.reportStats();
	}
	
	//Buy food
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		manager.buyFood();
	}

	//Buy beer
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
		manager.buyBeer();
	}
	
	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5 && action == GLFW_PRESS)
	{
		// tavern.buyMercenary(key - GLFW_KEY_1, &manager);
		if(menu.inMenu)
		{
			menu.selectOption(key - GLFW_KEY_1);
		}
		else
		{
			manager.buyMercenary(key - GLFW_KEY_1, &tavern);
		}
	}
	
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		//manager.buyMercenary(key - GLFW_KEY_1, &tavern);
        tavern.tavernCharacters[0].wave();
	}

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
	{
		//manager.buyMercenary(key - GLFW_KEY_1, &tavern);
        hud.homeScreenOn = false;
	}


	if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        tavern.showMercsForSale();
    }

	//Leave Tavern
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
        manager.inTavern = manager.inTavern ? false : true;
		camera.toggleGameViews();
		audio.playBackgroundMusic(manager.inTavern);
	}

   	//Toggle between lines and filled polygons
   	if (key == GLFW_KEY_L && action == GLFW_PRESS)
   	{
      	GLint polyType;
      	glGetIntegerv(GL_POLYGON_MODE, &polyType);
      	if(polyType == GL_FILL) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			} 
			else 
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
   	}

   	//Toggle culling.
   	if (key == GLFW_KEY_K && action == GLFW_PRESS)
   	{
      	cull = !cull;
      	if(cull) {
				glEnable(GL_CULL_FACE);
			} 
			else 
			{
				glDisable(GL_CULL_FACE);
			}
   	}
   	//Start wagon
   	if (key == GLFW_KEY_8 && action == GLFW_PRESS)
		{
			wagon.startWagon();
		}
	//}
		//testing frustum culling
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		fCuller.toggleMode();
		//will be using this to toggle it on and off at a specified points, maybe others too....
	}
	//freezes current projection matrix in for frustum culling
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		fCuller.holdView();
	}
	//Toggle hud
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		hud.on = !hud.on;
	}

	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
    	//Create about vector and add an element
	 //  	vector<string> about;
		// about.push_back("about test");

		// //Create an option and add it to a vector
		// option testOpt = {"test option", test};
		// vector<option> options;
		// options.push_back(testOpt);

		// //Set the data
		// menu.setData("Title", about, options);
  //       gamePaused = !gamePaused;
  //       if(!gamePaused){
  //       	printf("%s\n", "reseting start time");
  //       	wagon.setTimeStamp(glfwGetTime());
  //       }
        
    }
	//lower drawbridge
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		terrEv.lowerBridge();
	}
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		audio.pause();
	}
	if (key == GLFW_KEY_U && action == GLFW_PRESS)
	{
		audio.playSoundEffect(EXPLOSION_SOUND);
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
	{
		audio.playVoice(MAGMISS_VOICE);
	}

}

void window_size_callback(GLFWwindow* window, int w, int h){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	g_width = w;
	g_height = h;
	camera.setWindowSize(w, h);
}

/**
 * Models that use animation should use this udpate function.
 **/
void updateModels()
{
	wagon.updateWagon(dtDraw);
}

void checkCollisions(){
	//Check collisions here.
}

int main(int argc, char **argv)
{
    // please don't remove this, using it to demo splines
    // splineTest(); 

	// Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

   glfwWindowHint(GLFW_SAMPLES, 4);
   glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Open a window and create its OpenGL context
   g_width = 1024;
   g_height = 768;
   window = glfwCreateWindow( g_width, g_height, "Mercenary Manager", NULL, NULL);
   if( window == NULL ){
      fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, mouseScrollCB);

    // Initialize glad
   if (!gladLoadGL()) {
      fprintf(stderr, "Unable to initialize glad");
      glfwDestroyWindow(window);
      glfwTerminate();
      return 1;
   }

  	// Ensure we can capture the escape key being pressed below
  	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  	glfwSetCursorPos(window, 0.0, 0.0);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initGL();
	installShaders("geom_vert.glsl", "geom_frag.glsl");
	geom_pid = pid;

	installShaders("stencil_vert.glsl", "stencil_frag.glsl");
	stencil_pid = pid;

	installShaders("light_vert.glsl", "light_frag.glsl");
	light_pid = pid;

	installShaders("dirlight_vert.glsl", "dirlight_frag.glsl");
	dir_pid = pid;
	assert(glGetError() == GL_NO_ERROR);

	//SOMETHING WRONG HERE TODO
	initUniforms();
	assert(glGetError() == GL_NO_ERROR);
	fCuller.init();
	tavern.init(&matSetter, &fCuller);
	// terrEv.init(&matSetter, &fCuller);
	std::string str = "assets/bunny.obj";
	// initShape(&str[0u]); //initShape(argv[0]);

	assert(glGetError() == GL_NO_ERROR);
	menu.initMenu(&texLoader, h_ModelMatrix[0], h_vertPos[0], g_width, g_height, h_aTexCoord[0], &manager, &gamePaused);
  	
  	initModels();
  	tavern.loadTavernMeshes(&texLoader);
	assert(glGetError() == GL_NO_ERROR);

 	//used only for testing purposes
  	// terrEv.loadTerrEvMeshes(&texLoader);
  	// vec3 loc = terrain.getStartPosition();
  	// terrEv.addMerchantStand(vec3(loc.x - 95.5, loc.y, loc.z), glm::mat4(1.0f));
  	// terrEv.addMerchantStand(vec3(loc.x - 92.5, loc.y, loc.z), glm::rotate(glm::mat4(1.0f), (const float)90, glm::vec3(0, 1.0f, 0)));
  	// terrEv.addMerchantStand(vec3(loc.x - 89.5, loc.y, loc.z), glm::rotate(glm::mat4(1.0f), (const float)180, glm::vec3(0, 1.0f, 0)));
  	// terrEv.addEndCity(vec3(loc.x - 82.5, loc.y, loc.z));

  	//Create about vector and add an element


  	hud.initHUD(&texLoader);
  	hud.initHomeScreen(&texLoader);
  	initText2D( "Holstein.DDS" );
  	dtDraw = 0;
  	audio.playBackgroundMusic(true);



 //  	vector<string> about;
	// about.push_back("about test");

	// //Create an option and add it to a vector
	// option testOpt = {"test option", test};
	// vector<option> options;
	// options.push_back(testOpt);

	// //Set the data
	// menu.setData("Title", about, options);


   do{
   		timeNew = glfwGetTime();
		audio.checkTime();
		dtDraw = timeNew - timeOldDraw;
		
		t += h;
	

		// Update every 60Hz
		if(dtDraw >= (1.0 / 60.0) ) {
			checkUserInput();
			if (camera.isTavernView() && !camera.isFreeRoam())
			{
				checkCollisions();
			}
			updateModels();
			timeOldDraw += (1.0 / 60.0);
			//Draw an image
			drawGL();
		}
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

   } // Check if the ESC key was pressed or the window was closed
   while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0 );

   // Close OpenGL window and terminate GLFW
   glfwTerminate();

	return 0;
}
