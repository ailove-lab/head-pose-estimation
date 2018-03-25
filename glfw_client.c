
#include "zhelpers.h"

#include "glad.h"
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>

static const struct {
    float x, y, z;
} face[58] = {
    {-7.308957, 0.913869, 0.000000},
    {-6.775290,-0.730814,-0.012799},
    {-5.665918,-3.286078, 1.022951},
    {-5.011779,-4.876396, 1.047961},
    {-4.056931,-5.947019, 1.636229},
    {-1.833492,-7.056977, 4.061275},
    { 0.000000,-7.415691, 4.070434},
    { 1.833492,-7.056977, 4.061275},
    { 4.056931,-5.947019, 1.636229},
    { 5.011779,-4.876396, 1.047961},
    { 5.665918,-3.286078, 1.022951},
    { 6.775290,-0.730814,-0.012799},
    { 7.308957, 0.913869, 0.000000},
    { 5.311432, 5.485328, 3.987654},
    { 4.461908, 6.189018, 5.594410},
    { 3.550622, 6.185143, 5.712299},
    { 2.542231, 5.862829, 4.687939},
    { 1.789930, 5.393625, 4.413414},
    { 2.693583, 5.018237, 5.072837},
    { 3.530191, 4.981603, 4.937805},
    { 4.490323, 5.186498, 4.694397},
    {-5.311432, 5.485328, 3.987654},
    {-4.461908, 6.189018, 5.594410},
    {-3.550622, 6.185143, 5.712299},
    {-2.542231, 5.862829, 4.687939},
    {-1.789930, 5.393625, 4.413414},
    {-2.693583, 5.018237, 5.072837},
    {-3.530191, 4.981603, 4.937805},
    {-4.490323, 5.186498, 4.694397},
    { 1.330353, 7.122144, 6.903745},
    { 2.533424, 7.878085, 7.451034},
    { 4.861131, 7.878672, 6.601275},
    { 6.137002, 7.271266, 5.200823},
    { 6.825897, 6.760612, 4.402142},
    {-1.330353, 7.122144, 6.903745},
    {-2.533424, 7.878085, 7.451034},
    {-4.861131, 7.878672, 6.601275},
    {-6.137002, 7.271266, 5.200823},
    {-6.825897, 6.760612, 4.402142},
    {-2.774015,-2.080775, 5.048531},
    {-0.509714,-1.571179, 6.566167},
    { 0.000000,-1.646444, 6.704956},
    { 0.509714,-1.571179, 6.566167},
    { 2.774015,-2.080775, 5.048531},
    { 0.589441,-2.958597, 6.109526},
    { 0.000000,-3.116408, 6.097667},
    {-0.589441,-2.958597, 6.109526},
    {-0.981972, 4.554081, 6.301271},
    {-0.973987, 1.916389, 7.654050},
    {-2.005628, 1.409845, 6.165652},
    {-1.930245, 0.424351, 5.914376},
    {-0.746313, 0.348381, 6.263227},
    { 0.000000, 0.000000, 6.763430},
    { 0.746313, 0.348381, 6.263227},
    { 1.930245, 0.424351, 5.914376},
    { 2.005628, 1.409845, 6.165652},
    { 0.973987, 1.916389, 7.654050},
    { 0.981972, 4.554081, 6.301271}};

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"uniform vec3 color;\n"
"attribute vec3 vPos;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"uniform vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "zmq clint", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(face), face, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location  = glGetUniformLocation(program, "MVP");
    vcol_location = glGetUniformLocation(program, "color");
    vpos_location = glGetAttribLocation(program, "vPos");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(face[0]), (void*) 0);

    void* ctx = zmq_ctx_new();
    void* sub = zmq_socket(ctx, ZMQ_SUB);
    zmq_connect(sub, "tcp://localhost:5555");
    zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0);
    
    while (!glfwWindowShouldClose(window)) {

        float ratio;
        int width, height;
        mat4x4 m, v, p, mvp;

        
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
 
        mat4x4_identity(m);
        //mat4x4_rotate_Y(m, m, (float) glfwGetTime());
        //mat4x4_rotate_X(m, m, (float) glfwGetTime()*0.3);

        // double msg[16];
        // int s = zmq_recv(sub, (void*)msg, 128, 0);
        // for(int i=0; i<16; i++) printf("%.2f ", msg[i]); printf("\n");
        // for(int i=0; i<16; i++) m[i%4][i/4] = (float)msg[i];

        mat4x4_perspective(p, 3.14159/2.0, ratio, 0.001, 100.0);

        mat4x4_look_at(v, 
            (vec3){0.0, 5.0, 20.0}, // eye  
            (vec3){0.0, 0.0,  0.0}, // target
            (vec3){0.0, 1.0,  0.0});// up
        mat4x4_mul(p, p, v);
        mat4x4_mul(mvp, p, m);


        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glUniform3f(vcol_location, 1.0,1.0,1.0);
        glDrawArrays(GL_POINTS, 0, 58);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    zmq_close(sub);
    zmq_ctx_destroy(ctx);
        
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

