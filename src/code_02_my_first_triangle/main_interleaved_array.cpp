#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
 

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_1_my_first_triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewInit();

    printout_opengl_glsl_info();

	/* create render data in RAM */
	GLuint positionAttribIndex = 0;
	float positions[] = {	0.0, 0.0,  // 1st vertex
							0.5, 0.0,  // 2nd vertex
							0.5, 0.5
	};

	GLuint colorAttribIndex = 1;
	float colors[] = {	1.0, 0.0, 0.0,  // 1st vertex
						0.0, 1.0, 0.0,  // 2nd vertex
						0.0, 0.0, 1.0
	};

	GLuint positionColorBuffer;
	glCreateBuffers(1, &positionColorBuffer);

	std::vector<float> pc_buf;
	for (int i = 0; i < 3; ++i) {
		pc_buf.push_back(positions[i * 2]);
		pc_buf.push_back(positions[i * 2 + 1]);
		pc_buf.push_back(colors[i * 3]);
		pc_buf.push_back(colors[i * 3+1]);
		pc_buf.push_back(colors[i * 3+2]);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, positionColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 15, &pc_buf[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(positionAttribIndex);
	glEnableVertexAttribArray(colorAttribIndex);
	check_gl_errors(__LINE__, __FILE__);
	
	glVertexAttribPointer(positionAttribIndex, 2, GL_FLOAT, false, 20, 0);
	glVertexAttribPointer(colorAttribIndex, 3, GL_FLOAT, false, 20, (void*)8);
	check_gl_errors(__LINE__, __FILE__);

/*  \BEGIN IGNORATE DA QUI IN POI */
	/* create a vertex shader */
	std::string  vertex_shader_src = "#version 330\n \
        in vec2 aPosition;\
        in vec3 aColor;\
        out vec3 vColor;\
        void main(void)\
        {\
         gl_Position = vec4(aPosition, 0.0, 1.0);\
         vColor = aColor;\
        }\
       ";
	const GLchar* vs_source = (const GLchar*)vertex_shader_src.c_str();
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vs_source, NULL);
	glCompileShader(vertex_shader);
	check_shader(vertex_shader);

	/* create a fragment shader */
	std::string   fragment_shader_src = "#version 330 \n \
        out vec4 color;\
        in vec3 vColor;\
        void main(void)\
        {\
            color = vec4(vColor, 1.0);\
        }";
	const GLchar* fs_source = (const GLchar*)fragment_shader_src.c_str();
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fs_source, NULL);
	glCompileShader(fragment_shader);
	check_shader(fragment_shader);

	GLuint program_shader = glCreateProgram();
	glAttachShader(program_shader, vertex_shader);
	glAttachShader(program_shader, fragment_shader);
	glBindAttribLocation(program_shader, positionAttribIndex, "aPosition");
	glBindAttribLocation(program_shader, colorAttribIndex, "aColor");
	glLinkProgram(program_shader);
/*  \END IGNORATE  */


	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program_shader);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
