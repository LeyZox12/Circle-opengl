#include <iostream>
#include "glad/glad.h"
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include "glm.hpp"

#define PI 3.141592654

using namespace std;

const int POINT_COUNT = 40;

int main()
{
    float circle[POINT_COUNT * 9];
    GLuint indices[POINT_COUNT * 3];
    float ratio = (PI*2.f) / (float)POINT_COUNT;
    float radius = 1.f;
    float centerX = 0.f;
    float centerY = 0.f;
    for(int i = 0; i < POINT_COUNT; i++)
    {
        int index = i * 9;
        int i2 = i * 3;
        indices[i2] = i2;
        indices[i2+1] = i2+1;
        indices[i2+2] = i2+2;
        cout << i2 <<";" <<i2+1 << ";" << i2+2 << endl;;
        circle[index] = centerX;
        circle[index+1] = centerY;
        circle[index+2] = 0;

        circle[index+3] = centerX + cos(ratio*i) * radius;
        circle[index+4] = centerY + sin(ratio*i) * radius;
        circle[index+5] = 0;

        circle[index+6] = centerX + cos(ratio*(i+1)) * radius;
        circle[index+7] = centerY + sin(ratio*(i+1)) * radius;
        circle[index+8] = 0;
        cout << ratio * i << endl;
    }
    for(int k = 0; k < POINT_COUNT; k++)
    {
        for(int j = 0; j < 3; j++)
        {
            for(int i = 0; i < 3;i++)
            {
                cout << circle[k*9+j*3+i] <<";";
            }
            cout << endl;
        }
        cout << endl;
    }
    /*float circle[] = {
    -0.5f, -0.5f, 0.f,
    0.f, 0.5f, 0.f,
    0.5f, -0.5f, 0.f,

    };*/

    if(!glfwInit())
        cout << "couldn't load glfw" << endl;
    GLFWwindow* window;
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    
    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    
    const char *fragmentShaderSource = "version 330 core\n"
        "out vec4 FragColor\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
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
    //glDrawArray(GL_TRIANGLES, 0, 3);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, POINT_COUNT * 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteShader(fragment);
    glDeleteShader(vertex);
    glfwTerminate();
    return 0;
}
