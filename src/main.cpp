//internal includes
#include "common.h"
#include "ShaderProgram.h"

//External dependencies
#define GLFW_DLL

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "utils.h"

const GLint WIDTH = 1280, HEIGHT = 720;
const GLint SIZE = 128;
const GLint COMPUTE_GROUP_SIZE = 32;
GLuint H_prev, H_curr, H_next;
GLuint VAO, VBO, EBO, NRM;
GLuint BOX_VAO, BOX_VBO;
GLuint DIAMOND_VAO, DIAMOND_VBO;

Camera camera(glm::vec3(0.0f, 1.0f, 2.9f));
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
ShaderProgram DistortionShader;

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;
        camera.ProcessMouseMovement(xoffset, yoffset);
    } else {
        firstMouse = true;
    }
}

void distortion_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        DistortionShader.StartUseShader();
        DistortionShader.SetUniform("SIZE", SIZE);
        DistortionShader.SetUniform("DISTORTION_POS", glm::ivec2(SIZE / 2 - SIZE / 4 + random() % (SIZE / 2),
                                                                 SIZE / 2 - SIZE / 4 + random() % (SIZE / 2)));
        //DistortionShader.SetUniform("DISTORTION_POS", ivec2(75, 80));
        DistortionShader.SetUniform("DISTORTION_VAL", (random() % 5 + 3) / (float) 1000.0);
        glDispatchCompute(SIZE / COMPUTE_GROUP_SIZE, SIZE / COMPUTE_GROUP_SIZE, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        DistortionShader.StopUseShader();
    }
}


void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}


int initGL() {
    //грузим функции opengl через glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return 0;
}

void createBuffers() {
    GLfloat *ZEROS = new GLfloat[SIZE * SIZE]();
    GLint *IDX = new GLint[SIZE * SIZE * 6 * sizeof(GLint)];
    glm::vec4 *VERT = new glm::vec4[SIZE * SIZE];
    glm::vec4 *NORMALS = new glm::vec4[SIZE * SIZE];
    float step = 2.0 / (SIZE - 1);
    for (int j = 0; j < SIZE; j++) {
        for (int i = 0; i < SIZE; i++) {
            VERT[SIZE * j + i] = glm::vec4(-1 + i * step, 1 - j * step, 0, 1.0);
            NORMALS[SIZE * j + i] = glm::vec4(0, 0, -step, 1.0);
            //std::cout << VERT[i + SIZE * j].x << " " << VERT[i + SIZE * j].y << " " << SIZE * j + i << std::endl;
        }
    }
    for (int i = 0, pos = 0; i < SIZE - 1; i++) {
        for (int j = 0; j < SIZE - 1; j++, pos += 6) {
            IDX[pos] = i + SIZE * j;
            IDX[pos + 1] = IDX[pos] + 1;
            IDX[pos + 2] = IDX[pos] + SIZE;

            IDX[pos + 3] = IDX[pos + 1];
            IDX[pos + 4] = IDX[pos + 2];
            IDX[pos + 5] = IDX[pos + 4] + 1;
            //std::cout << IDX[pos] << " " << IDX[pos + 1] << " " << IDX[pos + 2] << " " << IDX[pos + 3] << " " << IDX[pos + 4] << " " << IDX[pos + 5] << std::endl;
        }
    }
    glGenBuffers(1, &H_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, H_prev);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * SIZE * sizeof(GLfloat), ZEROS, GL_STREAM_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, H_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &H_curr);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, H_curr);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * SIZE * sizeof(GLfloat), ZEROS, GL_STREAM_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, H_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &H_next);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, H_next);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * SIZE * sizeof(GLfloat), ZEROS, GL_STREAM_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, H_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * SIZE * sizeof(glm::vec4), VERT, GL_STREAM_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, VBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &NRM);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, NRM);
    glBufferData(GL_SHADER_STORAGE_BUFFER, SIZE * SIZE * sizeof(glm::vec4), NORMALS, GL_STREAM_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, NRM);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, SIZE * SIZE * 6 * sizeof(GLint), IDX, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &BOX_VAO);
    glBindVertexArray(BOX_VAO);
    glGenBuffers(1, &BOX_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, BOX_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(box), &box, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *) (6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &DIAMOND_VAO);
    glBindVertexArray(DIAMOND_VAO);
    glGenBuffers(1, &DIAMOND_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, DIAMOND_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(diamond), &diamond, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *) (6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    delete[] VERT;
    delete[] NORMALS;
    delete[] ZEROS;
    delete[] IDX;
}


void Compute() {
    glDispatchCompute(SIZE / COMPUTE_GROUP_SIZE, SIZE / COMPUTE_GROUP_SIZE, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    GLuint TMP = H_prev;
    H_prev = H_curr;
    H_curr = H_next;
    H_next = TMP;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, H_prev);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, H_curr);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, H_next);
}

int main(int argc, char **argv) {
    if (!glfwInit()) {
        return -1;
    }
    //запрашиваем контекст opengl версии 4.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Shallow water", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, distortion_callback);

    if (initGL() != 0) {
        return -1;
    }

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR) {
        gl_error = glGetError();
    }
    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> WaterShader;
    std::unordered_map<GLenum, std::string> BoxShader;
    std::unordered_map<GLenum, std::string> ComputeShader;
    std::unordered_map<GLenum, std::string> Distortion;
    std::unordered_map<GLenum, std::string> DiamondShader;

    WaterShader[GL_VERTEX_SHADER] = "shaders/vWaterShader.glsl";
    WaterShader[GL_FRAGMENT_SHADER] = "shaders/fWaterShader.glsl";
    ShaderProgram WaterMesh(WaterShader);

    BoxShader[GL_VERTEX_SHADER] = "shaders/vBoxMesh.glsl";
    BoxShader[GL_FRAGMENT_SHADER] = "shaders/fBoxMesh.glsl";
    ShaderProgram BoxMesh(BoxShader);
    auto texture = loadTexture("../texture/SwimmingPool.jpg");
    auto box_model = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -0.895, 0.0));

    DiamondShader[GL_VERTEX_SHADER] = "shaders/vBoxMesh.glsl";
    DiamondShader[GL_FRAGMENT_SHADER] = "shaders/fBoxMesh.glsl";
    ShaderProgram DiamondMesh(DiamondShader);
    auto texture_diamond = loadTexture("../texture/Diamond.jpeg");
    auto diamond_model = glm::translate(glm::mat4(1.0), glm::vec3(0.0, -0.75, 0.0));
    diamond_model = glm::scale(diamond_model, glm::vec3(0.25, 0.25, 0.25));
    diamond_model = glm::rotate(diamond_model, glm::radians(45.0f), glm::vec3(0.0, 0.0, 1.0));
    diamond_model = glm::rotate(diamond_model, glm::radians(45.0f), glm::vec3(1.0, 0.0, 0.0));

    ComputeShader[GL_COMPUTE_SHADER] = "shaders/ComputeShader.glsl";
    ShaderProgram ComputeHeights(ComputeShader);

    Distortion[GL_COMPUTE_SHADER] = "shaders/Distortion.glsl";
    DistortionShader = ShaderProgram(Distortion);

    glfwSwapInterval(1); // force 60 frames per second
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glGenVertexArrays(1, &VAO);
    createBuffers();
    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window)) {
        model = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float) WIDTH / (float) HEIGHT, 0.1f, 100.f);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        glfwPollEvents();
        //очищаем экран каждый кадр
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //DRAW_BOX
        glEnable(GL_CULL_FACE);
        BoxMesh.StartUseShader();
        BoxMesh.SetUniform("model", box_model);
        BoxMesh.SetUniform("projection", projection);
        BoxMesh.SetUniform("view", view);
        glBindVertexArray(BOX_VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, 0, 30);
        BoxMesh.StopUseShader();
        glDisable(GL_CULL_FACE);
        //DRAW_BOX

        //DRAW_Diamond
        DiamondMesh.StartUseShader();
        DiamondMesh.SetUniform("model", diamond_model);
        DiamondMesh.SetUniform("projection", projection);
        DiamondMesh.SetUniform("view", view);
        glBindVertexArray(DIAMOND_VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_diamond);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        DiamondMesh.StopUseShader();
        //DRAW_Diamond

        //DRAW WATER
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        ComputeHeights.StartUseShader();
        ComputeHeights.SetUniform("SIZE", SIZE);
        Compute();
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid *) 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, NRM);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid *) 0);
        glEnableVertexAttribArray(1);
        ComputeHeights.StopUseShader();
        WaterMesh.StartUseShader();
        WaterMesh.SetUniform("model", model);
        WaterMesh.SetUniform("view", view);
        WaterMesh.SetUniform("projection", projection);
        WaterMesh.SetUniform("camera", camera.Position);
        glDrawElements(GL_TRIANGLES, (SIZE - 1) * (SIZE - 1) * 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        WaterMesh.StopUseShader();
        //DRAW WATER
        glfwSwapBuffers(window);
    }

    //очищаем vbo и vao перед закрытием программы
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &BOX_VAO);
    glDeleteVertexArrays(1, &BOX_VBO);
    glDeleteVertexArrays(1, &DIAMOND_VAO);
    glDeleteVertexArrays(1, &DIAMOND_VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
    return 0;
}