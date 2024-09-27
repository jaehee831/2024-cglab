#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

const GLuint WIDTH = 1280, HEIGHT = 720;
glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
    start = glm::normalize(start);
    dest = glm::normalize(dest);
    float cosTheta = dot(start, dest);
    glm::vec3 rotationAxis;
    if (cosTheta < -1 + 0.001f) {
        rotationAxis = cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
        if (glm::length2(rotationAxis) < 0.01)
            rotationAxis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
        rotationAxis = normalize(rotationAxis);
        return glm::angleAxis(glm::radians(180.0f), rotationAxis);
    }
    rotationAxis = cross(start, dest);
    float s = sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;
    return glm::quat(s * 0.5f,
        rotationAxis.x * invs,
        rotationAxis.y * invs,
        rotationAxis.z * invs);
}

glm::vec3 get_trackball_vector(int x, int y) {
    glm::vec3 P = glm::vec3(1.0 * x / WIDTH * 2 - 1.0,
        1.0 * y / HEIGHT * 2 - 1.0,
        0);
    P.y = -P.y;
    float OP_squared = P.x * P.x + P.y * P.y;
    if (OP_squared <= 1 * 1)
        P.z = sqrt(1 * 1 - OP_squared);
    P = glm::normalize(P);
    return P;
}

glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lookatPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::quat rotQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

float speed = 0.5f;
const float min_fov = 1.0f;
const float max_fov = 120.0f;
float fov = 60.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void applyCameraTransformations(const glm::quat& rotTransform);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomSensitivity = -0.05f; // 줌 민감도, 음수로 방향을 반전시킴
    camPos += static_cast<float>(yoffset * zoomSensitivity) * camFront;
}

void onOffLight(GLenum light, bool state) {
    if (state) glEnable(light);
    else glDisable(light);
}

bool loadObjectMesh(const std::string& filePath, tinyobj::attrib_t& attr, std::vector<tinyobj::shape_t>& objShapes, std::vector<tinyobj::material_t>& objMaterials) {
    std::string warnings, errors;
    if (!tinyobj::LoadObj(&attr, &objShapes, &objMaterials, &warnings, &errors, filePath.c_str())) {
        std::cerr << "Error loading OBJ file: " << errors << std::endl;
        return false;
    }
    return true;
}

void displayMesh(const std::vector<tinyobj::shape_t>& objShapes, const tinyobj::attrib_t& attr) {
    for (size_t shapeIndex = 0; shapeIndex < objShapes.size(); ++shapeIndex) {
        size_t faceOffset = 0;
        const auto& positions = attr.vertices;
        const auto& normals = attr.normals;

        size_t numFaces = objShapes[shapeIndex].mesh.num_face_vertices.size();
        size_t currentFace = 0;

        while (currentFace < numFaces) {
            int verticesPerFace = objShapes[shapeIndex].mesh.num_face_vertices[currentFace];
            int v = 0;
            while (v < verticesPerFace) {
                auto index = objShapes[shapeIndex].mesh.indices[faceOffset + v];
                int vertexStart = 3 * index.vertex_index;
                int normalStart = 3 * index.normal_index;

                glm::vec3 vertexPoint(positions[vertexStart], positions[vertexStart + 1], positions[vertexStart + 2]);
                glm::vec3 normalDirection(normals[normalStart], normals[normalStart + 1], normals[normalStart + 2]);

                glNormal3f(normalDirection.x, normalDirection.y, normalDirection.z);
                glVertex3f(vertexPoint.x, vertexPoint.y, vertexPoint.z);
                v++;
            }
            faceOffset += verticesPerFace;
            currentFace++;
        }
    }
}

void setupLights(GLFWwindow* window) {
    glEnable(GL_LIGHTING);

    float pointLight[] = { 0.0, 10.0, 0.0, 1.0 };
    float La[] = { 0.1, 0.1, 0.1, 1.0 };
    float Ld[] = { 1.0, 1.0, 1.0, 1.0 };
    float Ls[] = { 1.0, 1.0, 1.0, 1.0 };
    float directionalLight[] = { 0.0, -10.0, 0.0, 0.0 };

    glLightfv(GL_LIGHT0, GL_POSITION, pointLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, La);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, Ld);
    glLightfv(GL_LIGHT0, GL_SPECULAR, Ls);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT1, GL_POSITION, directionalLight);
    glLightfv(GL_LIGHT1, GL_AMBIENT, La);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, Ld);
    glLightfv(GL_LIGHT1, GL_SPECULAR, Ls);
    glEnable(GL_LIGHT1);

    float ka[] = { 0.3, 0.8, 0.1, 1.0 };
    float kd[] = { 0.3, 0.8, 0.1, 1.0 };
    float ks[] = { 0.9, 0.9, 0.9, 1.0 };
    float shininess[] = { 30.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT, ka);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
    glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}


int main()
{
    std::cout << "Starting GLFW context, OpenGL 3.1" << std::endl;
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "glskeleton", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    if (!loadObjectMesh("../../bunny.obj", attrib, shapes, materials)) {

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glm::mat4 matModel = glm::identity<glm::mat4>();
    setupLights(window);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();

        // 색상과 깊이 버퍼 init
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMultMatrixf(glm::value_ptr(projection));

        glm::mat4 view = glm::lookAt(camPos, lookatPos, camUp);
        glm::mat4 modelView = view * matModel;
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMultMatrixf(glm::value_ptr(modelView));

        // 메시 draw 
        glBegin(GL_TRIANGLES);
        glColor3f(0.0f, 0.0f, 0.0f);
        displayMesh(shapes, attrib);
        glEnd();

        glfwSwapBuffers(window);
    }


    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    const int stateQ = glfwGetKey(window, GLFW_KEY_Q);
    const int stateW = glfwGetKey(window, GLFW_KEY_W);
    const int state1 = glfwGetKey(window, GLFW_KEY_1);
    const int state2 = glfwGetKey(window, GLFW_KEY_2);
    const int state3 = glfwGetKey(window, GLFW_KEY_3);
    const int state4 = glfwGetKey(window, GLFW_KEY_4);

    if (stateQ == GLFW_PRESS) {
        fov -= speed;
        fov = std::max(fov, min_fov);
    }
    else if (stateW == GLFW_PRESS) {
        fov += speed;
        fov = std::min(fov, max_fov);
    }

    if (state1 == GLFW_PRESS) onOffLight(GL_LIGHT0, true);
    if (state2 == GLFW_PRESS) onOffLight(GL_LIGHT0, false);
    if (state3 == GLFW_PRESS) onOffLight(GL_LIGHT1, true);
    if (state4 == GLFW_PRESS) onOffLight(GL_LIGHT1, false);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static glm::quat camDir = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    static glm::vec2 prevCursor(xpos, ypos);


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {

        glm::vec3 prev_pos = get_trackball_vector(prevCursor.x, prevCursor.y);
        glm::vec3 curr_pos = get_trackball_vector(xpos, ypos);
        glm::quat rotTransform = RotationBetweenVectors(prev_pos * rotQuat, curr_pos * rotQuat);
        glm::mat4 rotationMatrix = glm::toMat4(rotTransform);

        rotQuat *= rotTransform;
        camDir = glm::normalize(rotTransform * camDir);
        applyCameraTransformations(rotTransform);

    }

    prevCursor = glm::vec2(xpos, ypos);
}

void applyCameraTransformations(const glm::quat& rotTransform) {
    glm::mat4 rotMatrix = glm::mat4_cast(rotTransform);
    camPos = glm::vec3(glm::vec4(camPos - lookatPos, 1.0f) * rotMatrix) + lookatPos;
    camUp = glm::vec3(glm::vec4(camUp, 0.0f) * rotMatrix);
    camFront = glm::normalize(lookatPos - camPos);
}