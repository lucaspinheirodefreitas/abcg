#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imfilebrowser.h"

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_SPACE && m_map) {
      m_map = false;
    } else if (ev.key.keysym.sym == SDLK_SPACE) {
      m_map = true;
    }
    if (!m_map) {
      if (ev.key.keysym.sym == SDLK_UP && m_frente) {
        m_dollySpeed = 2.0f;
      }
      else if (ev.key.keysym.sym == SDLK_DOWN) {
        m_dollySpeed = -2.0f;
        m_frente = true;
      }
      else if (ev.key.keysym.sym == SDLK_LEFT) m_panSpeed = -2.0f;
      else if (ev.key.keysym.sym == SDLK_RIGHT) m_panSpeed = 2.0f;
    } else {
      if (ev.key.keysym.sym == SDLK_LEFT) m_panSpeed = -2.0f;
      else if (ev.key.keysym.sym == SDLK_RIGHT) m_panSpeed = 2.0f;
    }


  }
  if (ev.type == SDL_KEYUP) {
    if ((ev.key.keysym.sym == SDLK_UP) && m_dollySpeed > 0) m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_DOWN) && m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_LEFT) && m_panSpeed < 0) m_panSpeed = 0.0f;
    if ((ev.key.keysym.sym == SDLK_RIGHT) && m_panSpeed > 0) m_panSpeed = 0.0f;
  }
}

void OpenGLWindow::initializeGL() {
  glClearColor(0, 0, 0, 1);
  glEnable(GL_DEPTH_TEST);

  // Create programs
  for (const auto& name : m_shaderNames) {
    auto path{getAssetsPath() + "shaders/" + name};
    auto program{createProgramFromFile(path + ".vert", path + ".frag")};
    m_programs.push_back(program);
  }

  // Load default model
  loadModel(getAssetsPath() + "Maze.obj");
  m_mappingMode = 0;  // "From mesh" option
}

void OpenGLWindow::loadModel(std::string_view path) {
  m_model.loadDiffuseTexture(getAssetsPath() + "maps/brick.png");
  m_model.loadNormalTexture(getAssetsPath() + "maps/brick_normal.png");
  m_model.loadFromFile(path);
  m_model.setupVAO(m_programs.at(m_currentProgramIndex));
  m_trianglesToDraw = m_model.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_model.getKa();
  m_Kd = m_model.getKd();
  m_Ks = m_model.getKs();
  m_shininess = m_model.getShininess();
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  // Use currently selected program
  const auto program{m_programs.at(m_currentProgramIndex)};
  glUseProgram(program);

  // Get location of uniform variables
  GLint viewMatrixLoc{glGetUniformLocation(program, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(program, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(program, "modelMatrix")};
  GLint normalMatrixLoc{glGetUniformLocation(program, "normalMatrix")};
  GLint lightDirLoc{glGetUniformLocation(program, "lightDirWorldSpace")};
  GLint shininessLoc{glGetUniformLocation(program, "shininess")};
  GLint IaLoc{glGetUniformLocation(program, "Ia")};
  GLint IdLoc{glGetUniformLocation(program, "Id")};
  GLint IsLoc{glGetUniformLocation(program, "Is")};
  GLint KaLoc{glGetUniformLocation(program, "Ka")};
  GLint KdLoc{glGetUniformLocation(program, "Kd")};
  GLint KsLoc{glGetUniformLocation(program, "Ks")};
  GLint diffuseTexLoc{glGetUniformLocation(program, "diffuseTex")};
  GLint normalTexLoc{glGetUniformLocation(program, "normalTex")};
  GLint mappingModeLoc{glGetUniformLocation(program, "mappingMode")};

  // Set uniform variables used by every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  glUniform1i(diffuseTexLoc, 0);
  glUniform1i(normalTexLoc, 1);
  glUniform1i(mappingModeLoc, m_mappingMode);

  auto lightDirRotated{m_modelMatrix * m_lightDir};
  glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);

  // Set uniform variables of the current object
  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  auto modelViewMatrix{glm::mat3(m_viewMatrix * m_modelMatrix)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);

  if (m_map) {
    m_position_map = m_camera.m_at;
    // Mapa labirinto
    glm::mat4 model{1.0f};
    model = glm::mat4(1.0f);
    model = glm::translate(model, m_position_map);
    model =
        glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, m_scale_map);

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  } else {
    // Labirinto
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, m_scale_maze);

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  }

  m_model.render(m_trianglesToDraw, m_map);

  glUseProgram(0);
}

void OpenGLWindow::paintUI() {

  // File browser for models
  static ImGui::FileBrowser fileDialogModel;
  fileDialogModel.SetTitle("Load 3D Model");
  fileDialogModel.SetTypeFilters({".obj"});
  fileDialogModel.SetWindowSize(m_viewportWidth * 0.8f,
                                m_viewportHeight * 0.8f);

// Only in WebGL
#if defined(__EMSCRIPTEN__)
  fileDialogModel.SetPwd(getAssetsPath());
  fileDialogDiffuseMap.SetPwd(getAssetsPath() + "/maps");
  fileDialogNormalMap.SetPwd(getAssetsPath() + "/maps");
#endif

  // Create main window widget
  {
    auto widgetSize{ImVec2(220, 80)};

    if (!m_model.isUVMapped()) {
      // Add extra space for static text
      widgetSize.y += 26;
    }

    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    auto flags{ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration};
    ImGui::Begin("Widget window", nullptr, flags);

    // Menu
    {
      bool loadModel{};
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          ImGui::MenuItem("Load 3D Model...", nullptr, &loadModel);
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }
      if (loadModel) fileDialogModel.Open();
    }

    // Shader combo box
    {
      static std::size_t currentIndex{};

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("Shader", m_shaderNames.at(currentIndex))) {
        for (auto index : iter::range(m_shaderNames.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(m_shaderNames.at(index), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      // Set up VAO if shader program has changed
      if (static_cast<int>(currentIndex) != m_currentProgramIndex) {
        m_currentProgramIndex = currentIndex;
        m_model.setupVAO(m_programs.at(m_currentProgramIndex));
      }
    }

    // UV mapping box
    {
      std::vector<std::string> comboItems{"Triplanar", "Cylindrical",
                                          "Spherical"};

      if (m_model.isUVMapped()) comboItems.emplace_back("From mesh");

      ImGui::PushItemWidth(120);
      if (ImGui::BeginCombo("UV mapping",
                            comboItems.at(m_mappingMode).c_str())) {
        for (auto index : iter::range(comboItems.size())) {
          const bool isSelected{m_mappingMode == static_cast<int>(index)};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            m_mappingMode = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();
    }

    ImGui::End();
  }

  fileDialogModel.Display();
  if (fileDialogModel.HasSelected()) {
    loadModel(fileDialogModel.GetSelected().string());
    fileDialogModel.ClearSelected();

    if (m_model.isUVMapped()) {
      // Use mesh texture coordinates if available...
      m_mappingMode = 3;
    } else {
      // ...or triplanar mapping otherwise
      m_mappingMode = 0;
    }
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_camera.computeProjectionMatrix(width, height, m_projMatrix);
}

void OpenGLWindow::terminateGL() {
  for (const auto& program : m_programs) {
    glDeleteProgram(program);
  }
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime, m_viewMatrix);
  m_camera.truck(m_truckSpeed * deltaTime, m_viewMatrix);
  m_camera.pan(m_panSpeed * deltaTime, m_viewMatrix);

  if(m_camera.m_eye.x+0.1f>=m_camera.m_at.x) {
    m_frente=false;
  }
}