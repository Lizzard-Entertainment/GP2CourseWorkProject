/*
SUMMARY OF CODE CONTRIBUTIONS.

Euan:
	- Orbit camera.
	- Post processing shader tabbing. (tab key) (OPTIONAL ELEMENT)
	- Camera switching. (m key)

Tom:
	- Flying camera.
	- Ambient daylight system. (OPTIONAL ELEMENT)
	- mouse capture.

Calum:
	- Model position, rotation, scale.
	- On-screen controls display. (OPTIONAL ELEMENT) - PLANNED BUT NOT IMPLEMENTED.  COULDN'T GET FONT TO WORK.
*/

//Headers
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <gl/GLU.h>
#include <SDL_mouse.h>

//Our headers
#include "Vertex.h"
#include "Shader.h"
#include "Texture.h"
#include "GameObject.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include "Light.h"
#include "FBXLoader.h"
#include "PostProcessing.h"
#include "SkyboxMaterial.h"
#include "CameraType.h"
#include "PPFilterType.h"
#include "FontRenderer.h"

//Using statements
using glm::mat4;
using glm::vec4;
using glm::vec3;

//Paths
const std::string ASSET_PATH = "assets/";
const std::string FONT_PATH = "fonts/";
const std::string MODEL_PATH = "models/";
const std::string SHADER_PATH = "shaders/";
const std::string MODEL_TEXTURE_PATH = "textures/models/";
const std::string SKYBOX_TEXTURE_PATH = "textures/skybox/";

//Constant vectors
const vec3 X_AXIS = vec3(1, 0, 0);
const vec3 Y_AXIS = vec3(0, 1, 0);
const vec3 Z_AXIS = vec3(0, 0, 1);

//SDL Window
SDL_Window * window = NULL;

//SDL GL Context
SDL_GLContext glcontext = NULL;

//Window Width
const int WINDOW_WIDTH = 1280;

//Window Height
const int WINDOW_HEIGHT = 800;

//Flag for game's running state. 
bool running = true;

//Custom timer
int timer[] = {0,0}; //virtual hours/mins
clock_t clicks;
int dT = 0;

//Daylight-System
int ColorTemp[24][3] = {
	{ 60, 60, 60 },
	{ 60, 60, 60 },
	{ 60, 60, 60 },
	{ 60, 60, 60 },
	{ 84, 51, 134 },
	{ 78, 110, 168 },
	{ 97, 174, 178 },
	{ 109, 210, 217 },
	{ 170, 242, 241 },
	{ 117, 228, 244 },
	{ 133, 228, 232 },
	{ 170, 242, 241 },
	{ 175, 239, 238 },
	{ 190, 248, 247 },
	{ 205, 255, 254 },
	{ 170, 242, 241 },
	{ 218, 232, 196 },
	{ 238, 214, 118 },
	{ 248, 202, 130 },
	{ 237, 119, 74 },
	{ 220, 152, 187 },
	{ 122, 68, 83 },
	{ 60, 60, 60 } };


//Scene bare light colour
vec4 ambientLightColour = vec4(0.1f, 0.5f, 1.0f, 0.5f);
//vec4 ambientLightColour = vec4(2.0f, 2.0f, 1.0f, 1.0f);

//Main scene game objects
std::vector<GameObject*> displayList;
std::vector<GameObject*> camerasVec;  //Vector for the camera types.  
GameObject * orbitCamera;
GameObject * flyingCamera;
GameObject * FPCamera;
GameObject * mainCamera;  //This is switched out with the orbit or debug camera, and is used in any calculations etc.
GameObject * mainLight;
GameObject * skyBox;
PostProcessing postProcessor;
FontRenderer fontRenderer;

//Input globals
float cameraSpeed = 1.0f;

//Model Loading folder string
std::string modelSubPath;

void CheckForErrors()
{
    GLenum error;
    do
        error=glGetError();
	while(error!=GL_NO_ERROR);
}

//Global functions
void InitWindow(int width, int height, bool fullscreen)
{
	//Create a window
	window = SDL_CreateWindow
	(
		"Lizzard Entertainment",    // window title
		SDL_WINDOWPOS_CENTERED,     // x position, centered
		SDL_WINDOWPOS_CENTERED,     // y position, centered
		width,                      // width, in pixels
		height,                     // height, in pixels
		SDL_WINDOW_OPENGL			// flags		
	);
}

void CleanUp()
{
	// clean up in reverse
	//Clean up font renderer
	fontRenderer.destroy();

	//Clean up skybox
	if (skyBox)
	{
		skyBox->destroy();
		delete skyBox;
		skyBox = NULL;
	}

	//Clean up post processor
	postProcessor.destroy();

	//Clean up game objects
    auto iter=displayList.begin();
	while(iter!=displayList.end())
    {
        (*iter)->destroy();
        if ((*iter))
        {            
            (*iter)=NULL;
			delete (*iter);
            iter=displayList.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    displayList.clear();
    
	//Clean up cameras
	iter = camerasVec.begin();
	while (iter != camerasVec.end())
	{
		(*iter)->destroy();
		if ((*iter))
		{
			(*iter) = NULL;
			delete (*iter);			
			iter = camerasVec.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	camerasVec.clear();

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
}

//Function to initialise OpenGL
void initOpenGL()
{
    //Ask for version 3.2 of OpenGL    
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
	//Create OpenGL Context
	glcontext = SDL_GL_CreateContext(window);

	//Check for GLEW error
    glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
	}
    
    //Smooth shading
    glShadeModel( GL_SMOOTH );
    
    //clear the background to black
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    
    //Clear the depth buffer
    glClearDepth( 1.0f );
    
    //Enable depth testing
    glEnable( GL_DEPTH_TEST );
    
    //The depth test to go
    glDepthFunc( GL_LEQUAL );
    
    //Turn on best perspective correction
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
}

//Function to set/reset viewport
void setViewport( int width, int height )
{    
    //make sure height is always above 1
    if ( height == 0 ) height = 1;

    //Setup viewport
    glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );
}

void createSkyBox()
{
	Vertex triangleData[] = 
	{
		//Front
		{ vec3(-500, 500, 500) },// Top Left
		{ vec3(-500, -500, 500) },// Bottom Left
		{ vec3(500, -500, 500) }, //Bottom Right
		{ vec3(500, 500, 500) },// Top Right

		//back
		{ vec3(-500, 500, -500) },// Top Left
		{ vec3(-500, -500, -500) },// Bottom Left
		{ vec3(500, -500, -500) }, //Bottom Right
		{ vec3(500, 500, 500) }// Top Right
	};


	GLuint indices[] = {
		//front
		0, 1, 2,
		0, 3, 2,

		//left
		4, 5, 1,
		4, 1, 0,

		//right
		3, 7, 2,
		7, 6, 2,

		//bottom
		1, 5, 2,
		6, 2, 1,

		//top
		5, 0, 7,
		5, 7, 3,

		//back
		4, 5, 6,
		4, 7, 6
	};

	//creat mesh and copy in

	Mesh * pMesh = new Mesh();
	pMesh->init();

	pMesh->copyVertexData(8, sizeof(Vertex), (void**)triangleData);
	pMesh->copyIndexData(36, sizeof(GLuint), (void**)indices);

	Transform *t = new Transform();
	t->setPosition(0.0f, 0.0f, 0.0f);

	//load textures and skybox material + Shaders
	SkyBoxMaterial *material = new SkyBoxMaterial();
	material->init();

	std::string vsPath = ASSET_PATH + SHADER_PATH + "skyboxVS.glsl";
	std::string fsPath = ASSET_PATH + SHADER_PATH + "skyboxFS.glsl";
	material->loadShader(vsPath, fsPath);

	std::string posXTexturename = ASSET_PATH + SKYBOX_TEXTURE_PATH + "Sunny_left.png";
	std::string negXTexturename = ASSET_PATH + SKYBOX_TEXTURE_PATH + "Sunny_right.png";
	std::string posYTexturename = ASSET_PATH + SKYBOX_TEXTURE_PATH + "Sunny_up.png";
	std::string negYTexturename = ASSET_PATH + SKYBOX_TEXTURE_PATH + "Sunny_down.png";
	std::string posZTexturename = ASSET_PATH + SKYBOX_TEXTURE_PATH + "Sunny_front.png";
	std::string negZTexturename = ASSET_PATH + SKYBOX_TEXTURE_PATH + "Sunny_back.png";
	
	material->loadCubeTexture(posXTexturename, negXTexturename, posYTexturename, negYTexturename, posZTexturename, negZTexturename);
	
	//create gameobject but don't add to queue!
	skyBox = new GameObject();
	skyBox->setName("Skybox");
	skyBox->setMaterial(material);
	skyBox->setTransform(t);
	skyBox->setMesh(pMesh);

	CheckForErrors();
}


void Initialise()
{
	//Initialise font renderer
	fontRenderer.init(WINDOW_WIDTH, WINDOW_HEIGHT, ASSET_PATH + FONT_PATH + "oratorstd.otf", 16, 
		ASSET_PATH + SHADER_PATH + "textureVS.glsl", ASSET_PATH + SHADER_PATH + "textureFS.glsl");

	//grab mouse
	int MouseTrapVar;
	MouseTrapVar = SDL_SetRelativeMouseMode(SDL_TRUE);

	//Forward declare draw methods
	void ComplexDraw(std::string modelFile, std::string diffuseFile, std::string specularFile, std::string normalFile, std::string heightFile, vec3 position, vec3 rotation, vec3 scale, std::string name);
	void BasicDraw(std::string modelFile, std::string diffuseFile, vec3 position, vec3 rotation, vec3 scale, std::string name);

	//Create Skybox
	createSkyBox();

	//Set shader paths
	std::string vsPath = ASSET_PATH + SHADER_PATH + "passThroughVS.glsl";
	std::string fsPath = ASSET_PATH + SHADER_PATH + "ColourFilterPPFS.glsl";

	//Initialise post-processor
	postProcessor.init(WINDOW_WIDTH, WINDOW_HEIGHT, vsPath, fsPath);

#pragma region Euan - Orbit Camera

	//Set up orbitcamera gameobject - this will be the initial camera
	orbitCamera = new GameObject();
	orbitCamera->setName("Orbit Camera");
    
	//Set up orbitcamera transform
	Transform *t = new Transform();
	t->setPosition(0.0f, 30.0f, 70.0f);
	orbitCamera->setTransform(t);
    
	//Set up orbitcamera camera
	Camera * c = new Camera();
	c->setLookAt(0.0f, 0.0f, 0.0f);
	orbitCamera->setCamera(c);

	//Push to cameras vector
	camerasVec.push_back(orbitCamera);

#pragma endregion

#pragma region Tom -Flying camera 

	//Set up debugcamera gameobject
	flyingCamera = new GameObject();
	flyingCamera->setName("Flying Camera");

	//Set up debugcamera transform
	t = new Transform();
	t->setPosition(0.0f, 2.0f, 10.0f);
	flyingCamera->setTransform(t);

	//Set up debugcamera camera
	c = new Camera();
	c->setLookAt(0.0f, 0.0f, 0.0f);
	flyingCamera->setCamera(c);

	//Push to cameras vector
	camerasVec.push_back(flyingCamera);

#pragma endregion


#pragma region Main Camera

	//Adds the main camera to the list of game objects, since it's the game object that will be worked on.
	//The cameras act as definitions for the main camera, and are not worked on directly.
	mainCamera = camerasVec[ORBIT_CAMERA];
	displayList.push_back(mainCamera);

#pragma endregion

#pragma endregion

	//Set up main light game object
	mainLight = new GameObject();
	mainLight->setName("MainLight");

	//Set up main light transform.
	t = new Transform();
	t->setPosition(0.0f, 10.0f, 0.0f);
	mainLight->setTransform(t);

	//Main light light
	Light * light = new Light();
	mainLight->setLight(light);

	//Add main light to game object list.
	displayList.push_back(mainLight);

    //Initialise all other game objects
    for(auto iter=displayList.begin();iter!=displayList.end();iter++)
    {
        (*iter)->init();
    }

#pragma region Calum - Model positioning.

	// Draw Ground
	modelSubPath = "Ground/";
	BasicDraw("Ground.fbx", "Ground.png", vec3(0.0f, 0.0f, 0.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), "Ground");


	//Draw Jeeps
	modelSubPath = "ArmouredCar/";
	BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(5.0f, 0.0f, 60.0f), vec3(0.0f, 250.0f, 0.0f), vec3(2.0f, 2.0f, 2.0f), "Jeep1");
	BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(20.0f, 0.0f, 60.0f), vec3(0.0f, -40.0f, 0.0f), vec3(2.0f, 2.0f, 2.0f), "Jeep2");
	BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(100.0f, 5.0f, 60.0f), vec3(0.0f, -180.0f, 0.0f), vec3(2.0f, 2.0f, 2.0f), "Jeep2");
	BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(10.0f, 0.0f, 40.0f), vec3(0.0f, -180.0f, 0.0f), vec3(2.0f, 2.0f, 2.0f), "Jeep2");
	BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -180.0f, 0.0f), vec3(2.0f, 2.0f, 2.0f), "Jeep2");

	//Draw tanks
	modelSubPath = "Tank/";
	BasicDraw("Tank.fbx", "Tank.png", vec3(-45.0f, 1.0f, 60.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(-30.0f, 1.0f, 60.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(-12.5f, 1.0f, 60.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(-45.0f, 1.0f, 30.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(-30.0f, 1.0f, 30.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(-12.5f, 1.0f, 30.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(50.0f, 1.0f, -100.0f), vec3(-90.0f, 0.0f, -180.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(80.0f, 1.0f, -100.0f), vec3(-90.0f, 0.0f, -180.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(50.0f, 1.0f, -120.0f), vec3(-90.0f, 0.0f, -180.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	BasicDraw("Tank.fbx", "Tank.png", vec3(80.0f, 1.0f, -120.0f), vec3(-90.0f, 0.0f, -180.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");

	//Draw Building
	modelSubPath = "Building/";
	BasicDraw("Building1.fbx", "building.png", vec3(-10.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	BasicDraw("Building1.fbx", "building.png", vec3(-40.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	BasicDraw("Building1.fbx", "building.png", vec3(20.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	BasicDraw("Building1.fbx", "building.png", vec3(50.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	BasicDraw("Building1.fbx", "building.png", vec3(80.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	BasicDraw("Building1.fbx", "building.png", vec3(110.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	BasicDraw("Building1.fbx", "building.png", vec3(140.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	BasicDraw("Building1.fbx", "building.png", vec3(150.0f, 10.0f, 80.0f), vec3(0.0f, -90.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");

	BasicDraw("Tank.fbx", "Tank.png", vec3(0.0f, -100.0f, -0.0f), vec3(-90.0f, 0.0f, -180.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");

	/*
	//Parallax jeeps
	modelSubPath = "ArmouredCar/";
	ComplexDraw("armoredrecon.fbx", "armoredrecon_diff.png", "armoredrecon_N.png", "armoredrecon_spec.png", "armoredrecon_Height.png", vec3(-10.0f, 0.0f, 0.0f), vec3(0.0f, -40.0f, 0.0f), vec3(1.0f), "Jeep1");		
	ComplexDraw("armoredrecon.fbx", "armoredrecon_diff.png", "armoredrecon_N.png", "armoredrecon_spec.png", "armoredrecon_Height.png", vec3(10.0f, 0.0f, 0.0f), vec3(0.0f, 40.0f, 0.0f), vec3(1.0f), "Jeep2");

	// Draw Ground
	modelSubPath = "Ground/";
	BasicDraw("Ground.fbx", "Ground.png", vec3(0.0f, 0.0f, 0.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), "Ground");

	//Draw building - NONE OF THESE LOOK GOOD.
	modelSubPath = "Building/";
	ComplexDraw("Building1.fbx", "building_d.png", "building_Tn.png", "building_Ts.png", "building_Th.png", vec3(15.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	ComplexDraw("Building1.fbx", "building_d.png", "building_Tn.png", "building_Ts.png", "building_Th.png", vec3(-12.5f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");
	ComplexDraw("Building1.fbx", "building_d.png", "building_Tn.png", "building_Ts.png", "building_Th.png", vec3(-40.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building");

	//Draw Tanks - looks OK.
	modelSubPath = "Tank/";
	ComplexDraw("Tank.fbx", "tank_d.png", "tank_Tn.png", "tank_Ts.png", "tank_Th.png", vec3(-30.0f, 0.0f, 60.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");
	ComplexDraw("Tank.fbx", "tank_d.png", "tank_Tn.png", "tank_Ts.png", "tank_Th.png", vec3(-12.5f, 1.0f, 60.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(20.0f, 20.0f, 20.0f), "Tank");

	//Draw Tents with UV
	//BasicDraw("Tent.fbx", "TentUV2.png", vec3(30.0f, 1.0f, -40.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", true);
	
	//ComplexDraw("Tent.fbx", "Camo.png", "Camo.png", vec3(60.0f, 1.0f, -40.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");

	////Draw tanks
	//BasicDraw("Tank.fbx", "Tank.png", vec3(-30.0f, 1.0f, 60.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(31.73838f, 31.73838f, 31.73838f), "Tank", "Focusable");
	//BasicDraw("Tank.fbx", "Tank.png", vec3(-12.5f, 1.0f, 60.0f), vec3(-90.0f, 0.0f, -90.0f), vec3(31.73838f, 31.73838f, 31.73838f), "Tank", "Focusable");
	//
	////Draw Building
	//BasicDraw("Building1.fbx", "building.png", vec3(-12.5f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building", "Focusable");
	//BasicDraw("Building1.fbx", "building.png", vec3(-40.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building", "Focusable");
	//BasicDraw("Building1.fbx", "building.png", vec3(190.0f, 4.5f, -40.0f), vec3(0.0f, 0.0f, 0.0f), vec3(1.79655f, 0.502816f, 1), "Building", "Focusable");

	////Draw Tents
	//BasicDraw("Tent.fbx", "Camo.png", vec3(30.0f, 1.0f, -40.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");
	//BasicDraw("Tent.fbx", "Camo.png", vec3(60.0f, 1.0f, -40.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");

	//BasicDraw("Tent.fbx", "Camo.png", vec3(30.0f, 1.0f, -100.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");
	//BasicDraw("Tent.fbx", "Camo.png", vec3(60.0f, 1.0f, -100.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");

	//BasicDraw("Tent.fbx", "Camo.png", vec3(110.0f, 1.0f, -40.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");
	//BasicDraw("Tent.fbx", "Camo.png", vec3(140.0f, 1.0f, -40.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");

	//BasicDraw("Tent.fbx", "Camo.png", vec3(110.0f, 1.0f, -100.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");
	//BasicDraw("Tent.fbx", "Camo.png", vec3(140.0f, 1.0f, -100.0f), vec3(-90.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");

	//BasicDraw("Tent.fbx", "Camo.png", vec3(140.0f, 5.0f, 60.0f), vec3(-90.0f, 0.0f, 180.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");
	//BasicDraw("Tent.fbx", "Camo.png", vec3(165.0f, 5.0f, 60.0f), vec3(-90.0f, 0.0f, 180.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");
	//BasicDraw("Tent.fbx", "Camo.png", vec3(190.0f, 5.0f, 60.0f), vec3(-90.0f, 0.0f, 180.0f), vec3(1.0f, 1.0f, 0.7f), "Tent", "Focusable");

	////Draw armoured cars
	//BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(20.0f, 0.0f, 60.0f), vec3(0.0f, 140.0f, 0.0f), vec3(3.0f, 3.0f, 3.0f), "Car", "Focusable");

	//BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(5.0f, 0.0f, 60.0f), vec3(0.0f, 170.0f, 0.0f), vec3(3.0f, 3.0f, 3.0f), "Car", "Focusable");

	//modelSubPath = "ArmouredCar/";
	///BasicDraw("armoredrecon.fbx", "armoredrecon_diff.png", vec3(-10.0f, -1000000.0f, 0.0f), vec3(0.0f, 40.0f, 0.0f), vec3(1.0f), "Car");
	*/

#pragma endregion
}

#pragma region Tom - Ambient daylight system.

void DayLightChange(int hours, int mins)
{
	float conversion = 0.00390625;
	int HoursF = hours + 1;
	if (HoursF == 24) HoursF = 0;

	float R = (ColorTemp[HoursF][0] - ColorTemp[hours][0]); 
	R = ((R / 59) * mins) + ColorTemp[hours][0];
	float G = (ColorTemp[HoursF][1] - ColorTemp[hours][1]);
	G = ((G/ 59) * mins) + ColorTemp[hours][1];
	float B = (ColorTemp[HoursF][2] - ColorTemp[hours][2]);
	B = ((B/ 59) * mins) + ColorTemp[hours][2];

	Light * light = new Light();
	light = mainLight->getLight();

	light->setDiffuseColour((R * conversion), (G * conversion), (B * conversion), 1.0f);
}

void Timer()
{
	clicks = clock();
	if ((clicks-dT) > 24){ // 24 real sec -> 24 virtual hours

		timer[0]++; //mins
		timer[1]; //hours

		
		if (timer[0] == 60)
		{
			timer[1]++;
			timer[0] = 0;
		}
		if (timer[1] == 24)
		{
			timer[1] = 0;
		}
		DayLightChange(timer[1], timer[0]);
		dT = clicks;

	//DEBUG
	//	std::cout << "Time: " << timer[1] << ":" << timer[0] << std::endl << std::endl;
	}
}
#pragma endregion

//Function to update the game state
void update()
{
	skyBox->update();

    //Update all game objects.
    for(auto iter=displayList.begin();iter!=displayList.end();iter++)
    {
		//Shader breaking - 4th iteration
        (*iter)->update();
    }

	Timer();
}

void renderGameObject(GameObject * pObject)
{
	if (!pObject)
		return;

	pObject->render();

	Mesh * currentMesh = pObject->getMesh();
	Transform * currentTransform = pObject->getTransform();
	Material * currentMaterial = (Material*)pObject->getMaterial();  //We know it'll be a standard material.

	if (currentMesh && currentMaterial && currentTransform)
	{
		currentMaterial->bind();
		currentMesh->bind();

		//Original stuff
		GLint MVPLocation = currentMaterial->getUniformLocation("MVP");
		GLint ModelLocation = currentMaterial->getUniformLocation("Model");
		GLint ambientMatLocation = currentMaterial->getUniformLocation("ambientMaterialColour");
		GLint ambientLightLocation = currentMaterial->getUniformLocation("ambientLightColour");
		GLint diffuseMatLocation = currentMaterial->getUniformLocation("diffuseMaterialColour");
		GLint diffuseLightLocation = currentMaterial->getUniformLocation("diffuseLightColour");
		GLint lightDirectionLocation = currentMaterial->getUniformLocation("lightDirection");
		GLint specularMatLocation = currentMaterial->getUniformLocation("specularMaterialColour");
		GLint specularLightLocation = currentMaterial->getUniformLocation("specularLightColour");
		GLint specularpowerLocation = currentMaterial->getUniformLocation("specularPower");
		GLint cameraPositionLocation = currentMaterial->getUniformLocation("cameraPosition");
		GLint diffuseTextureLocation = currentMaterial->getUniformLocation("diffuseMap");
		GLint specTextureLocation = currentMaterial->getUniformLocation("specMap");
		GLint bumpTextureLocation = currentMaterial->getUniformLocation("bumpMap");
		GLint heightTextureLocation = currentMaterial->getUniformLocation("heightMap");
		Camera * cam = mainCamera->getCamera();
		Light* light = mainLight->getLight();

		mat4 MVP = cam->getProjection()*cam->getView()*currentTransform->getModel();
		mat4 Model = currentTransform->getModel();

		vec4 ambientMaterialColour = currentMaterial->getAmbientColour();
		vec4 diffuseMaterialColour = currentMaterial->getDiffuseColour();
		vec4 specularMaterialColour = currentMaterial->getSpecularColour();
		float specularPower = currentMaterial->getSpecularPower();

		vec4 diffuseLightColour = light->getDiffuseColour();
		vec4 specularLightColour = light->getSpecularColour();
		vec3 lightDirection = light->getDirection();

		vec3 cameraPosition = mainCamera->getTransform()->getPosition();

		glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(Model));
		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform4fv(ambientMatLocation, 1, glm::value_ptr(ambientMaterialColour));
		glUniform4fv(ambientLightLocation, 1, glm::value_ptr(ambientLightColour));

		glUniform4fv(diffuseMatLocation, 1, glm::value_ptr(diffuseMaterialColour));
		glUniform4fv(diffuseLightLocation, 1, glm::value_ptr(diffuseLightColour));
		glUniform3fv(lightDirectionLocation, 1, glm::value_ptr(lightDirection));

		glUniform4fv(specularMatLocation, 1, glm::value_ptr(specularMaterialColour));
		glUniform4fv(specularLightLocation, 1, glm::value_ptr(specularLightColour));

		glUniform3fv(cameraPositionLocation, 1, glm::value_ptr(cameraPosition));
		glUniform1f(specularpowerLocation, specularPower);

		glUniform1i(diffuseTextureLocation, 0);
		glUniform1i(specTextureLocation, 1);
		glUniform1i(bumpTextureLocation, 2);
		glUniform1i(heightTextureLocation, 3);

		glDrawElements(GL_TRIANGLES, currentMesh->getIndexCount(), GL_UNSIGNED_INT, 0);

		currentMaterial->unbind();
	}

	for (int i = 0; i < pObject->getChildCount(); i++)
	{		
		renderGameObject(pObject->getChild(i));
	}
}

void renderSkyBox()
{
	skyBox->render();

	Mesh * currentMesh = skyBox->getMesh();
	SkyBoxMaterial * currentMaterial = (SkyBoxMaterial*)skyBox->getMaterial();
	if (currentMesh && currentMaterial)
	{
		Camera * cam = mainCamera->getCamera();

		currentMaterial->bind();
		currentMesh->bind();

		GLint cameraLocation = currentMaterial->getUniformLocation("cameraPos");
		GLint viewLocation = currentMaterial->getUniformLocation("view");
		GLint projectionLocation = currentMaterial->getUniformLocation("projection");
		GLint cubeTextureLocation = currentMaterial->getUniformLocation("cubeTexture");

		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(cam->getProjection()));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(cam->getView()));
		glUniform4fv(cameraLocation, 1, glm::value_ptr(mainCamera->getTransform()->getPosition()));
		glUniform1i(cubeTextureLocation, 0);

		glDrawElements(GL_TRIANGLES, currentMesh->getIndexCount(), GL_UNSIGNED_INT, 0);

		currentMaterial->unbind();
	}
	CheckForErrors();
}

//Function to render(aka draw)
void render()
{
	//Bind Framebuffer
	postProcessor.bind();

	//Clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
	glClearDepth(1.0f);

    //clear the colour and depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
	//render skybox
	renderSkyBox();

    //alternative sytanx
	for (auto iter = displayList.begin(); iter != displayList.end(); iter++)
	{
		renderGameObject((*iter));
	}

	//now switch to normal framebuffer
	postProcessor.preDraw();

	//Grab stuff from shader
	GLint colourFilterLocation = postProcessor.getUniformVariableLocation("colourFilter");
	glUniformMatrix4fv(colourFilterLocation, 1, GL_FALSE, glm::value_ptr(getShader()));

	//Post processor draw
	postProcessor.draw();

	//Draw string
	//fontRenderer.drawString(100, 100, "FONTTEST");  - FONT TUTORIAL DID NOT WORK.

	//Swap buffers and draw to scene.
    SDL_GL_SwapWindow(window);
}

#pragma region Euan - Modified model loading.
void ComplexDraw(std::string modelFile, std::string diffuseFile, std::string normalFile, std::string specularFile, std::string heightFile, vec3 position, vec3 rotation, vec3 scale, std::string name)
{
	GameObject * go = loadFBXFromFile(ASSET_PATH + MODEL_PATH + modelFile);

	for (int i = 0; i < go->getChildCount(); i++)
	{
		Material * material = new Material();
		material->init();
		material->loadShader(ASSET_PATH + SHADER_PATH + "ParallaxMappingVS.glsl", ASSET_PATH + SHADER_PATH + "ParallaxMappingFS.glsl");
		
		material->loadDiffuseMap(ASSET_PATH + MODEL_TEXTURE_PATH + modelSubPath + diffuseFile);
		material->loadBumpMap(ASSET_PATH + MODEL_TEXTURE_PATH + modelSubPath + normalFile);
		material->loadSpecularMap(ASSET_PATH + MODEL_TEXTURE_PATH + modelSubPath + specularFile);
		material->loadHeightMap(ASSET_PATH + MODEL_TEXTURE_PATH + modelSubPath + heightFile);
		go->getChild(i)->setMaterial(material);
	}
	go->getTransform()->setPosition(position);
	go->getTransform()->setRotation(rotation);
	go->getTransform()->setScale(scale);
	go->setName(name);

	displayList.push_back(go);
}

void BasicDraw(std::string modelFile, std::string diffuseFile, vec3 position, vec3 rotation, vec3 scale, std::string name)
{
	GameObject * go = loadFBXFromFile(ASSET_PATH + MODEL_PATH + modelFile);

	for (int i = 0; i < go->getChildCount(); i++)
	{
		Material * material = new Material();
		material->init();
		material->loadDiffuseMap(ASSET_PATH + MODEL_TEXTURE_PATH + modelSubPath + diffuseFile);
		material->loadShader(ASSET_PATH + SHADER_PATH + "textureVS.glsl", ASSET_PATH + SHADER_PATH + "textureFS.glsl");
		go->getChild(i)->setMaterial(material);
	}
	go->getTransform()->setPosition(position);
	go->getTransform()->setRotation(rotation);
	go->getTransform()->setScale(scale);
	go->setName(name);

	displayList.push_back(go);
}
#pragma endregion

#pragma region Tom - Mouse input for flying camera.
void HandleMouse(Sint32 x, Sint32 y)
{
		float sensitivity = 0.025f;

		Camera * c = flyingCamera->getCamera();
		vec3 LookAt = c->getLookAt();
		LookAt.x += (x*sensitivity);
		LookAt.y += (-y*sensitivity);
		c->setLookAt(LookAt);
}
#pragma endregion

#pragma region Euan - Keyboard input.
void HandleKeyboard(SDL_Keycode key)
{
	//Switch main camera and return out of the method.
	if (key == SDLK_m)
	{
		//Increment camera index.
		cameraIndex++;

		//If camera index exceeds 2, set to 0.
		if (cameraIndex > 1) cameraIndex = 0;

		//Assign main camera to position in cameras vector. 
		mainCamera = camerasVec[cameraIndex];

		//Remove the first element in display list (maincamera), then re-add the main camera at the front - one before the beginning, then clean up the vector.		
		displayList.erase(displayList.begin());
		displayList.insert(displayList.begin(), mainCamera);
		displayList.shrink_to_fit();

		//Debug
		std::cout << mainCamera->getName() << std::endl;

		//Return out.  No further processing on this key press.
		return;
	}

	//Switch shaders
	if (key == SDLK_TAB)
	{
		//Increment shader index
		nextShader();

		//Debug
		std::cout << "Debug - Current Post Processing Shader: " << getShaderName() << std::endl << "Debug - Shader Index: " << std::to_string(getShaderIndex()) << std::endl << std::endl;
		return;
	}

	//exits the program when pressing ESC, since the cursor is trapped
	if (key == SDLK_ESCAPE)
	{
		running = false;	
	}

	//Camera movement.
	switch (cameraIndex)
	{
		case ORBIT_CAMERA:
		{
			//Orbit camera controls: A-D = Pan.  W-S = Pitch.  Z-C = Zoom
			switch (key)
			{
				case SDLK_a:
				{
					mainCamera->getTransform()->rotateAroundPoint(-cameraSpeed * 2, Y_AXIS);
					break;
				}

				case SDLK_d:
				{
					mainCamera->getTransform()->rotateAroundPoint(cameraSpeed * 2, Y_AXIS);
					break;
				}

				case SDLK_w:
				{
					mainCamera->getTransform()->rotateAroundPoint(-cameraSpeed * 2, X_AXIS);
					break;
				}

				case SDLK_s:
				{
					mainCamera->getTransform()->rotateAroundPoint(cameraSpeed * 2, X_AXIS);
					break;
				}

				case SDLK_z:
				{
					if (glm::distance(mainCamera->getTransform()->getPosition(), mainCamera->getCamera()->getLookAt()) > 4.0f)
					{
						mainCamera->getTransform()->zoom(-cameraSpeed, mainCamera->getCamera()->getLookAt());
						break;
					}					
					break;
				}

				case SDLK_c:
				{
					if (glm::distance(mainCamera->getTransform()->getPosition(), mainCamera->getCamera()->getLookAt()) < 100.0f)
					{
						mainCamera->getTransform()->zoom(cameraSpeed, mainCamera->getCamera()->getLookAt());
						break;
					}
					break;
				}

				default:
					break;
			}
			return;
		}

		case FLYING_CAMERA:
		{
			//Flying Camera controls: WASD for movement, mouse to aim.
#pragma region Tom - Flying camera implementation.

			float Mx = 0.0f;
			float My = 0.0f;
			float Mz = 0.0f;

			Camera * c = mainCamera->getCamera();
			vec3 LookAt = c->getLookAt();

			switch (key)
			{
				case SDLK_a:
				{
					Mx = (-1.0f);
					My = 0.0f;
					Mz = 0.0f;

					vec3 oldMinCamPos = mainCamera->getTransform()->getPosition();
					mainCamera->getTransform()->setPosition(oldMinCamPos.x + Mx, oldMinCamPos.y + My, oldMinCamPos.z);

					c->setLookAt(LookAt.x + Mx, LookAt.y, LookAt.z);

					std::cout << "old pos: " << oldMinCamPos.x << " , " << oldMinCamPos.y << " , " << oldMinCamPos.z << std::endl;
					std::cout << "new pos: " << oldMinCamPos.x + Mx << " , " << oldMinCamPos.y + My << " , " << oldMinCamPos.z << std::endl << std::endl;

					break;
				}

				case SDLK_d:
				{


					Mx = 1.0f;
					My = 0.0f;
					Mz = 0.0f;

					vec3 oldMinCamPos = mainCamera->getTransform()->getPosition();
					mainCamera->getTransform()->setPosition(oldMinCamPos.x + Mx, oldMinCamPos.y + My, oldMinCamPos.z);

					c->setLookAt(LookAt.x + Mx, LookAt.y, LookAt.z);

					std::cout << "old pos: " << oldMinCamPos.x << " , " << oldMinCamPos.y << " , " << oldMinCamPos.z << std::endl;
					std::cout << "new pos: " << oldMinCamPos.x + Mx << " , " << oldMinCamPos.y + My << " , " << oldMinCamPos.z << std::endl << std::endl;

					break;
				}

				case SDLK_s:
				{
					//vec3 DirectionToFocus = glm::normalize(mainCamera->getTransform()->getPosition() - LookAt);

					mainCamera->getTransform()->zoom(1.0f, LookAt);
				//	LookAt.z = mainCamera->getTransform()->getPosition().z+28.0f;
					
					c->setLookAt(LookAt.x, LookAt.y, mainCamera->getTransform()->getPosition().z - 1.0f);

					break;
				}

				case SDLK_w:
				{
					mainCamera->getTransform()->zoom(-1.0f, LookAt);

					c->setLookAt(LookAt.x, LookAt.y, mainCamera->getTransform()->getPosition().z - 1.0f);

					break;
				}


				default:
					break;

			}

			vec3 oldMinCamPos = mainCamera->getTransform()->getPosition();
			mainCamera->getTransform()->setPosition(oldMinCamPos.x + Mx, oldMinCamPos.y + My, oldMinCamPos.z);

			// --DEBUG--
			std::cout << "old pos: " << oldMinCamPos.x << " , " << oldMinCamPos.y << " , " << oldMinCamPos.z << std::endl;
			std::cout << "new pos: " << oldMinCamPos.x + Mx << " , " << oldMinCamPos.y + My << " , " << oldMinCamPos.z << std::endl << std::endl;

			return;
#pragma endregion

		}

		default:
			break;
	}
}
#pragma endregion


//Main Method
int main(int argc, char * arg[])
{    
    // init everyting - SDL, if it is nonzero we have a problem
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "ERROR SDL_Init " <<SDL_GetError()<< std::endl;        
        return -1;
    }
    
	int imageInitFlags = IMG_INIT_JPG | IMG_INIT_PNG;
	int returnInitFlags = IMG_Init(imageInitFlags);
	if (((returnInitFlags) & (imageInitFlags)) != imageInitFlags) 
	{
		//Handle error
		std::cout << "ERROR SDL_Image Init " << IMG_GetError() << std::endl;
	}

	if (TTF_Init() == -1) 
	{
		std::cout << "TTF_Init: " << TTF_GetError();
	}
    
	//Initialise window
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, false);

    //Call our InitOpenGL Function
    initOpenGL();
    CheckForErrors();

    //Set our viewport
	setViewport(WINDOW_WIDTH, WINDOW_HEIGHT);

	//Initialise the scene
    Initialise();
    
    //Value to hold the event generated by SDL
    SDL_Event event;

	//clear console
	//system("cls");

    //Game Loop
	while (running)
    {
        //While we still have events in the queue
        while (SDL_PollEvent(&event)) 
		{
            //Get event type
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) 
			{
                //set our boolean which controls the loop to false
                running = false;
            }
			
			if (event.type == SDL_KEYDOWN)
			{
				//Handle keyboard inputs
				HandleKeyboard(event.key.keysym.sym);
			}

			if (event.type == SDL_MOUSEMOTION)
			{
				//Handle mouse input
				HandleMouse(event.motion.xrel, event.motion.yrel);
			}
        }

		//Update and render all game objects
		update();
		render();   
    } 

	//Clean up and exit.
	CleanUp();    
    return 0;
}