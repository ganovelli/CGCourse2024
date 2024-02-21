#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\matrix_stack.h"

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
	

	/* create a  cube   centered at the origin with side 2*/
	renderable r_cube	= shape_maker::cube(0.5,0.3,0.0);

	/* create a  cylinder with base on the XZ plane, and height=2*/
	renderable r_cyl	= shape_maker::cylinder(30,0.2,0.1,0.5);

	/* create a  rectangle with base on the XY plane, centered at the origin and side 2*/
	renderable r_plane = shape_maker::quad();

	check_gl_errors(__LINE__, __FILE__);

	/* Transformation to setup the point of view on the scene */
	glm::mat4 proj = glm::perspective(glm::radians(45.f), 1.33f, 0.1f, 100.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 5, 10.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	

	/*Initialize the matrix to implement the continuos rotation aroun the y axis*/
	glm::mat4 R = glm::mat4(1.f);

	/*matrix to raise te car up (that is, along Y) */
	glm::mat4 raiseY = glm::translate(glm::mat4(1.f), glm::vec3(0, 0.5, 0.0));

	/* define a transformation that will be applyed the the body of the car */
	glm::mat4 rotcar = glm::translate(glm::mat4(1.f), glm::vec3(0.0, 0.0, -2.0));
	rotcar = glm::rotate(rotcar, -0.1f, glm::vec3(1.0, 0.0, 0.0));
	rotcar = glm::translate(rotcar, glm::vec3(0.0, 0.0, 2.0));
	
	glm::mat4 cube_to_car_body = glm::scale(rotcar, glm::vec3(1.0, 0.5, 2.0));
	cube_to_car_body = glm::translate(cube_to_car_body, glm::vec3(0.0, 1.0, 0.0));

	glm::mat4 cube_to_spoiler;
	glm::mat4 a1 = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 a2 = glm::shear(glm::mat4(1.0), glm::vec3(0.f), glm::vec2(0.f), glm::vec2(0.f), glm::vec2(0.f, 1.0));
	glm::mat4 a3 = glm::scale(glm::mat4(1.0), glm::vec3(1.0, 0.1, 2.0));
	glm::mat4 a4 = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 1.0, 0.0));
	cube_to_spoiler = rotcar*a1*a2*a3*a4;

	glm::mat4 cyl_to_wheel;
	glm::mat4 w1 = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -1.0, 0.0));
 	glm::mat4 w2 = glm::scale(glm::mat4(1.0), glm::vec3(0.5, 0.1, 0.5));
	glm::mat4 w3 = glm::rotate(glm::mat4(1.0), glm::radians(90.f),glm::vec3(0.0, 0.0, 1.0));
	cyl_to_wheel = w3*w2*w1;
	
	std::vector<glm::mat4> wheels;
	wheels.resize(4);
	wheels[0]= glm::translate(glm::mat4(1.0), glm::vec3(-1.15, 0.0, -1.0));;
	wheels[0] = wheels[0] *cyl_to_wheel;
	 
	wheels[1] = glm::translate(glm::mat4(1.0), glm::vec3( 1.15, 0.0, -1.0));;
	wheels[1] = wheels[1] * cyl_to_wheel;

	wheels[2] = glm::translate(glm::mat4(1.0), glm::vec3(-1.15, 0.0,  1.0));;
	wheels[2] = wheels[2] * cyl_to_wheel;

	wheels[3] = glm::translate(glm::mat4(1.0), glm::vec3( 1.15, 0.0, 1.0));;
	wheels[3] = wheels[3] * cyl_to_wheel;

	glm::mat4 model_plane = glm::rotate(glm::mat4(1.f),glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
	model_plane = glm::scale(model_plane, glm::vec3(3.f, 3.f, 1.f));

	glEnable(GL_DEPTH_TEST);
	glUseProgram(basic_shader.program);

	matrix_stack stack;

	float angle = 0;
	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		/* one full rotation every 6 seconds*/
		angle = (60.f*clock() / CLOCKS_PER_SEC);

		R = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 1.f, 0.f));

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(basic_shader["uRot"], 1, GL_FALSE, &R[0][0]);

        glUseProgram(basic_shader.program);
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		
		/* render box and cylinders so that the look like a car */
		r_cube.bind();
		stack.load_identity();

		stack.push(); 

		/* mult raiseY to the current matrix on the stack so it will be applied
		to the future matrices*/
		stack.mult(raiseY);

		stack.push();
		stack.mult(cube_to_car_body);
		/*draw the cube tranformed into the car's body*/
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 1.0,0.0,0.0);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		stack.push();
		stack.mult(cube_to_spoiler);
		/*draw the cube tranformed into the car's roof/spoiler */
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 1.0, 1.0, 0.0);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();


		/*draw the wheels */
		r_cyl.bind();
		glUniform3f(basic_shader["uColor"], 0.0, 0.0, 1.0);
		for (int iw = 0; iw < 4; ++iw) {
			stack.push();
			stack.mult(wheels[iw]);
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glDrawElements(r_cyl().mode, r_cyl().count, r_cyl().itype, 0);
			stack.pop();
		}

		stack.pop(); 

		r_plane.bind();
		glUniform3f(basic_shader["uColor"], 0.4, 0.3, 0.0);
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &model_plane[0][0]);
		glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
	glUseProgram(0);
    glfwTerminate();
    return 0;
}
