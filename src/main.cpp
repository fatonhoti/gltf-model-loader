#include "glad.h"
#include <GLFW/glfw3.h>

#include <glm/gtx/transform.hpp>

#include <iostream>
#include <filesystem>
#include <cassert>

#include "asset_manager.hpp"
#include "graphics_shader.hpp"
#include "camera.hpp"
#include "mesh.hpp"

GLFWwindow* window;
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 1024;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);

void init_glfw_glad();
void render_test_triangle();

int main()
{
    init_glfw_glad();

    Camera camera{};
    camera.origin = { 0.0f, 0.0f, 5.0f };
	camera.vertical_fov = 45.0f;
	camera.aspect_ratio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
	camera.near_plane = 0.001f;
	camera.far_plane = 150.0f;
    camera.movement_speed = 15.0f;
	camera.update_projection_matrix();
    glfwSetWindowUserPointer(window, &camera);

    GraphicsShader shader("default.vert", "default.frag");
    shader.use();

    if (!AssetManager::load_model("mazda_rx-7.glb", AssetManager::FILE_FORMAT::GLB)) {
        printf("Failed to load model :(\n");
        return EXIT_FAILURE;
    }

    float deltatime{ 0.0f };
    float last_frame{ 0.0f };
    while (!glfwWindowShouldClose(window))
    {
        const float current_frame = static_cast<float>(glfwGetTime());
		deltatime = current_frame - last_frame;
		last_frame = current_frame;

        const auto P = camera.projection_matrix;
        const auto V = camera.get_view_matrix();
        const auto PV = P * V;

        shader.set_mat4("u_PV", PV);
        shader.set_vec3("u_CameraPosition", camera.origin);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        for (const auto& [_, model] : AssetManager::models) {
            for (const auto& mesh : model.meshes) {
                shader.set_mat4("u_ModelMatrix", mesh.model_matrix);
                glBindVertexArray(mesh.VAO);
                glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(mesh.offset * sizeof(uint32_t)));
            }
        }

        //render_test_triangle(shader);
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.move(FORWARD, deltatime);
    
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.move(BACKWARD, deltatime);
    
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.move(LEFT, deltatime);
    
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.move(RIGHT, deltatime);
    
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.move(UP, deltatime);
    
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.move(DOWN, deltatime);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    camera->update_projection_matrix();
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        static bool wireframe_mode = false;
        wireframe_mode = !wireframe_mode;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->mouse_callback(xpos, ypos);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    double cursor_x, cursor_y;
	glfwGetCursorPos(window, &cursor_x, &cursor_y);

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	camera->mouse_button_callback(cursor_x, cursor_y, button, action, mods);
}

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------\n";
    std::cout << "Debug message (" << id << "): " << message << "\n";

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << "\n";

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << "\n";

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << "\n";
    std::cout << std::endl;
}

void init_glfw_glad()
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "glTF-viewer", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD.\n");
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(135.0f/255.0f, 206.0f/255.0f, 235.0f/255.0f, 1.0f);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
}

void render_test_triangle(const Shader& shader)
{
    static GLuint VBO, VAO;
    if (VAO == 0) {
        // test triangle
        GLfloat vertices[] = {
            -0.5f, -0.5f, 0.0f, // left
             0.5f, -0.5f, 0.0f, // right
             0.0f,  0.5f, 0.0f  // top
        }; 
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }
    shader.use();
    glBindVertexArray(VAO);   
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
