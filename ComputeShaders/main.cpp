#include <iostream>
#define GLFW_DLL
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"


#include "glad/glad.h"
#include "glm.hpp"
#include <GLFW/glfw3.h>

#define PI 3.141592654

using namespace std;

struct Point
{
    glm::vec4 pos;
    glm::vec4 oldPos;
    glm::vec4 vel;
    glm::vec4 extra;
};

void onWindowResize(GLFWwindow* window, int width, int height);
void drawCircleBatch(Point* points, GLuint& program);
void drawRect(glm::vec4 rect, GLuint& program);

int winx = 480;
int winy = 480;

vector<Point> pts;
vector<glm::vec4> rects;

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
    float ratio = 2*PI / 200.f;
    for(int i =0; i < 20; i++)
    {
        pts.emplace_back(Point());
        pts[i].pos.x = -200 + i * 50;
        pts[i].pos.y = 5;
        pts[i].extra.x = 20.f;
        pts[i].oldPos.x = pts[i].pos.x;
        pts[i].oldPos.y = pts[i].pos.y;
        pts[i].vel.x = 0;
        pts[i].vel.y = 0;
        pts[i].extra.y= 1.f / 240.f;
    }
    rects.emplace_back(glm::vec4(-1000.f, -400.f, 2000.f, 105.f));
    
    const char *vertexShaderSource = "#version 460 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    
    const char *fragmentShaderSource = "#version 460 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\n";
    
    const char *computeShaderSource = 
        "#version 460 core\n"
        "layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;\n"
        "struct Point{\n"
        "vec4 pos;\n"
        "vec4 oldPos;\n"
        "vec4 vel;\n"
        "vec4 extra;\n"
        "};\n"
        "layout(std430, binding=0) buffer pts{\n"
        "Point p[];\n"
        "};\n"
        "layout(std430, binding=1) buffer rectangles{\n"
        "vec4 rects[];\n"
        "};\n"
        //"//uniform int rectCount;\n";
        "void main(){\n"
        "   uint i = gl_GlobalInvocationID.x;\n"
        "   float px = p[i].pos.x;\n"
        "   float py = p[i].pos.y;\n"
        "   p[i].vel.y -= 9.8;\n"
        "   float xdelta = p[i].pos.x * 2.0 - p[i].oldPos.x + p[i].vel.x * p[i].extra.y * p[i].extra.y;\n"
        "   float ydelta = p[i].pos.y * 2.0 - p[i].oldPos.y + p[i].vel.y * p[i].extra.y * p[i].extra.y;\n"
        "   p[i].pos.x = xdelta;\n"
        "   p[i].pos.y = ydelta;\n"
        "   p[i].oldPos.x = px;\n"
        "   p[i].oldPos.y = py;\n"
        "   for(int r = 0; r < 1; r++)\n"
        "   {\n"
        "       if(p[i].pos.x + p[i].extra.x > rects[r].x && p[i].pos.x - p[i].extra.x < rects[r].x + rects[r].z && p[i].pos.y - p[i].extra.x < rects[r].y)\n"
        "          p[i].pos.y = rects[r].y + p[i].extra.x;\n"
        "   }\n"
        "}\n";


    
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    
    GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &computeShaderSource, NULL);
    glCompileShader(compute);

    GLint isCompiled = 0;
    glfwSwapInterval(1);
    glGetShaderiv(compute, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(compute, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(compute, maxLength, &maxLength, &errorLog[0]);
        for(int i = 0; i < maxLength; i++)cout << errorLog[i];
        cout << endl;
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(compute); // Don't leak the shader.
        return 1;
    }

    GLuint program = glCreateProgram();
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, compute);
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glLinkProgram(computeProgram);
    //glDrawArray(GL_TRIANGLES, 0, 3);
    float f = 0;
    

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    double oldTime = 1.f;
    int maxFPS = 60;
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    
    GLuint ssboRects;
    glGenBuffers(1, &ssboRects);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboRects);

    size_t byteSizeRects = sizeof(glm::vec4) * rects.size();
    size_t byteSizePoints = sizeof(Point) * pts.size();
    
    while (!glfwWindowShouldClose(window))
    {
        double dt = glfwGetTime() - oldTime;
        oldTime = glfwGetTime();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        startTime = clock();
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizePoints, pts.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboRects);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizeRects, rects.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboRects);
        //GLuint loc = glGetUniformLocation(computeProgram, "rectCount");
        //glUniform1i(loc, rects.size());
        glUseProgram(computeProgram);
        glDispatchCompute(floor(pts.size()/ 8.f), 1, 1);
        //glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        Point* ptsNew = (Point*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byteSizePoints, GL_MAP_READ_BIT);
        for(int i = 0; i < pts.size(); i++)
            pts[i] = ptsNew[i];
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        drawCircleBatch(pts.data(), program);
        drawRect(rects[0], program);
        ImGui::Text(("fps:" + format("{:.0f}", 1.f / dt)).c_str());
        ImGui::SliderInt("max fps", &maxFPS, 15, 300);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteShader(fragment);
    glDeleteShader(vertex);
    glDeleteShader(compute);
    glfwTerminate();
    return 0;
}

void drawCircleBatch(Point* points, GLuint& program)
{
    int step = pts.size()* 9;
    int stepI = pts.size()* 3;
    float circle[pts.size()* 9 * pts.size()];
    GLuint indices[pts.size()* 3 * pts.size()];
    float ratio = (PI*2.f) / (float)pts.size();
    for(int pIndex=0; pIndex < pts.size(); pIndex++)
    {
        const Point& p = points[pIndex];
        float radius = p.extra.x / (float)winx;
        float centerX = p.pos.x / (float)winx;
        float centerY = p.pos.y / (float)winx;
        for(int i = 0; i < pts.size(); i++)
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
    glDrawElements(GL_TRIANGLES, pts.size()* 3 * pts.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void onWindowResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, height, height);
    winx = height;
    winy = height;
}

void drawRect(glm::vec4 rect, GLuint& program)
{
    float vertices[12] = 
    {
        rect.x/(float)winx, rect.y/(float)winx, 0.f,
        rect.x/(float)winx + rect.z/(float)winx, rect.y/(float)winx, 0.f,
        rect.x/(float)winx + rect.z/(float)winx, rect.y/(float)winx - rect.w/(float)winx, 0.f,
        rect.x/(float)winx, rect.y/(float)winx - rect.w/(float)winx, 0.f
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
