all:
	g++ lighthouse.cpp GLSL_helper.cpp MStackHelp.cpp -DGL_GLEXT_PROTOTYPES -lGL -lGLU -lglut
