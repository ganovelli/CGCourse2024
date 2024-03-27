#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "3dparty/nanosvg/src/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "3dparty/nanosvg/src/nanosvgrast.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\carousel\carousel.h"
#include "..\common\carousel\carousel_to_renderable.h"



#include "..\common\carousel\carousel_loader.h"

#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>
#include "..\common\matrix_stack.h"
#include "..\common\intersection.h"
#include "..\common\trackball.h"

trackball tb[2];
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

matrix_stack stack;
float scaling_factor = 1.0;


/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		tb[curr_tb].mouse_press(proj, view, xpos, ypos);
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			tb[curr_tb].mouse_release();
		}
}

/* callback function called when a mouse wheel is rotated */
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	if (curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	/* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
	if (action == GLFW_PRESS)
		curr_tb = 1 - curr_tb;
}



	int main(int argc, char** argv)
	{
		race r;
		
		carousel_loader::load("small_test.svg", "terrain_256.png",r);
		
		//add 10 cars
		for (int i = 0; i < 10; ++i)		
			r.add_car();

		GLFWwindow* window;

		/* Initialize the library */
		if (!glfwInit())
			return -1;

		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(800, 800, "CarOusel", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			return -1;
		}
		/* declare the callback functions on mouse events */
		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		glfwSetCursorPosCallback(window, cursor_position_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetKeyCallback(window, key_callback);

		/* Make the window's context current */
		glfwMakeContextCurrent(window);

		glewInit();

		printout_opengl_glsl_info();

		renderable fram = shape_maker::frame();

		renderable r_cube = shape_maker::cube();

		renderable r_track;
		r_track.create();
		game_to_renderable::to_track(r, r_track);

		renderable r_terrain;
		r_terrain.create();
		game_to_renderable::to_heightfield(r, r_terrain);
		
		renderable r_trees;
		r_trees.create();
		game_to_renderable::to_tree(r, r_trees);

		renderable r_lamps;
		r_lamps.create();
		game_to_renderable::to_lamps(r, r_lamps);

		shader basic_shader;
		basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");

		/* use the program shader "program_shader" */
		glUseProgram(basic_shader.program);
		
		/* define the viewport  */
		glViewport(0, 0, 800, 800);

		tb[0].reset();
		tb[0].set_center_radius(glm::vec3(0, 0, 0), 1.f);
		curr_tb = 0;

		proj = glm::perspective(glm::radians(45.f), 1.f, 1.f, 10.f);

		view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);

		r.start(11,0,0,600);
		r.update();

		 matrix_stack stack;

		 glEnable(GL_DEPTH_TEST);
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClearColor(0.3f, 0.3f, 0.3f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			check_gl_errors(__LINE__, __FILE__);

			r.update();
			stack.load_identity();
			stack.push();
			stack.mult(tb[0].matrix());
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
			fram.bind();
			glDrawArrays(GL_LINES, 0, 6);

			glColor3f(0, 0, 1);
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
			glEnd();


			float s = 1.f/r.bbox().diagonal();
			glm::vec3 c = r.bbox().center();

			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
			stack.mult(glm::translate(glm::mat4(1.f), -c));

			 
			glDepthRange(0.01, 1);
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 1, 1, 1.0);
			r_terrain.bind();
			glDrawArrays(GL_POINTS, 0, r_terrain.vn);
			glDepthRange(0.0, 1);

			for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
				stack.push();
				stack.mult(r.cars()[ic].frame);
				stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0,0.1,0.0)));
				glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
				glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
				fram.bind();
				glDrawArrays(GL_LINES, 0, 6);
				stack.pop();
			}

			fram.bind();
			for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic) {
				stack.push();
				stack.mult(r.cameramen()[ic].frame);
				stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(4, 4,4)));
				glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
				glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
				glDrawArrays(GL_LINES, 0, 6);
				stack.pop();
			}
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	
			r_track.bind();
			glPointSize(3.0);
			glUniform3f(basic_shader["uColor"], 0.2f, 0.3f, 0.2f);
			glDrawArrays(GL_LINE_STRIP, 0, r_track.vn);
			glPointSize(1.0);


			r_trees.bind();
			glUniform3f(basic_shader["uColor"], 0.f, 1.0f, 0.f);
			glDrawArrays(GL_LINES, 0, r_trees.vn);


			r_lamps.bind();
			glUniform3f(basic_shader["uColor"], 1.f, 1.0f, 0.f);
			glDrawArrays(GL_LINES, 0, r_lamps.vn);




			stack.pop();
			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}
		glUseProgram(0);
		glfwTerminate();
		return 0;
	}

 
