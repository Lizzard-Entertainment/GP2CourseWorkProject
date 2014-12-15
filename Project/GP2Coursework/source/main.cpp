//TO DO
//FIX SHADER ERROR.  NOT SURE WHERE TO START.

//Headers
#include <iostream>
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

//Scene bare light colour
vec4 ambientLightColour = vec4(1.0f, 1.0f, 1.0f, 1.0f);

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
		SDL_WINDOW_OPENGL           // flags
	);
}

void CleanUp()
{

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

	// clean up in reverse
	postProcessor.destroy();
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
	Transform *t2 = new Transform();
	t2->setPosition(0.0f, 0.0f, 10.0f);
	flyingCamera->setTransform(t2);

	//Set up debugcamera camera
	Camera * c2 = new Camera();
	c2->setAspectRatio((float)(WINDOW_WIDTH / WINDOW_HEIGHT));
	c2->setLookAt(0.0f, 0.0f, 0.0f);
	flyingCamera->setCamera(c2);

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
	t->setPosition(0.0f, 0.0f, 10.0f);
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
	//The debug and orbit camera act as definitions for the main camera, and are not worked on directly.
	mainCamera = camerasVec[FLYING_CAMERA];
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

//Function to update the game state
void update()
{
    //Update all game objects.
    for(auto iter=displayList.begin();iter!=displayList.end();iter++)
    {
        (*iter)->update();
    }
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
		//SHADER TABBING SEEMS TO BREAK HERE.
		renderGameObject(pObject->getChild(i));
	}
}

//Function to render(aka draw)
void render()
{
	//Bind Framebuffer
	postProcessor.bind();

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


//-- TOM

void HandleMouse(Sint32 x, Sint32 y)
{

	Camera * c = flyingCamera->getCamera();
	vec3 oldLookAt = c->getLookAt();

	c->setLookAt(oldLookAt.x + (x*0.01f), oldLookAt.y + (y*0.01f), 0.0f);


	/* If the mouse is moving to the left */
	if (x < 0)
	{
		std::cout << "moving left " << x << std::endl;
	}
	/* If the mouse is moving to the right */
	else if (x > 0)
		std::cout << "moving right " << std::endl;
	/* If the mouse is moving up */
	else if (y < 0)
		std::cout << "moving up " << std::endl;
	/* If the mouse is moving down */
	else if (y > 0)
		std::cout << "moving down " << std::endl;

}

//--TOM

void HandleInput(SDL_Keycode key)
{
	float cameraSpeed = 1.0f;
	vec3 origin = vec3(0.0f, 0.0f, 0.0f);

	//Switch main camera and return out of the method.
	if (key == SDLK_m)
	{
		//Increment camera index.
		cameraIndex++;

		//If camera index exceeds 2, set to 0.
		if (cameraIndex > 2) cameraIndex = 0;

		//Assign main camera to position in cameras vector.
		mainCamera = camerasVec[cameraIndex];

		std::cout << "Debug - Main Camera: " << mainCamera->getName() << std::endl << std::endl;

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
			switch (key)
			{
#pragma region Orbit camera controls: A-D = Pan.  W-S = Pitch.  Z-C = Zoom
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
#pragma endregion
			}
			return;
		}




		case FLYING_CAMERA:
		{

#pragma region Flying Camera controls: WASD for movement, mouse to aim.
#pragma region Tom

			float Mx;
			float My;

			// ------ TOM
			switch (key)
			{

				case SDLK_g:
				{
					Mx = cameraSpeed*(-1);
					My = 0;
					break;
				}

				case SDLK_j:
				{
					Mx = cameraSpeed*1;
					My = 0;
				}

				case SDLK_z:
				{
					My = cameraSpeed*(-1);
					Mx = 0;
				}

				case SDLK_h:
				{
					My = cameraSpeed*1;
					Mx = 0;
				}


				default:
					break;

			}

			vec3 oldMinCamPos = mainCamera->getTransform()->getPosition();
			mainCamera->getTransform()->setPosition(oldMinCamPos.x + Mx, oldMinCamPos.y + My, oldMinCamPos.z);

			return;


// --- TOM


			
#pragma endregion
#pragma endregion
		}
		case FIRST_PERSON_CAMERA:
		{
#pragma region First Person Camera controls: WASD movement, mouse to aim (and shoot)
#pragma region Calum
			/*
			TODO - CALUM
			first person camera when you get around to it.  WASD movement, mouse to aim (and shoot)
			*/
			return;
#pragma endregion
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

	//Update console
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