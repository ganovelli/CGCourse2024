#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>

#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"
#include "../common/matrix_stack.h"
#include "../common/intersection.h"
#include "../common/trackball.h"
#include "../common/frame_buffer_object.h"

/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

int width, height;
/* light direction in world space*/
glm::vec4 Ldir;

trackball tb[2];
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;


/* object that will be rendered in this scene*/
renderable  r_plane;

/* program shaders used */
shader tex_shader;

matrix_stack stack;
float scaling_factor = 1.0;


/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
	if (action == GLFW_PRESS)
		curr_tb = 1 - curr_tb;

}
void print_info() {
}
unsigned int texture, inputmeshPos,inputmeshId;
void set_storage() {
	// texture size

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
		GL_FLOAT, NULL);
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	check_gl_errors(__LINE__, __FILE__);
}

std::string shaders_path = "../../src/code_15_gpu_raytracing/shaders/";
shader rt_shader;
 
int iTime_loc, uWidth_loc,  uBbox_loc;


int main(int argc, char ** argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	width = 1024;
	height = 1024;

		/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "code_15_gpu_raytracing", NULL, NULL);
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



	check_gl_errors(__LINE__, __FILE__);
	rt_shader.create_program((shaders_path + "glsl_raytracer.comp").c_str());
	check_gl_errors(__LINE__, __FILE__);

	set_storage();
	
	check_gl_errors(__LINE__, __FILE__);
	glUseProgram(rt_shader.program);
	glUniform1i(rt_shader["iTime"], 0 * clock());

	glDispatchCompute((unsigned int)width, (unsigned int)height, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	check_gl_errors(__LINE__, __FILE__);

	/* load the shaders */
	tex_shader.create_program((shaders_path + "tex.vert").c_str(), (shaders_path + "tex.frag").c_str());

	/* crete a rectangle*/
	shape s_plane;
	shape_maker::rectangle(s_plane, 10, 10);
	s_plane.to_renderable(r_plane);

	print_info();
	/* define the viewport  */
	glViewport(0, 0, width, height);

	/* avoid rendering back faces */
	// uncomment to see the plane disappear when rotating it
	glDisable(GL_CULL_FACE);

	tb[0].reset();
	tb[0].set_center_radius(glm::vec3(0, 0, -4), 1.f);
	curr_tb = 0;

	proj = glm::frustum(-1.f, 1.f, -1.f, 1.f, 2.f, 100.f);
	view = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 proj_inv = glm::inverse(proj);
	glm::mat4 view_inv = glm::inverse(view);


	glUseProgram(rt_shader.program);
	glUniformMatrix4fv(rt_shader["uProj_inv"], 1, GL_FALSE, &proj_inv[0][0]);
	glUniform2i(rt_shader["uResolution"], width, height);

	frame_buffer_object fbo;
	fbo.create(width, height);
	
	int nf = 0;
	int cstart = clock();
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
                if (clock() - cstart > CLOCKS_PER_SEC ) {
			std::cout << nf << std::endl;
			nf = 0;
			cstart = clock();
		}
		nf++;

		/* Render here */
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		check_gl_errors(__LINE__, __FILE__, false);

		

		glUseProgram(rt_shader.program);
	 	glUniformMatrix4fv(rt_shader["uView_inv"], 1, GL_FALSE, &view_inv[0][0]);

		stack.push();
		stack.mult(tb[0].matrix());
		
 		glUniformMatrix4fv(rt_shader["uModel_inv"], 1, GL_FALSE, &glm::inverse(stack.m())[0][0]);
		stack.pop();

		if (tb[0].is_changed()) {
			glBindFramebuffer(GL_FRAMEBUFFER,fbo.id_fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		check_gl_errors(__LINE__, __FILE__, false);
		glUseProgram(rt_shader.program);
		glUniform1i(iTime_loc, clock());
		glDispatchCompute((unsigned int)width/32, (unsigned int)height/32, 1);
		check_gl_errors(__LINE__, __FILE__,false);
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		glUseProgram(tex_shader.program);
		glUniform1i(tex_shader["tex"], 0);
		glUniformMatrix4fv(tex_shader["uModel"], 1, GL_FALSE, &glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))[0][0]);
		r_plane.bind();
		glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);
		glUseProgram(0);

		
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
