
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"

/* 
GLM library for math  https://github.com/g-truc/glm 
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly. 
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_4_my_first_car", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewInit();

    printout_opengl_glsl_info();

	shader basic_shader;
    basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
	basic_shader.bind("uP");
	basic_shader.bind("uV");
	basic_shader.bind("uT");
	check_shader(basic_shader.vertex_shader);
	check_shader(basic_shader.fragment_shader);
    validate_shader_program(basic_shader.program);

	/* Set the uT matrix to Identity */
	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	/* create a  cube   centered at the origin with side 2*/
	renderable r_cube	= shape_maker::cube(0.5,0.6,0.0);

	/* create a  cylinder with base on the XZ plane, and height=2*/
	renderable r_cyl	= shape_maker::cylinder(30,0.2,0.1,0.5);

	/* create 3 lines showing the reference frame*/
	renderable r_frame	= shape_maker::frame(4.0);

	check_gl_errors(__LINE__, __FILE__);

	/* Transformation to setup the point of view on the scene */
	glm::mat4 proj = glm::perspective(glm::radians(45.f), 1.33f, 0.1f, 100.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 5, 10.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glEnable(GL_DEPTH_TEST);

	/*Initialize the matrix to implement the continuos rotation aroun the y axis*/
	glm::mat4 R = glm::mat4(1.f);

	int it = 0;
	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		it++;
		/*incremente the rotation by 0.01 radians*/
		R = glm::rotate(R, 0.5f, glm::vec3(0.f, 1.f, 0.f));

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 M = view*R;
        glUseProgram(basic_shader.program);
		glUniformMatrix4fv(basic_shader["uP"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(basic_shader["uV"], 1, GL_FALSE, &M[0][0]);
		check_gl_errors(__LINE__, __FILE__);
		

		/* render box and cylinders so that the look like a car */
		/* instructions: you need to set up the matrix to set as the uniform variable
		uT in the vertex shader (file basic.vert):
		glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, YOUR_MATRIX_GOES_HERE);
		before rendering every shape so that the shape is transformed as needed.
		You will have to render at least once the cube and four times the cylinder.
		Then again, you can free you immagination and make a more inventive drawing.
		*/
	 	r_cyl.bind();
	 	glDrawElements(GL_TRIANGLES,r_cyl.in, GL_UNSIGNED_INT, 0);

		r_cube.bind();
		glDrawElements(GL_TRIANGLES, r_cube.in, GL_UNSIGNED_INT, 0);
		/* ******************************************************/


		r_frame.bind();
		glDrawArrays(GL_LINES, 0, 6);

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
