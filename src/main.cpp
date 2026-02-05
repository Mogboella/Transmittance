
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_style.h"

#include <string>

#include "camera.h"
#include "model.h"
#include "shaders.h"

using namespace std;

int width = 1920, height = 1080;
bool showUI = true;
const char *project_name = "Lab 2 - Transmitance Effects";

Camera camera(0.0f, 5.0f, 30.0f, 0.0f, 1.0f, 0.0f, -90.0f, -5.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// -- skybox faces ---
const char *vaticanFaces[6] = {
    "assets/skybox/vatican/right.jpg", "assets/skybox/vatican/left.jpg",
    "assets/skybox/vatican/top.jpg",   "assets/skybox/vatican/bottom.jpg",
    "assets/skybox/vatican/front.jpg", "assets/skybox/vatican/back.jpg"};

const char *dockFaces[6] = {
    "assets/skybox/docks/right.png", "assets/skybox/docks/left.png",
    "assets/skybox/docks/top.png",   "assets/skybox/docks/bottom.png",
    "assets/skybox/docks/front.png", "assets/skybox/docks/back.png"};

const char **cubemapOptions[] = {dockFaces, vaticanFaces};
const char *cubemapNames[] = {"Dock", "Vatican"};
int currentCubemap = 0;

// --- globals for input ---
Camera *gCamera = nullptr;
float gLastX = 0.0f;
float gLastY = 0.0f;
bool gFirstMouse = true;

struct TransmittanceVars {
  bool useReflection = true;
  bool useRefraction = true;
  bool useFresnel = true;
  bool useDispersion = true;

  float IOR = 1.52f;
  float dispersionStrength = 0.01f;
  float fresnelBase = 0.04f;

  bool rotate = true;
  float rotateSpeedDeg = 30.0f;
  float rotateAngleDeg = 0.0f;

  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
  glm::vec3 rotAxis = glm::vec3(0, 1, 0);

  glm::vec3 baseRotationDeg = glm::vec3(0.0f);
};

struct SceneObject {
  std::string name;
  Model model;
  TransmittanceVars p;

  SceneObject(const std::string &n, const std::string &path)
      : name(n), model(path.c_str()) {}
};

void framebuffer_size_callback(GLFWwindow *window, int w, int h) {
  glViewport(0, 0, w, h);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {

  if (showUI)
    return;

  if (!gCamera)
    return;

  if (gFirstMouse) {
    gLastX = (float)xpos;
    gLastY = (float)ypos;
    gFirstMouse = false;
  }

  float xoffset = (float)xpos - gLastX;
  float yoffset = gLastY - (float)ypos; // reversed: y ranges bottom->top
  gLastX = (float)xpos;
  gLastY = (float)ypos;

  gCamera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  if (!gCamera)
    return;
  gCamera->ProcessMouseScroll((float)yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
    showUI = !showUI;
    glfwSetInputMode(window, GLFW_CURSOR,
                     showUI ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

    gFirstMouse = true;
  }
}

void processInput(GLFWwindow *window) {

  if (showUI)
    return;

  if (!gCamera)
    return;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    gCamera->ProcessKeyboard(FRONT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    gCamera->ProcessKeyboard(BACK, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    gCamera->ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    gCamera->ProcessKeyboard(RIGHT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    gCamera->ProcessKeyboard(UP, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    gCamera->ProcessKeyboard(DOWN, deltaTime);
}

int main() {

  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
      glfwCreateWindow(width, height, project_name, nullptr, nullptr);

  if (!window) {
    cerr << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cerr << "Failed to initialize GLAD\n";
    return -1;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ApplyCustomStyle();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330 core");

  gCamera = &camera;
  gLastX = width / 2.0f;
  gLastY = height / 2.0f;

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

  // Configure OpenGL
  glEnable(GL_DEPTH_TEST);

  // Load shaders
  Shader shader("shaders/main.vert", "shaders/main.frag");
  Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");
  unsigned int cubemapTexture = skyboxShader.loadCubemap(cubemapOptions[0]);

  // Load Models
  vector<SceneObject> objects;
  objects.emplace_back("Ball", "assets/models/ball.obj");
  objects.emplace_back("Skull", "assets/models/skull.obj");
  objects.emplace_back("Teapot", "assets/models/utah_teapot.obj");
  objects.emplace_back("Ring", "assets/models/ring.obj");
  objects.emplace_back("Teardrop", "assets/models/teardrop.obj");
  objects.emplace_back("Star", "assets/models/star.obj");

  const float spacing = 10.0f;

  // Side-by-side layout centered around origin
  float startX = -spacing * (objects.size() - 1) * 0.5f;
  for (int i = 0; i < (int)objects.size(); i++) {
    objects[i].p.position.x = startX + i * spacing;
    objects[i].p.position.z = 0.0f;
  }

  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

      -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
      -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

      1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

      -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

      -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

  unsigned int skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glBindVertexArray(0);

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Clear buffers
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ImGui UI
    if (showUI) {
      ImGui::Begin("Material Controls");
      static int lastCubemap = currentCubemap;
      if (ImGui::Combo("Cubemap", &currentCubemap, cubemapNames,
                       IM_ARRAYSIZE(cubemapNames))) {
        if (currentCubemap != lastCubemap) {
          cubemapTexture =
              skyboxShader.loadCubemap(cubemapOptions[currentCubemap]);
          lastCubemap = currentCubemap;
        }
      }
      for (int i = 0; i < (int)objects.size(); i++) {
        SceneObject &o = objects[i];

        // Collapsible section per object
        if (ImGui::TreeNode((o.name + "##" + std::to_string(i)).c_str())) {

          ImGui::Checkbox("Reflection", &o.p.useReflection);
          ImGui::Checkbox("Refraction", &o.p.useRefraction);
          ImGui::BeginDisabled(!o.p.useRefraction);
          ImGui::Checkbox("Fresnel", &o.p.useFresnel);
          ImGui::EndDisabled();
          ImGui::Checkbox("Dispersion", &o.p.useDispersion);

          ImGui::Separator();
          // Replace material dropdown with a direct IOR slider (1.0 - 2.5).
          // Disable when refraction is turned off.
          ImGui::BeginDisabled(!o.p.useRefraction);
          ImGui::SliderFloat("Refractive Index (IOR)", &o.p.IOR, 1.0f, 2.5f,
                             "%.3f");
          ImGui::EndDisabled();

          ImGui::BeginDisabled(!o.p.useDispersion || !o.p.useRefraction);
          ImGui::SliderFloat("Dispersion Strength", &o.p.dispersionStrength,
                             0.0f, 0.05f);
          ImGui::EndDisabled();

          ImGui::BeginDisabled(!o.p.useFresnel);
          ImGui::SliderFloat("F0 (Normal-incidence reflectance)",
                             &o.p.fresnelBase, 0.0f, 1.0f);
          ImGui::EndDisabled();

          ImGui::TreePop();
        }
      }

      ImGui::End();
    }

    // Set transformations
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.zoom), (float)width / (float)height, 0.1f, 500.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

    // Use shader and set uniforms
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("cameraPos", camera.position);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    shader.setInt("skybox", 0);

    for (auto &o : objects) {
      // animate rotation
      if (o.p.rotate) {
        o.p.rotateAngleDeg += o.p.rotateSpeedDeg * deltaTime;
        if (o.p.rotateAngleDeg > 360.0f)
          o.p.rotateAngleDeg -= 360.0f;
      }

      // enforce UI constraints: Fresnel requires refraction
      if (!o.p.useRefraction)
        o.p.useFresnel = false;

      // set per-object uniforms
      shader.setBool("useReflection", o.p.useReflection);
      shader.setBool("useRefraction", o.p.useRefraction);
      shader.setBool("useFresnel", o.p.useFresnel);
      shader.setBool("useDispersion", o.p.useDispersion);

      shader.setFloat("refractiveIndex", o.p.IOR);
      shader.setFloat("dispersionStrength", o.p.dispersionStrength);
      shader.setFloat("fresnelBase", o.p.fresnelBase);

      // model matrix
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, o.p.position);
      // Only use the animation rotation, no more 'baseRotationDeg' needed
      model = glm::rotate(model, glm::radians(o.p.rotateAngleDeg), o.p.rotAxis);
      model = glm::scale(model, o.p.scale);
      shader.setMat4("model", model);

      o.model.Draw(shader);
    }

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    skyboxShader.use();
    skyboxShader.setMat4("projection", projection);
    skyboxShader.setMat4("view", skyboxView);
    skyboxShader.setInt("skybox", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    if (showUI) {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();

  return 0;
}
