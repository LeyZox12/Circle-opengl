#include <iostream>
#define GLFW_DLL
#include <vector>
#include <cmath>
#include <chrono>

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"


#include "glad/glad.h"
#include "glm.hpp"
#include <GLFW/glfw3.h>

#define PI 3.141592654

using namespace std;

const int POINT_COUNT = 40;

struct Point
{
    float x;
    float y;
    float radius;
};

struct Rect
{
    float x;
    float y;
    float w;
    float h;
};
void save()
{
    cout << "button aint no way" << endl;
}
void onWindowResize(GLFWwindow* window, int width, int height);
void drawCircleBatch(const vector<Point> points, GLuint& program);
void drawRect(Rect rect, GLuint& program);
int main()
{
    if(!glfwInit())
        cout << "couldn't load glfw" << endl;
    GLFWwindow* window;
    window = glfwCreateWindow(480, 480, "Hello World", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, onWindowResize);   
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    clock_t startTime = clock();
    vector<Point> pts;
    float ratio = 2*PI / 200.f;
    for(int i =0; i < 20; i++)
    {
        pts.emplace_back(Point());
        pts[i].x = -1 + i * 0.1;
        pts[i].y = -0.5;
        pts[i].radius = 0.025f;
    }

    
    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    
    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\n";
    
    
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    //glDrawArray(GL_TRIANGLES, 0, 3);
    float f = 0;
    
    Rect rect;
    rect.x = -1.f;
    rect.y = -.95f;
    rect.w = 2.f;
    rect.h = -0.05f;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    double oldTime = 1.f;
    int maxFPS = 60;
    while (!glfwWindowShouldClose(window))
    {
        while(glfwGetTime() - oldTime < (double)(1.0 / (float)maxFPS)); 
        double dt = glfwGetTime() - oldTime;
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        startTime = clock();
        glClear(GL_COLOR_BUFFER_BIT);
        drawCircleBatch(pts, program);
        drawRect(rect, program);
        ImGui::Text(("fps:" + format("{:.0f}", 1.0 / dt)).c_str());
        ImGui::SliderInt("max fps", &maxFPS, 15, 10000);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
        oldTime = glfwGetTime();
    }
    glDeleteShader(fragment);
    glDeleteShader(vertex);
    glfwTerminate();
    return 0;
}

void drawCircleBatch(const vector<Point> points, GLuint& program)
{
    int step = POINT_COUNT * 9;
    int stepI = POINT_COUNT * 3;
    float circle[POINT_COUNT * 9 * points.size()];
    GLuint indices[POINT_COUNT * 3 * points.size()];
    float ratio = (PI*2.f) / (float)POINT_COUNT;
    for(int pIndex=0; pIndex < points.size(); pIndex++)
    {
        const Point& p = points[pIndex];
        float radius = p.radius;
        float centerX = p.x;
        float centerY = p.y;
        for(int i = 0; i < POINT_COUNT; i++)
        {
            int index = step * pIndex + i * 9;
            int i2 = stepI * pIndex + i * 3;
            indices[i2] = i2;
            indices[i2+1] = i2+1;
            indices[i2+2] = i2+2;
            circle[index] = centerX;
            circle[index+1] = centerY;
            circle[index+2] = 0;

            circle[index+3] = centerX + cos(ratio*i) * radius;
            circle[index+4] = centerY + sin(ratio*i) * radius;
            circle[index+5] = 0;

            circle[index+6] = centerX + cos(ratio*(i+1)) * radius;
            circle[index+7] = centerY + sin(ratio*(i+1)) * radius;
            circle[index+8] = 0;
        }
    }

    GLuint VAO;
    glGenBuffers(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW);
    //float* data = circle.data();


    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, POINT_COUNT * 3 * points.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void onWindowResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, height, height);
}

void drawRect(Rect rect, GLuint& program)
{
    float vertices[12] = 
    {
        rect.x, rect.y, 0.f,
        rect.x + rect.w, rect.y, 0.f,
        rect.x + rect.w, rect.y + rect.h, 0.f,
        rect.x, rect.y + rect.h, 0.f
    };

    GLuint indices[6] = 
    {
        0, 1, 2,
        0, 2, 3
    };
    GLuint VAO;
    glGenBuffers(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //float* data = circle.data();


    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
