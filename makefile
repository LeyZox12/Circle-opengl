CXXFLAGS = g++  -std=c++23 -I"include" -I"../../../glfw-3.4/include" -L"../../../glew-2.1.0/lib" -I../../../SFML-3.0.0/include -L../../../SFML-3.0.0/lib -I../../../glm -L"../../../glfw-3.4/lib"

final : main.o
	$(CXXFLAGS) main.o glad.o -o testing -lglfw3dll -lopengl32

main.o : main.cpp
	$(CXXFLAGS) main.cpp glad.c -c

