#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 1280, HEIGHT = 720;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    std::cout << "Starting GLFW context, OpenGL 3.1" << std::endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "glskeleton", NULL, NULL);
    glfwMakeContextCurrent(window);
    

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    bool colorChange = true;
    float color = 0.2f; 
    bool dimming = true;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        if (dimming)
        {
            color -= 0.00125f / 10;
            if (color <= 0.2f)
            {
                color = 0.2f;
                dimming = false;
                colorChange = !colorChange;
            }
        }
        else
        {
            color += 0.00125f / 10;
            if (color >= 0.7f)
            {
                color = 0.7f;
                dimming = true;
                colorChange = !colorChange;
            }
        }
 

        if (colorChange) {
            glColor3f(color, 0.3f, 0.3f); 
        }
        else {
            glColor3f(0.3f, color, 0.3f); 
        }

        float time = glfwGetTime();
        float angle = fmod(time, 2 * PI); 
        float cosA = cos(angle);
        float sinA = sin(angle);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

       
        glBegin(GL_TRIANGLES);
   
        glVertex3f(-0.5f * cosA - -0.5f * sinA, -0.5f * sinA + -0.5f * cosA, 0.0f);
        glVertex3f(0.5f * cosA - -0.5f * sinA, 0.5f * sinA + -0.5f * cosA, 0.0f);
        glVertex3f(0.0f * cosA - 0.5f * sinA, 0.0f * sinA + 0.5f * cosA, 0.0f);
        glEnd();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
}
