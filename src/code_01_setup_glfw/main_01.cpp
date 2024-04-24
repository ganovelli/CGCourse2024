#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
 
/* printout few basic info about your hardware*/
void printout_opengl_glsl_info() {
    const GLubyte* renderer		= glGetString(GL_RENDERER);
    const GLubyte* vendor		= glGetString(GL_VENDOR);
    const GLubyte* version		= glGetString(GL_VERSION);
    const GLubyte* glslVersion	= glGetString(GL_SHADING_LANGUAGE_VERSION);

    std::cout << "GL Vendor            :" << vendor << std::endl;
    std::cout << "GL Renderer          :" << renderer << std::endl;
    std::cout << "GL Version (string)  :" << version << std::endl;
    std::cout << "GLSL Version         :" << glslVersion << std::endl;

}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
  

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_01_setup_glfw_glew", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

 
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewInit();

    printout_opengl_glsl_info();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */

		/* clear the color buffer */
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.5, 0.0, 0.0);
        glVertex3f(0.5, 0.5, 0.0);
        glEnd();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
