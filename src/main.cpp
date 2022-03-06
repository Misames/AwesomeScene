#define M_PI 3.14159265358979323846
#if defined(_WIN32) && defined(_MSC_VER)
#elif defined(__APPLE__)
#elif defined(__linux__)
#endif

#include <iostream>

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Camera.hpp"
#include "GLShader.h"

using namespace std;
using namespace tinyobj;

// Texture
int width, height;

// Shader
GLShader mainShader;
GLShader skyboxShader;

// Model
attrib_t attribs;
vector<shape_t> shapes;
vector<material_t> materials;
vector<float> listData;
string warm, err;
bool ret = LoadObj(&attribs, &shapes, &materials, &warm, &err, "models/wolf.obj", "", true, false);
int indexVertex = 0;

// VAO , VBO et IBO de la skybox
GLuint skyboxVAO; // la structure d'attributs stockee en VRAM
GLuint skyboxVBO; // les vertices de l'objet stockees en VRAM
GLuint skyboxIBO; // les indices de l'objet stockees en VRAM

// Framebuffer
GLuint FBO;

void Initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
        cout << "erreur d'initialisation de GLEW!" << endl;

    cout << "Version : " << glGetString(GL_VERSION) << endl;
    cout << "Vendor : " << glGetString(GL_VENDOR) << endl;
    cout << "Renderer : " << glGetString(GL_RENDERER) << endl;

    mainShader.LoadVertexShader("shader/vertex.glsl");
    mainShader.LoadFragmentShader("shader/fragment.glsl");
    mainShader.Create();

    skyboxShader.LoadVertexShader("shader/SkyboxCubemap.vs");
    skyboxShader.LoadFragmentShader("shader/SkyboxCubemap.fs");
    skyboxShader.Create();

    //////////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                        /////////////////
    /////////////////                  CHARGER UN .OBJ                       /////////////////
    /////////////////                                                        /////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    for (auto &shape : shapes)
    {
        int index_offset = 0;
        for (int j = 0; j < shape.mesh.num_face_vertices.size(); j++)
        {
            int fv = shape.mesh.num_face_vertices[j];
            for (int k = 0; k < fv; k++)
            {
                index_t idx = shape.mesh.indices[index_offset + k];
                listData.push_back(attribs.vertices[3 * idx.vertex_index + 0]);
                listData.push_back(attribs.vertices[3 * idx.vertex_index + 1]);
                listData.push_back(attribs.vertices[3 * idx.vertex_index + 2]);

                if (!attribs.normals.empty())
                {
                    listData.push_back(attribs.normals[3 * idx.normal_index + 0]);
                    listData.push_back(attribs.normals[3 * idx.normal_index + 1]);
                    listData.push_back(attribs.normals[3 * idx.normal_index + 2]);
                }

                if (!attribs.texcoords.empty())
                {
                    listData.push_back(attribs.texcoords[2 * idx.texcoord_index + 0]);
                    listData.push_back(attribs.texcoords[2 * idx.texcoord_index + 1]);
                }
            }
            index_offset += fv;
            indexVertex += fv;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                        /////////////////
    /////////////////                  CHARGER UNE TEXTURE                   /////////////////
    /////////////////                                                        /////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    uint8_t *data = stbi_load("img/brick.png", &width, &height, nullptr, STBI_rgb_alpha);
    GLuint textureid;

    glGenTextures(1, &textureid);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                        /////////////////
    /////////////////                  SKYBOX                                /////////////////
    /////////////////                                                        /////////////////
    //////////////////////////////////////////////////////////////////////////////////////////

    float Skybox[] =
        {-1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,
         -1.0f, -1.0f, -1.0f,
         -1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, -1.0f,
         -1.0f, 1.0f, -1.0f};

    unsigned int SkyboxIndices[] =
        {// Droite
         1, 2, 6,
         6, 5, 1,
         // Gauche
         0, 4, 7,
         7, 3, 0,
         // Haut
         4, 5, 6,
         6, 7, 4,
         // Bas
         0, 3, 2,
         2, 1, 0,
         // Derriere
         0, 1, 5,
         5, 4, 0,
         // Devant
         3, 7, 6,
         6, 2, 3};

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxIBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Skybox), &Skybox, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxIndices), &SkyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //////////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                        /////////////////
    /////////////////                  FRAMEBUFFER                           /////////////////
    /////////////////                                                        /////////////////
    //////////////////////////////////////////////////////////////////////////////////////////

    // Gestion du framebuffer FBO
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // rétablie le FBO par défaut

    string PathFace[] =
        {"img/pisa_posx.jpg",
         "img/pisa_negx.jpg",
         "img/pisa_posy.jpg",
         "img/pisa_negy.jpg",
         "img/pisa_posz.jpg",
         "img/pisa_negz.jpg"};

    GLuint cubeMapText;
    glGenTextures(1, &cubeMapText);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapText);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; i++)
    {
        int width, height, nrChannels;
        unsigned char *data = stbi_load(PathFace[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
            cout << "erreur chargement d'une images de cubemap" << endl;
    }
}

void Shutdown()
{
    mainShader.Destroy();
    skyboxShader.Destroy();
    glDeleteFramebuffers(1, &FBO);
}

void Display(GLFWwindow *window, Camera cam)
{
    int offscreenWidth = 100, offscreenHeight = 100;
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glViewport(0, 0, offscreenWidth, offscreenHeight);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    uint32_t basicProgram = mainShader.GetProgram();
    glUseProgram(basicProgram);

    const GLint position = glGetAttribLocation(basicProgram, "a_position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, &listData[0]);

    const GLint texAttrib = glGetAttribLocation(basicProgram, "a_texcoords");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, &listData[0]);

    const int normal = glGetAttribLocation(basicProgram, "a_normal");

    static const int stride = sizeof(float) * 8;

    glEnableVertexAttribArray(normal);
    glVertexAttribPointer(normal, 3, GL_FLOAT, false, stride, &listData[0]);

    glUseProgram(basicProgram);

    float time = glfwGetTime();
    const int timeLocation = glGetUniformLocation(basicProgram, "u_time");
    glUniform1f(timeLocation, time);

    float rotationMatrix[] = {
        cosf(time), 0.f, -sinf(time), 0.0f, // 1ere colonne
        0.0f, 1.0f, 0.0f, 0.f,              // 2eme colonne
        sinf(time), 0.f, cosf(time), 0.0f,  // 3eme colonne
        0.0f, 0.0f, 0.0f, 1.0f              // 4eme colonne
    };

    const int rotationLocation = glGetUniformLocation(basicProgram, "u_rotationMatrix");
    glUniformMatrix4fv(rotationLocation, 1, GL_FALSE, rotationMatrix);

    float translationMatrix[] = {
        1.0f, 0.0f, 0.0f, 0.0f,            // 1ere colonne
        0.0f, 1.0f, 0.0f, 0.0f,            // 2eme colonne
        0.0f, 0.0f, 1.0f, 0.0f,            // 3eme colonne
        cosf(time), -100.0f, -350.0f, 1.0f // 4eme colonne
    };

    const int translationLocation = glGetUniformLocation(basicProgram, "u_translationMatrix");
    glUniformMatrix4fv(translationLocation, 1, GL_FALSE, translationMatrix);

    float znear = 0.1f, zfar = 1000.0f, fov = 45.0f;
    cam.Matrix(fov, znear, zfar, mainShader, "u_projectionMatrix");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.f, 1.f, 0.f, 1.f);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitNamedFramebuffer(GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER,
                           0, 0, 100, 100,
                           0, 0, width, height,
                           GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glDrawArrays(GL_TRIANGLES, listData[0], indexVertex);
}

static void error_callback(int error, const char *description)
{
    cout << "Error GFLW " << error << " : " << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
    GLFWwindow *window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(640, 480, "Awesome Scene", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    Initialize();

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    while (!glfwWindowShouldClose(window))
    {
        camera.Inputs(window);
        Display(window, camera);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Shutdown();
    glfwTerminate();

    return 0;
}