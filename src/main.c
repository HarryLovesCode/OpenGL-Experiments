#include <stdio.h>
#include <stdlib.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#include <GLES3/gl3.h>

#include "callbacks.h"
#include "loader.h"
#include "scene.h"
#include "util.h"

GLFWwindow *window; // GLFW window
GLuint vao, vbos[3]; // Vertex buffers
GLuint prog, tex, u_view, u_proj, u_mod, u_lpos, u_lcol, u_cpos; // GLSL related variables

Camera cam;
Object world;
Mesh model;
int width, height;
double mouse_x, mouse_y;

void draw() {
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(prog);

    camera_update(&cam);
    camera_aspect(&cam, width, height);
    camera_translation(&cam, 0, 0, 0);

    object_update(&world);
    object_translation(&world, 0, 0, -3);
    object_rotation(&world, 0.0, -mouse_x * 0.005, 0.0);

    glUniformMatrix4fv(u_proj, 1, GL_FALSE, &cam.m_pro[0][0]);
    glUniformMatrix4fv(u_view, 1, GL_FALSE, &cam.m_view[0][0]);
    glUniformMatrix4fv(u_mod, 1, GL_FALSE, &world.m_out[0][0]);

    glUniform3f(u_lpos, 0, 0, 0);
    glUniform3f(u_lcol, 1, 1, 1);
    glUniform3f(u_cpos, 0, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, model.draw_count);

    glUseProgram(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        fputs("Failed to initialize GLFW", stderr);
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 16);

    window = glfwCreateWindow(1920, 1080, "Hello World!", NULL, NULL);

    if (!window) {
        fputs("Failed to create GLFW window", stderr);
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfw_key_callback);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    prog = load_program("./res/vs.glsl", "./res/fs.glsl");
    tex = load_texture("./res/helmet.jpg");
    u_proj = glGetUniformLocation(prog, "m_proj");
    u_view = glGetUniformLocation(prog, "m_view");
    u_mod = glGetUniformLocation(prog, "m_mod");
    u_lpos = glGetUniformLocation(prog, "l_pos");
    u_lcol = glGetUniformLocation(prog, "l_col");
    u_cpos = glGetUniformLocation(prog, "c_pos");


    mesh_load(&model, "../res/helmet.obj");
    cam = camera_create(1.0f, 1920, 1080, 0.1f, 100.0f);
    world = object_create();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(3, vbos);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, model.v_size, &model.v_data.data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, model.n_size, &model.n_data.data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
    glBufferData(GL_ARRAY_BUFFER, model.u_size, &model.u_data.data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    EmscriptenFullscreenStrategy strategy;
    strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
    strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
    strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    emscripten_request_fullscreen_strategy(NULL, 1, &strategy);

    emscripten_set_main_loop(draw, 0, 0);

    exit(EXIT_SUCCESS);
}
