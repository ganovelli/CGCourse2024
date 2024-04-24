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
bool slice = true;
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
shader tex_shader, tex_shader3d,basic_shader;

matrix_stack stack;
float scaling_factor = 1.0;


/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
        glm::mat4 vm = (curr_tb == 0) ? view :view*tb[0].matrix();
        tb[curr_tb].mouse_move(proj, vm, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                glm::mat4 vm = (curr_tb == 0) ? view : view*tb[0].matrix();
                tb[curr_tb].mouse_press(proj, vm, xpos, ypos);
        }
        else
                if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                        tb[curr_tb].mouse_release();
                }
}

float plane_y = 0;
/* callback function called when a mouse wheel is rotated */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
        if (curr_tb == 0)
                tb[0].mouse_scroll(xoffset, yoffset);
        else
                plane_y += (yoffset > 0) ? 0.01 : -0.01;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        /* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
        if (action == GLFW_PRESS)
                if (key == 65)
                        slice = !slice;
                else
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

std::string shaders_path = "../../src/code_16_gpu_volume_rendering/shaders/";
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
        window = glfwCreateWindow(width, height, "code_16_gpu_volume_rendering", NULL, NULL);
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

        // create a 3d texture
        int w, h, d;
        w = 128;
        h = 256;
        d = 256;
        GLuint id;
        glGenTextures(1, &id);
        check_gl_errors(__LINE__, __FILE__);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, id);

        char * data = new char[w*h*d*2];
        FILE*f = fopen(argv[1], "rb");
        if (f == 0)
                exit(-1);
        fread(data,  1, w*h*d, f);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, w, h, d, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        delete[] data;
        fclose(f);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        check_gl_errors(__LINE__, __FILE__);
        rt_shader.create_program((shaders_path + "glsl_volume_renderer.comp").c_str());
        check_gl_errors(__LINE__, __FILE__);

        set_storage();

        check_gl_errors(__LINE__, __FILE__);
        glUseProgram(rt_shader.program);
        glUniform1i(rt_shader["iTime"], 0 * clock());
        glUniform1i(rt_shader["uMaxSteps"], 1000);

        glDispatchCompute((unsigned int)width/32, (unsigned int)height/32, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        check_gl_errors(__LINE__, __FILE__);

        /* load the shaders */
        tex_shader.create_program((shaders_path + "tex.vert").c_str(), (shaders_path + "tex.frag").c_str());
        tex_shader3d.create_program((shaders_path + "tex3d.vert").c_str(), (shaders_path + "tex3d.frag").c_str());
        basic_shader.create_program((shaders_path + "basic.vert").c_str(), (shaders_path + "basic.frag").c_str());
        /* crete a rectangle*/
        shape s_plane;
        shape_maker::rectangle(s_plane, 1, 1);
        s_plane.to_renderable(r_plane);
        renderable r_cube = shape_maker::cube();

        print_info();
        /* define the viewport  */
        glViewport(0, 0, width, height);

        /* avoid rendering back faces */
        // uncomment to see the plane disappear when rotating it
        glDisable(GL_CULL_FACE);

        tb[0].reset();
        tb[0].set_center_radius(glm::vec3(0, 0, 0), 3.f);
        tb[1].reset();
        tb[1].set_center_radius(glm::vec3(0, 0, 0), 3.f);
        curr_tb = 0;

        proj = glm::frustum(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
//	proj = glm::ortho(-300.f, 300.f, -300.f, 300.f, 1.f, 1000.f);
//	view = glm::lookAt(glm::vec3(0,0,512), glm::vec3(0,0,0), glm::vec3(0.f, 1.f, 0.f));
        view = glm::lookAt(glm::vec3(0, 0, 4 ), glm::vec3(0, 0, 0), glm::vec3(0.f, 1.f, 0.f));
        //	view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 proj_inv = glm::inverse(proj);
        glm::mat4 view_inv = glm::inverse(view);

        glUseProgram(tex_shader3d.program);
        glUniform3f(tex_shader3d["uNCells"], w, h, d);
        glUniform1i(tex_shader3d["uVolume"], 1);
        glUseProgram(0);

        glUseProgram(rt_shader.program);
        glUniformMatrix4fv(rt_shader["uProj_inv"], 1, GL_FALSE, &proj_inv[0][0]);
        glUniform2i(rt_shader["uResolution"], width, height);
        glUniform1i(rt_shader["uVolume"], 1);
        glUniform3f(rt_shader["uNCells"], w,h,d);
        glUseProgram(0);
        int _ = true;

        frame_buffer_object fbo;
        fbo.create(width, height);

        int nf = 0;
        int cstart = clock();



        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
                if (clock() - cstart > 1000) {
                        std::cout << nf << std::endl;
                        nf = 0;
                        cstart = clock();
                }
                nf++;

                /* Render here */
                glClearColor(0.4, 0.4, 0.4, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                check_gl_errors(__LINE__, __FILE__, false);

                if (slice) {
                        glDisable(GL_CULL_FACE);
                        stack.push();
                        stack.mult(tb[0].matrix());

                        glUseProgram(basic_shader.program);
                        glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
                        glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
                        glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
                        r_cube.bind();
                        glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
                        glUseProgram(0);


                        stack.mult(tb[1].matrix());
                        stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, plane_y, 0)));


                        glm::mat4 pM = glm::translate(glm::mat4(1.f), glm::vec3(0, plane_y, 0));
                        pM = tb[1].matrix()*pM;

                        glUseProgram(tex_shader3d.program);
                        glUniformMatrix4fv(tex_shader3d["uModelPlane"], 1, GL_FALSE, &pM[0][0]);
                        glUniformMatrix4fv(tex_shader3d["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
                        glUniformMatrix4fv(tex_shader3d["uProj"] , 1, GL_FALSE, &proj[0][0]);
                        glUniformMatrix4fv(tex_shader3d["uView"] , 1, GL_FALSE, &view[0][0]);
                        r_plane.bind();
                        glDrawElements(r_plane().mode, r_plane().count, r_plane().itype,0);
                        glUseProgram(0);
                        stack.pop();
                }
                else
                {

                        glUseProgram(rt_shader.program);
                        glUniformMatrix4fv(rt_shader["uView_inv"], 1, GL_FALSE, &view_inv[0][0]);

                        stack.push();
                        stack.mult(tb[0].matrix());
                //	stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1.5, 1.0, 1.0)));
                        stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1 / 64.f, 1 / 128.f,1/128.f)));
                        stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-64, -128, -128)));
                        glUniformMatrix4fv(rt_shader["uModel_inv"], 1, GL_FALSE, &glm::inverse(stack.m())[0][0]);
                        stack.pop();

                        if (tb[0].is_changed()) {
                                glBindFramebuffer(GL_FRAMEBUFFER, fbo.id_fbo);
                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
                                glClear(GL_COLOR_BUFFER_BIT);
                                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                                glUniform1i(rt_shader["uMaxSteps"], 1000);
                        }
                        else
                                glUniform1i(rt_shader["uMaxSteps"], 1000);

                        check_gl_errors(__LINE__, __FILE__, false);
                        glUseProgram(rt_shader.program);
                        glUniform1i(rt_shader["iTime"], (int)clock());
                        glDispatchCompute((unsigned int)width / 32, (unsigned int)height / 32, 1);
                        check_gl_errors(__LINE__, __FILE__, false);


                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, texture);

                        glUseProgram(tex_shader.program);
                        glUniform1i(tex_shader["tex"], 0);
                        glUniformMatrix4fv(tex_shader["uModel"], 1, GL_FALSE, &glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))[0][0]);
                        r_plane.bind();
                        glDrawElements(r_plane().mode, r_plane().count, r_plane().itype, 0);
                        glUseProgram(0);
                }

                /* Swap front and back buffers */
                glfwSwapBuffers(window);

                /* Poll for and process events */
                glfwPollEvents();
        }

        glfwTerminate();
        return 0;
}
