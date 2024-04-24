#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
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

int width, height;

/* light direction in world space*/
glm::vec4 Ldir;

/* projector */
struct projector {
	glm::mat4 view,proj;
	texture tex;
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
frame_buffer_object fbo;

/* object that will be rendered in this scene*/
renderable r_frame, r_plane,r_line,r_torus,r_cube, r_sphere;

/* program shaders used */
shader texture_shader,flat_shader;

/* textures used */
texture skybox, reflection_map;

/* mode selected */
int selected = 0;

/* implementation of view controller */

/* azimuthal and elevation angle*/
float d_alpha, d_beta;
float start_xpos, start_ypos;
bool is_dragging;
glm::mat4 view_rot,view_frame;


/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if(curr_tb<2)
		tb[curr_tb].mouse_move(proj, view, xpos, ypos);
	else {
		if (is_dragging) {
			d_alpha += (float)(xpos - start_xpos)/width;
			d_beta += (float)(ypos - start_ypos)/height;
			start_xpos = (float)xpos;
			start_ypos = (float)ypos;
			view_rot = glm::rotate(glm::rotate(glm::mat4(1.f), d_alpha, glm::vec3(0,1,0)), d_beta, glm::vec3(1,0,0));
		}
	}
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (curr_tb < 2)
			tb[curr_tb].mouse_press(proj, view, xpos, ypos);
		else {
			is_dragging = true;
			start_xpos = (float) xpos;
			start_ypos = (float) ypos;
		}
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			if(curr_tb<2)
				tb[curr_tb].mouse_release(); 
			else
				is_dragging = false;
		}
}

/* callback function called when a mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
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

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);

}

void print_info() {
}



void load_textures() {
	std::string path = "./textures/envmap/";
	skybox.load_cubemap(path + "posx.jpg", path + "negx.jpg", 
						path + "posy.jpg", path + "negy.jpg",
						path + "posz.jpg", path + "negz.jpg",1);

	glActiveTexture(GL_TEXTURE2);
	reflection_map.create_cubemap(2048, 2048,3);
}

void gui_setup() {
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("Texture mode"))
	{
	 if (ImGui::Selectable("just diffuse grey", selected == 0)) selected = 0;
	 if (ImGui::Selectable("Projective Texturing", selected == 1)) selected = 1;
	 if (ImGui::Selectable("Skybox", selected == 2)) selected = 2;
	 if (ImGui::Selectable("Reflection", selected == 3)) selected = 3;
	 if (ImGui::Selectable("Refraction", selected == 4)) selected = 4;
	 if (ImGui::Selectable("Reflection Map", selected == 5)) selected = 5;
	 ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Trackball")) {
		if (ImGui::Selectable("control scene", curr_tb == 0)) curr_tb = 0;
		if (ImGui::Selectable("control ligth", curr_tb == 1)) curr_tb = 1;
		if (ImGui::Selectable("control view", curr_tb == 2)) curr_tb = 2;

		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void draw_torus() {
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(1.0, 0.5, 0.0)));
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2, 0.2, 0.2)));
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	r_torus.bind();
	glDrawElements(r_torus().mode, r_torus().count, r_torus().itype, 0);
	stack.pop();
}

void draw_plane() {
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	r_plane.bind();
	glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);
}

void draw_large_cube() {
	r_cube.bind();
	glUniform1i(texture_shader["uRenderMode"], 2);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::scale(glm::mat4(1.f), glm::vec3(40.0, 40.0, 40.0))[0][0]);
	glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
}

/* used when rendering offscreen to create the environment map on-the-fly*/
void draw_scene_no_target() {
	draw_large_cube();
	glUniform1i(texture_shader["uRenderMode"], 1);
	draw_plane();
	draw_torus();
}

void draw_sphere() {
	stack.push();
	stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.0, 0.5, 0.0)));
	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.5, 0.5, 0.5)));
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	r_sphere.bind();
	glDrawElements(r_sphere().mode, r_sphere().count, r_sphere().itype, 0);
	stack.pop();
}
void draw_scene_target_only() {
	glUniform1i(texture_shader["uRenderMode"], 5);
	draw_sphere();
}


void draw_scene() {
	if (selected > 1) {
		draw_large_cube();
	}

	glUniform1i(texture_shader["uRenderMode"], (selected == 2) ? 1 : selected);
	draw_plane();
	draw_sphere();
	draw_torus();
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
	window = glfwCreateWindow(width, height, "code_12_onthefly_coordinates", NULL, NULL);
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


	/* load the shaders */
	std::string shaders_path = "../../src/code_12_onthefly_coordinates/shaders/";
	texture_shader.create_program((shaders_path+"texture.vert").c_str(), (shaders_path+"texture.frag").c_str());


	flat_shader.create_program((shaders_path + "flat.vert").c_str(), (shaders_path + "flat.frag").c_str());


	/* Set the uT matrix to Identity */
	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	fbo.create(2048, 2048);

	/* create a  long line*/
	r_line = shape_maker::line(100.f);

	/* create 3 lines showing the reference frame*/
	r_frame = shape_maker::frame(4.0);
	
	/* create a rectangle*/
	shape s_plane;
	shape_maker::rectangle(s_plane, 1, 1);
	s_plane.compute_tangent_space();
	s_plane.to_renderable(r_plane);

	/* create a torus */
	shape  s_torus;
	shape_maker::torus(s_torus, 0.5, 2.0, 50, 50);
	s_torus.compute_tangent_space();
	s_torus.to_renderable(r_torus);

	/* create a torus */
	r_cube = shape_maker::cube();

	/* create a sphere */
	r_sphere = shape_maker::sphere(3);

	/* initial light direction */
	Ldir = glm::vec4(0.0, 1.0, 0.0, 0.0);

	/* light projection */
	Lproj.proj = glm::frustum(-0.1f,0.1f, -0.05f, 0.05f, 2.f, 40.f);
	Lproj.view = glm::lookAt(glm::vec3(4, 4, 6.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	Lproj.tex.load("./textures/batman.png",0);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 100.f);
	view = glm::lookAt(glm::vec3(0, 3, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(texture_shader.program);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(texture_shader["uLPView"], 1, GL_FALSE, &Lproj.view[0][0]);
	glUniformMatrix4fv(texture_shader["uLPProj"], 1, GL_FALSE, &Lproj.proj[0][0]);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);


	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniform3f(flat_shader["uColor"], 1.0, 1.0, 1.0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
	check_gl_errors(__LINE__, __FILE__, true);

	print_info();


	/* set the trackball position */
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	tb[1].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	view_rot = glm::mat4(1.f);
	curr_tb = 0;

	/* define the viewport  */
	glViewport(0, 0, width, height);

	load_textures();
	
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.5, 0.5, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		gui_setup();

		/* rotate the view accordingly to view_rot*/
		/* Exercise: find a simpler series of operations to define curr_view*/
		view_frame = inverse(view);
		glm::mat4 curr_view = view_frame;
		curr_view[3] = glm::vec4(0, 0, 0, 1);
		curr_view = ::view_rot*curr_view;
		curr_view[3] = view_frame[3];
		curr_view = inverse(curr_view);


		/* light direction transformed by the trackball tb[1]*/
		glm::vec4 curr_Ldir = tb[1].matrix()*Ldir;

		stack.push();
		stack.mult(tb[0].matrix());

		stack.push();
		glUseProgram(texture_shader.program);
		glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &curr_view[0][0]);
		glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform4fv(texture_shader["uLdir"], 1, &curr_Ldir[0]);
		glUniform1i(texture_shader["uRenderMode"], selected);
		glUniform1i(texture_shader["uColorImage"], 0);
		glUniform1i(texture_shader["uSkybox"], 1);
		glUniform1i(texture_shader["uReflectionMap"], 2);
		

		check_gl_errors(__LINE__, __FILE__, true);

		/* on-the-fly computation of the environment  map for the sphere */
		if( selected == 5 ){		
			/* draw the scene six times, one for each face of the cube  */
			glm::vec3 tar[6] = { glm::vec3(1.f,0,0),glm::vec3(-1.f,0.f,0),glm::vec3(0.f,1.f,0),glm::vec3(0.f,-1.f,0),glm::vec3(0.f,0,1),glm::vec3(0.f,0,-1.f) };
			glm::vec3 up[6]  = { glm::vec3(0.0,-1,0),glm::vec3(0.0,-1.f,0),glm::vec3(0.0,0.f,1),glm::vec3(0.0,0.0,-1),glm::vec3(0.f,-1,0),glm::vec3(0.f,-1,0) };

			glm::mat4 projsB;
			projsB = glm::perspective(3.14f/2.f, 1.f, 0.1f, 100.0f);
			glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &projsB[0][0]);

			glBindFramebuffer(GL_FRAMEBUFFER, fbo.id_fbo);

			/* the point of view is set to the center of the sphere */
			glm::vec3 eye = glm::vec3(0, 0.5, 0.0);

			/* the point of view is transformed by the trackball */
			eye =  tb[0].matrix()*glm::vec4(eye,1.0);

			for (unsigned int i = 0; i < 6; ++i) {
				/* set to which face of the cubemap the rendering on this direction will be written */
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, reflection_map.id, 0);
				glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &glm::lookAt(eye, eye + tar[i], up[i])[0][0]);

				/* the viewport is set to the same size as the faces of the cubemap*/
				glViewport(0, 0, 2048, 2048);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				draw_scene_no_target();
			}
			glBindFramebuffer(GL_FRAMEBUFFER,0);
		}
		/* --------------------------------------------------------*/
 
		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &curr_view[0][0]);
		glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);

		if (selected == 5) {
			draw_scene_no_target();
			draw_scene_target_only();
		}else
			draw_scene();


		stack.pop();

		// render the reference frame
		glUseProgram(flat_shader.program);
		glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &curr_view[0][0]);
		glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(flat_shader["uColor"], -1.0, 1.0, 1.0);

		r_frame.bind();
		glDrawArrays(GL_LINES, 0, 6);
		glUseProgram(0);

		check_gl_errors( __LINE__,__FILE__,true);
		stack.pop();

		// render the light direction
		stack.push();
		stack.mult(tb[1].matrix());
		 
		glUseProgram(flat_shader.program);
		glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(flat_shader["uColor"], 1.0,1.0,1.0);
		r_line.bind();
		glDrawArrays(GL_LINES, 0, 2);
		glUseProgram(0);

		stack.pop();


		check_gl_errors(__LINE__, __FILE__);
	
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
