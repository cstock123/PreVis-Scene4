/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

/***********************
 SHADER MANAGER INSTRUCTIONS
 
 HOW TO ADD A SHADER:
 1) Create a #define in ShaderManager.h that will be used to identify your shader
 2) Add an init function in ShaderManager.cpp and put your initialization code there
 - be sure to add a prototype of this function in ShaderManager.h
 3) Call your init function from initShaders in ShaderManager.cpp and save it to the
 respective location in shaderMap. See example
 
 HOW TO USE A SHADER IN THE RENDER LOOP
 1) first, call shaderManager.setCurrentShader(int name) to set the current shader
 2) To retrieve the current shader, call shaderManager.getCurrentShader()
 3) Use the return value of getCurrentShader() to render
 ***********************/
 
/***********************
 SPLINE INSTRUCTIONS

 1) Create a spline object, or an array of splines (for a more complex path)
 2) Initialize the splines. I did this in initGeom in this example. There are 
	two constructors for it, for order 2 and order 3 splines. The first uses
	a beginning, intermediate control point, and ending. In the case of Bezier splines, 
	the path is influenced by, but does NOT necessarily touch, the control point. 
	There is a second constructor, for order 3 splines. These have two control points. 
	Use these to create S-curves. The constructor also takes a duration of time that the 
	path should take to be completed. This is in seconds. 
 3) Call update(frametime) with the time between the frames being rendered. 
	3a) Call isDone() and switch to the next part of the path if you are using multiple 
	    paths or something like that. 
 4) Call getPosition() to get the vec3 of where the current calculated position is. 
 ***********************/

#include <chrono>
#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Time.h"
#include "physics/PhysicsObject.h"
#include "physics/ColliderSphere.h"
#include "physics/ColliderMesh.h"
#include "Constants.h"
#include "Spider.h"
#include "PigSpider.h"
#include "ShaderManager.h"
#include "Spline.h"
#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

TimeData Time;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
    
  ShaderManager * shaderManager;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> sphere;
	shared_ptr<Shape> cube;
	shared_ptr<Shape> detectiveModel;
    shared_ptr<Shape> noirSpiderPart;
    vector<shared_ptr<Shape>> noirSpiderParts;
    shared_ptr<Shape> spider_pig;
    shared_ptr<Model> regular_pig;
    shared_ptr<Model> barn;
    shared_ptr<Model> cartoon_spider;
	shared_ptr<Shape> miles;

	vector<shared_ptr<PhysicsObject>> physicsObjects;
	Spider spider;
    PigSpider pigSpider;

	// Two part path
  Spline splinepath[2];

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;
    
    float testScale = 0;
    float rotateTheta = 0;
    float scanDown = 0;
    float spiderScale = 0.1;
    float spiderRotate = 0;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	enum SceneType { SCENE_START, SCENE_MILES, SCENE_GWEN, SCENE_NOIR_BITE, SCENE_NOIR_PORTAL, SCENE_PIG, SCENE_MINECRAFT, SCENE_ALL };
	SceneType currentScene = SCENE_START;
    
    struct {
        vec3 eye = vec3(0);
        vec3 target = vec3(0, 0, -1);
        vec3 up = vec3(0, 1, 0);
    } camera;
    
    
    bool is_spider = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
        
        if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            is_spider = true;
        }
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

        // create the Instance of ShaderManager which will initialize all shaders in its constructor
		shaderManager = new ShaderManager(resourceDirectory);
	}

	void initGeom(const std::string& resourceDirectory)
	{
		//EXAMPLE new set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information. We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/models/SmoothSphere.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = sphere->min.x;
		gMin.y = sphere->min.y;

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/models/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			cube = make_shared<Shape>();
			cube->createShape(TOshapes[0]);
			cube->measure();
			cube->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/models/detective.obj").c_str());	
		if (!rc) {
			cerr << errStr << endl;
		} else {
			detectiveModel = make_shared<Shape>();
			detectiveModel->createShape(TOshapes[0]);
			detectiveModel->measure();
			detectiveModel->init();
		}
        
        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/models/spider-noir.obj").c_str());
        if (!rc) {
            cerr << errStr << endl;
        } else {
            for (int i = 0; i < TOshapes.size(); i++) {
                noirSpiderPart = make_shared<Shape>();
                noirSpiderPart->createShape(TOshapes[0]);
                noirSpiderPart->measure();
                noirSpiderPart->init();
                
                noirSpiderParts.push_back(noirSpiderPart);
            }
            
        }
        
        
        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/models/pig.obj").c_str());
        if (!rc) {
            cerr << errStr << endl;
        } else {
            spider_pig = make_shared<Shape>();
            spider_pig->createShape(TOshapes[0]);
            spider_pig->measure();
            spider_pig->init();
        }
        
        regular_pig = make_shared<Model>((resourceDirectory + "/models/full_pig.obj").c_str());
        barn = make_shared<Model>((resourceDirectory + "/models/barn.obj").c_str());
        cartoon_spider = make_shared<Model>((resourceDirectory + "/models/cartoonSpider.obj").c_str());
        
		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/models/miles-spider.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			miles = make_shared<Shape>();
			miles->createShape(TOshapes[0]);
			miles->measure();
			miles->init();
		}
	}

	/**
	 * Initialize objects with physics interactions here.
	 * There are two types of colliders: spheres and meshes.
	 * Note that spheres can collide with meshes and other spheres, but meshes can't collide with other meshes.
	 */
	void initPhysicsObjects() {
		PhysicsObject::setCulling(false);

		auto physicsBall = make_shared<PhysicsObject>(vec3(0, 0, -10), sphere, make_shared<ColliderSphere>(sphere->size.x / 2));
		physicsBall->setMass(5);
		physicsBall->setElasticity(0.5);
		physicsBall->setFriction(0.25);
		physicsObjects.push_back(physicsBall);

		physicsBall = make_shared<PhysicsObject>(vec3(-1, -3, -10), sphere, make_shared<ColliderSphere>(sphere->size.x / 2));
		physicsBall->setElasticity(0.5);
		physicsBall->setFriction(0.25);
		physicsObjects.push_back(physicsBall);

		cube->findEdges(); // need to call this for shapes used as collision meshes
		auto physicsCube = make_shared<PhysicsObject>(vec3(2, -4, -10), cube, make_shared<ColliderMesh>(cube));
		physicsCube->setElasticity(0.5);
		physicsCube->setFriction(0.25);
		physicsCube->orientation = rotate(quat(1, 0, 0, 0), 45.0f, vec3(0, 1, 0));
		physicsObjects.push_back(physicsCube);
    
        // Give spider sphere to draw
		spider.initialize(sphere);
        pigSpider.initialize(sphere, spider_pig);
    
		// init splines
		splinepath[0] = Spline(glm::vec3(-6,0,-5), glm::vec3(-1,-5,-5), glm::vec3(1, 5, -5), glm::vec3(2,0,-5), 5);
		splinepath[1] = Spline(glm::vec3(2,0,-5), glm::vec3(3,-5,-5), glm::vec3(-0.25, 0.25, -5), glm::vec3(0,0,-5), 5);
	
	}
    
    mat4 SetProjectionMatrix(shared_ptr<Program> curShader) {
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        float aspect = width/(float)height;
        mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 100.0f);
        glUniformMatrix4fv(curShader->getUniform("P"), 1, GL_FALSE, value_ptr(Projection));
        return Projection;
    }
    
    /*void SetViewMatrix(shared_ptr<Program> curShader) {
        auto View = make_shared<MatrixStack>();
        View->pushMatrix();
        glUniformMatrix4fv(curShader->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        View->popMatrix();
    }*/
    
    void SetViewMatrix(shared_ptr<Program> curShader) {
        auto View = make_shared<MatrixStack>();
        View->pushMatrix();
        View->lookAt(camera.eye, camera.target, camera.up);
        glUniformMatrix4fv(curShader->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
        View->popMatrix();
    }

	void render(float frametime)
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderManager->setCurrentShader(SIMPLEPROG);
		switch (currentScene) {
			case SCENE_MILES:
				renderMilesScene(frametime);
				break;
			case SCENE_GWEN:
				renderGwenScene(frametime);
				break;
			case SCENE_NOIR_BITE:
				renderNoirBiteScene(frametime);
				break;
			case SCENE_NOIR_PORTAL:
				renderNoirPortalScene(frametime);
				break;
			case SCENE_PIG:
				renderPigScene(frametime);
				break;
			case SCENE_MINECRAFT:
				renderMinecraftScene(frametime);
				break;
			case SCENE_ALL:
				renderAllScene(frametime);
				break;
			default:
        		renderSimpleProg(frametime);
				break;
		}
	}

	void nextScene() {
		switch (currentScene) {
			case SCENE_START:
				currentScene = SCENE_MILES;
				setupMilesScene();
				break;
			case SCENE_MILES:
				currentScene = SCENE_GWEN;
				setupGwenScene();
				break;
			case SCENE_GWEN:
				currentScene = SCENE_NOIR_BITE;
				setupNoirBiteScene();
				break;
			case SCENE_NOIR_BITE:
				currentScene = SCENE_NOIR_PORTAL;
				setupNoirPortalScene();
				break;
			case SCENE_NOIR_PORTAL:
				currentScene = SCENE_PIG;
				setupPigScene();
				break;
			case SCENE_PIG:
				currentScene = SCENE_MINECRAFT;
				setupMinecraftScene();
				break;
			case SCENE_MINECRAFT:
				currentScene = SCENE_ALL;
				setupAllScene();
				break;
			case SCENE_ALL:
				// last scene?
				break;
		}
	}
    
    void renderSimpleProg(float frametime) {
        shared_ptr<Program> simple = shaderManager->getCurrentShader();

        auto Model = make_shared<MatrixStack>();

        simple->bind();
            // Apply perspective projection.
            SetProjectionMatrix(simple);
            SetViewMatrix(simple);

			// Demo of Bezier Spline
			glm::vec3 position;

			if(!splinepath[0].isDone())
			{
				splinepath[0].update(frametime);
				position = splinepath[0].getPosition();
			} else {
				splinepath[1].update(frametime);
				position = splinepath[1].getPosition();
			}

            // draw mesh
            Model->pushMatrix();
            Model->loadIdentity();
            //"global" translate
            Model->translate(position);
                Model->pushMatrix();
                Model->scale(vec3(0.5, 0.5, 0.5));
                glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                sphere->draw(simple);
                Model->popMatrix();
            Model->popMatrix();
            // spider
            Model->pushMatrix();
                Model->loadIdentity();
                Model->translate(vec3(0, 0, -1));
                Model->scale(2);
                Model->rotate(M_PI, YAXIS);
                spider.draw(simple, Model);
            Model->popMatrix();

			for (auto obj : physicsObjects) {
				obj->draw(simple, Model);
			}
        simple->unbind();
    }

	void updatePhysics(float dt) {
		for (int i = 0; i < physicsObjects.size(); i++) {
			for (int j = i + 1; j < physicsObjects.size(); j++) {
				physicsObjects[i]->checkCollision(physicsObjects[j].get());
			}
		}
		for (auto obj : physicsObjects) {
			obj->update();
		}
	}

	float milesRotation;
	vec3 milesPortalScale;
	vec3 milesPortalPosition;
	float tMiles;
	Spline milesCameraPath;
	void setupMilesScene() {
		milesRotation = M_PI / 4;
		milesPortalScale = vec3(0, 0, 0.1);
		tMiles = 0;
		milesPortalPosition = vec3(1, -1, -8);
		milesCameraPath = Spline(vec3(0), vec3(0, 0, -4), milesPortalPosition, 1);
	}

	void renderMilesScene(float frametime) {
        shared_ptr<Program> simple = shaderManager->getCurrentShader();
        auto Model = make_shared<MatrixStack>();

		tMiles += frametime;
		if (tMiles < 1) {

		}
		else if (tMiles < 2) {
			milesPortalScale.x = tMiles - 1;
			milesPortalScale.y = milesPortalScale.x;
		}
		else if (tMiles < 2.5) {

		}
		else if (tMiles < 3.5) {
			milesRotation += M_PI / 2 * frametime;
		}
		else if (tMiles < 4) {

		}
		else if (tMiles < 5) {
			if (!milesCameraPath.isDone()) {
				milesCameraPath.update(frametime);
				camera.eye = milesCameraPath.getPosition();
				camera.target = camera.eye + vec3(0, 0, -1);
			}
		}
		else {
			nextScene();
		}

		simple->bind();
            // Apply perspective projection.
            SetProjectionMatrix(simple);
            SetViewMatrix(simple);
			Model->pushMatrix();
				Model->translate(vec3(-2, -4, -6));
				Model->rotate(milesRotation, vec3(0, 1, 0));
                glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				miles->draw(simple);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(milesPortalPosition);
				Model->scale(milesPortalScale);
                glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				sphere->draw(simple);
			Model->popMatrix();
		simple->unbind();
	}

	void setupGwenScene() {

	}

	void renderGwenScene(float frametime) {
        nextScene();
	}

	Spline noirCameraPath[2];
	Spline noirSpiderPath[2];
	float detectiveRotation;
	float tNoir;
	vec3 cameraOffsetFromSpider;
	vec3 noirPortalLocation;
	float noirPortalScale;
	void setupNoirBiteScene() {
		tNoir = 0;
		vec3 handLocation = vec3(2, 0.3, -7);
		noirPortalLocation = handLocation + vec3(2, -2, -7);
		cameraOffsetFromSpider = vec3(0, 0, 2);
		noirCameraPath[0] = Spline(vec3(0), vec3(3, 0, -2), handLocation + cameraOffsetFromSpider, 2);
		noirSpiderPath[0] = Spline(vec3(4, 0, -7), vec3(3, 2, -7), handLocation, 1);
		noirSpiderPath[1] = Spline(vec3(2, 0.3, -7), vec3(2.5, 1, -7), noirPortalLocation, 2);
		spider.location = noirSpiderPath[0].getPosition();
		noirPortalScale = 0;
		detectiveRotation = 0;
		camera.eye = vec3(0);
		camera.target = vec3(0, 0, -1);
	}

	void renderNoirBiteScene(float frametime) {
		shaderManager->setCurrentShader(GREYPROG);
		shared_ptr<Program> simple = shaderManager->getCurrentShader();
        auto Model = make_shared<MatrixStack>();

		tNoir += frametime;

		if (tNoir < 1) {

		}
		else if (tNoir < 3) {
			if (!noirCameraPath[0].isDone()) {
				noirCameraPath[0].update(frametime);
				camera.eye = noirCameraPath[0].getPosition();
				camera.target = camera.eye + vec3(0, 0, -1);
			}
		}
		else if (tNoir < 4) {

		}
		else if (tNoir < 5) {
			if (!noirSpiderPath[0].isDone()) {
				noirSpiderPath[0].update(frametime);
				spider.location = noirSpiderPath[0].getPosition();
			}
		}
		else if (tNoir < 5.5) {
			noirPortalScale = (tNoir - 5) * 2;
		}
		else if (tNoir < 7.5) {
			detectiveRotation += frametime * M_PI * 8;
			if (!noirSpiderPath[1].isDone()) {
				noirSpiderPath[1].update(frametime);
				spider.location = noirSpiderPath[1].getPosition();
			}
		}
		else {
			nextScene();
		}

		simple->bind();
			SetProjectionMatrix(simple);
			SetViewMatrix(simple);
			Model->pushMatrix();
				Model->translate(vec3(0, -3, -7));
				Model->rotate(detectiveRotation, vec3(0, 1, 0));
            	glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				detectiveModel->draw(simple);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(spider.location);
				Model->rotate(M_PI_2, vec3(0, 1, 0));
				Model->translate(-spider.location);
				spider.draw(simple, Model);
			Model->popMatrix();
			Model->pushMatrix();
				Model->translate(noirPortalLocation);
				Model->scale(noirPortalScale);
            	glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				sphere->draw(simple);
			Model->popMatrix();
		simple->unbind();
	}

	void setupNoirPortalScene() {
		camera.eye = vec3(0);
		camera.target = vec3(0, 0, -1);
	}

	void renderNoirPortalScene(float frametime) {
        shaderManager->setCurrentShader(GREYPROG);
        shared_ptr<Program> grey = shaderManager->getCurrentShader();
        
        auto Model = make_shared<MatrixStack>();
        
        bool insertSpider = false;
        bool growSpider = false;
        bool rotateSpider = false;
        
        grey->bind();
            // Apply perspective projection.
            SetProjectionMatrix(grey);
            SetViewMatrix(grey);
        
            // draw mesh
            Model->pushMatrix();
                Model->loadIdentity();
                //"global" translate
        
                Model->translate(vec3(0, 1, -4));
                rotateTheta += frametime;
                Model->rotate(rotateTheta, vec3(0, 1, 0));
                    if (testScale < 2) {
                        testScale += frametime;
                    } else {
                        insertSpider = true;
                    }
                    Model->scale(vec3(testScale, 0.1, testScale));
                    glUniformMatrix4fv(grey->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                    sphere->draw(grey);
            Model->popMatrix();
        
            if (insertSpider) {
            }
        
            if (insertSpider) {
                Model->pushMatrix();
                    Model->loadIdentity();
                    if (scanDown > -2.5) scanDown -= (frametime * 2.5);
                    else growSpider = true;
                
                    Model->translate(vec3(0, 1 + scanDown, -4));
                
                    if (growSpider) {
                        if (spiderScale < 0.3) spiderScale += frametime;
                        else rotateSpider = true;
                    }
                
                    if (rotateSpider) {
                        if (spiderRotate < 3.14159265 * 2) spiderRotate += (5 * frametime);
						else nextScene();
                        Model->rotate(spiderRotate, vec3(0, 1, 0));
                    }
                
                    Model->scale(spiderScale);
                    glUniformMatrix4fv(grey->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                    for(int i = 0; i < noirSpiderParts.size(); i++) {
                        noirSpiderParts[i]->draw(grey);
                    }
                Model->popMatrix();
            }
        grey->unbind();
	}

    Spline pigCameraPath[2];
    Spline pigSpiderPath;
    float tPig;
    float pSpiderRotate;
    glm::vec3 pSpiderPosition;
	
    void setupPigScene() {
        tPig = 0;
        pSpiderRotate = 0;
        pSpiderPosition = vec3(0, 0, -4);
        pigCameraPath[0] = Spline(vec3(0), vec3(0, -.6, -5.8), vec3(0, 0, 0), 3);
        pigCameraPath[1] = Spline(vec3(0), vec3(0, -10, 0), vec3(0, -42, 0), 2.1);
        pigSpiderPath = Spline(pSpiderPosition, vec3(0, -10, -4), vec3(0, -42, -4), 2);
	}
    
	void renderPigScene(float frametime) {
        
        tPig += frametime;
        shared_ptr<Program> simple = shaderManager->getCurrentShader();
        
        auto Model = make_shared<MatrixStack>();
        
        simple->bind();
        // Apply perspective projection.
        SetProjectionMatrix(simple);
        SetViewMatrix(simple);
        
        // Barn
        Model->pushMatrix();
        Model->loadIdentity();
        Model->translate(vec3(-4, -1.5, -10));
        Model->scale(vec3(1, 1, 1));
        Model->rotate(-(M_PI / 2), vec3(1, 0, 0));
        Model->rotate((M_PI / 6), vec3(0, 0, 1));
        glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
        barn->draw(simple, Model);
        Model->popMatrix();
        
        // Ground
        Model->pushMatrix();
        Model->loadIdentity();
        Model->translate(vec3(0, -1, 0));
        Model->scale(vec3(100, 0.5, 100));
        glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
        cube->draw(simple);
        Model->popMatrix();
        
        //Other ground
        Model->pushMatrix();
        Model->loadIdentity();
        Model->translate(vec3(0, -50, 0));
        Model->scale(vec3(100, 0.5, 100));
        glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
        cube->draw(simple);
        Model->popMatrix();
        //Mini spider
        Model->pushMatrix();
        Model->loadIdentity();
        Model->translate(vec3(0, -.35, -3.2));
        Model->rotate((M_PI / 2), vec3(1, 0, 0));
        Model->scale(vec3(.01, .01, .01));
        glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
        cartoon_spider->draw(simple, Model);
        Model->popMatrix();
        
        // Regular old pig
        if(tPig < 4){
            pigCameraPath[0].update(frametime);
            if (!pigCameraPath[0].isDone()) {
                camera.eye = pigCameraPath[0].getPosition();
                camera.target = camera.eye + vec3(0, 0, -1);
            }
            Model->pushMatrix();
            Model->loadIdentity();
            Model->translate(vec3(-1.3, -.8, -4));
            Model->rotate(-(M_PI / 2), vec3(1, 0, 0));
            Model->scale(vec3(1, 1, 1));
            glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
            regular_pig->draw(simple, Model);
            Model->popMatrix();
        }
        // Suddently becomes Pig Spider!
        else {
            Model->pushMatrix();
            Model->loadIdentity();
            Model->translate(pigSpiderPath.getPosition());
            Model->rotate(pSpiderRotate, vec3(0, 0, 1));
            Model->scale(5);
            Model->rotate(M_PI, YAXIS);
            pigSpider.draw(simple, Model);
            Model->popMatrix();
        }
        
        // Then the Portal happens
        if(tPig > 5){
            if(tPig > 5.5){
                pigCameraPath[1].update(frametime);
                if (!pigCameraPath[1].isDone()) {
                    camera.eye = pigCameraPath[1].getPosition();
                    camera.target = camera.eye + vec3(0, 0, -1);
                }
                if(!pigSpiderPath.isDone()){
                    pigSpiderPath.update(frametime);
                }
                if(pSpiderRotate > -9){
                    pSpiderRotate -= .08;
                }
            }
            Model->pushMatrix();
            Model->loadIdentity();
            Model->translate(vec3(1, -.8, -4));
            Model->scale(vec3(1, .1, 1));
            glUniformMatrix4fv(simple->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
            sphere->draw(simple);
            Model->popMatrix();
        }
        
        simple->unbind();
	}

	void setupMinecraftScene() {

	}

	void renderMinecraftScene(float frametime) {

	}

	void setupAllScene() {

	}

	void renderAllScene(float frametime) {
		// Scene showing all spiders at once
	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initPhysicsObjects();
	application->nextScene();

	auto lastTime = chrono::high_resolution_clock::now();
	float accumulator = 0.0f;
	Time.physicsDeltaTime = 0.02f;

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{

		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();

		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		accumulator += deltaTime;
		while (accumulator >= Time.physicsDeltaTime) {
			application->updatePhysics(Time.physicsDeltaTime);
			accumulator -= Time.physicsDeltaTime;
		}

		// Render scene.
		application->render(deltaTime);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
