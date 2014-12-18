//TO DO
//FIX SHADER ERROR.  NOT SURE WHERE TO START.

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
#include "CameraTypeEnum.h"

//Using statements
using glm::mat4;
using glm::vec4;
using glm::vec3;

//Paths
const std::string ASSET_PATH = "assets/";
const std::string DRAWING_SHADER_PATH = "shaders/drawing/";
const std::string POSTP_SHADER_PATH = "shaders/postprocessing/";
const std::string TEXTURE_PATH = "textures/";
const std::string FONT_PATH = "fonts/";
const std::string MODEL_PATH = "models/";

//Constant vectors
const vec3 X_AXIS = vec3(1, 0, 0);
const vec3 Y_AXIS = vec3(0, 1, 0);
const vec3 Z_AXIS = vec3(0, 0, 1);


//SDL Window
SDL_Window * window = NULL;

//SDL GL Context
SDL_GLContext glcontext = NULL;

//Window Width
const int WINDOW_WIDTH = 640;

//Window Height
const int WINDOW_HEIGHT = 480;

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

//Main scene game objects
std::vector<GameObject*> displayList;
std::vector<GameObject*> camerasVec;  //Vector for the camera types.  
GameObject * orbitCamera;
GameObject * flyingCamera;
GameObject * FPCamera;
GameObject * mainCamera;  //This is switched out with the orbit or debug camera, and is used in any calculations etc.
GameObject * mainLight;
PostProcessing postProcessor;

//Post Processing array.   {Path, Name}
std::string PostProcessingArray[6][2]
{
	{ "NonePPFS.glsl" , "NONE"},
	{ "BlurFilterPPFS.glsl" , "BLUR"},
	{ "BWPPFS.glsl" , "BLACK AND WHITE" },
	{ "SepiaPPFS.glsl", "SEPIA" },
	{ "PolaroidPPFS.glsl", "POLAROID" },
	{ "InvertedPPFS.glsl" , "INVERTED" }
};

//Post processing index
int PPindex = 0;


//Input globals
float cameraSpeed = 1.0f;
vec3 origin = vec3(0.0f, 0.0f, 0.0f);

//Texture
GLuint fontTexture = 0;

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
		"Coursework",             // window title
		SDL_WINDOWPOS_CENTERED,     // x position, centered
		SDL_WINDOWPOS_CENTERED,     // y position, centered
		width,                        // width, in pixels
		height,                        // height, in pixels
										// flags
		SDL_WINDOW_OPENGL	

	);


}

//font texture
void createFontTexture()
{
	std::string fontPath = ASSET_PATH + FONT_PATH + "OratorStd.otf";
	fontTexture = loadTextureFromFont(fontPath, 64, "Hello");
}

void CleanUp()
{
	// clean up in reverse

	//Clean up game objects
    auto iter=displayList.begin();
	while(iter!=displayList.end())
    {
        (*iter)->destroy();
        if ((*iter))
        {
            delete (*iter);
            (*iter)=NULL;
            iter=displayList.erase(iter);
        }
        else
        {
            iter++;
        }
    }
    displayList.clear();
    
	//Clean up post processor
	postProcessor.destroy();

	//Clean up cameras
	iter = camerasVec.begin();
	while (iter != camerasVec.end())
	{
		(*iter)->destroy();
		if ((*iter))
		{
			delete (*iter);
			(*iter) = NULL;
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


void Initialise()
{
	int temp;
	//trap the cursor inside the window - not really needed
	//SDL_SetWindowGrab(window, SDL_TRUE);
	//grab mouse
	temp = SDL_SetRelativeMouseMode(SDL_TRUE);



	//Forward declare modeldrawcall
	void ModelDrawCall(std::string modelFile, std::string vertexShaderFile, std::string fragmentShaderFile,
		std::string diffuseFile, std::string specularFile, std::string normalFile, std::string heightFile, vec3 position, vec3 rotation);

	//Set shader paths
	std::string vsPath = ASSET_PATH + POSTP_SHADER_PATH + "passThroughVS.glsl";
	std::string fsPath = ASSET_PATH + POSTP_SHADER_PATH + PostProcessingArray[PPindex][0];

	//Initialise post-processor
	postProcessor.init(WINDOW_WIDTH, WINDOW_HEIGHT, vsPath, fsPath);

#pragma region Euan

#pragma region Orbit Camera

	//Set up orbitcamera gameobject - this will be the initial camera
	orbitCamera = new GameObject();
	orbitCamera->setName("Orbit Camera");
    
	//Set up orbitcamera transform
	Transform *t = new Transform();
    t->setPosition(0.0f, 2.0f, 10.0f);
	orbitCamera->setTransform(t);
    
	//Set up orbitcamera camera
	Camera * c = new Camera();
	c->setAspectRatio((float)(WINDOW_WIDTH / WINDOW_HEIGHT));
	c->setLookAt(0.0f, 0.0f, 0.0f);
	orbitCamera->setCamera(c);

	//Push to cameras vector
	camerasVec.push_back(orbitCamera);

#pragma endregion

#pragma region Tom
#pragma region Debug camera - TODO: NOT IMPLEMENTED.  IDENTICAL TO ORBIT FOR THE TIME BEING.

	//Set up debugcamera gameobject
	flyingCamera = new GameObject();
	flyingCamera->setName("Flying Camera");

	//Set up debugcamera transform
	t = new Transform();
	t->setPosition(0.0f, 2.0f, 10.0f);
	flyingCamera->setTransform(t);

	//Set up debugcamera camera
	c = new Camera();
	c->setAspectRatio((float)(WINDOW_WIDTH / WINDOW_HEIGHT));
	c->setLookAt(0.0f, 0.0f, 0.0f);
	flyingCamera->setCamera(c);

	//Push to cameras vector
	camerasVec.push_back(flyingCamera);

#pragma endregion
#pragma endregion

#pragma region Calum
#pragma region First Person camera - TODO: NOT IMPLEMENTED.  IDENTICAL TO ORBIT FOR THE TIME BEING.

	//Equate to orbit for the time being.
	FPCamera = new GameObject();
	FPCamera->setName("First Person Camera");

	//Set up debugcamera transform
	t = new Transform();
	t->setPosition(0.0f, 5.0f, 10.0f);
	FPCamera->setTransform(t);

	//Set up debugcamera camera
	c = new Camera();
	c->setAspectRatio((float)(WINDOW_WIDTH / WINDOW_HEIGHT));
	c->setLookAt(0.0f, 0.0f, 0.0f);
	FPCamera->setCamera(c);

	//Push to cameras vector
	camerasVec.push_back(FPCamera);

#pragma endregion
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
	t->setPosition(0.0f, 0.0f, 0.0f);
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



#pragma region Calum
	/*TODO: Calum
	Place models and stuff
	*/
#pragma endregion

	ModelDrawCall("armoredrecon.fbx", "BumpmappingVS.glsl", "BumpmappingFS.glsl",
		"armoredrecon_diff.png", "armoredrecon_spec.png", "armoredrecon_N.png", "", vec3(2.5f, 0.0f, 0.0f), vec3(0.0f, -40.0f, 0.0f));

	ModelDrawCall("armoredrecon.fbx", "ParallaxMappingVS.glsl", "ParallaxMappingFS.glsl", 
		"armoredrecon_diff.png", "armoredrecon_spec.png", "armoredrecon_N.png", "armoredrecon_Height.png", vec3(-2.5f, 0.0f, 0.0f), vec3(0.0f, 40.0f, 0.0f));
}


#pragma region Tom - messing around with the lights (daylightsystem)

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
	Material * currentMaterial = pObject->getMaterial();

	if (currentMesh && currentMaterial && currentTransform)
	{
		currentMaterial->bind();
		currentMesh->bind();

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
	}

	for (int i = 0; i < pObject->getChildCount(); i++)
	{
		//SHADER TABBING SEEMS TO BREAK HERE - 4th iteration.
		renderGameObject(pObject->getChild(i));
	}
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
    
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //alternative sytanx
	for (auto iter = displayList.begin(); iter != displayList.end(); iter++)
	{
		renderGameObject((*iter));
	}

	createFontTexture();

	//now switch to normal framebuffer
	postProcessor.draw();
    SDL_GL_SwapWindow(window);
}

#pragma region Euan

void ModelDrawCall(std::string modelFile, std::string vertexShaderFile, std::string fragmentShaderFile, 
	std::string diffuseFile, std::string specularFile, std::string normalFile, std::string heightFile, vec3 position, vec3 rotation)
{
	GameObject * go = loadFBXFromFile(ASSET_PATH + MODEL_PATH + modelFile);
	for (int i = 0; i < go->getChildCount(); i++)
	{
		Material * material = new Material();
		material->init();
		material->loadShader(ASSET_PATH + DRAWING_SHADER_PATH + vertexShaderFile, ASSET_PATH + DRAWING_SHADER_PATH + fragmentShaderFile);
		material->loadDiffuseMap(ASSET_PATH + TEXTURE_PATH + diffuseFile);
		material->loadSpecularMap(ASSET_PATH + TEXTURE_PATH + specularFile);
		material->loadBumpMap(ASSET_PATH + TEXTURE_PATH + normalFile);
		if (heightFile != "") material->loadHeightMap(ASSET_PATH + TEXTURE_PATH + heightFile);  //BUMP MAPPING DOESN'T USE A HEIGHT MAP.  IF THIS IS NULL, SKIP OVER.
		go->getChild(i)->setMaterial(material);
	}
	go->getTransform()->setPosition(position.x, position.y, position.z);
	go->getTransform()->setRotation(rotation.x, rotation.y, rotation.z);
	displayList.push_back(go);
}

#pragma region Tom
void HandleMouse(Sint32 x, Sint32 y)
{
		float sensitivity = 0.025f;

		Camera * c = flyingCamera->getCamera();
		vec3 oldLookAt = c->getLookAt();
		vec3 newLookAt(0.0f, 0.0f, 0.0f);
		newLookAt.x = oldLookAt.x + (x*sensitivity);
		newLookAt.y = oldLookAt.y + (y*sensitivity);
		c->setLookAt(oldLookAt.x + (x*sensitivity), oldLookAt.y + (y*sensitivity), oldLookAt.z);


}
#pragma endregion

void HandleInput(SDL_Keycode key)
{
	//Switch main camera and return out of the method.
	if (key == SDLK_m)
	{
		//Increment camera index.
		cameraIndex++;

		//If camera index exceeds 2, set to 0.
		if (cameraIndex > 2) cameraIndex = 0;

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

	//Switch shaders - BROKEN AT THE MOMENT DUE TO BRAIN'S AMD/INTEL FIX. 
	if (key == SDLK_TAB)
	{
		//Increment PPS index.  If the index exceeds the capacity of the array, set index to 0.
		PPindex++;
		if (PPindex >= (sizeof(PostProcessingArray) / sizeof(*PostProcessingArray)))
			PPindex = 0;

		//Change post processing shader
		postProcessor.changeFragmentShaderFilename(PostProcessingArray[PPindex][0], ASSET_PATH + POSTP_SHADER_PATH);
		std::cout << "Debug - Current Post Processing Shader: " << PostProcessingArray[PPindex][1] << std::endl << std::endl;
		return;
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
					mainCamera->getTransform()->rotateAroundPoint(-cameraSpeed, Y_AXIS, origin);
					break;
				}

				case SDLK_d:
				{
					mainCamera->getTransform()->rotateAroundPoint(cameraSpeed, Y_AXIS, origin);
					break;
				}

				case SDLK_w:
				{
					//if (mainCamera->getTransform()->getPosition().y < 7.0f)  LIMITATIONS TEMPORARILY OMITTED DUE TO BUG.
					//{
					mainCamera->getTransform()->rotateAroundPoint(-cameraSpeed, X_AXIS, origin);
					break;
					//}
					//else break;
				}

				case SDLK_s:
				{
					//if (mainCamera->getTransform()->getPosition().y > 1.0f) LIMITATIONS TEMPORARILY OMITTED DUE TO BUG.
					//{
					mainCamera->getTransform()->rotateAroundPoint(cameraSpeed, X_AXIS, origin);
					break;
					//}
					//else break;
				}

				case SDLK_z:
				{
					mainCamera->getTransform()->zoom(-cameraSpeed, origin);
					break;
				}

				case SDLK_c:
				{
					mainCamera->getTransform()->zoom(cameraSpeed, origin);
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
#pragma region Tom

			float Mx = 0.0f;
			float My = 0.0f;

			Camera * c = flyingCamera->getCamera();
			vec3 LookAt = c->getLookAt();

			switch (key)
			{
				case SDLK_a:
				{
					Mx = (-1.0f);
					My = 0.0f;
					break;
				}

				case SDLK_d:
				{
					Mx = 1.0f;
					My = 0.0f;
					
					break;
				}

				case SDLK_s:
				{
					//vec3 DirectionToFocus = glm::normalize(mainCamera->getTransform()->getPosition() - LookAt);

					mainCamera->getTransform()->forwardT(1.0f, LookAt);
				//	LookAt.z = mainCamera->getTransform()->getPosition().z+28.0f;

					

					c->setLookAt(LookAt.x, LookAt.y, mainCamera->getTransform()->getPosition().z - 1.0f);

					break;
				}

				case SDLK_w:
				{
					mainCamera->getTransform()->forwardT(-1.0f, LookAt);

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
		case FIRST_PERSON_CAMERA:
		{
#pragma region Calum
			/*
			TODO - CALUM
			first person camera when you get around to it.  WASD movement, mouse to aim (and shoot)
			*/
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
	system("cls");

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
				HandleInput(event.key.keysym.sym);
			}

			if (event.type == SDL_MOUSEMOTION)
			{
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