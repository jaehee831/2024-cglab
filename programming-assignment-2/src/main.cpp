#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


// Window dimensions
const GLuint WIDTH = 1280, HEIGHT = 720;
bool toggle = true;
bool cursorEvent = false;
std::string modelPath = "../bunny.obj";

struct Color {
    GLubyte r;
    GLubyte g;
    GLubyte b;
};

Color objectId = { 50, 50, 50 };

Color modelColor1 = { 0, 100, 100 }; 
Color modelColor2 = { 100, 100, 0 }; 

void handleMouseClick(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double cursorX, cursorY;
        glfwGetCursorPos(window, &cursorX, &cursorY); 

        glReadBuffer(GL_BACK); 
        GLubyte detectedColor[3];
        glReadPixels(cursorX, HEIGHT - cursorY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &detectedColor[0]); 
       
        bool isObjectHit = detectedColor[0] == objectId.r && detectedColor[1] == objectId.g && detectedColor[2] == objectId.b;
        if (isObjectHit) {
            cursorEvent = true;
            toggle = !toggle;
        }
        else {
            cursorEvent = false; 
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

struct Vertex {
    GLfloat x, y, z;
};

std::vector<Vertex> extractVertices(const tinyobj::attrib_t& attrib) {
    std::vector<Vertex> vertices;
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        vertices.push_back({ attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2] });
    }
    return vertices;
}

void drawMeshWithStructs(const std::vector<Vertex>& vertices, const std::vector<tinyobj::shape_t>& shapes) {
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glVertex3f(vertices[index.vertex_index].x,
                vertices[index.vertex_index].y,
                vertices[index.vertex_index].z);
        }
    }
}

void loadModel(const std::string& modelPath, tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes, std::vector<tinyobj::material_t>& materials) {
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }
}

void setColor(const Color& color) {
    glColor3ub(color.r, color.g, color.b);
}

int main()
{
    std::cout << "Starting GLFW context, OpenGL 3.1" << std::endl;
    
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "glskeleton", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, handleMouseClick);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    glm::mat4 matModel1 = glm::mat4(1.5f);
    glm::mat4 matModel2 = glm::mat4(1.5f);
    glm::mat4 matView = glm::lookAt(glm::vec3(0, 4, 4),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0));
    glm::mat4 matProj = glm::perspective(glm::radians(60.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    loadModel(modelPath, attrib, shapes, materials);
    std::vector<Vertex> extractedVertices = extractVertices(attrib);
    std::cout << "Successfully loaded " << modelPath <<std::endl;
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(&matProj[0][0]);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawBuffer(GL_BACK);
        glm::mat4 modelView1 = matView * matModel1;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(modelView1));
        std::vector<Vertex> extractedVertices = extractVertices(attrib);
        glBegin(GL_TRIANGLES);
        setColor(objectId);

        drawMeshWithStructs(extractedVertices, shapes);

        glEnd();

        glDrawBuffer(GL_FRONT);
        glClear(GL_COLOR_BUFFER_BIT);
        glm::mat4 modelView2 = matView * matModel2;
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(glm::value_ptr(modelView2));
        glBegin(GL_TRIANGLES);
        setColor(toggle
            ? modelColor2 : modelColor1);

        drawMeshWithStructs(extractedVertices, shapes);

        glEnd();

        glFinish();
    }
    glfwTerminate();
    return 0;
}