#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "Mesh.hpp"
#include "Collision.hpp"
#include "SkyBox.hpp"

#include <iostream>

const float toRadians = 3.14159265f / 180.0f;
const float fromRadians = 180.0f / 3.14159265f;

const unsigned int SHADOW_WIDTH = 10480;
const unsigned int SHADOW_HEIGHT = 10480;

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

glm::mat4 initial_view;
glm::vec3 front_direction;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLboolean isTransparentLoc;
GLboolean showShadowLoc;
GLboolean showFogLoc;

GLint lightSpotPosLoc;
GLint lightSpotDirLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 0.3f, 0.0f),
    glm::vec3(0.0f, 0.0f, -15.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

Collision myCollisionDetection = Collision();

GLfloat cameraSpeed = 1.0f;

GLboolean pressedKeys[1024];

// models
Model3D teapot;
Model3D terrain;
Model3D grass;
Model3D tree_bark1;
Model3D arrow;
Model3D mountain;
Model3D tree_leaves1;
Model3D target;
Model3D bow;
Model3D cottage;
Model3D tree_bark2;
Model3D tree_leaves2;
Model3D clover;

GLfloat angle;

// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;

//mouse
bool firstMouse = true;
float yaw = 90.0f;
float pitch = 0.0f;
float lastX = 0, lastY = 0;
bool mouseClicked = false;

//plain texture
Texture plainTexture;

//for shadows
GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;
bool showShadows = false;

//make sun pass
float sun_position_y = 0.0f;
float sun_position_z = 1.0f;
bool inc_y = true;
bool dayCycleCompleted = false;
bool changeDayNightMode = false;

//make arrow fly
bool shotArrow = false;
bool getInitialPosition = true;
glm::vec3 arrowPosition;
float acceleration_gravity = 0.098f;
float mass = 1.0f;
float gravity = acceleration_gravity * mass;
float acceleration;
float horiz_velocity = 0.3f;
float vert_velocity = 0.1f;
bool goingUp = true;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

//target state
int target_state = 0;

//toggle bow and arrow
bool showBowAndArrow = false;

//cottage inside detection
bool isInsideCottage = false;

//bow taking
bool bowAquired = false;

//night mode
bool enableNightMode = false;
bool enableDayNightCycle = false;

std::vector<const GLchar*> faces;
std::vector<const GLchar*> darkFaces;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouseClicked = true;
        }
        else if (action == GLFW_RELEASE) {
            mouseClicked = false;
        }
    }

}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    glm::vec3 newSpotDir = myCamera.getFrontDirection();
    glUniform3fv(lightSpotDirLoc, 1, glm::value_ptr(newSpotDir));
}

void processInputs() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed * deltaTime);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

        //send new spot light position
        glm::vec3 newSpotPosition = myCamera.getPosition();
        glUniform3fv(lightSpotPosLoc, 1, glm::value_ptr(newSpotPosition));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed * deltaTime);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

        //send new spot light position
        glm::vec3 newSpotPosition = myCamera.getPosition();
        glUniform3fv(lightSpotPosLoc, 1, glm::value_ptr(newSpotPosition));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed * deltaTime);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

        //send new spot light position
        glm::vec3 newSpotPosition = myCamera.getPosition();
        glUniform3fv(lightSpotPosLoc, 1, glm::value_ptr(newSpotPosition));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed * deltaTime);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

        //send new spot light position
        glm::vec3 newSpotPosition = myCamera.getPosition();
        glUniform3fv(lightSpotPosLoc, 1, glm::value_ptr(newSpotPosition));
    }

    if (mouseClicked) {
        if (showBowAndArrow) {
            goingUp = true;
            shotArrow = true;
            vert_velocity = 0.1f;
            getInitialPosition = true;
        }
    }

    if (pressedKeys[GLFW_KEY_KP_ADD]) {
        if (bowAquired) {
            showBowAndArrow = true;
        }
    }
    if (pressedKeys[GLFW_KEY_KP_SUBTRACT]) {
        if (bowAquired) {
            showBowAndArrow = false;
        }
    }

    if (pressedKeys[GLFW_KEY_KP_MULTIPLY]) {
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "showSpotLight"), true);
    }

    if (pressedKeys[GLFW_KEY_KP_DIVIDE]) {
        myBasicShader.useShaderProgram();
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "showSpotLight"), false);
    }
    

    if (pressedKeys[GLFW_KEY_Z]) {
        glm::vec3 position = myCamera.getPosition();
        printf("%f, %f, %f\n", position.x, position.y, position.z);
    }
    if (pressedKeys[GLFW_KEY_O]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showShadowLoc, true);
        showShadows = true;
    }
    if (pressedKeys[GLFW_KEY_P]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showShadowLoc, false);
        showShadows = false;
    }

    if (pressedKeys[GLFW_KEY_K]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showFogLoc, true);
    }

    if (pressedKeys[GLFW_KEY_L]) {
        myBasicShader.useShaderProgram();
        glUniform1i(showFogLoc, false);
    }

    if (pressedKeys[GLFW_KEY_N]) {
        enableNightMode = true;
        myBasicShader.useShaderProgram();
        glm::vec3 lightColor = glm::vec3(0.05f, 0.05f, 0.05f); //dark light
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "nightModeEnabled"), true);
        mySkyBox.Load(darkFaces);
    }

    if (pressedKeys[GLFW_KEY_M]) {
        enableNightMode = false;
        myBasicShader.useShaderProgram();
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "nightModeEnabled"), false);
        mySkyBox.Load(faces);
    }

    if (pressedKeys[GLFW_KEY_KP_1]) {
        enableDayNightCycle = true;
        sun_position_y = 0.0f;
        sun_position_z = 1.0f;
    }
    if (pressedKeys[GLFW_KEY_KP_2]) {
        enableDayNightCycle = false;
        lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "nightModeEnabled"), false);
        dayCycleCompleted = false;
        changeDayNightMode = false;
        mySkyBox.Load(faces);
    }

    // line view
    if (pressedKeys[GLFW_KEY_KP_7]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // point view
    if (pressedKeys[GLFW_KEY_KP_8]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    // normal view
    if (pressedKeys[GLFW_KEY_KP_9]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetMouseButtonCallback(myWindow.getWindow(), mouse_button_callback);
}

void initOpenGLState() {
    //glClearColor(0.8, 0.8, 0.8, 1.0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_CLAMP);
}

void initModels() {
    //teapot = Model3D();
    //teapot.LoadModel("models/teapot/teapot20segUT.obj", "models/teapot/", false);
    terrain = Model3D();
    terrain.LoadModel("models/scene/scene2.obj", "models/scene/", false);
    grass = Model3D();
    grass.LoadModel("models/forest/grass/grass.obj", "models/forest/grass/", true);
    tree_bark1 = Model3D();
    tree_bark1.LoadModel("models/forest/tree1/tree2.obj", "models/forest/tree1/", false);
    tree_leaves1 = Model3D();
    tree_leaves1.LoadModel("models/forest/tree1/tree1.obj", "models/forest/tree1/", true);
    clover = Model3D();
    clover.LoadModel("models/forest/clover/clover.obj", "models/forest/clover/", true);
    arrow = Model3D();
    arrow.LoadModel("models/arrow/arrow4.obj", "models/arrow/", false);
    target = Model3D();
    target.LoadModel("models/target/target2.obj", "models/target/", false);
    bow = Model3D();
    bow.LoadModel("models/bow/bow.obj", "models/bow/", false);
    cottage = Model3D();
    cottage.LoadModel("models/cottage/cottage2.obj", "models/cottage/", false);

}


void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");

    depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
}

void initSkyBoxShader()
{
    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), 
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 
        0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.01f, 50.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    isTransparentLoc = glGetUniformLocation(myBasicShader.shaderProgram, "isTransparent");
    glUniform1i(isTransparentLoc, false);

    showShadowLoc = glGetUniformLocation(myBasicShader.shaderProgram, "showShadow");
    glUniform1i(showShadowLoc, false);

    showFogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "showFog");
    glUniform1i(showFogLoc, false);

    //spot light
    //set the spot light position
    lightSpotPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPos");
    glm::vec3 newSpotPosition = myCamera.getPosition();
    glUniform3fv(lightSpotPosLoc, 1, glm::value_ptr(newSpotPosition));
    //set the spot light direction
    lightSpotDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDir");
    glm::vec3 newSpotDir = myCamera.getFrontDirection();
    glUniform3fv(lightSpotDirLoc, 1, glm::value_ptr(newSpotDir));
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "showSpotLight"), false);
    //send cutoffs
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "cutOff"), glm::cos(glm::radians(12.5f)));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "outerCutOff"), glm::cos(glm::radians(15.0f)));



}

void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    // 
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initFaces()
{

    faces.push_back("skybox/daySkybox/posx.jpg");
    faces.push_back("skybox/daySkybox/negx.jpg");
    faces.push_back("skybox/daySkybox/posy.jpg");
    faces.push_back("skybox/daySkybox/negy.jpg");
    faces.push_back("skybox/daySkybox/posz.jpg");
    faces.push_back("skybox/daySkybox/negz.jpg");
}


void initDarkFaces()
{
    darkFaces.push_back("skybox/nightSkybox/posx.tga");
    darkFaces.push_back("skybox/nightSkybox/negx.tga");
    darkFaces.push_back("skybox/nightSkybox/posy.tga");
    darkFaces.push_back("skybox/nightSkybox/negy.tga");
    darkFaces.push_back("skybox/nightSkybox/posz.tga");
    darkFaces.push_back("skybox/nightSkybox/negz.tga");
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix

    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = -8.0f, far_plane = 8.0f;
    glm::mat4 lightProjection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void renderTeapot(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    teapot.RenderModel(shader);
}


void renderTerrain(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    terrain.RenderModel(shader);
}

void renderTree(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        
    }
    tree_bark1.RenderModel(shader);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "isTransparent"), true);
    tree_leaves1.RenderModel(shader);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "isTransparent"), false);

}

void renderClover(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0f, 1.0f, 2.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    }
    clover.RenderModel(shader);

}

void renderGrass(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    }
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "isTransparent"), true);
    grass.RenderModel(shader);
    glUniform1i(glGetUniformLocation(shader.shaderProgram, "isTransparent"), false);

}

void renderArrow(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);

    model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.05f, -0.02f, 0.1f));
    
    
    //glm::inverse(view)
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::inverse(view) * model));
    
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    arrow.RenderModel(shader);
}

void renderShootingArrow(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
   
    if (getInitialPosition) {
        getInitialPosition = false;
        arrowPosition = myCamera.getPosition();
        //arrowPosition = glm::vec3(arrowPosition.x, 0.19f, arrowPosition.z);
        glm::vec3 front_direction = myCamera.getFrontDirection();
        float shooting_angle = glm::dot(front_direction, glm::vec3(0.0f, 1.0f, 0.0f));
        vert_velocity = sin(shooting_angle);
    }

    glm::vec3 velocity_vector = glm::vec3(0.0f, vert_velocity, 0.0f);
    float rot_angle = glm::dot(velocity_vector, glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

    if (goingUp) {
        acceleration = -gravity / mass;
        
        vert_velocity = vert_velocity + acceleration * deltaTime;
        if (vert_velocity > 0) {
            arrowPosition.y += vert_velocity * deltaTime + acceleration * deltaTime * deltaTime / 2;
        }
        else { //velocity is zero, ball will begin to fall down
            goingUp = false;
        }
        rot_angle = -rot_angle/2;
    }
    else {
        acceleration = gravity / mass;
        vert_velocity = vert_velocity + acceleration * deltaTime;
        if (arrowPosition.y > 0.01f) {
            arrowPosition.y -= vert_velocity * deltaTime + acceleration * deltaTime * deltaTime / 2;
        }
        else {
            //arrow hits the floor
            goingUp = true;
            shotArrow = false;
            vert_velocity = 0.1f;
            getInitialPosition = true;
        }
    }
 
    arrowPosition += glm::vec3(0.0f, 0.0f, 0.1f * horiz_velocity);
    //printf("%f %f\n", rot_angle, velocity_vector.y);
    model = glm::translate(model, arrowPosition);
    model = glm::rotate(model, rot_angle , glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    //collision detection
    if (arrowPosition.z >= 4.9f && arrowPosition.z <= 5.1f && arrowPosition.x > -0.1f + target_state*0.5f && 
        arrowPosition.x < 0.1f + target_state * 0.5f
        && arrowPosition.y > 0.23f && arrowPosition.y < 0.43f) {
        target_state ++;
        if (target_state > 5) {
            target_state = 0;
        }
        
        shotArrow = false;
    }

    // draw teapot
    arrow.RenderModel(shader);

}

void renderTarget(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(target_state * 0.5f, 0.2f, 5.0f));
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    target.RenderModel(shader);
}

void renderBowInCottage(gps::Shader shader) {

    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::rotate(model, 90 * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-2.6f, 3.9f, -0.27f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));


    // draw teapot
    bow.RenderModel(shader);

}

void renderBow(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::rotate(model, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(-0.20f, -0.20f, -0.20f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr( glm::inverse(view) * model));

    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));


    // draw teapot
    bow.RenderModel(shader);
}


void renderCottage(gps::Shader shader, bool depthPass) {
    // select active shader program
    shader.useShaderProgram();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f,0.0f, 3.0f));
    model = glm::rotate(model, 180 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));

    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // draw teapot
    cottage.RenderModel(shader);
}

void renderScene() {
    

	//render the scene
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    //render shadows
    if (showShadows) {
        depthMapShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        renderTerrain(myBasicShader, false);
        renderTarget(depthMapShader, true);
        renderTree(depthMapShader, true);
        renderCottage(depthMapShader, true);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //render scene
        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        myBasicShader.useShaderProgram();
       
        //bind the shadow map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(computeLightSpaceTrMatrix()));
    }
    else {

        //render scene
        glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        myBasicShader.useShaderProgram();
    }

    
    renderTerrain(myBasicShader, false);
    renderClover(myBasicShader, false);
    renderTree(myBasicShader, false);
    renderTarget(myBasicShader, false);
    renderCottage(myBasicShader, false);
    renderGrass(myBasicShader, false);

    if (!bowAquired) {
        renderBowInCottage(myBasicShader);
    }
    else {
        if (showBowAndArrow) {
            renderBow(myBasicShader);
            if (shotArrow) {
                renderShootingArrow(myBasicShader);
            }
            else {
                renderArrow(myBasicShader);
            }
        }
    }
    mySkyBox.Draw(skyboxShader, view, projection);
    
}

void checkIfInsideCottage() {
    //stairs and hall coordinates
    glm::vec3 cameraPosition = myCamera.getPosition();
    glm::vec2 M = glm::vec2(cameraPosition.x, cameraPosition.z);
    glm::vec2 A = glm::vec2(-1.23f, 4.23f);
    glm::vec2 B = glm::vec2(-1.40f, 4.01f);
    glm::vec2 D = glm::vec2(-2.19f, 4.97f);
    //check if on stairs or hall
    if (myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M)) {
        if (!isInsideCottage) {
            isInsideCottage = true;
            myCamera.move(gps::MOVE_UP, 0.2f);
        }
    }
    else {
        B = glm::vec2(-2.28f, 3.41f);
        A = glm::vec2(-1.46f, 4.39f);
        //check if inside cottage
        if (myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M)) {
            if (!isInsideCottage) {
                isInsideCottage = true;
                myCamera.move(gps::MOVE_UP, 0.2f);
            }
        }
        else {
            if (isInsideCottage) {
                isInsideCottage = false;
                myCamera.move(gps::MOVE_DOWN, 0.2f);
            }
        }
    }
}

void checkIfBowAquired() {
    glm::vec3 cameraPosition = myCamera.getPosition();
    glm::vec2 M = glm::vec2(cameraPosition.x, cameraPosition.z);
    glm::vec2 A = glm::vec2(-2.69f, 4.17f);
    glm::vec2 B = glm::vec2(-2.61f, 4.07f);
    glm::vec2 D = glm::vec2(-2.78f, 4.06f);
    //check if on stairs or hall
    if (myCollisionDetection.checkIfPointInsideRectangle(A, B, D, M)) {
        bowAquired = true;
    }
}

void dayNightCycle() {
    myBasicShader.useShaderProgram();

    //chenage day night mode
    if (changeDayNightMode) {
        if (dayCycleCompleted) {
            glm::vec3 lightColor = glm::vec3(0.05f, 0.05f, 0.05f); //dark light
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
            glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "nightModeEnabled"), true);
            mySkyBox.Load(darkFaces);
            changeDayNightMode = false;   
        }
        else {
            glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
            glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "nightModeEnabled"), false);
            mySkyBox.Load(faces);
            changeDayNightMode = false;
        }
    }

    //compute new light coordinates
    if (inc_y) {
        if (sun_position_y <= 1.0f) {
            sun_position_y += 0.002f;
        }
        else {
            inc_y = false;
        }
    }
    else {
        if (sun_position_y >= 0.0f) {
            sun_position_y -= 0.002f;
        }
        else {
            inc_y = true;
        }
    }
    if (sun_position_z >= -1.0f) {
        sun_position_z -= 0.002f;
    }
    else {
        sun_position_z = 1.0f;
        sun_position_y = 0.0f;
        dayCycleCompleted = !dayCycleCompleted;
        changeDayNightMode = true;
    }
    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 0.0f + sun_position_y, 0.0f + sun_position_z);
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
}
 
void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initFBO();
    initFaces();
    initDarkFaces();
    initSkyBoxShader();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processInputs();
	    renderScene();
        if (enableDayNightCycle) {
            dayNightCycle();
        }
        checkIfInsideCottage();
        if (!bowAquired) {
            checkIfBowAquired();
        }

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		//glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
