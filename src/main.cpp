// GLEW_STATIC force le linkage statique
// c-a-d que le code de glew est directement injecte dans l'executable
#define GLEW_STATIC
#define TINYOBJLOADER_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "tiny_obj_loader.h"

#include <iostream>

// _WIN32 indique un programme Windows
// _MSC_VER indique la version du compilateur VC++
#if defined(_WIN32) && defined(_MSC_VER)
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#elif defined(__APPLE__)
#elif defined(__linux__)
#endif

using namespace std;
using namespace tinyobj;

void Initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
        std::cout << "erreur d'initialisation de GLEW!" << std::endl;

    // Logs
    std::cout << "Version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Vendor : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer : " << glGetString(GL_RENDERER) << std::endl;

    // get toutes les textures et models de la première frame
}

void Shutdown()
{
}

void Display(GLFWwindow *window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void error_callback(int error, const char *description)
{
    std::cout << "Error GFLW " << error << " : " << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                                                                                                                         /////////////////
    /////////////////                                                                  CHARGER UN .OBJ                                                                        /////////////////
    /////////////////                                                                                                                                                         /////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    attrib_t attribs;
    vector<shape_t> shapes;
    vector<material_t> materials;

    string warm;
    string err;

    bool ret = LoadObj(&attribs, &shapes, &materials, &warm, &err, "Lowpoly_tree_sample.obj", "", true, false);

    vector<float> listData;

    int index = 0;

    for (auto &shape : shapes)
    {

        int index_offset = 0;

        for (int j = 0; j < shape.mesh.num_face_vertices.size(); j++)
        {
            int fv = shape.mesh.num_face_vertices[j];

            for (int k = 0; k < fv; k++)
            {
                index_t idx = shape.mesh.indices[index_offset + k];

                // on stock les vertices de l'obj chargé
                listData.push_back(attribs.vertices[3 * idx.vertex_index + 0]);
                listData.push_back(attribs.vertices[3 * idx.vertex_index + 1]);
                listData.push_back(attribs.vertices[3 * idx.vertex_index + 2]);
                // on stock les normals de l'obj chargé
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
            index += fv;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////                                                                                                                                                         /////////////////
    /////////////////                                                                  CHARGER UN .OBJ                                                                        /////////////////
    /////////////////                                                                                                                                                         /////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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