#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\matrix_stack.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "..\common\gltf_loader.h"

/* 
GLM library for math  https://github.com/g-truc/glm 
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly. 
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  



int main(int arcgc, char**argv)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_08_gltf_loader", NULL, NULL);
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


	// declare a gltf_loader
	gltf_loader gltfL;

	box3 bbox;
	std::vector <renderable> obj;

	// load a gltf scene into a vector of objects of type renderable "obj"
	// alo return a box containing the whole scene
	gltfL.load_to_renderable(argv[1],obj, bbox);

	glm::mat4 proj = glm::perspective(glm::radians(45.f), 1.f, 0.2f, 20.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 1, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);

	matrix_stack stack;

	glEnable(GL_DEPTH_TEST);

	float angle = 0;
	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		/* one full rotation every 6 seconds*/
		angle = (60.f*clock() / CLOCKS_PER_SEC);
		
		glUniformMatrix4fv(basic_shader["uRot"], 1, GL_FALSE, &glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 1.f, 0.f))[0][0]);
		stack.load_identity();
		stack.push();
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		stack.push();
		float scale = 1.f / bbox.diagonal();
		//transate and scale so the the whole scene is included in the unit cube centered in 
		// the origin in workd space
 		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, scale)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-bbox.center())));

		// render each renderable
		for (unsigned int i = 0; i < obj.size(); ++i) {
			obj[i].bind();
			stack.push();
			// each object had its own transformation that was read in the gltf file
			stack.mult(obj[i].transform);

			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
			glDrawElements(obj[i]().mode, obj[i]().count, obj[i]().itype, 0);
			stack.pop();
		}
		stack.pop();
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
