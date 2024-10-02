// Autor: Nedeljko Tesanovic
// Opis: Testiranje dubine, Uklanjanje lica, Transformacije, Prostori i Projekcije

#define _CRT_SECURE_NO_WARNINGS
#define CRES 30


#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "model.hpp"

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
void call(glm::mat4 model, glm::mat4 projection, glm::mat4 view, int isUvOn, int isPointLightOn);
void call_gouraud(glm::mat4 model, glm::mat4 projection, glm::mat4 view, int isUvOn, int isPointLightOn);
Shader* phongShader;
Shader* frameRotationShader;
Shader* basicShader;
Shader* gouraudShader;
Shader* frameGouraudShader;


bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool checkCamera(glm::vec3 temp);

float minX = -0.5f; // Replace with the actual minimum X coordinate of the room
float maxX = 0.5f;  // Replace with the actual maximum X coordinate of the room
float minY = -0.5f; // Replace with the actual minimum Y coordinate of the room
float maxY = 0.5f;  // Replace with the actual maximum Y coordinate of the room
float minZ = -1.0f; // Replace with the actual minimum Z coordinate of the room
float maxZ = 1.0f;

static unsigned loadImageToTexture(const char* filePath);

static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}

int main(void)
{

   
    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 800;
    unsigned int wHeight = 800;
    const char wTitle[] = "[Generic Title]";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }
    
    glfwMakeContextCurrent(window);

    
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }
    float SCALE = 2.0;
    float SCALE2 = 3.0;

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++
    float vertices[] =
    {
        //X    Y    Z       R    G    B    A
        //back
    -0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
     0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
     0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    -0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    -0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    //front
    -0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
     0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
     0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
     0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
    -0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    -0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2, 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2, 1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2, 1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2,   1.0f, 0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2, 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2,  -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2,  -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2,  -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2,  -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2,  -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2,  -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2,  0.0f, 1.0f,  0.0f,  1.0f,  1.0f,
         0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f * SCALE, -0.5f * SCALE,  0.5f * SCALE2,  0.0f, 1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f * SCALE, -0.5f * SCALE, -0.5f * SCALE2,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2,  0.0f,  -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2,  0.0f,  -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2,  0.0f,  -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2,  0.0f,  -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f * SCALE,  0.5f * SCALE,  0.5f * SCALE2,  0.0f,  -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f * SCALE,  0.5f * SCALE, -0.5f * SCALE2,  0.0f,  -1.0f,  0.0f,  0.0f,  1.0f

    };
    unsigned int stride = (8) * sizeof(float); 
    
    unsigned int VAO[8];
    glGenVertexArrays(8, VAO);

    unsigned int VBO[8];
    glGenBuffers(8, VBO);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned diffuseMap = loadImageToTexture("res/silver_diffuse.png");
    unsigned specularMap = loadImageToTexture("res/silver_specular.png");
    unsigned monaLisa = loadImageToTexture("res/mona_lisa.jpg");
    unsigned napoleon = loadImageToTexture("res/napoleon.jpg");
    unsigned medusa = loadImageToTexture("res/medusaa.jpg");
    unsigned liberty = loadImageToTexture("res/liberty.jpg");
    unsigned kevin = loadImageToTexture("res/vangog.jpg");
    unsigned belvedere = loadImageToTexture("res/grant.jpg");
    unsigned name = loadImageToTexture("res/name.png");
    

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            PROGRESS BAR            +++++++++++++++++++++++++++++++++++++++++++++++++
    float white = 255 / 255.0;
    float green_r = 128 / 255.0;
    float green_g = 255 / 255.0;
    float green_b = 128 / 255.0;

    float progressBarVertices[] = {
        0.18 * SCALE, -0.3 * SCALE, 0.495f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.18 * SCALE, -0.3 * SCALE, 0.495f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
       0.18 * SCALE, -0.24 * SCALE, 0.495f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
       -0.18 * SCALE, -0.24 * SCALE, 0.495f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,

       0.17 * SCALE, -0.25 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
       0.17 * SCALE, -0.29 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
       0.085 * SCALE, -0.25 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.085 * SCALE, -0.29 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.0 * SCALE, -0.25 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.0 * SCALE, -0.29 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.085 * SCALE, -0.25 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.085 * SCALE, -0.29 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
       -0.17 * SCALE, -0.25 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
       -0.17 * SCALE, -0.29 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
    };


    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(progressBarVertices), progressBarVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    float frameVertices[] = {
        -0.35 * SCALE, 0.4 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.34 * SCALE, 0.39 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.35 * SCALE, 0.2 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.34 * SCALE, 0.19 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.16 * SCALE, 0.2 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 0.0,
        -0.15 * SCALE, 0.19 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f,  0.0, 0.0,
        -0.16 * SCALE, 0.4 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f,  1.0, 1.0,
        -0.15 * SCALE, 0.39 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f,  0.0, 1.0,
        -0.35 * SCALE, 0.4 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.34 * SCALE, 0.39 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,

        0.35 * SCALE, 0.36 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.34 * SCALE, 0.35 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.35 * SCALE, 0.16 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.34 * SCALE, 0.15 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.16 * SCALE, 0.16 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.15 * SCALE, 0.15 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.16 * SCALE, 0.36 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.15 * SCALE, 0.35 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.35 * SCALE, 0.36 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.34 * SCALE, 0.35 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,

        -0.32 * SCALE, 0.1 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.31 * SCALE, 0.09 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.32 * SCALE, -0.1 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.31 * SCALE, -0.11 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.13 * SCALE, -0.1 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.12 * SCALE, -0.11 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.13 * SCALE, 0.1 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.12 * SCALE, 0.09 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.32 * SCALE, 0.1 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        -0.31 * SCALE, 0.09 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,

        0.3 * SCALE, 0.05 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.29 * SCALE, 0.04 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.3 * SCALE, -0.16 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.29 * SCALE, -0.17 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.11 * SCALE, -0.16 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.1 * SCALE, -0.17 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.11 * SCALE, 0.05 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.1 * SCALE, 0.04 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.3 * SCALE, 0.05 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
        0.29 * SCALE, 0.04 * SCALE, 0.49f * SCALE2, 0.0f, 0.0f, -1.0f, green_r, green_g,
    };

    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frameVertices), frameVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float textures[] = {
        -0.34 * SCALE, 0.39 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 1.0,
        -0.16 * SCALE, 0.39 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 1.0,
        -0.34 * SCALE, 0.2 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 0.0,
        -0.16 * SCALE, 0.2 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 0.0,

    0.34 * SCALE, 0.35 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 1.0,
    0.16 * SCALE, 0.35 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 1.0,
    0.34 * SCALE, 0.16 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 0.0,
    0.16 * SCALE, 0.16 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 0.0,

    -0.31 * SCALE, 0.09 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 1.0,
    -0.13 * SCALE, 0.09 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 1.0,
    -0.31 * SCALE, -0.1 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 0.0,
    -0.13 * SCALE, -0.1 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 0.0,

    0.29 * SCALE, 0.04 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 1.0,
    0.11 * SCALE, 0.04 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 1.0,
    0.29 * SCALE, -0.16 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 0.0, 0.0,
    0.11 * SCALE, -0.16 * SCALE, 0.4875f * SCALE2, 0.0f, 0.0f, -1.0f, 1.0, 0.0,

     -0.34 * SCALE, 0.39 * SCALE, -0.4875f * SCALE2, 0.0f, 0.0f, 1.0f, 0.0, 1.0,
     -0.06 * SCALE, 0.39 * SCALE, -0.4875f * SCALE2, 0.0f, 0.0f, 1.0f, 1.0, 1.0,
     -0.34 * SCALE, 0.0 * SCALE, -0.4875f * SCALE2, 0.0f, 0.0f, 1.0f, 0.0, 0.0,
     -0.06 * SCALE, 0.0 * SCALE, -0.4875f * SCALE2, 0.0f, 0.0f, 1.0f, 1.0, 0.0,

     0.34 * SCALE, 0.25 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, 0.0, 1.0,
     0.06 * SCALE, 0.25 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, 1.0, 1.0,
     0.34 * SCALE, -0.14 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, 0.0, 0.0,
     0.06 * SCALE, -0.14 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, 1.0, 0.0,
    };

    glBindVertexArray(VAO[5]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textures), textures, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float uvFrameVertices[] = {
       -0.35 * SCALE, 0.4 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.34 * SCALE, 0.39 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.35 * SCALE, 0.0 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.34 * SCALE, -0.01 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.06 * SCALE, 0.0 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.05 * SCALE, -0.01 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.06 * SCALE, 0.4 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.05 * SCALE, 0.39 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.35 * SCALE, 0.4 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.34 * SCALE, 0.39 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        /*-0.35 * SCALE, 0.4 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.34 * SCALE, 0.39 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.34 * SCALE, -0.01 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.35 * SCALE, 0.0 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.05 * SCALE, -0.01 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.06 * SCALE, 0.0 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.06 * SCALE, 0.4 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.05 * SCALE, 0.39 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.34 * SCALE, 0.39 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        -0.35 * SCALE, 0.4 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,*/


        0.35 * SCALE, 0.26 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.34 * SCALE, 0.25 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.35 * SCALE, -0.14 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.34 * SCALE, -0.15 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.06 * SCALE, -0.14 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.05 * SCALE, -0.15 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.06 * SCALE, 0.26 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.05 * SCALE, 0.25 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.35 * SCALE, 0.26 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g,
        0.34 * SCALE, 0.25 * SCALE, -0.49f * SCALE2, 0.0f, 0.0f, 1.0f, green_r, green_g
    };

    glBindVertexArray(VAO[4]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvFrameVertices), uvFrameVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++
    float circle[(CRES + 2)*8];
    float r = 0.02 * SCALE;

    circle[0] = 0.26*SCALE;
    circle[1] = -0.275*SCALE;
    circle[2] = 0.495f * SCALE2;
    circle[3] = 0.0f;  
    circle[4] = 0.0f;
    circle[5] = -1.0f;
    circle[6] = -1.0f;
    circle[7] = -1.0f;
    int i;
    for (i = 0; i <= CRES; i++)
    {

        circle[8 + 8 * i] = circle[0] + r * cos((3.141592 / 180) * (i * 360 / CRES));
        circle[8 + 8 * i + 1] = circle[1] + r * sin((3.141592 / 180) * (i * 360 / CRES)) * ((float)wWidth / (int)wHeight);
        circle[8 + 8 * i + 2] = circle[2];
        circle[8 + 8 * i + 3] = 0.0f;
        circle[8 + 8 * i + 4] = 0.0f;
        circle[8 + 8 * i + 5] = -1.0f;
        circle[8 + 8 * i + 6] = -1.0f;
        circle[8 + 8 * i + 7] = -1.0f;
    }

    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    float transparentTexture[] = {
       0.4, -0.8, 0.0, 1.0,
       0.4, -1.0, 0.0, 0.0,

       1.0, -0.8, 1.0, 1.0,
       1.0, -1.0, 1.0, 0.0
    };

    glBindVertexArray(VAO[6]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentTexture), transparentTexture, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    
    float smallCubeVertices[] =
    {
    0.1, -0.5f * SCALE, 0.0,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
     0.2f, -0.5f * SCALE, 0.0,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE, 0.0,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE, 0.0,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
    0.1,  -0.4875f * SCALE, 0.0,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
    0.1, -0.5f * SCALE, 0.0,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,

    0.1, -0.5f * SCALE,  0.1f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
     0.2f, -0.5f * SCALE,  0.1f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE,  0.1f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE,  0.1f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
    0.1,  -0.4875f * SCALE,  0.1f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
    0.1, -0.5f * SCALE,  0.1f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,

    0.1,  -0.4875f * SCALE,  0.1f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    0.1,  -0.4875f * SCALE, 0.0, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    0.1, -0.5f * SCALE, 0.0, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    0.1, -0.5f * SCALE, 0.0, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    0.1, -0.5f * SCALE,  0.1f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    0.1,  -0.4875f * SCALE,  0.1f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,

     0.2f,  -0.4875f * SCALE,  0.1f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.2f, -0.5f * SCALE, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.2f, -0.5f * SCALE, 0.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.2f, -0.5f * SCALE,  0.1f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE,  0.1f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,

    0.1, -0.5f * SCALE, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     0.2f, -0.5f * SCALE, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     0.2f, -0.5f * SCALE,  0.1f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     0.2f, -0.5f * SCALE,  0.1f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    0.1, -0.5f * SCALE,  0.1f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    0.1, -0.5f * SCALE, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

    0.1,  -0.4875f * SCALE, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE,  0.1f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     0.2f,  -0.4875f * SCALE,  0.1f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
    0.1,  -0.4875f * SCALE,  0.1f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
    0.1,  -0.4875f * SCALE, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
    };

    glm::vec3 cubeCenter = glm::vec3(0.15, (-0.5f * SCALE - 0.4875f * SCALE) / 2, 0.5);

    glBindVertexArray(VAO[7]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(smallCubeVertices), smallCubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++


    //unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    //unsigned int phongShader = createShader("phong.vert", "phong.frag");
    gouraudShader = new Shader("gouraud.vert", "gouraud.frag");
    phongShader = new Shader("phong.vert", "phong.frag");
    basicShader = new Shader("basic.vert", "basic.frag");
    frameRotationShader = new Shader("frame_rotation.vert", "frame_rotation.frag");
    frameGouraudShader = new Shader("frame_gouraud.vert", "frame_gouraud.frag");
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++            UNIFORME            +++++++++++++++++++++++++++++++++++++++++++++++++

    glm::mat4 model = glm::mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    
    glm::mat4 view; //Matrica pogleda (kamere)
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // lookAt(Gdje je kamera, u sta kamera gleda, jedinicni vektor pozitivne Y ose svijeta  - ovo rotira kameru)
    
    glm::mat4 projectionP = glm::perspective(glm::radians(90.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    glm::mat4 projectionO = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f); //Matrica ortogonalne projekcije (Lijeva, desna, donja, gornja, prednja i zadnja ravan)

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f);


    glBindVertexArray(VAO[0]);

    glClearColor(0.5, 0.5, 0.5, 1.0);
    glCullFace(GL_BACK);//Biranje lica koje ce se eliminisati (tek nakon sto ukljucimo Face Culling)

    glEnable(GL_DEPTH_TEST);

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, scroll_callback);

    bool selectedPhong = true;
    float rotationSpeed = 1;
    int progressLevel = 4;
    bool buttonClicked = false;
    int isImageMoving = 1;
    bool altKeyPressed = false;
    bool zKeyPressed = false;
    bool iKeyPressed = false;
    int isUvOn = 1;
    int isPointLightOn = 1;
    bool wireframe = false;

    Model lightCube("res/10507_Golf_Ball_v1_L3.obj");
    Shader unifiedShader("light.vert", "light.frag");
    glm::mat4 modelLight = glm::mat4(1.0f);
    glm::vec3 remotePosition = glm::vec3(0.0f, 0.45f*SCALE, 0.0f);
    glm::vec3 scaleVector = glm::vec3(0.1f);
    modelLight = glm::translate(model, remotePosition);
    modelLight = glm::scale(modelLight, scaleVector);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float cameraSpeed = 2.5f * deltaTime; // adjust accordingly

        glm::vec3 temp = cameraPos;
        
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            buttonClicked = true;
            isImageMoving = 0;
            altKeyPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            buttonClicked = false;
            isImageMoving = 1;
            altKeyPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !zKeyPressed && progressLevel < 10) {
            progressLevel += 2;
            rotationSpeed += 0.5;
            zKeyPressed = true;
            altKeyPressed = false;
        }
        else if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE) {
            zKeyPressed = false;
            altKeyPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !iKeyPressed && progressLevel > 4) {
            progressLevel -= 2;
            rotationSpeed -= 0.5;
            iKeyPressed = true;
            altKeyPressed = false;
        }
        else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) {
            iKeyPressed = false;
            altKeyPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            temp += cameraSpeed * cameraFront;
            altKeyPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            altKeyPressed = false;
            temp -= cameraSpeed * cameraFront;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            temp -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            altKeyPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            temp += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            altKeyPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS && !isUvOn) {
            isUvOn = 1;
            altKeyPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && isUvOn) {
            isUvOn = 0;
            altKeyPressed = false;
        }
        if (glm::length(glm::vec3(cameraPos[0], cameraPos[1], cameraPos[2]) - cubeCenter) <= 0.63)
            std::cout << "MOZE" << "\n";
        if (glm::length(glm::vec3(cameraPos[0], cameraPos[1], cameraPos[2]) - cubeCenter) <= 0.63 &&
            glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !isPointLightOn) {
            isPointLightOn = 1;
            altKeyPressed = false;
        }
        if (glm::length(glm::vec3(cameraPos[0], cameraPos[1], cameraPos[2]) - cubeCenter) <= 0.63 &&
            glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && isPointLightOn) {
            isPointLightOn = 0;
            altKeyPressed = false;
        }


        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !selectedPhong) {
            altKeyPressed = false;
            selectedPhong = true;

        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && selectedPhong) {
            selectedPhong = false;
            altKeyPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !altKeyPressed) {
            altKeyPressed = true;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            wireframe = true;
        }

        wireframe = altKeyPressed;

        
        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        

        if (checkCamera(temp)) {
            cameraPos = temp;
        }
        cameraPos = temp;

        projection = glm::perspective(glm::radians(fov), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        


        if (selectedPhong) {
            phongShader->use();
            call(model, projection, view, isUvOn, isPointLightOn);
            phongShader->use();
        }
        else {
            gouraudShader->use();
            call_gouraud(model, projection, view, isUvOn, isPointLightOn);
            gouraudShader->use();
        }

        phongShader->setMat4("uV", view);
        gouraudShader->setMat4("uV", view);

        phongShader->setMat4("uP", projection);
        gouraudShader->setMat4("uP", projection);
        
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        phongShader->setInt("isMaterial", 0);
        gouraudShader->setInt("isMaterial", 0);
        phongShader->setInt("isPointLightOn", isPointLightOn);
        gouraudShader->setInt("isPointLightOn", isPointLightOn);
        
        phongShader->setFloat("uTexture.shine", 32.0);
        gouraudShader->setFloat("uTexture.shine", 32.0);
        phongShader->setInt("wallOff", 1);
        gouraudShader->setInt("wallOff", 1);
        glBindVertexArray(VAO[0]);
        glActiveTexture(GL_TEXTURE15);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        phongShader->setInt("uTexture.kD", 15);
        gouraudShader->setInt("uTexture.kD", 15);
        glActiveTexture(GL_TEXTURE16);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        phongShader->setInt("uTexture.kS", 16);
        gouraudShader->setInt("uTexture.kS", 16);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        phongShader->setInt("wallOff", 0);
        gouraudShader->setInt("wallOff", 0);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLES, 6, 6);
        glDrawArrays(GL_TRIANGLES, 12, 6);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLES, 18, 6);


        phongShader->setInt("wallOff", 0);
        gouraudShader->setInt("wallOff", 0);

        phongShader->setInt("isMaterial", 1);
        gouraudShader->setInt("isMaterial", 1);
        phongShader->setFloat("uMaterial.shine", 0.6 * 128);
        gouraudShader->setFloat("uMaterial.shine", 0.6 * 128);
        phongShader->setVec3("uMaterial.kA", 0.0, 0.05, 0.0);
        gouraudShader->setVec3("uMaterial.kA", 0.0, 0.05, 0.0);
        phongShader->setVec3("uMaterial.kD", 0.4, 0.5, 0.4);
        gouraudShader->setVec3("uMaterial.kD", 0.4, 0.5, 0.4);
        phongShader->setVec3("uMaterial.kS", 0.04, 0.7, 0.04);
        gouraudShader->setVec3("uMaterial.kS", 0.04, 0.7, 0.04);

        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLES, 24, 12);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLES, 30, 6);

        glBindVertexArray(VAO[1]);
        phongShader->setFloat("uMaterial.shine", 0.6 * 128);
        gouraudShader->setFloat("uMaterial.shine", 0.6 * 128);
        phongShader->setVec3("uMaterial.kA", 0.1745, 0.01175, 0.1175);
        gouraudShader->setVec3("uMaterial.kA", 0.1745, 0.01175, 0.1175);
        phongShader->setVec3("uMaterial.kD", 0.61424, 0.04136, 0.04136);
        gouraudShader->setVec3("uMaterial.kD", 0.61424, 0.04136, 0.04136);
        phongShader->setVec3("uMaterial.kS", 0.72811, 0.626959, 0.62959);
        gouraudShader->setVec3("uMaterial.kS", 0.72811, 0.626959, 0.62959);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        phongShader->setFloat("uMaterial.shine", 0.6 * 128);
        gouraudShader->setFloat("uMaterial.shine", 0.6 * 128);
        phongShader->setVec3("uMaterial.kA", 0.0, 0.05f, 0.0);
        gouraudShader->setVec3("uMaterial.kA", 0.0, 0.05f, 0.0);
        phongShader->setVec3("uMaterial.kD", 0.4f, 0.5f, 0.4f);
        gouraudShader->setVec3("uMaterial.kD", 0.4f, 0.5f, 0.4f);
        phongShader->setVec3("uMaterial.kS", 0.04f, 0.7f, 0.04f);
        gouraudShader->setVec3("uMaterial.kS", 0.04f, 0.7f, 0.04f);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, progressLevel);

        glDisable(GL_CULL_FACE);
        glBindVertexArray(VAO[7]);
        phongShader->setFloat("uMaterial.shine", 0.6 * 128);
        phongShader->setVec3("uMaterial.kA", 0.1745, 0.01175, 0.1175);
        phongShader->setVec3("uMaterial.kD", 0.61424, 0.04136, 0.04136);
        phongShader->setVec3("uMaterial.kS", 0.72811, 0.626959, 0.62959);
        gouraudShader->setFloat("uMaterial.shine", 0.6 * 128);
        gouraudShader->setVec3("uMaterial.kA", 0.1745, 0.01175, 0.1175);
        gouraudShader->setVec3("uMaterial.kD", 0.61424, 0.04136, 0.04136);
        gouraudShader->setVec3("uMaterial.kS", 0.72811, 0.626959, 0.62959);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
        glDrawArrays(GL_TRIANGLE_STRIP, 6, 6);
        glDrawArrays(GL_TRIANGLE_STRIP, 12, 6);
        glDrawArrays(GL_TRIANGLE_STRIP, 18, 6);
        glDrawArrays(GL_TRIANGLE_STRIP, 24, 6);
        glDrawArrays(GL_TRIANGLE_STRIP, 30, 6);
        glEnable(GL_CULL_FACE);

        glBindVertexArray(VAO[2]);
        if (selectedPhong) {
            frameRotationShader->use();
        }
        else {
            frameGouraudShader->use();
        }


        frameRotationShader->setInt("isTexture", 0);
        frameGouraudShader->setInt("isTexture", 0);
        frameRotationShader->setInt("isPointLightOn", isPointLightOn);
        frameGouraudShader->setInt("isPointLightOn", isPointLightOn);
        //std::cout << isUvOn;
        frameRotationShader->setInt("uvOn", isUvOn);
        frameGouraudShader->setInt("uvOn", isUvOn);
        frameRotationShader->setMat4("uM", model);
        frameGouraudShader->setMat4("uM", model);
        frameRotationShader->setMat4("uP", projection);
        frameGouraudShader->setMat4("uP", projection);
        frameRotationShader->setMat4("uV", view);
        frameGouraudShader->setMat4("uV", view);
        frameRotationShader->setVec3("uViewPos", cameraPos[0], cameraPos[1], cameraPos[2]);
        frameGouraudShader->setVec3("uViewPos", cameraPos[0], cameraPos[1], cameraPos[2]);

        frameRotationShader->setVec3("spotLight[0].pos", 0.0, 0.45f * 2, 0.0);
        frameGouraudShader->setVec3("spotLight[0].pos", 0.0, 0.45f * 2, 0.0);
        
        //phongShader->setVec3("spotLight.pos", cameraPos[0], cameraPos[1], cameraPos[2]);
        frameRotationShader->setVec3("spotLight[0].direction", 0.0f, -0.12f, 0.5f);
        frameGouraudShader->setVec3("spotLight[0].direction", 0.0f, -0.12f, 0.5f);
        frameRotationShader->setFloat("spotLight[0].constant", 1.0f);
        frameGouraudShader->setFloat("spotLight[0].constant", 1.0f);
        frameRotationShader->setFloat("spotLight[0].linear", 0.07f);
        frameGouraudShader->setFloat("spotLight[0].linear", 0.07f);
        frameRotationShader->setFloat("spotLight[0].quadratic", 0.03f);
        frameGouraudShader->setFloat("spotLight[0].quadratic", 0.03f);
        frameRotationShader->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(27.5f)));
        frameGouraudShader->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(27.5f)));
        frameRotationShader->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(41.25f)));
        frameGouraudShader->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(41.25f)));
        frameRotationShader->setVec3("spotLight[0].kA", 0.05f, 0.05f, 0.05f);
        frameGouraudShader->setVec3("spotLight[0].kA", 0.05f, 0.05f, 0.05f);
        frameRotationShader->setVec3("spotLight[0].kD", 1.0f, 1.0f, 1.0f);
        frameGouraudShader->setVec3("spotLight[0].kD", 1.0f, 1.0f, 1.0f);
        frameRotationShader->setVec3("spotLight[0].kS", 1.0f, 1.0f, 1.0f);
        frameGouraudShader->setVec3("spotLight[0].kS", 1.0f, 1.0f, 1.0f);

        frameRotationShader->setVec3("spotLight[1].pos", cameraPos[0], cameraPos[1], cameraPos[2]);
        frameGouraudShader->setVec3("spotLight[1].pos", cameraPos[0], cameraPos[1], cameraPos[2]);
        //std::cout << "front" << cameraFront[1] << "\n";
        //std::cout << "front" << cameraFront[1] << "\n";

        frameRotationShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
        frameGouraudShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
        frameRotationShader->setFloat("spotLight[1].constant", 1.0f);
        frameGouraudShader->setFloat("spotLight[1].constant", 1.0f);
        frameRotationShader->setFloat("spotLight[1].linear", 0.5f);
        frameGouraudShader->setFloat("spotLight[1].linear", 0.3f);
        //frameRotationShader->setFloat("spotLight[1].linear", 0.09f);
        //frameRotationShader->setFloat("spotLight[1].linear", 0.09f);
        //frameRotationShader->setFloat("spotLight[1].quadratic", 0.03f);
        frameRotationShader->setFloat("spotLight[1].quadratic", 0.2f);
        frameGouraudShader->setFloat("spotLight[1].quadratic", 0.2f);
        frameRotationShader->setFloat("spotLight[1].cutOff", glm::cos(glm::radians(7.5f)));
        frameGouraudShader->setFloat("spotLight[1].cutOff", glm::cos(glm::radians(12.0f)));
        frameRotationShader->setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(11.25f)));
        frameGouraudShader->setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(18.0f)));
        
        frameRotationShader->setVec3("spotLight[1].kA", 0.0f, 0.0f, 0.0f);
        frameGouraudShader->setVec3("spotLight[1].kA", 0.0f, 0.0f, 0.0f);
        frameRotationShader->setVec3("spotLight[1].kD", 0.5f, 0.0f, 0.5f);
        frameGouraudShader->setVec3("spotLight[1].kD", 0.5f, 0.0f, 0.5f);
        frameRotationShader->setVec3("spotLight[1].kS", 0.5f, 0.0f, 0.5f);
        frameGouraudShader->setVec3("spotLight[1].kS", 0.5f, 0.0f, 0.5f);

        frameRotationShader->setVec3("pointLight.pos", 0.0f, -0.5f, 0.0f);
        frameGouraudShader->setVec3("pointLight.pos", 0.0f, -0.5f, 0.0f);
        frameRotationShader->setVec3("pointLight.kA", 0.05f, 0.05f, 0.05f);
        frameGouraudShader->setVec3("pointLight.kA", 0.05f, 0.05f, 0.05f);
        frameRotationShader->setVec3("pointLight.kD", 0.2f, 0.2f, 0.0f);
        frameGouraudShader->setVec3("pointLight.kD", 0.2f, 0.2f, 0.0f);

        frameRotationShader->setVec3("pointLight.kS", 0.3f, 0.3f, 0.0f);
        frameGouraudShader->setVec3("pointLight.kS", 0.3f, 0.3f, 0.0f);
        frameRotationShader->setFloat("pointLight.constant", 1.0f);
        frameGouraudShader->setFloat("pointLight.constant", 1.0f);
        frameRotationShader->setFloat("pointLight.linear", 0.15f);
        frameGouraudShader->setFloat("pointLight.linear", 0.15f);
        frameRotationShader->setFloat("pointLight.quadratic", 0.05f);
        frameGouraudShader->setFloat("pointLight.quadratic", 0.05f);

        frameRotationShader->setVec2("uPos", isImageMoving * 0.05 * cos(glfwGetTime() * rotationSpeed), isImageMoving * 0.05 * (sin(glfwGetTime() * rotationSpeed)));
        frameGouraudShader->setVec2("uPos", isImageMoving * 0.05 * cos(glfwGetTime() * rotationSpeed), isImageMoving * 0.05 * (sin(glfwGetTime() * rotationSpeed)));

        float time = cos(glfwGetTime());
        frameRotationShader->setVec3("objectColor", 1.0f, 1.0f, 0.0);
        frameGouraudShader->setVec3("objectColor", 1.0f, 1.0f, 0.0);
        frameRotationShader->setInt("onlyUv", 0);
        frameGouraudShader->setInt("onlyUv", 0);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 1, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 2, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 3, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 5, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 6, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 7, 3);
        /*glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);*/

        glBindVertexArray(VAO[5]);
        frameRotationShader->setInt("isPointLightOn", isPointLightOn);
        frameGouraudShader->setInt("isPointLightOn", isPointLightOn);
        frameRotationShader->setInt("uvOn", isUvOn);
        frameGouraudShader->setInt("uvOn", isUvOn);
        frameRotationShader->setInt("uvOn", isUvOn);
        frameGouraudShader->setInt("uvOn", isUvOn);
        frameRotationShader->setInt("isTexture", 1);
        frameGouraudShader->setInt("isTexture", 1);
        frameRotationShader->setVec3("spotLight[0].pos", 0.0, 0.45f * 2, 0.0);
        frameGouraudShader->setVec3("spotLight[0].pos", 0.0, 0.45f * 2, 0.0);

        frameRotationShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
        frameGouraudShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
        frameRotationShader->setFloat("spotLight[1].constant", 1.0f);
        frameGouraudShader->setFloat("spotLight[1].constant", 1.0f);
        frameRotationShader->setFloat("spotLight[1].linear", 0.5f);
        frameGouraudShader->setFloat("spotLight[1].linear", 0.3f);
        //frameRotationShader->setFloat("spotLight[1].linear", 0.09f);
        //frameRotationShader->setFloat("spotLight[1].linear", 0.09f);
        //frameRotationShader->setFloat("spotLight[1].quadratic", 0.03f);
        frameRotationShader->setFloat("spotLight[1].quadratic", 0.2f);
        frameGouraudShader->setFloat("spotLight[1].quadratic", 0.2f);
        frameRotationShader->setFloat("spotLight[1].cutOff", glm::cos(glm::radians(7.5f)));
        frameGouraudShader->setFloat("spotLight[1].cutOff", glm::cos(glm::radians(12.0f)));
        frameRotationShader->setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(11.25f)));
        frameGouraudShader->setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(18.0f)));

        frameRotationShader->setVec3("spotLight[1].kA", 0.0f, 0.0f, 0.0f);
        frameGouraudShader->setVec3("spotLight[1].kA", 0.0f, 0.0f, 0.0f);
        frameRotationShader->setVec3("spotLight[1].kD", 0.5f, 0.0f, 0.5f);
        frameGouraudShader->setVec3("spotLight[1].kD", 0.5f, 0.0f, 0.5f);
        frameRotationShader->setVec3("spotLight[1].kS", 0.5f, 0.0f, 0.5f);
        frameGouraudShader->setVec3("spotLight[1].kS", 0.5f, 0.0f, 0.5f);

        frameRotationShader->setInt("onlyUv", 1);
        frameGouraudShader->setInt("onlyUv", 1);
        glFrontFace(GL_CW);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, kevin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        frameRotationShader->setInt("uTexture.kD", 6);
        frameGouraudShader->setInt("uTexture.kD", 6);
        frameGouraudShader->setInt("uTex", 6);
        frameRotationShader->setInt("uvOn", isUvOn);
        frameGouraudShader->setInt("uvOn", isUvOn);
        frameRotationShader->setInt("onlyUv", 1);
        frameGouraudShader->setInt("onlyUv", 1);
        glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
        glFrontFace(GL_CCW);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, belvedere);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        frameRotationShader->setInt("uTexture.kD", 7);
        frameGouraudShader->setInt("uTexture.kD", 7);
        frameGouraudShader->setInt("uTex", 7);
        frameRotationShader->setInt("uvOn", isUvOn);
        frameGouraudShader->setInt("uvOn", isUvOn);
        frameRotationShader->setInt("onlyUv", 1);
        frameGouraudShader->setInt("onlyUv", 1);
        glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

        frameRotationShader->setInt("isPointLightOn", isPointLightOn);
        frameGouraudShader->setInt("isPointLightOn", isPointLightOn);
        frameRotationShader->setInt("onlyUv", 0);
        frameGouraudShader->setInt("onlyUv", 0);
        //phongShader->setVec3("spotLight.pos", cameraPos[0], cameraPos[1], cameraPos[2]);
        frameRotationShader->setVec3("spotLight[0].direction", 0.0f, -0.12f, 0.5f);
        frameRotationShader->setInt("uvOn", isUvOn);
        frameGouraudShader->setInt("uvOn", isUvOn);
        frameRotationShader->setInt("isTexture", 1);
        frameGouraudShader->setInt("isTexture", 1);
        frameGouraudShader->setVec3("spotLight[0].direction", 0.0f, -0.12f, 0.5f);
        frameRotationShader->setFloat("spotLight[0].constant", 1.0f);
        frameGouraudShader->setFloat("spotLight[0].constant", 1.0f);
        frameRotationShader->setFloat("spotLight[0].linear", 0.15f);
        frameGouraudShader->setFloat("spotLight[0].linear", 0.15f);
        frameRotationShader->setFloat("spotLight[0].quadratic", 0.08f);
        frameGouraudShader->setFloat("spotLight[0].quadratic", 0.08f);
        frameRotationShader->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(27.5f)));
        frameGouraudShader->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(27.5f)));
        frameRotationShader->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(41.25f)));
        frameGouraudShader->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(41.25f)));
        frameRotationShader->setVec3("spotLight[0].kA", 0.05f, 0.05f, 0.05f);
        frameGouraudShader->setVec3("spotLight[0].kA", 0.05f, 0.05f, 0.05f);
        frameRotationShader->setVec3("spotLight[0].kD", 0.6f, 0.6f, 0.6f);
        frameGouraudShader->setVec3("spotLight[0].kD", 0.6f, 0.6f, 0.6f);
        frameRotationShader->setVec3("spotLight[0].kS", 0.6f, 0.6f, 0.6f);
        frameGouraudShader->setVec3("spotLight[0].kS", 0.6f, 0.6f, 0.6f);
        frameRotationShader->setInt("isPointLightOn", isPointLightOn);
        frameGouraudShader->setInt("isPointLightOn", isPointLightOn);
        frameRotationShader->setInt("uvOn", isUvOn);
        frameGouraudShader->setInt("uvOn", isUvOn);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, monaLisa);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        frameRotationShader->setInt("uTexture.kD", 2);
        frameGouraudShader->setInt("uTexture.kD", 2);
        frameRotationShader->setInt("isTexture", 1);
        frameGouraudShader->setInt("isTexture", 1);
        frameRotationShader->setVec3("objectColor", 1.0f, 1.0f, 1.0f);
        frameGouraudShader->setVec3("objectColor", 1.0f, 1.0f, 1.0f);
        frameGouraudShader->setInt("uTex", 2);
        frameRotationShader->setInt("onlyUv", 0);
        frameGouraudShader->setInt("onlyUv", 0);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, medusa);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        frameRotationShader->setInt("uTexture.kD", 3);
        frameGouraudShader->setInt("uTexture.kD", 3);
        frameGouraudShader->setInt("uTex", 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, liberty);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        frameRotationShader->setInt("uTexture.kD", 4);
        frameGouraudShader->setInt("uTexture.kD", 4);
        frameGouraudShader->setInt("uTex", 4);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, napoleon);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        frameRotationShader->setInt("uTexture.kD", 5);
        frameGouraudShader->setInt("uTexture.kD", 5);
        frameGouraudShader->setInt("uTex", 5);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
        
        frameRotationShader->setInt("onlyUv", 0);
        frameGouraudShader->setInt("onlyUv", 0);

        frameRotationShader->setInt("isTexture", 0);
        frameGouraudShader->setInt("isTexture", 0);
        glFrontFace(GL_CW);
        glBindVertexArray(VAO[2]);
        frameRotationShader->setVec3("objectColor", 1.0f, time, time);
        frameGouraudShader->setVec3("objectColor", 1.0f, time, time);
        frameRotationShader->setInt("onlyUv", 0);
        frameGouraudShader->setInt("onlyUv", 0);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 10, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 11, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 12, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 13, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 14, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 15, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 16, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 17, 3);

        frameRotationShader->setVec3("objectColor", 1.0f, time, 1.0f);
        frameGouraudShader->setVec3("objectColor", 1.0f, time, 1.0f);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 20, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 21, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 22, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 23, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 24, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 25, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 26, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 27, 3);
       // glDrawArrays(GL_TRIANGLE_STRIP, 20, 10);
        
        frameRotationShader->setVec3("objectColor", time, time, 1.0f);
        frameGouraudShader->setVec3("objectColor", time, time, 1.0f);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 30, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 31, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 32, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 33, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 34, 3);
        glFrontFace(GL_CW);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 35, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 36, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 37, 3);
        //glDrawArrays(GL_TRIANGLE_STRIP, 30, 10);
        /*glBindVertexArray(VAO[2]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(circle) / (8 * sizeof(float)));*/

        glBindVertexArray(VAO[4]);
        frameRotationShader->setVec3("objectColor", 1.0f, 0.0f, 0.0f);
        frameGouraudShader->setVec3("objectColor", 1.0f, 0.0f, 0.0f);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 1, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 2, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 3, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 5, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 6, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 7, 3);

        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 10, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 11, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 12, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 13, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 14, 3);
        glFrontFace(GL_CCW);
        glDrawArrays(GL_TRIANGLE_STRIP, 15, 3);
        glDrawArrays(GL_TRIANGLE_STRIP, 16, 3);
        glFrontFace(GL_CW);
        glDrawArrays(GL_TRIANGLE_STRIP, 17, 3);
        frameRotationShader->setVec2("uPos", 0, 0);
        frameGouraudShader->setVec2("uPos", 0, 0);

        glBindVertexArray(VAO[3]);
        if (buttonClicked) {
            frameRotationShader->setVec3("objectColor", 0.0f, 0.0f, 0.0f);
            frameGouraudShader->setVec3("objectColor", 0.0f, 0.0f, 0.0f);
        }
        else {
            frameRotationShader->setVec3("objectColor", 1.0f, 0.0f, 1.0f);
            frameGouraudShader->setVec3("objectColor", 1.0f, 0.0f, 1.0f);
        }
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, CRES+2);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(VAO[6]);

        basicShader->use();
        glDisable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, name);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        basicShader->setInt("uTex", 9);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glFrontFace(GL_CCW);
        unifiedShader.use();
        unifiedShader.setVec3("uLightPos", 0.0, 1, 3);
        unifiedShader.setVec3("uViewPos", 0, 0, 5);
        unifiedShader.setVec3("uLightColor", 1, 1, 1);
        unifiedShader.setMat4("uP", projection);
        unifiedShader.setMat4("uV", view);
        unifiedShader.setMat4("uM", modelLight);
        lightCube.Draw(unifiedShader);

        
        

        //phongShader->use();

        glDisable(GL_BLEND);

        /*if (!buttonClicked) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }*/

        //glEnable(GL_DEPTH_TEST);

        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);

        //Odstranjivanje lica (Prethodno smo podesili koje lice uklanjamo sa glCullFace)
        /*if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            glEnable(GL_CULL_FACE);
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            glDisable(GL_CULL_FACE);
        }*/
        //std::cout << glm::length(glm::vec3(cameraPos[0], cameraPos[1], cameraPos[2]) - cubeCenter) << "\n";
        
        glBindVertexArray(0);
        glUseProgram(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ POSPREMANJE +++++++++++++++++++++++++++++++++++++++++++++++++


    glDeleteBuffers(6, VBO);
    glDeleteVertexArrays(6, VAO);
    //glDeleteProgram(phongShader);

    glfwTerminate();
    return 0;
}

bool checkCamera(glm::vec3 temp) {
    if (temp.x < minX || temp.x > maxX ||
        temp.y < minY || temp.y > maxY ||
        temp.z < minZ || temp.z > maxZ) {
        return false;
    }
    return true;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
    phongShader->use();
    phongShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
    gouraudShader->use();
    gouraudShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
    //frameRotationShader->use();
    //frameRotationShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
    //frameGouraudShader->use();
    //frameGouraudShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
}

void call(glm::mat4 model, glm::mat4 projection, glm::mat4 view, int isUvOn, int isPointLightOn) {
    phongShader->use();
    phongShader->setInt("uvOn", isUvOn);
    phongShader->setInt("isPointLightOn", isPointLightOn);
    
    phongShader->setMat4("uM", model);
    phongShader->setMat4("uP", projection);
    phongShader->setMat4("uV", view);
    phongShader->setVec3("uViewPos", cameraPos[0], cameraPos[1], cameraPos[2]);
    phongShader->setFloat("uMaterial.shine", 0.6 * 128);
    phongShader->setVec3("uMaterial.kA", 0.1745, 0.01175, 0.1175);
    phongShader->setVec3("uMaterial.kD", 0.61424, 0.04136, 0.04136);
    phongShader->setVec3("uMaterial.kS", 0.72811, 0.626959, 0.62959);

    phongShader->setVec3("spotLight[0].pos", 0.0, 0.45f * 2, 0.0);
    //phongShader->setVec3("spotLight.pos", cameraPos[0], cameraPos[1], cameraPos[2]);
    phongShader->setVec3("spotLight[0].direction", 0.0f, -0.12f, 0.5f);
    phongShader->setFloat("spotLight[0].constant", 1.0f);
    phongShader->setFloat("spotLight[0].linear", 0.09f);
    phongShader->setFloat("spotLight[0].quadratic", 0.03f);
    phongShader->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(27.5f)));
    phongShader->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(41.25f)));
    phongShader->setVec3("spotLight[0].kA", 0.0f, 0.0f, 0.0f);
    phongShader->setVec3("spotLight[0].kD", 1.0f, 1.0f, 1.0f);
    phongShader->setVec3("spotLight[0].kS", 1.0f, 1.0f, 1.0f);

   

    phongShader->setVec3("spotLight[1].pos", cameraPos[0], cameraPos[1], cameraPos[2]);
    //std::cout << "front" << cameraFront[1] << "\n";

    phongShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
    phongShader->setFloat("spotLight[1].constant", 1.0f);
    phongShader->setFloat("spotLight[1].linear", 0.2f);
    phongShader->setFloat("spotLight[1].linear", 0.5f);
    phongShader->setFloat("spotLight[1].quadratic", 0.1f);
    phongShader->setFloat("spotLight[1].quadratic", 0.2f);
    phongShader->setFloat("spotLight[1].cutOff", glm::cos(glm::radians(7.5f)));
    phongShader->setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(11.25f)));
    phongShader->setVec3("spotLight[1].kA", 0.0f, 0.0f, 0.0f);
    phongShader->setVec3("spotLight[1].kD", 0.5f, 0.0f, 0.5f);
    phongShader->setVec3("spotLight[1].kS", 0.5f, 0.0f, 0.5f);

    phongShader->setVec3("pointLight.pos", 0.0f, -0.5f, 0.0f);
    phongShader->setVec3("pointLight.kA", 0.05f, 0.05f, 0.05f);
    phongShader->setVec3("pointLight.kD", 0.3f, 0.3f, 0.0f);
    phongShader->setVec3("pointLight.kS", 0.3f, 0.3f, 0.0f);
    phongShader->setFloat("pointLight.constant", 1.0f);
    phongShader->setFloat("pointLight.linear", 0.09f);
    phongShader->setFloat("pointLight.quadratic", 0.032f);
}

void call_gouraud(glm::mat4 model, glm::mat4 projection, glm::mat4 view, int isUvOn, int isPointLightOn) {
    gouraudShader->use();
    gouraudShader->setInt("uvOn", isUvOn);
    gouraudShader->setInt("isPointLightOn", isPointLightOn);

    gouraudShader->setMat4("uM", model);
    gouraudShader->setMat4("uP", projection);
    gouraudShader->setMat4("uV", view);
    gouraudShader->setVec3("uViewPos", cameraPos[0], cameraPos[1], cameraPos[2]);
    gouraudShader->setFloat("uMaterial.shine", 0.6 * 128);
    gouraudShader->setVec3("uMaterial.kA", 0.1745, 0.01175, 0.1175);
    gouraudShader->setVec3("uMaterial.kD", 0.61424, 0.04136, 0.04136);
    gouraudShader->setVec3("uMaterial.kS", 0.72811, 0.626959, 0.62959);

    gouraudShader->setVec3("spotLight[0].pos", 0.0, 0.45f * 2, 0.0);
    //phongShader->setVec3("spotLight.pos", cameraPos[0], cameraPos[1], cameraPos[2]);
    gouraudShader->setVec3("spotLight[0].direction", 0.0f, -0.12f, 0.5f);
    gouraudShader->setFloat("spotLight[0].constant", 1.0f);
    gouraudShader->setFloat("spotLight[0].linear", 0.09f);
    gouraudShader->setFloat("spotLight[0].quadratic", 0.03f);
    gouraudShader->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(27.5f)));
    gouraudShader->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(41.25f)));
    gouraudShader->setVec3("spotLight[0].kA", 0.0f, 0.0f, 0.0f);
    gouraudShader->setVec3("spotLight[0].kD", 1.0f, 1.0f, 1.0f);
    gouraudShader->setVec3("spotLight[0].kS", 1.0f, 1.0f, 1.0f);

    gouraudShader->setVec3("spotLight[1].pos", cameraPos[0], cameraPos[1], cameraPos[2]);
    //std::cout << "front" << cameraFront[1] << "\n";

    gouraudShader->setVec3("spotLight[1].direction", cameraFront[0], cameraFront[1], cameraFront[2]);
    gouraudShader->setFloat("spotLight[1].constant", 1.0f);
    gouraudShader->setFloat("spotLight[1].linear", 0.2f);
    gouraudShader->setFloat("spotLight[1].quadratic", 0.1f);
    gouraudShader->setFloat("spotLight[1].cutOff", glm::cos(glm::radians(7.5f)));
    gouraudShader->setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(11.25f)));
    gouraudShader->setVec3("spotLight[1].kA", 0.0f, 0.0f, 0.0f);
    gouraudShader->setVec3("spotLight[1].kD", 0.5f, 0.0f, 0.5f);
    gouraudShader->setVec3("spotLight[1].kS", 0.5f, 0.0f, 0.5f);

    gouraudShader->setVec3("pointLight.pos", 0.0f, -0.5f, 0.0f);
    gouraudShader->setVec3("pointLight.kA", 0.05f, 0.05f, 0.05f);
    gouraudShader->setVec3("pointLight.kD", 0.3f, 0.3f, 0.0f);
    gouraudShader->setVec3("pointLight.kS", 0.3f, 0.3f, 0.0f);
    gouraudShader->setFloat("pointLight.constant", 1.0f);
    gouraudShader->setFloat("pointLight.linear", 0.09f);
    gouraudShader->setFloat("pointLight.quadratic", 0.032f);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    std::cout << "tuu";
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}



unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);
    
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
