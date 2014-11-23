//Headers
#include <iostream>
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

//Using statements
using glm::mat4;
using glm::vec4;
using glm::vec3;

//Paths
const std::string ASSET_PATH = "assets/";
const std::string SHADER_PATH = "shaders/";
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
GameObject * orbitCamera;
GameObject * debugCamera;
GameObject * mainCamera;  //This is switched out with the orbit or debug camera, and is used in any calculations etc.
GameObject * mainLight;
PostProcessing postProcessor;

//Main camera controls
bool isDebugCam = false; //When true, the camera can be controlled freely by the player.  When false, the camera orbits the centre of the scene.  Keyboard M

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
	window = SDL_CreateWindow(
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
	//Set shader paths
	std::string vsPath = ASSET_PATH + SHADER_PATH + "/passThroughVS.glsl";
	std::string fsPath = ASSET_PATH + SHADER_PATH + "/SimplePostProcessFS.glsl";

	//Initialise post-processor
	postProcessor.init(WINDOW_WIDTH, WINDOW_HEIGHT, vsPath, fsPath);

#pragma region Orbit Camera

	//Set up orbit camera gameobject
	orbitCamera = new GameObject();
	orbitCamera->setName("OrbitCamera");
    
	//Set up main camera transform
	Transform *t = new Transform();
    t->setPosition(0, 0, 10);
	orbitCamera->setTransform(t);
    
	//Set up main camera camera
	Camera * c = new Camera();
	c->setAspectRatio((float)(WINDOW_WIDTH / WINDOW_HEIGHT));
	c->setLookAt(0.0f, 0.0f, 0.0f);
	orbitCamera->setCamera(c);

	//Add main camera to game object list.
    displayList.push_back(orbitCamera);

#pragma endregion

#pragma region Debug camera

	//Set up orbit camera gameobject
	debugCamera = new GameObject();
	debugCamera->setName("DebugCamera");

	//Set up main camera transform
	t = new Transform();
	t->setPosition(0, 0, 10);
	debugCamera->setTransform(t);

	//Set up main camera camera
	c = new Camera();
	c->setAspectRatio((float)(WINDOW_WIDTH / WINDOW_HEIGHT));
	debugCamera->setCamera(c);

	//Add main camera to game object list.
	displayList.push_back(debugCamera);

#pragma endregion

#pragma region Main Light

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

#pragma endregion

	//Set main camera
	mainCamera = orbitCamera;

    //Initialise all  game objects
    for(auto iter=displayList.begin();iter!=displayList.end();iter++)
    {
        (*iter)->init();
    }
    
	//Load and place model with bump mapping.
	std::string modelPath = ASSET_PATH + MODEL_PATH + "armoredrecon.fbx";
	GameObject * go = loadFBXFromFile(modelPath);
	for (int i = 0; i < go->getChildCount(); i++)
	{
		Material * material = new Material();
		material->init();
		std::string vsPath = ASSET_PATH + SHADER_PATH + "/BumpmappingVS.glsl";
		std::string fsPath = ASSET_PATH + SHADER_PATH + "/BumpmappingFS.glsl";
		material->loadShader(vsPath, fsPath);

		std::string diffTexturePath = ASSET_PATH + TEXTURE_PATH + "/armoredrecon_diff.png";
		material->loadDiffuseMap(diffTexturePath);

		std::string specTexturePath = ASSET_PATH + TEXTURE_PATH + "/armoredrecon_spec.png";
		material->loadSpecularMap(specTexturePath);

		std::string bumpTexturePath = ASSET_PATH + TEXTURE_PATH + "/armoredrecon_N.png";
		material->loadBumpMap(bumpTexturePath);

		go->getChild(i)->setMaterial(material);
	}
	go->getTransform()->setPosition(2.0f, 0.0f, 0.0f);
	go->getTransform()->setRotation(0.0f, -40.0f, 0.0f);
	displayList.push_back(go);

	//Load and place model with parallex mapping.
	go = loadFBXFromFile(modelPath);
	for (int i = 0; i < go->getChildCount(); i++)
	{
		Material * material = new Material();
		material->init();
		std::string vsPath = ASSET_PATH + SHADER_PATH + "/ParallaxMappingVS.glsl";
		std::string fsPath = ASSET_PATH + SHADER_PATH + "/ParallaxMappingFS.glsl";
		material->loadShader(vsPath, fsPath);

		std::string diffTexturePath = ASSET_PATH + TEXTURE_PATH + "/armoredrecon_diff.png";
		material->loadDiffuseMap(diffTexturePath);

		std::string specTexturePath = ASSET_PATH + TEXTURE_PATH + "/armoredrecon_spec.png";
		material->loadSpecularMap(specTexturePath);

		std::string bumpTexturePath = ASSET_PATH + TEXTURE_PATH + "/armoredrecon_N.png";
		material->loadBumpMap(bumpTexturePath);

		std::string heightTexturePath = ASSET_PATH + TEXTURE_PATH + "/armoredrecon_Height.png";
		material->loadHeightMap(heightTexturePath);

		go->getChild(i)->setMaterial(material);
	}
	go->getTransform()->setPosition(-2.0f, 0.0f, 0.0f);
	go->getTransform()->setRotation(0.0f, -40.0f, 0.0f);
	displayList.push_back(go);
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
    
    //alternative sytanx
	for (auto iter = displayList.begin(); iter != displayList.end(); iter++)
	{
		renderGameObject((*iter));
	}

	//now switch to normal framebuffer
	postProcessor.draw();
    SDL_GL_SwapWindow(window);
}

void HandleInput(SDL_Keycode key)
{
	float cameraSpeed = 1.0f;
	vec3 origin = vec3(0, 0, 0);
	
	//Toggle debug cam and return out of the method.
	if (key == SDLK_m)
	{
		//Switch between cameras.
		if (mainCamera->getName() == "OrbitCamera")
		{
			mainCamera = debugCamera;
			isDebugCam = true;
		}
		else
		{
			mainCamera = orbitCamera;
			isDebugCam = false;
		}


		std::cout << "Main camera: " << mainCamera->getName() << std::endl;

		//Return out.  No further processing on this key press.
		return;
	}

	//Controls for debug cam: WSAD directional movement, Q-E rotate.
	if (isDebugCam)
	{
		switch (key)
		{
			case SDLK_w:
			{
				mainCamera->getTransform()->translate(vec3(0, 0, -cameraSpeed));
				break;
			}

			case SDLK_a:
			{
				mainCamera->getTransform()->translate(vec3(-cameraSpeed, 0, 0));
				break;
			}

			case SDLK_s:
			{
				mainCamera->getTransform()->translate(vec3(0, 0, cameraSpeed));
				break;
			}

			case SDLK_d:
			{
				mainCamera->getTransform()->translate(vec3(cameraSpeed, 0, 0));
				break;
			}

			case SDLK_z:
			{
				break;
			}

			case SDLK_c:
			{
				break;
			}
		}
	}
	//Controls for orbit camera:  A-D Pan left and right.  W-S Pan up and down.  Q-E Zoom in and out.
	else
	{
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
				if (mainCamera->getTransform()->getPosition().y < 7.0f)
				{
					mainCamera->getTransform()->rotateAroundPoint(-cameraSpeed, X_AXIS, origin);
					break;
				}
			}

			case SDLK_s:
			{
				if (mainCamera->getTransform()->getPosition().y > -7.0f)
				{
					mainCamera->getTransform()->rotateAroundPoint(cameraSpeed, X_AXIS, origin);
					break;
				}
			}

			case SDLK_z:
			{
				mainCamera->getTransform()->zoom(50);
				break;
			}

			case SDLK_c:
			{
				mainCamera->getTransform()->zoom(-cameraSpeed);
				break;
			}
		}
	}
}

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
        }

		//Update and render all game objects
		update();
		render();   
    } 

	//Clean up and exit.
	CleanUp();    
    return 0;
}


//std::cout << "Neat debugging statement!" << std::to_string(valueToString) << std::endl;