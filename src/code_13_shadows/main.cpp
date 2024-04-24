#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"
#include "../common/matrix_stack.h"
#include "../common/intersection.h"
#include "../common/trackball.h"
#include "../common/view_manipulator.h"
#include "../common/texture.h"
#include "../common/frame_buffer_object.h"


/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_access.hpp>


int width, height;

/* light direction in world space*/
glm::vec4 Ldir;

/* projector */
float depth_bias;
float distance_light;

struct projector {
	glm::mat4 view_matrix,proj_matrix;
	texture tex;
	glm::mat4 set_projection(glm::mat4 _view_matrix, box3 box) {
		view_matrix = _view_matrix;

		/* TBD: set the view volume properly so that they are a close fit of the 
		bounding box passed as parameter */
		proj_matrix =  glm::ortho(-4.f, 4.f, -4.f, 4.f,0.f, distance_light*2.f);
//		proj_matrix = glm::perspective(3.14f/2.f,1.0f,0.1f, distance_light*2.f);
		return proj_matrix;
	}
	glm::mat4 light_matrix() {
		return proj_matrix*view_matrix;
	}
	// size of the shadow map in texels
	int sm_size_x, sm_size_y;
};


projector Lproj;


/* trackballs for controlloing the scene (0) or the light direction (1) */
trackball tb[2];

/* which trackball is currently used */
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view ;

/* matrix stack*/
matrix_stack stack;

/* a frame buffer object for the offline rendering*/
frame_buffer_object fbo, fbo_blur;

/* object that will be rendered in this scene*/
renderable r_frame, r_plane,r_line,r_torus,r_cube, r_sphere,r_quad;

/* program shaders used */
shader depth_shader,shadow_shader,flat_shader,fsq_shader,blur_shader;

/* implementation of view controller */
/* azimuthal and elevation angle*/
view_manipulator view_man;


/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (ImGui::GetIO().WantCaptureMouse) return;
	if(curr_tb<2)
		tb[curr_tb].mouse_move(proj, view, xpos, ypos);
	else
		view_man.mouse_move(xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (curr_tb < 2)
			tb[curr_tb].mouse_press(proj, view, xpos, ypos);
		else 
			view_man.mouse_press(xpos, ypos);
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			if(curr_tb<2)
				tb[curr_tb].mouse_release(); 
			else
				view_man.mouse_release();
		}
}

/* callback function called when a mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse) return;

	if(curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
 /* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
 if(action == GLFW_PRESS)
	 curr_tb = 1 - curr_tb;

}

/* callback function called when the windows is resized */
void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 100.f);
}

void print_info() {
}

/* which algorithm to use */
static int selected_mode = 0;

/* paramters of the VSM (it should be 0.5) */
static float k_plane_approx = 0.5;

void gui_setup() {
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("Shadow mode"))
	{
	 if (ImGui::Selectable("none", selected_mode == 0)) selected_mode = 0;
	 if (ImGui::Selectable("Basic shadow mapping", selected_mode == 1)) selected_mode = 1;
	 if (ImGui::Selectable("bias", selected_mode == 2)) selected_mode = 2;
	 if (ImGui::Selectable("slope bias", selected_mode == 3)) selected_mode = 3;
	 if (ImGui::Selectable("back faces", selected_mode == 4)) selected_mode = 4;
	 if (ImGui::Selectable("PCF", selected_mode == 5)) selected_mode = 5;
	 if (ImGui::Selectable("Variance SM", selected_mode == 6)) selected_mode = 6;
	 ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("parameters"))
	{
		bool redo_fbo = false;
		const char* items[] = { "32","64","128","256","512", "1024", "2048", "4096"};
		static int item_current = 1;
		int xi, yi;
		if (ImGui::ListBox("sm width", &xi, items, IM_ARRAYSIZE(items), 8))
			if (Lproj.sm_size_x != 1 << (5 + xi)) {
				Lproj.sm_size_x = 1 << (5 + xi);
				redo_fbo = true;
			}
		if(ImGui::ListBox("sm height", &yi, items, IM_ARRAYSIZE(items), 8))
			if (Lproj.sm_size_y != 1 << (5 + yi)) {
				Lproj.sm_size_y = 1 << (5 + yi);
				redo_fbo = true;
			}
		if (ImGui::SliderFloat("distance", &distance_light, 2.f, 100.f)) 
			Lproj.set_projection(glm::lookAt(glm::vec3(0, distance_light, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f))*inverse(tb[1].matrix()), box3(1.0));
		ImGui::SliderFloat("  plane approx", &k_plane_approx,0.0,1.0);
		if (redo_fbo) {
			fbo.remove();
			fbo.create(Lproj.sm_size_x, Lproj.sm_size_y,true);
			fbo_blur.remove();
			fbo_blur.create(Lproj.sm_size_x, Lproj.sm_size_y, true);
		}
		 
		ImGui::InputFloat("depth bias", &depth_bias);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Trackball")) {
		if (ImGui::Selectable("control scene", curr_tb == 0)) curr_tb = 0;
		if (ImGui::Selectable("control light", curr_tb == 1)) curr_tb = 1;
		if (ImGui::Selectable("control view", curr_tb == 2)) curr_tb = 2;

		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void draw_torus(  shader & sh) {
	glUniformMatrix4fv(sh["uT"], 1, GL_FALSE, &stack.m()[0][0]);
	r_torus.bind();
	glDrawElements(r_torus().mode, r_torus().count, r_torus().itype, 0);
}

void draw_plane(  shader & sh) {
	glUniformMatrix4fv(sh["uT"], 1, GL_FALSE, &stack.m()[0][0]);
	r_plane.bind();
	glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);

}


void draw_pole(shader & sh) {
	r_sphere.bind();
	glDrawElements(r_sphere().mode, r_sphere().count, r_sphere().itype, 0);
}

void draw_sphere(  shader & sh) {
	glUniformMatrix4fv(sh["uT"], 1, GL_FALSE, &stack.m()[0][0]);
	r_sphere.bind();
	glDrawElements(r_sphere().mode, r_sphere().count, r_sphere().itype, 0);
}

void draw_cube(shader & sh) {
	glUniformMatrix4fv(sh["uT"], 1, GL_FALSE, &stack.m()[0][0]);
	r_cube.bind();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_cube().ind);
	glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
}

void draw_scene(  shader & sh) {
	if (sh.has_uniform("uDiffuseColor")) glUniform3f(sh["uDiffuseColor"], 0.6f, 0.6f, 0.6f);
	stack.push();
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(4.0, 4.0, 4.0)));
	draw_plane(sh);
	stack.pop();

	if (sh.has_uniform("uDiffuseColor")) glUniform3f(sh["uDiffuseColor"], 0.0f, 0.4f, 0.5f);
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-0.6, 0.3, 0.0)));
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2, 0.2, 0.2)));
	draw_plane(sh);
	stack.pop();

	// draw sphere
	//stack.push();
	//stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.0, 0.5, 0.0)));
	//stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.5, 0.5, 0.5)));
	//draw_sphere(sh);
	//stack.pop();

	// draw pole
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.0, 0.5, 0.0)));
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.1, 0.5, 0.1)));
	glUniformMatrix4fv(sh["uT"], 1, GL_FALSE, &stack.m()[0][0]);
	//draw_sphere(sh);
	draw_cube(sh);
	stack.pop();

	// torus	
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(1.0, 0.5, 0.0)));
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2, 0.2, 0.2)));
	draw_torus(sh);
	stack.pop();

}

void draw_full_screen_quad() {
	r_quad.bind();
	glDrawElements(GL_TRIANGLES, 6,GL_UNSIGNED_INT, 0);
}

void draw_texture(GLint tex_id ) {
	GLint at;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUseProgram(fsq_shader.program);
	glUniform1i(fsq_shader["uTexture"], 3);
	draw_full_screen_quad();
	glUseProgram(0);
	glActiveTexture(at);
}



void blur_texture(GLint tex_id) {
	GLint at;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	check_gl_errors(__LINE__, __FILE__, true);

	glBindFramebuffer(GL_FRAMEBUFFER,fbo_blur.id_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_blur.id_tex, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(blur_shader.program);
	glUniform2f(blur_shader["uBlur"], 0.0,1.f / fbo_blur.h);
	glUniform1i(blur_shader["uTexture"], 3);
	draw_full_screen_quad();
	check_gl_errors(__LINE__, __FILE__, true);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);	
	glBindTexture(GL_TEXTURE_2D, fbo_blur.id_tex);
	glUniform2f(blur_shader["uBlur"], 1.f / fbo_blur.w, 0.0);
	draw_full_screen_quad();
	check_gl_errors(__LINE__, __FILE__, true);

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(at);
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	width = 1000;
	height = 800;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "code_13_shadows", NULL, NULL);
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
	glfwSetWindowSizeCallback(window, window_size_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	
	printout_opengl_glsl_info();

	check_gl_errors(__LINE__, __FILE__, true);

	/* load the shaders */
	std::string shaders_path = "../../src/code_13_shadows/shaders/";
	depth_shader.create_program((shaders_path+"depthmap.vert").c_str(), (shaders_path+"depthmap.frag").c_str());
	shadow_shader.create_program((shaders_path + "shadow_mapping.vert").c_str(), (shaders_path + "shadow_mapping.frag").c_str());
	fsq_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "fsq.frag").c_str());
	flat_shader.create_program((shaders_path + "flat.vert").c_str(), (shaders_path + "flat.frag").c_str());
	blur_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "blur.frag").c_str());

	/* Set the uT matrix to Identity */
	glUseProgram(depth_shader.program);
	glUniformMatrix4fv(depth_shader["uT"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(shadow_shader.program);
	glUniformMatrix4fv(shadow_shader["uT"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uT"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);
	/* create a  long line*/
	r_line = shape_maker::line(100.f);

	/* create 3 lines showing the reference frame*/
	r_frame = shape_maker::frame(4.0);
	
	/* create a rectangle*/
	shape s_plane;
	shape_maker::rectangle(s_plane, 1, 1);
	s_plane.to_renderable(r_plane);

	/* create a torus */
	shape  s_torus;
	shape_maker::torus(s_torus, 0.5, 2.0, 50, 50);
	s_torus.to_renderable(r_torus);
	check_gl_errors(__LINE__, __FILE__, true);

	/* create a cube */
	shape s_cube;
	shape_maker::cube(s_cube);
	s_cube.compute_edges();
	s_cube.to_renderable(r_cube);

	/* create a sphere */
	r_sphere = shape_maker::sphere(3);

	/* create a quad with size 2 centered to the origin and on the XY pane */
	/* this is a quad used for the "full screen quad" rendering */
	r_quad = shape_maker::quad();

	/* initial light direction */
	Ldir = glm::vec4(0.0, 1.0, 0.0, 0.0);

	/* light projection */
	Lproj.sm_size_x = 512;
	Lproj.sm_size_y = 512;
	depth_bias = 0;
	distance_light = 2;
	k_plane_approx = 0.5;

	Lproj.view_matrix = glm::lookAt(glm::vec3(0, distance_light, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 100.f);
	view = glm::lookAt(glm::vec3(0, 3, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(depth_shader.program);
	glUniformMatrix4fv(depth_shader["uLightMatrix"], 1, GL_FALSE, &Lproj.light_matrix()[0][0]);
	glUniformMatrix4fv(depth_shader["uT"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	glUseProgram(shadow_shader.program);
	glUniformMatrix4fv(shadow_shader["uP"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(shadow_shader["uV"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(shadow_shader["uLightMatrix"], 1, GL_FALSE, &Lproj.light_matrix()[0][0]);
	glUniform1i(shadow_shader["uShadowMap"], 0);
	glUniform2i(shadow_shader["uShadowMapSize"], Lproj.sm_size_x, Lproj.sm_size_y);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uP"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(flat_shader["uV"], 1, GL_FALSE, &view[0][0]);
	glUniform3f(flat_shader["uColor"], 1.0, 1.0, 1.0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
	check_gl_errors(__LINE__, __FILE__, true);

	print_info();
 
	/* set the trackball position */
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	tb[1].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	view_man.reset();
	curr_tb = 0;

	/* define the viewport  */
	glViewport(0, 0, width, height);
	
	check_gl_errors(__LINE__, __FILE__, true);
	fbo.create(Lproj.sm_size_x, Lproj.sm_size_y,true);
	fbo_blur.create(Lproj.sm_size_x, Lproj.sm_size_y, true);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		gui_setup();

		Ldir = glm::vec4(0.f, 1.f, 0.f,0.f);
		/* rotate the view accordingly to view_rot*/
		glm::mat4 curr_view = view_man.apply_to_view(view);

		/* light direction transformed by the trackball tb[1]*/
		glm::vec4 curr_Ldir = tb[1].matrix()*Ldir;

		stack.push();
		stack.mult(tb[0].matrix());

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.id_fbo);
		glViewport(0, 0, Lproj.sm_size_x, Lproj.sm_size_y);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glUseProgram(depth_shader.program);

		Lproj.view_matrix = glm::lookAt(glm::vec3(0, distance_light, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f))*inverse(tb[1].matrix()) *inverse(tb[0].matrix());
		Lproj.set_projection(Lproj.view_matrix, box3(2.0));

		glUniformMatrix4fv(depth_shader["uLightMatrix"], 1, GL_FALSE, &Lproj.light_matrix()[0][0]);
		glUniformMatrix4fv(depth_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform1f(depth_shader["uPlaneApprox"], k_plane_approx);


		if (selected_mode == 4 || selected_mode == 5) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		}

		draw_scene(depth_shader);

		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER,0);

		if (selected_mode == 6) {
			blur_texture(fbo.id_tex);
		}

		glViewport(0, 0, width, height);

		glUseProgram(shadow_shader.program);
		glUniformMatrix4fv(shadow_shader["uLightMatrix"], 1, GL_FALSE, &Lproj.light_matrix()[0][0]);
		glUniformMatrix4fv(shadow_shader["uV"], 1, GL_FALSE, &curr_view[0][0]);
		glUniformMatrix4fv(shadow_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform1fv(shadow_shader["uBias"],1, &depth_bias);
		glUniform2i(shadow_shader["uShadowMapSize"], Lproj.sm_size_x, Lproj.sm_size_y );
		glUniform1i(shadow_shader["uRenderMode"], selected_mode);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo.id_tex);

		draw_scene(shadow_shader);

		// render the reference frame
		glUseProgram(flat_shader.program);
		glUniformMatrix4fv(flat_shader["uV"], 1, GL_FALSE, &curr_view[0][0]);
		glUniformMatrix4fv(flat_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(flat_shader["uColor"], -1.0, 1.0, 1.0);

		r_frame.bind();
		glDrawArrays(GL_LINES, 0, 6);
		glUseProgram(0);

		check_gl_errors(__LINE__, __FILE__, true);
	

		stack.pop();


		// draw the light   frustum
		r_cube.bind();
		stack.push();
		stack.mult(inverse(Lproj.light_matrix()));
		glUseProgram(flat_shader.program);
		glUniform3f(flat_shader["uColor"], 0.0, 0.0, 1.0);
		glUniformMatrix4fv(flat_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_cube.elements[1].ind);
		glDrawElements(r_cube.elements[1].mode, r_cube.elements[1].count, r_cube.elements[1].itype, 0);

		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0,0, -1)));
		stack.mult(glm::scale(glm::mat4(1.f),glm::vec3(1, 1, 0)));
		glUniform3f(flat_shader["uColor"], 1.0, 1.0, 0.0);
		glUniformMatrix4fv(flat_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r_cube.elements[0].ind);
		glDrawElements(r_cube.elements[0].mode, r_cube.elements[0].count, r_cube.elements[0].itype, 0);
		stack.pop();



// glDisable(GL_DEPTH_TEST);
// glViewport(0, 0, 512, 512);
// blur_texture(Lproj.tex.id);
// draw_texture(Lproj.tex.id);
// glEnable(GL_DEPTH_TEST);
// draw_texture(fbo.id_tex);


		check_gl_errors(__LINE__, __FILE__);


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// show the shadow map 
		glViewport(0, 0, 200, 200);
		glDisable(GL_DEPTH_TEST);
		draw_texture(fbo.id_tex);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, width, height);



		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
