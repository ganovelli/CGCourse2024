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

/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

int width,height;

/* projection matrix*/
glm::mat4 proj;
float near = 2.0;

/* view matrix and view_frame*/
glm::mat4 view, view_frame;

/* a bool variable that indicates if we are currently rotating the trackball*/
bool is_trackball_dragged;

/* p0 and p1 points on the sphere (just like in the slides) */
glm::vec3 p0, p1;

/* matrix to transform the scene according to the trackball */
glm::mat4 trackball_matrix;

float scaling_factor = 1.f;
glm::mat4  scaling_matrix;

glm::vec3 viewport_to_view(float pX, float pY) {
	glm::vec2 res;
	// x and y in NDC space
	res.x = (pX / float(width))*2.f - 1.f; 
	res.y = ((height-pY) / float(height))*2.f - 1.f;
	glm::mat4 invProj = glm::inverse(proj);
	//  from NDC to view space
	glm::vec4 res_np = invProj*glm::vec4(res.x, res.y, -1, 1);
	res_np /= res_np.w;

	return glm::vec3(res_np.x, res_np.y, res_np.z);
}

/*
o: origin of the ray
d: direction of the ray
c: center of the sphere (in this setup is 0,0,0 )
radius: radius of the sphere
*/
bool ray_sphere_intersection(glm::vec3& int_point, glm::vec3 o, glm::vec3 d, glm::vec3 c, float radius) {
	glm::vec3 oc = o - c;
	float A = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
	float B = 2 * glm::dot(d, oc);
	float C = glm::dot(oc, oc) - radius * radius;

	float dis = B * B - 4 * A * C;

	if (dis > 0) {
		float t0 = (-B - sqrt(dis)) / (2 * A);
		float t1 = (-B + sqrt(dis)) / (2 * A);
		float t = std::min<float>(t0, t1);
		int_point =  o + glm::vec3(t * d[0], t * d[1], t * d[2]);
		return true;
	}
	return false;
}

/* handles the intersection between the position under the mouse and the sphere.
*/
bool cursor_sphere_intersection(glm::vec3 & int_point, double xpos, double ypos) {

	bool hit = false;
	// convert mouse position from the screen to  view space
	glm::vec3 pos  = viewport_to_view((float)xpos, (float)ypos);

	/* here build the ray, test if it intersects the sphere and, if so, 
	write the intersection point on "int_point"
	*/
	glm::vec3 o = view_frame*  glm::vec4(glm::vec3(0.f, 0.f, 0.f), 1.f);
	glm::vec3 d = view_frame*  glm::vec4( pos , 0.f);
	glm::vec3 c = glm::vec4(glm::vec3(0.f, 0.f, 0.f), 1.f);

	hit = ray_sphere_intersection(int_point, o, d, c, 2.f);
	if (hit)
		int_point -= c;

	// return true if there is intersection
	return hit;
}

/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// if the trackball is not being dragged then do nothing
	if (!is_trackball_dragged)
		return;

	// if it is dragged then p0 has been already found, 
	// check if the trackball is still under the  mouse pointer
	if (cursor_sphere_intersection(p1, xpos, ypos)) {
		/* here we have both p0 and p1, compute the rotation matrix
			corresponding to the movement between p0 and p1
			(check the slides)
		*/
		// your code here...
		glm::vec3 rotation_vector = glm::cross(glm::normalize(p0), glm::normalize(p1));

		/* avoid near null rotation axis*/
		if (glm::length(rotation_vector) > 0.01) {
			float alpha = glm::asin(glm::length(rotation_vector));
			glm::mat4 delta_rot = glm::rotate(glm::mat4(1.f), alpha, rotation_vector);
			trackball_matrix = delta_rot * trackball_matrix;

			/*p1 becomes the p0 value for the next movement */
			p0 = p1;
		}
	}
 	else
 		is_trackball_dragged = false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glm::vec3 int_point;

		/*
		mouse button has been pressed, check if it's over the sphere
		*/
		if (cursor_sphere_intersection(int_point, xpos, ypos)) {
			//the first point has been found
			p0 = int_point;

			// set the state of trackball being dragged to true
			is_trackball_dragged = true;
		}
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			// mouse button has been released, the trackball is no more being  dragged
			is_trackball_dragged = false;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scaling_factor *= (yoffset>0) ? 1.1f : 0.97f;
	std::cout << scaling_factor << std::endl;
	scaling_matrix = glm::scale(glm::mat4(1.f), glm::vec3(scaling_factor, scaling_factor, scaling_factor));
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), near, 20.f);
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	/* Create a windowed mode window and its OpenGL context */
	width = 1000;
	height = 800;
	window = glfwCreateWindow(width, height, "code_08_raster", NULL, NULL);
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
	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();
	glEnable(GL_MULTISAMPLE);

	printout_opengl_glsl_info();

	/* load the shaders */
	shader basic_shader;
	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
	shader flat_shader;
	flat_shader.create_program("shaders/flat.vert", "shaders/flat.frag");

	/* Set the uT matrix to Identity */
	glUseProgram(basic_shader.program);
//	glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	/* create a  quad   centered at the origin with side 2*/
	renderable r_quad = shape_maker::quad();


	/* create a  cube   centered at the origin with side 2*/
	renderable r_cube = shape_maker::cube(0.5f, 0.3f, 0.0f);

	/* create a  sphere   centered at the origin with radius 1*/
	renderable r_sphere = shape_maker::sphere(5);


	/* create 3 lines showing the reference frame*/
	renderable r_frame = shape_maker::frame(4.0);

	check_gl_errors(__LINE__, __FILE__);

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f),width/float(height), near, 20.f);
	view = glm::lookAt(glm::vec3(0, 6, 8.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	view_frame = glm::inverse(view);

	glEnable(GL_DEPTH_TEST);

	matrix_stack stack;
	trackball_matrix = scaling_matrix = glm::mat4(1.f);
	 

	/* define the viewport  */
	glViewport(0, 0, 1000, 800);

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"]		, 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"]		, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uModel"]		, 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniform3f(basic_shader["uColor"], 1.0,0.0,0.0);

	glUseProgram(flat_shader.program);
	
	glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniform4f(flat_shader["uColor"], 1.0, 1.0, 1.0,1.0);

	glDepthFunc(GL_LESS);

//	#define	TRACKBALL
//	#define ZFIGHT
// 	#define SCISSOR
//	#define STENCIL
	#define BLENDING

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT);
		glClearColor(0.f, 0.f, 0.f, 1.f);

#ifdef SCISSOR
			glEnable(GL_SCISSOR_TEST);
			glScissor(400, 400, 200, 200);
#endif
		glUseProgram(basic_shader.program);
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);

		stack.push();
		stack.mult(scaling_matrix*trackball_matrix);

		/* DRAW the trackball */
#ifdef TRACKBALL
			r_cube.bind();
			//// draw a frame (red = x, green = y, blue = z )
			stack.push();
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.05, 1.0, 0.05)));
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, 1.0, 0)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 0.0, 1.0, 0.0);
			glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
			stack.pop();

			stack.push();
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1, 0.05, 0.05)));
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(1.0, 0.0, 0)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
			glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
			stack.pop();

			stack.push();
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f, 1.f)));
			stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.0, 0.0, 1.f)));
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 0.0, 0.0, 1.0);
			glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
			stack.pop();

			// draw one sphere
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 0.5, 0.5, 0.5);
			r_sphere.bind();
			glDrawElements(r_sphere().mode, r_sphere().count, r_sphere().itype, 0);
#endif
		///* end draw trackball */


#ifdef  ZFIGHT 
			 r_cube.bind();
			 glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			 glUniform3f(basic_shader["uColor"], 0.4, 0.4, 0.4);
			 glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);

			 glEnable(GL_POLYGON_OFFSET_FILL);
			 glPolygonOffset(1.0, 1.0);
			 
			 glUniform3f(basic_shader["uColor"], 0.4, 0.8, 0.7);
			 stack.push();
			 stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.1, 0.1, 0)));
			 glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			 glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
			 stack.pop();
			 
			 glDisable(GL_POLYGON_OFFSET_FILL);
#endif
		
		
#ifdef  STENCIL 
			r_cube.bind();

			// enable the stencil test
			glEnable(GL_STENCIL_TEST);

			// enable the writing on all the 8 bits of the stencil buffer
			glStencilMask(0xFF);

			// set the stencil test so that *every* fragment passes
			glStencilFunc(GL_ALWAYS, 1, 0xFF);

			// all the fragments that pass both stencil and depth test write 1 on the stencil buffer (1 is the reference value passed
			// in the glStencilFunc call
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			glUseProgram(basic_shader.program);

			for (int i = 0; i < 4; ++i) {
				stack.push();
				stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.35 * i, 0.15 * i, -0.4 * i)));
				glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
				glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
				glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
				stack.pop();
			}


			// disallow writing on any of the bits of the stencil buffer
			glStencilMask(0x00);

			// either disable the depth test or set the depth function so that every fragment passes.
			// The difference is that is the depth test is disabled you cannot write on the depth buffer.
			// try one way or another and look for the difference

			glDisable(GL_DEPTH_TEST);
			//		glDepthFunc(GL_ALWAYS);

			// now it's time to render the scaled up version of the cubes. Set the stencil function to allow
			// all fragment pass only if the corresponding value on the stencil is not 1 (that is, that pixel
			// was not covered while rendering the cubes
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

			glUseProgram(flat_shader.program);

			glUniform4f(flat_shader["uColor"], 0.f, 0.f, 1.f,1.0);
			glUniformMatrix4fv(flat_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
			for (int i = 0; i < 4; ++i) {
				stack.push();
				stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.35 * i, 0.15 * i, -0.4 * i)));
				stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1.05, 1.05, 1.05)));

				glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
				glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
				stack.pop();
			}



			glDisable(GL_STENCIL_TEST);

			// restore the depth test function to its default 
			glEnable(GL_DEPTH_TEST);
			//glDepthFunc(GL_LESS);
			glStencilMask(0xFF);
#endif
		
#ifdef BLENDING 
			glUseProgram(flat_shader.program);
			glUniformMatrix4fv(flat_shader["uProj"], 1, GL_FALSE, &proj[0][0]);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			r_quad.bind();
			stack.push();
			glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform4f(flat_shader["uColor"], 1.f, 0.f, 0.f,1.0);
			glDrawElements(r_quad().mode, r_quad().count, r_quad().itype, 0);
			stack.mult(glm::translate(glm::mat4(1.f),glm::vec3(0,0,0.4)));
			glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform4f(flat_shader["uColor"], 0.f, 1.f, 0.f,0.5);
			glDrawElements(r_quad().mode, r_quad().count, r_quad().itype, 0);
			stack.pop();
#endif
		/* end blending */


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
