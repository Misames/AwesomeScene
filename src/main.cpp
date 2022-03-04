#if defined(_WIN32) && defined(_MSC_VER)
#elif defined(__APPLE__)
#elif defined(__linux__)
#endif

#define M_PI 3.14159265358979323846

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <cmath>

#include "GLShader.h"
#include "Vertex.hpp"

using namespace std;
using namespace tinyobj;

// Window
int width, height;

// Shader
GLShader myShader;
GLShader SkyboxShader;

// Model 3D
attrib_t attribs;
vector<shape_t> shapes;
vector<material_t> materials;
vector<float> listData;
string warm, err;
bool ret = LoadObj(&attribs, &shapes, &materials, &warm, &err, "wolf.obj", "", true, false);
int indexVertex = 0;

// VAO , VBO et IBO de la skybox
GLuint SkyboxVAO; // la structure d'attributs stockee en VRAM
GLuint SkyboxVBO; // les vertices de l'objet stockees en VRAM
GLuint SkyboxIBO; // les indices de l'objet stockees en VRAM

void Initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
        cout << "erreur d'initialisation de GLEW!" << endl;

    cout << "Version : " << glGetString(GL_VERSION) << endl;
    cout << "Vendor : " << glGetString(GL_VENDOR) << endl;
    cout << "Renderer : " << glGetString(GL_RENDERER) << endl;

    myShader.LoadVertexShader("vertex.glsl");
    myShader.LoadFragmentShader("fragment.glsl");
    myShader.Create();

    SkyboxShader.LoadVertexShader("SkyboxCubemap.vs");
    SkyboxShader.LoadFragmentShader("SkyboxCubemap.fs");
    SkyboxShader.Create();

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

    //////////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                        /////////////////
    /////////////////                  CHARGER UNE TEXTURE                   /////////////////
    /////////////////                                                        /////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    uint8_t *data = stbi_load("brick.png", &width, &height, nullptr, STBI_rgb_alpha);
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

    glGenVertexArrays(1, &SkyboxVAO);
    glGenBuffers(1, &SkyboxVAO);
    glGenBuffers(1, &SkyboxVBO);
    glGenBuffers(1, &SkyboxIBO);
    glBindVertexArray(SkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, SkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Skybox), &Skybox, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SkyboxIBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxIndices), &SkyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    string PathFace[] = {
        "pisa_posx.jpg",
        "pisa_negx.jpg",
        "pisa_posy.jpg",
        "pisa_negy.jpg",
        "pisa_posz.jpg",
        "pisa_negz.jpg"};

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
        {
            cout << "erreur chargement d'une images de cubemap" << endl;
        }
    }
}

void Shutdown()
{
    myShader.Destroy();
    SkyboxShader.Destroy();
}

void Display(GLFWwindow *window)
{

    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    uint32_t basicProgram = myShader.GetProgram();
    glUseProgram(basicProgram);

    // rappel: stride du dragon = 8 * sizeof(float)
    const GLint POSITION = glGetAttribLocation(basicProgram, "a_position");
    glEnableVertexAttribArray(POSITION);
    glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, &listData[0]);

    const GLint texAttrib = glGetAttribLocation(basicProgram, "a_texcoords");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, &listData[0]);

    // static const int NORMAL = 1; // retro-ingenieurisez moi !
    // une valeur de -1 indique que le location n'existe pas
    const int NORMAL = glGetAttribLocation(basicProgram, "a_normal");

    static const int stride = sizeof(float) * 8;

    glEnableVertexAttribArray(NORMAL);
    // pour forcer la meme valeur a chaque sommet on utiliserait plutot
    // glVertexAttrib3f(NORMAL, 1.f, 1.f, 0.f);
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, false, stride, &listData[0]);

    // si le parametre de glUseProgram() vaut zero
    // on desactive les shaders
    // toujours appeler cette fonction avant les glDraw***
    glUseProgram(basicProgram);

    // on passe les uniform-s ici:
    // ici 1 float (1f)
    float time = glfwGetTime();
    const int timeLocation = glGetUniformLocation(basicProgram, "u_time");
    glUniform1f(timeLocation, time);

    // tout en mat4
    // Rotation autour de l'axe forward
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

    // fov=45°, aspect-ratio=width/height, znear=0.1, zfar=1000.0
    float fov = 45.0f;
    float radianFov = fov * (float)(M_PI / 180.0);
    float aspect = (float)width / (float)height;
    float znear = 0.1f, zfar = 1000.0f;
    float cot = 1.0f / tanf(radianFov / 2.0f);

    float projectionMatrix[] = {
        cot / aspect, 0.0f, 0.0f, 0.0f,                           // 1ere colonne
        0.0f, cot, 0.0f, 0.0f,                                    // 2eme colonne
        0.0f, 0.0f, -(znear + zfar) / (zfar - znear), -1.0f,      // 3eme colonne
        0.0f, 0.0f, -(2.0f * znear * zfar) / (zfar - znear), 0.0f // 4eme colonne
    };

    const int projectionLocation = glGetUniformLocation(basicProgram, "u_projectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, projectionMatrix);

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

    while (!glfwWindowShouldClose(window))
    {

        Display(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Shutdown();
    glfwTerminate();

    return 0;
}