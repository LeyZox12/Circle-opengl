#include <iostream>
#define GLFW_DLL
#include <vector>
#include <cmath>
#include <chrono>
#include <fstream>
#include <sstream>
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
void addPoint(glm::vec2 point, float radius);
bool readFileIntoString( const std::string &fileName, std::string &destination);

const int POINT_COUNT = 6;

int winx = 1480;
int winy = 1480;

vector<Point> pts;
vector<glm::vec4> rects;
vector<glm::vec4> acc;
vector<float> vertices;
vector<GLuint> indices;

GLuint VAO;
GLuint VBO;
GLuint EBO;

int main()
{
    if(!glfwInit())
        cout << "couldn't load glfw" << endl;
    GLFWwindow* window;
    window = glfwCreateWindow(winx, winy, "Hello World", NULL, NULL);
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
    for(int i =0; i < 10000; i++)
    {
        addPoint(glm::vec2(sin(i*0.6)*(winx -100), sin(i*0.01)*300), 7.5f);
    }
    rects.emplace_back(glm::vec4(-winx, winx, 100, -winx*2));
    rects.emplace_back(glm::vec4(-winx, -winx+100, winx*2, -155.f));
    rects.emplace_back(glm::vec4(winx-100, winx, 100, -winx*2));
    
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
    
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);
    
    string loadedFile = "";
    readFileIntoString("res/update.frag", loadedFile);
    const char* updateShaderSource = loadedFile.c_str();
    GLuint update = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(update, 1, &updateShaderSource, NULL);
    glCompileShader(update);

    loadedFile = "";
    readFileIntoString("res/collision.comp", loadedFile);
    const char* collisionShaderSource = loadedFile.c_str();
    GLuint collision = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(collision, 1, &collisionShaderSource, NULL);
    glCompileShader(collision);

    loadedFile = "";
    readFileIntoString("res/render.comp", loadedFile);
    const char* renderShaderSource = loadedFile.c_str();
    GLuint render = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(render, 1, &renderShaderSource, NULL);
    glCompileShader(render);
    GLint success = 0;
    glGetShaderiv(render, GL_COMPILE_STATUS, &success);
    cout << success<< endl;

    if (success != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(render, 1024, &log_length, message);
        cout << message << endl;
    }


    GLuint program = glCreateProgram();
    GLuint updateProgram = glCreateProgram();
    GLuint collisionProgram = glCreateProgram();
    GLuint renderProgram = glCreateProgram();
    glAttachShader(updateProgram, update);
    glAttachShader(collisionProgram, collision);
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glAttachShader(renderProgram, render);
    glLinkProgram(program);
    glLinkProgram(updateProgram);
    glLinkProgram(collisionProgram);
    glLinkProgram(renderProgram);
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

    GLuint ssboAcc;
    glGenBuffers(1, &ssboAcc);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboAcc);

    GLuint ssboVertices;
    glGenBuffers(1, &ssboVertices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);

    GLuint ssboIndices;
    glGenBuffers(1, &ssboIndices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);

    size_t byteSizeRects = sizeof(glm::vec4) * rects.size();
    
    float startClock = glfwGetTime();

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VAO);
    glGenBuffers(1, &EBO);

    while (!glfwWindowShouldClose(window))
    {
        double dt = glfwGetTime() - oldTime;
        oldTime = glfwGetTime();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        startTime = clock();
        if(glfwGetTime() - startClock > 0.01)
        {
            static int i = 0;
            //addPoint(glm::vec2(sin(i*0.6)*(winx -200), winx), 5.f);
            startClock = glfwGetTime();
        }
        size_t byteSizePoints = sizeof(Point) * pts.size();
        size_t byteSizeAcc = sizeof(glm::vec4) * pts.size();
        size_t byteSizeVertices = sizeof(float) * pts.size() * POINT_COUNT * 9;
        size_t byteSizeIndices = sizeof(GLuint) * pts.size() * POINT_COUNT * 3;

        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizePoints, pts.data(), GL_DYNAMIC_DRAW);
        glUseProgram(updateProgram);
        glDispatchCompute(ceil(pts.size()/ 8.f), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        Point* ptsUpdated = (Point*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byteSizePoints, GL_MAP_READ_BIT);
        for(int i = 0; i < pts.size(); i++)
        {
            pts[i] = ptsUpdated[i];
            acc[i] = glm::vec4(0, 0, 0, 0);
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizePoints, pts.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboRects);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizeRects, rects.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboRects);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboAcc);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizeAcc, acc.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboAcc);

        glUseProgram(collisionProgram);
        glDispatchCompute(ceil(pts.size()/ 8.f), ceil(pts.size()/ 8.f), 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        
        Point* ptsCollided = (Point*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byteSizePoints, GL_MAP_READ_BIT);

        pts.assign(ptsCollided, ptsCollided + pts.size());

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboAcc);
        glm::vec4* updatedAcc = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byteSizeAcc, GL_MAP_READ_BIT);
        for(int i = 0; i < pts.size(); i++)
        {
            pts[i].pos.x += updatedAcc[i].x;
            pts[i].pos.y += updatedAcc[i].y;
        }

        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizePoints, pts.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizeVertices, vertices.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboVertices);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
        glBufferData(GL_SHADER_STORAGE_BUFFER, byteSizeIndices, indices.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboIndices);

        glUseProgram(renderProgram);
        glDispatchCompute(ceil(pts.size()/ 8.f), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboVertices);
        float* computedVertices = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byteSizeVertices, GL_MAP_READ_BIT);
        vertices.assign(computedVertices, computedVertices + vertices.size());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIndices);
        GLuint* computedIndices = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, byteSizeIndices, GL_MAP_READ_BIT);
        indices.assign(computedIndices, computedIndices + indices.size());
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        //for(auto& v : indices) cout << v << endl;
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        float* vertDat = vertices.data();
        glBufferData(GL_ARRAY_BUFFER, byteSizeVertices, vertDat, GL_STATIC_DRAW);
        GLuint* iDat = indices.data();
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, byteSizeIndices, iDat, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, pts.size()* 3 * POINT_COUNT, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        //drawCircleBatch(pts.data(), program);
        for(int i = 0; i < 3; i++)
            drawRect(rects[i], program);
        ImGui::Text(("fps:" + format("{:.0f}", 1.f / dt)).c_str());
        ImGui::Text(("point count:" + to_string(pts.size())).c_str());
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteShader(fragment);
    glDeleteShader(vertex);
    glDeleteShader(update);
    glDeleteShader(collision);
    glfwTerminate();
    return 0;
}
void drawCircleBatch(Point* points, GLuint& program)
{
    int step = POINT_COUNT * 9;
    int stepI = POINT_COUNT * 3;
    vector<float> circle;
    vector<GLuint> indices;
    float ratio = (PI*2.f) / (float)POINT_COUNT;
    for(int pIndex=0; pIndex < pts.size(); pIndex++)
    {
        const Point& p = points[pIndex];
        float radius = p.extra.x / (float)winx;
        float centerX = p.pos.x / (float)winx;
        float centerY = p.pos.y / (float)winx;
        for(int i = 0; i < POINT_COUNT; i++)
        {
            int index = step * pIndex + i * 9;
            int i2 = stepI * pIndex + i * 3;
            indices.push_back(i2);
            indices.push_back(i2+1);
            indices.push_back(i2+2);
            circle.push_back(centerX);
            circle.push_back(centerY);
            circle.push_back(0);

            circle.push_back(centerX + cos(ratio*i) * radius);
            circle.push_back(centerY + sin(ratio*i) * radius);
            circle.push_back(0);

            circle.push_back(centerX + cos(ratio*(i+1)) * radius);
            circle.push_back(centerY + sin(ratio*(i+1)) * radius);
            circle.push_back(0);
        }
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float* dat = circle.data();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * POINT_COUNT * 9 * pts.size(), dat, GL_STATIC_DRAW);
    //float* data = circle.data();
    GLuint* idat = indices.data();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * pts.size() * 3 * POINT_COUNT, idat, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, pts.size()* 3 * POINT_COUNT, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void onWindowResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, height, height);
    winx = height;
    winy = height;
    rects[0] = (glm::vec4(-winx, winx, 100, -winx*2));
    rects[1] = (glm::vec4(-winx, -winx+100, winx*2, -155.f));
    rects[2] = (glm::vec4(winx-100, winx, 100, -winx*2));
}

void drawRect(glm::vec4 rect, GLuint& program)
{
    float vertices[12] = 
    {
        rect.x/(float)winx, rect.y/(float)winx, 0.f,
        rect.x/(float)winx + rect.z/(float)winx, rect.y/(float)winx, 0.f,
        rect.x/(float)winx + rect.z/(float)winx, rect.y/(float)winx + rect.w/(float)winx, 0.f,
        rect.x/(float)winx, rect.y/(float)winx + rect.w/(float)winx, 0.f
    };

    GLuint indices[6] = 
    {
        0, 1, 2,
        0, 2, 3
    };
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
    //float* data = circle.data();


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void addPoint(glm::vec2 pos, float radius)
{
        pts.emplace_back(Point());
        int i = pts.size()-1;
        pts[i].pos.x = pos.x;
        pts[i].pos.y = pos.y;
        pts[i].extra.x = radius;
        pts[i].oldPos.x = pts[i].pos.x;
        pts[i].oldPos.y = pts[i].pos.y;
        pts[i].vel.x = 0;
        pts[i].vel.y = 0;
        pts[i].extra.y= 1.f / 240.f;
        acc.emplace_back(glm::vec4(0, 0, 0, 0));
        vertices.resize(pts.size() * POINT_COUNT * 9);
        indices.resize(pts.size() * POINT_COUNT * 3);
}

bool readFileIntoString( const std::string &fileName, std::string &destination)
{
    std::ifstream in( fileName, std::ios::in | std::ios::binary );
    if ( in ) {
        in.seekg( 0, std::ios::end );
        destination.resize( in.tellg() );
        in.seekg( 0, std::ios::beg );
        in.read( &destination[ 0 ], destination.size() );
        in.close();
        return true;
    }
    return false;
}
