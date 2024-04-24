#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <random>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "../common/debugging.h"
#include "../common/renderable.h"
#include "../common/shaders.h"
#include "../common/simple_shapes.h"
#include "../common/matrix_stack.h"
#include "../common/intersection.h"
#include "../common/trackball.h"
#include "../common/view_manipulator.h"
#include "../common/frame_buffer_object.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../common/gltf_loader.h"

/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_access.hpp>

/* light direction in world space*/
glm::vec4 Ldir;

/* projector */
float depth_bias;
float distance_light;

int g_buffer_size_x, g_buffer_size_y;
float radius, depthscale;

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
frame_buffer_object fbo, fbo_ao,fbo_blur;

/* object that will be rendered in this scene*/
renderable r_frame,r_quad,r_line;
std::vector<renderable> r_objs;


/* program shaders used */
shader ao_shader, g_buffer_shader,final_shader/*,depth_shader,shadow_shader*/,flat_shader,fsq_shader,blur_shader;

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
void print_info() {
}



static bool use_ao = 0;
static int fps;
void gui_setup() {
	ImGui::BeginMainMenuBar();

	ImGui::Text((std::string("FPS: ")+std::to_string(fps)).c_str());
	if (ImGui::BeginMenu("parameters"))
	{
		bool redo_fbo = false;
		const char* items[] = { "32","64","128","256","512", "1024", "2048", "4096"};
		static int item_current = 1;
		int xi, yi;
		ImGui::Checkbox("use AO", &use_ao);
		if (ImGui::ListBox("sm width", &xi, items, IM_ARRAYSIZE(items), 8))
			if (g_buffer_size_x != 1 << (5 + xi)) {
				g_buffer_size_x = 1 << (5 + xi);
				redo_fbo = true;
			}
		if(ImGui::ListBox("sm height", &yi, items, IM_ARRAYSIZE(items), 8))
			if (g_buffer_size_y != 1 << (5 + yi)) {
				g_buffer_size_y = 1 << (5 + yi);
				redo_fbo = true;
			}
		if (redo_fbo) {
			fbo.remove();
			fbo.create(g_buffer_size_x, g_buffer_size_y,true);
			fbo_ao.remove();
			fbo_ao.create(g_buffer_size_x, g_buffer_size_y, true);
			fbo_blur.remove();
			fbo_blur.create(g_buffer_size_x, g_buffer_size_y, true);
		}
		 
		ImGui::SliderFloat("radius", &radius,0.0,50.f);
		ImGui::SliderFloat("depthscale", &depthscale, 0.001f, 0.1f);
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



void draw_scene(  shader & sh) {
 	stack.push();
	float sf = 1.f / r_objs[0].bbox.diagonal();
	glm::vec3 c = r_objs[0].bbox.center();
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(sf, sf, sf)));
	stack.mult(glm::translate(glm::mat4(1.f), -c));
	glUniformMatrix4fv(sh["uT"], 1, GL_FALSE, &stack.m()[0][0]);
	r_objs[0].bind();
	glDrawElements(r_objs[0]().mode, r_objs[0]().count, r_objs[0]().itype, 0);
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

void ssaoKernel() {
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	std::vector<float> _;
	srand(clock());
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		ssaoKernel.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glUseProgram(ao_shader.program);
	ao_shader.bind("uSamples");
	ao_shader.bind("uNoise");
	glUniform3fv(ao_shader["uSamples"], (int) ssaoKernel.size(), &ssaoKernel[0].x);
	glUniform1i(ao_shader["uNoise"], 4);
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
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

int main(int argc,char**argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(1000, 800, "code_14_ambient_obscurance", NULL, NULL);
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
	glEnable(GL_MULTISAMPLE);


	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	
	printout_opengl_glsl_info();

	check_gl_errors(__LINE__, __FILE__, true);

	/* load the shaders */
	std::string shaders_path = "../../src/code_14_ambient_obscurance/shaders/";
	ao_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "ao.frag").c_str());
	ssaoKernel();
	g_buffer_shader.create_program((shaders_path + "g_buffer.vert").c_str(), (shaders_path + "g_buffer.frag").c_str());
	final_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "final.frag").c_str());

	fsq_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "fsq.frag").c_str());
	flat_shader.create_program((shaders_path + "flat.vert").c_str(), (shaders_path + "flat.frag").c_str());
	blur_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "blur.frag").c_str());

	/* create a  long line*/
	r_line = shape_maker::line(100.f);

	/* create 3 lines showing the reference frame*/
	r_frame = shape_maker::frame(4.0);
	


 	 
	box3 bbox;
	gltf_loader gltf_l;
	gltf_l.load(argv[1]);
	gltf_l.create_renderable(r_objs, bbox);

	/* create a quad with size 2 centered to the origin and on the XY pane */
	r_quad = shape_maker::quad();

	/* initial light direction */
	Ldir = glm::vec4(0.0, 1.0, 0.0, 0.0);

	/* light projection */
	check_gl_errors(__LINE__, __FILE__, true);
	g_buffer_size_x = 512;
	g_buffer_size_y = 512;
	check_gl_errors(__LINE__, __FILE__, true);

	/* Transformation to setup the point of view on the scene */
	proj = glm::frustum(-1.f, 1.f, -0.8f, 0.8f, 2.f,15.f);
	view = glm::lookAt(glm::vec3(0, 3, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	view = glm::lookAt(glm::vec3(0, 0, 7.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(g_buffer_shader.program);
	glUniformMatrix4fv(g_buffer_shader["uP"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(g_buffer_shader["uV"], 1, GL_FALSE, &view[0][0]);
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
	glViewport(0, 0, 1000, 800);
	
	check_gl_errors(__LINE__, __FILE__, true);
	fbo.create(g_buffer_size_x, g_buffer_size_y,true);
	fbo_blur.create(g_buffer_size_x, g_buffer_size_y, true);
	fbo_ao.create(g_buffer_size_x, g_buffer_size_y, true);


	texture ao_tex;
	ao_tex.create(g_buffer_size_x, g_buffer_size_y,GL_RGB);
	/* Loop until the user closes the window */
	double last_time = glfwGetTime();
	int n_frames = 0;

	while (!glfwWindowShouldClose(window))
	{
		double current_time = glfwGetTime();
		n_frames++;
		if (current_time - last_time >= 1.0) { 
			fps = n_frames;
			n_frames = 0;
			last_time += 1.0;
		}

		/* Render here */
		glClearColor(1.0f, 0.6f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		gui_setup();

		/* rotate the view accordingly to view_rot*/
		glm::mat4 curr_view = view_man.apply_to_view(view);

		/* light direction transformed by the trackball tb[1]*/
		glm::vec4 curr_Ldir = tb[1].matrix()*Ldir;

		stack.push();
		stack.mult(tb[0].matrix());

  	 	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id_fbo);

		glBindTexture(GL_TEXTURE_2D, fbo.id_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, g_buffer_size_x, g_buffer_size_y, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		GLenum bufferlist[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, bufferlist);
 		glViewport(0, 0, g_buffer_size_x, g_buffer_size_y);
//glViewport(0, 0, 1000, 800);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glUseProgram(g_buffer_shader.program);
		glUniformMatrix4fv(g_buffer_shader["uV"], 1, GL_FALSE, &curr_view[0][0]);
		glUniformMatrix4fv(g_buffer_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);

		draw_scene(g_buffer_shader);
		check_gl_errors(__LINE__, __FILE__, true);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

 //	draw_texture(fbo.id_tex1);
//goto swapbuffers;
//		glDrawBuffers(1, bufferlist);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_ao.id_fbo);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glUseProgram(ao_shader.program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo.id_tex);

		glUniform1i(ao_shader["uDepthMap"], 0);
		glUniform1f(ao_shader["uRadius"], radius);
		glUniform1f(ao_shader["uDepthScale"], depthscale);
		glUniform2f(ao_shader["uSize"], (float)g_buffer_size_x, (float)g_buffer_size_y);
		glUniform2f(ao_shader["uRND"], 2.0,3.0);

		draw_full_screen_quad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		blur_texture(fbo_ao.id_tex);
		blur_texture(fbo_ao.id_tex);
		blur_texture(fbo_ao.id_tex);

		glViewport(0, 0, 1000, 800);

// draw_texture(fbo_ao.id_tex);
// goto swapbuffers;

//		glViewport(0, 0, 1000, 800);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
 		glUseProgram(final_shader.program);
		glUniform1i(final_shader["uUseAO"], use_ao);
		glUniform1i(final_shader["uNormalMap"], 0);
		glUniform1i(final_shader["uAOMap"], 1);
		glActiveTexture(GL_TEXTURE0);
 		glBindTexture(GL_TEXTURE_2D, fbo.id_tex1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fbo_ao.id_tex);

		glm::vec4 LVS = curr_view*curr_Ldir;
		glUniform3f(final_shader["uLVS"], LVS.x, LVS.y, LVS.z);

 		draw_full_screen_quad();


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

		// render the light direction
		stack.push();
		stack.mult(tb[1].matrix());

		glUseProgram(flat_shader.program);
		glUniformMatrix4fv(flat_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(flat_shader["uColor"], 1.0, 1.0, 1.0);
		r_line.bind();
		glDrawArrays(GL_LINES, 0, 2);
		glUseProgram(0);
swapbuffers:
		stack.pop();

		// glDisable(GL_DEPTH_TEST);
		// glViewport(0, 0, 512, 512);
	  	//	blur_texture(Lproj.tex.id);
		// draw_texture(Lproj.tex.id);
		//glEnable(GL_DEPTH_TEST);
		// draw_texture(fbo.id_tex);


		check_gl_errors(__LINE__, __FILE__);


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// show the shadow map 
		//glViewport(0, 0, 200, 200);
		//glDisable(GL_DEPTH_TEST);
		//draw_texture(fbo.id_tex);
		//glEnable(GL_DEPTH_TEST);
		//glViewport(0, 0, 1000, 800);



		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
