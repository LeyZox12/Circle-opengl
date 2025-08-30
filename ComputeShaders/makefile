CXXFLAGS = g++  -std=c++23 -I"include" -I"../../../glfw-3.4/include" -L"../../../glew-2.1.0/lib" -I../../../SFML-3.0.0/include -L../../../SFML-3.0.0/lib -I../../../glm -L"../../../glfw-3.4/lib" -I../../../imgui

GUIDIR = ../../../imgui

final : main.cpp glad.c 
	$(CXXFLAGS) main.cpp $(GUIDIR)/imgui.cpp $(GUIDIR)/imgui_draw.cpp $(GUIDIR)/imgui_tables.cpp $(GUIDIR)/imgui_widgets.cpp $(GUIDIR)/backends/imgui_impl_opengl3.cpp $(GUIDIR)/backends/imgui_impl_glfw.cpp $(GUIDIR)/imgui_demo.cpp glad.c -o testing -lglfw3dll -lopengl32
