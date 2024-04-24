#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "indexed", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewInit();

    printout_opengl_glsl_info();
    GLuint va;
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    /* create render data in RAM */
    GLuint positionAttribIndex = 0;
    float positions[] = {   0.0, 0.0,  // 1st vertex
                            0.5, 0.0,  // 2nd vertex
                            0.5, 0.5,  // 3nd vertex
                            0.0, 0.5  // 4th vertex
    };

    /* define and crate a renderable object */
    renderable r;
    r.create();
    r.add_vertex_attribute<float>(positions, 8, positionAttribIndex, 2);

    GLuint colorAttribIndex = 1;
    float colors[] =      { 1.0, 0.0, 0.0,  // 1st vertex
                            0.0, 1.0, 0.0,  // 2nd vertex
                            0.0, 0.0, 1.0,   // 3rd vertex
                            1.0, 1.0, 1.0   // 4th vertex
    };

    r.add_vertex_attribute<float>(colors, 12, colorAttribIndex, 3);


    GLuint indices[] = { 0,1,2,0,2,3 };
    r.add_indices< GLuint> (indices, 6, GL_TRIANGLES);

    shader basic_shader;
    basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
   
	/* use the program shader "program_shader" */
	glUseProgram(basic_shader.program);

    /* use the array and element buffers */
    r.bind();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(r().mode, r().count, r().itype, NULL);
        check_gl_errors(__LINE__,__FILE__);
 
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
	glUseProgram(0);

    glfwTerminate();
    return 0;
}
