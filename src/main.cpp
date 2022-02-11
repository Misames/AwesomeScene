// GLEW_STATIC force le linkage statique
// c-a-d que le code de glew est directement injecte dans l'executable
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <iostream>
#include "GLShader.h"

// _WIN32 indique un programme Windows
// _MSC_VER indique la version du compilateur VC++
#if defined(_WIN32) && defined(_MSC_VER)
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#elif defined(__APPLE__)
#elif defined(__linux__)
#endif

GLShader g_BasicShader;

void Initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
        std::cout << "erreur d'initialisation de GLEW!" << std::endl;

    // Logs
    std::cout << "Version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Vendor : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer : " << glGetString(GL_RENDERER) << std::endl;

    // Shader
    g_BasicShader.LoadVertexShader("basic.vs");
    g_BasicShader.LoadFragmentShader("basic.fs");
    g_BasicShader.Create();
}

void Shutdown()
{
    g_BasicShader.Destroy();
}

void Display(GLFWwindow *window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // etape a. A vous de recuperer/passer les variables width/height
    glViewport(0, 0, width, height);

    // etape b. Notez que glClearColor est un etat, donc persistant
    glClearColor(0.5f, 0.5f, 0.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // etape c. on specifie le shader program a utiliser
    auto basicProgram = g_BasicShader.GetProgram();
    glUseProgram(basicProgram);

    // etape d.
    static const float triangle[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.0f, 0.5f};

    int loc_position = glGetAttribLocation(basicProgram, "a_position");
    glEnableVertexAttribArray(loc_position);
    glVertexAttribPointer(loc_position, 2, GL_FLOAT, false, sizeof(float) * 2, triangle);

    // etape e.

    // etape f. dessin de triangles dont la definition provient d’un tableau
    // le rendu s’effectue ici en prenant 3 sommets a partir du debut du tableau (0)
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // on suppose que la phase d’echange des buffers front et back
    // le « swap buffers » est effectuee juste apres
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

// voir https://www.glfw.org/documentation.html
// et https://www.glfw.org/docs/latest/quick.html
int main(void)
{
    GLFWwindow *window;

    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Premier Projet OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // defini la fonction de rappel utilisee lors de l'appui d'une touche
    glfwSetKeyCallback(window, key_callback);

    // toutes nos initialisations vont ici
    Initialize();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        Display(window);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // ne pas oublier de liberer la memoire etc...
    Shutdown();
    glfwTerminate();

    return 0;
}