#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_2_wrapping", NULL, NULL);
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
    float positions[] = {   -0, -0.0,	// 1st vertex
                            0.5, -0.0,  // 2nd vertex
                            0.5, 0.5,	// 3nd vertex
                            0.0, 0.5    // 4th vertex
    };
    renderable r;
    r.create();

	r.add_vertex_attribute<float>(positions, 8, positionAttribIndex, 2);
	check_gl_errors(__LINE__, __FILE__);

    GLuint colorAttribIndex = 1;
    float colors[] =      { 1.0, 0.0, 0.0,  // 1st vertex
                            0.0, 1.0, 0.0,  // 2nd vertex
                            0.0, 0.0, 1.0,  // 3rd vertex
                            1.0, 1.0, 1.0   // 4th vertex
    };
    r.add_vertex_attribute<float>(colors, 12, colorAttribIndex, 3);
	check_gl_errors(__LINE__, __FILE__);


    GLuint indices[] = { 0,1,2,0,2,3 };
    r.add_indices<GLuint>(indices, 3, GL_TRIANGLES);

	shader basic_shader;
    basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
	basic_shader.bind("uDelta");
	check_shader(basic_shader.vertex_shader);
	check_shader(basic_shader.fragment_shader);
    validate_shader_program(basic_shader.program);

	check_gl_errors(__LINE__, __FILE__);

	r.bind();

	int it = 0;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		it++;
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(basic_shader.program);

       /* update the value of uDelta in the fragment shader */
		glUniform1f(basic_shader["uDelta"], (it % 100) / 200.0);

        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
        check_gl_errors(__LINE__,__FILE__);
        glUseProgram(0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
