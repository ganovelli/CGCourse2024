#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"
#include "../common/matrix_stack.h"
#include <glm/ext/quaternion_float.hpp>

/* 
GLM library for math  https://github.com/g-truc/glm 
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly. 
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  

void out_mat(glm::mat4 m) {
	std::cout << std::endl;
	for (int i=0; i < 4; ++i) {
		for (int j=0; j < 4; ++j)
			std::cout << m[i][j] << " ";
		std::cout << std::endl;
	}

}

int main(void)
{

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_05_my_first_car", NULL, NULL);
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

	check_gl_errors(__LINE__, __FILE__);

	/* create a  cube   centered at the origin with side 2*/
	renderable r_cube	= shape_maker::cube(0.5,0.3,0.0);

	/* create a  cylinder with base on the XZ plane, and height=2*/
	renderable r_cyl	= shape_maker::cylinder(30,0.2,0.1,0.5);

	renderable r_plane = shape_maker::quad();

	renderable r_frame = shape_maker::frame();

	check_gl_errors(__LINE__, __FILE__);

	/* Transformation to setup the point of view on the scene */
	glm::mat4 proj = glm::perspective(glm::radians(30.f), 1.33f, 0.1f, 10.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glEnable(GL_DEPTH_TEST);


	/*Initialize the matrix to implement the continuos rotation aroun the y axis*/
	glm::mat4 R = glm::mat4(1.f);

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);


	glm::quat quaternion_A = glm::angleAxis(glm::radians(0.f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::quat quaternion_B = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 A = glm::mat4_cast(quaternion_A);
	A = glm::translate(glm::mat4(1.f), glm::vec3(0.01, 0.0, 0.0))*A;
	glm::mat4 B = glm::mat4_cast(quaternion_B);

	
	matrix_stack stack;
	float t_ = 0;
	float t;
	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		/* one full rotation every 6 seconds*/
        t_ = (clock() / float(CLOCKS_PER_SEC));
		float a = floor(t_);
		t = (((int)a)% 2==0)?t_-a:1-(t_-a);

		/* Render here */
		glClearColor(1, 1, 1,1.0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, 500, 400);
		r_frame.bind();
		glUniform1f(basic_shader["uScale"], 0.5);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &A[0][0]);
		glDrawArrays(GL_LINES, 0, 6);

		glUniform1f(basic_shader["uScale"], 0.5f);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &B[0][0]);
		glDrawArrays(GL_LINES, 0, 6);
		
		glm::mat4 Interp = A*(1 - t) + B*t;
		glUniform1f(basic_shader["uScale"], 1.f);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &Interp[0][0]);
		glDrawArrays(GL_LINES, 0, 6);

		stack.load_identity();
		stack.mult(Interp);
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2, 0.2, 0.2)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(1,1,1)));
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		r_cube.bind();
 		glDrawElements(r_cube().mode,r_cube().count,r_cube().itype , 0);

		glViewport(500, 0, 500, 400);
		r_frame.bind();
		glUniform1f(basic_shader["uScale"], 0.5);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &A[0][0]);
		glDrawArrays(GL_LINES, 0, 6);

		glUniform1f(basic_shader["uScale"], 0.5f);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &B[0][0]);
		glDrawArrays(GL_LINES, 0, 6);

		Interp = glm::mat3_cast(glm::slerp(quaternion_A, quaternion_B, t));
//		glm::mat4 Interp = A*(1 - t) + B*t;
		glUniform1f(basic_shader["uScale"], 1.f);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &Interp[0][0]);
		glDrawArrays(GL_LINES, 0, 6);

		stack.load_identity();
		stack.mult(Interp);
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2, 0.2, 0.2)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(1, 1, 1)));
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		r_cube.bind();
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);


		glViewport(0, 400, 500, 400);

		glViewport(500, 400, 500, 400);
		//	glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
	glUseProgram(0);
    glfwTerminate();
    return 0;
}
