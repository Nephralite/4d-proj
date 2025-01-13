#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

// Lazy globals because I'm a C programmer and don't like classes);
GLuint programId;
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;

// Function to load shader files
std::string loadShaderCode(const std::string& filename)
{
    std::ifstream shaderFile(filename);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    return buffer.str();
}

// Compile and create shader object and returns its id
GLuint compileShaders(std::string shader, GLenum type)
{

    const char* shaderCode = shader.c_str();
    GLuint shaderId = glCreateShader(type);

    if (shaderId == 0) { // Error: Cannot create shader object
        std::cout << "Error creating shaders";
        return 0;
    }

    // Attach source code to this object
    glShaderSource(shaderId, 1, &shaderCode, nullptr);
    glCompileShader(shaderId); // compile the shader object

    GLint compileStatus;

    // check for compilation status
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus) { // If compilation was not successful
        int length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        char* cMessage = new char[length];

        // Get additional information
        glGetShaderInfoLog(shaderId, length, &length, cMessage);
        std::cout << "Cannot Compile Shader: " << cMessage;
        delete[] cMessage;
        glDeleteShader(shaderId);
        return 0;
    }

    return shaderId;
}

// Creates a program containing vertex and fragment shader
// links it and returns its ID
GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId)
{
    programId = glCreateProgram(); // create a program

    if (programId == 0) {
        std::cout << "Error Creating Shader Program";
        return 0;
    }

    // Attach both the shaders to it
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // Create executable of this program
    glLinkProgram(programId);

    GLint linkStatus;

    // Get the link status for this program
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) { // If the linking failed
        std::cout << "Error Linking program";
        glDetachShader(programId, vertexShaderId);
        glDetachShader(programId, fragmentShaderId);
        glDeleteProgram(programId);

        return 0;
    }

    return programId;
}

// Initialize and put everything together
GLuint init()
{
    // clear the framebuffer each frame with black color
    glClearColor(0, 0, 0, 0);

    GLfloat vertices[16*4] = {
        0.5f,  0.5f, -0.5f, 0.5f,  // ana front top right
        0.5f, -0.5f, -0.5f, 0.5f,  // ana front bottom right
       -0.5f,  0.5f, -0.5f, 0.5f,  // ana front top left
       -0.5f, -0.5f, -0.5f, 0.5f,  // ana front bottom left
        0.5f,  0.5f, 0.5f, 0.5f,  // ana back top right
        0.5f, -0.5f, 0.5f, 0.5f,  // ana back bottom right
       -0.5f,  0.5f, 0.5f, 0.5f,  // ana bottom top left
       -0.5f, -0.5f, 0.5f, 0.5f,  // ana back bottom left
        0.5f,  0.5f, -0.5f, -0.5f,  // kata front top right
        0.5f, -0.5f, -0.5f, -0.5f,  // kata front bottom right
       -0.5f,  0.5f, -0.5f, -0.5f,  // kata front top left
       -0.5f, -0.5f, -0.5f, -0.5f,  // kata front bottom left
        0.5f,  0.5f, 0.5f, -0.5f,  // kata back top right
        0.5f, -0.5f, 0.5f, -0.5f,  // kata back bottom right
       -0.5f,  0.5f, 0.5f, -0.5f,  // kata bottom top left
       -0.5f, -0.5f, 0.5f, -0.5f,  // kata back bottom left
    };

     GLint tetra_indices[40][4] = {  // i calculated these all by hand, so maybe you can see why I'm not rendering any
        //other 4d objects lol, i absolutely could've copied these from misatope, but I feel like doing it manually
        //was an interesting exercise in trying to understand 4d
        //each of these 4 numbers produce a tetrahedron, and a cube is sliced into 5 tetrahedron, so a hypercube
        //requires 40 tetrahedron for the 8 cubes it is made from
        //the basic cube of 0,1,2,3,4,5,6,7
        {0,1,3,5},
        {3,5,6,7},
        {0,4,5,6},
        {0,2,3,6},
        {0,3,5,6},
        //the "new" cube of 8,9,10,11,12,13,14,15
        {8,9,11,13},
        {11,13,14,15},
        {8,12,13,14},
        {8,10,11,14},
        {9,11,13,14},
        //the connecting "top" cube of 0,2,4,6,8,10,12,14
       { 0,2,6,10},
        {6,10,12,14},
        {0,8,10,12},
       { 0,4,6,12},
        {0,6,10,12},
        //the connecting "bottom" cube of 1,3,5,7,9,11,13,15
        {1,3,7,11},
        {7,11,13,15},
        {1,9,11,13},
        {1,5,7,13},
        {1,7,11,13},
        //the connecting "left" cube of 0,1,4,5,8,9,12,13
        {0,1,5,9},
        {5,9,12,13},
       {0,8,9,12},
        {0,4,5,12},
        {0,5,9,12},
        //the connecting "right" cube of 2,3,6,7,10,11,14,15
        {2,3,7,10},
        {7,10,14,15},
        {2,10,11,14},
        {2,6,7,14},
        {2,7,10,14},
        //the connecting "front" cube of 0,1,2,3,8,9,10,11
        {0,1,3,9},
        {3,9,10,11},
        {0,8,9,10},
        {0,2,3,10},
        {0,3,9,10},
        //the connecting "back" cube of 4,5,6,7,12,13,14,15
        {4,5,7,13},
        {7,13,14,15},
        {4,12,13,14},
        {4,6,7,14},
        {4,7,13,14}
    };

    //I wrote that in tetras for 4d correctness, but we do need to convert those back to triangles
    GLint indices[40*12];
    int j = 0;
    //a,b,c,d needs to become a,b,c, a,b,d ,a,c,d, b,c,d,
    for (int i=0; i<40; i++) {
        indices[j] = tetra_indices[i][0];
        indices[j+1] = tetra_indices[i][1];
        indices[j+2] = tetra_indices[i][2];
        indices[j+3] = tetra_indices[i][0];
        indices[j+4] = tetra_indices[i][1];
        indices[j+5] = tetra_indices[i][3];
        indices[j+6] = tetra_indices[i][0];
        indices[j+7] = tetra_indices[i][2];
        indices[j+8] = tetra_indices[i][3];
        indices[j+9] = tetra_indices[i][1];
        indices[j+10] = tetra_indices[i][2];
        indices[j+11] = tetra_indices[i][3];
        j += 12;
    }

    GLfloat colours[16*3]  = { //one colour per vertex
        1.0f, 0.5f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.0f, 0.5f, 0.5f,
        0.5f, 0.0f, 0.5f,
        0.5f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f,
        0.5f, 0.5f, 0.5f,
    };

    // allocate vertex buffer space and pass data to it
    GLuint vboId, eboId, vaoId, colourId;
    glGenVertexArrays(1, &vaoId);
    glGenBuffers(1, &vboId);
    glGenBuffers(1, &eboId);
    glGenBuffers(1, &colourId);
    // Bind it so that rest of vao operations affect this vao
    glBindVertexArray(vaoId);
    //vertices to vertex array
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //indices to index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //set vao
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    //colours to colour array
    glBindBuffer(GL_ARRAY_BUFFER, colourId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    //unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //load, compile and set shaders
    std::string vertexShader = loadShaderCode("/home/jade/CLionProjects/4dProjectionDemo/four.vert");
    std::string fragmentShader = loadShaderCode("/home/jade/CLionProjects/4dProjectionDemo/shader.frag");
    GLuint vShaderId = compileShaders(vertexShader, GL_VERTEX_SHADER);
    GLuint fShaderId = compileShaders(fragmentShader, GL_FRAGMENT_SHADER);
    GLuint programId = linkProgram(vShaderId, fShaderId);
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vShaderId);
    glDeleteShader(fShaderId);
    //render settings that maybe should be in main

    // Use this program for rendering.
    glUseProgram(programId);
    //enable wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //z buffering
    glEnable(GL_DEPTH_TEST);

    //a bunch of 4d variables
    //uniform vec4 up;
    GLfloat up[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    GLint upLoc = glGetUniformLocation(programId, "up");
    glUniform4fv(upLoc, 1, up);
    //uniform vec4 over;
    GLfloat over[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLint overLoc = glGetUniformLocation(programId, "over");
    glUniform4fv(overLoc, 1, over);
    //uniform double vAngle;
    GLfloat vAngle = glm::radians(45.0f);
    GLint vAngleLoc = glGetUniformLocation(programId, "vAngle");
    glUniform1f(vAngleLoc, vAngle);

    //camera setup
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    GLint projectionLoc = glGetUniformLocation(programId, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //std::cout << fromLoc << toLoc<< upLoc << overLoc << vAngleLoc << projectionLoc << std::endl;

    return programId;
}

// Function that does the drawing
// glut calls this function whenever it needs to redraw
void display(GLFWwindow *window) {
    // clear the color buffer before each drawing
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //more camera stuff
    GLint modelLoc = glGetUniformLocation(programId, "model");
    GLint viewLoc = glGetUniformLocation(programId, "view");
    glm::mat4 model = glm::rotate((glm::mat4(1.0f)), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //uniform vec4 from;
    float camW = cos(glfwGetTime()) * 3.0f;
    GLfloat from[4] = {0.0f, 0.0f, -2.0f, camW};
    GLint fromLoc = glGetUniformLocation(programId, "from");
    //std::cout << from[0] <<" " << from[1] << std::endl;
    glUniform4fv(fromLoc, 1, from);
    //uniform vec4 to;
    GLfloat to[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    GLint toLoc = glGetUniformLocation(programId, "to");
    glUniform4fv(toLoc, 1, to);

    //draw triangles starting from index 0
    glDrawElements(GL_TRIANGLES, 480, GL_UNSIGNED_INT, nullptr);

    //recalculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void processInput(GLFWwindow *window)
{
    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// main function
// sets up window to which we'll draw
int main(int argc, char** argv)
{
    // initialize and configure GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Cube on OpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewInit();
    init();
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    while(!glfwWindowShouldClose(window)) {
        processInput(window);
        display(window);
    }
    glfwTerminate();
    return 0;
}