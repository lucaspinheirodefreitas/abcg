#include "openglwindow.h"

#include <tiny_obj_loader.h>

#include <glm/gtc/matrix_inverse.hpp>

using namespace std;

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_SPACE && m_map) {
      m_map = false;
    } else if (ev.key.keysym.sym == SDLK_SPACE) {
      m_map = true;
    }
    if (!m_map) {
      if (ev.key.keysym.sym == SDLK_UP) m_dollySpeed = 1.0f;
      if (ev.key.keysym.sym == SDLK_DOWN) m_dollySpeed = -1.0f;
    }

    if (ev.key.keysym.sym == SDLK_LEFT) m_panSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_RIGHT) m_panSpeed = 1.0f;
    if (ev.key.keysym.sym == SDLK_q) m_truckSpeed = -1.0f;
    if (ev.key.keysym.sym == SDLK_e) m_truckSpeed = 1.0f;
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

  // Enable depth buffering
  glEnable(GL_DEPTH_TEST);

  // Create program
  // Mudar para normal basta substituir aqui o path do arquivo:

  m_program = createProgramFromFile(getAssetsPath() + "/shaders/blinnphong.vert",
                                    getAssetsPath() + "/shaders/blinnphong.frag");
  // Load model
  m_model.loadFromFile(getAssetsPath() + "Maze.obj");
  m_trianglesToDraw = m_model.getNumTriangles();
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  // Use currently selected program
  glUseProgram(m_program);

  // Get location of uniform variables
  GLint viewMatrixLoc{glGetUniformLocation(m_program, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_program, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(m_program, "modelMatrix")};
  GLint normalMatrixLoc{glGetUniformLocation(m_program, "normalMatrix")};
  GLint lightDirLoc{glGetUniformLocation(m_program, "lightDirWorldSpace")};
  GLint shininessLoc{glGetUniformLocation(m_program, "shininess")};
  GLint IaLoc{glGetUniformLocation(m_program, "Ia")};
  GLint IdLoc{glGetUniformLocation(m_program, "Id")};
  GLint IsLoc{glGetUniformLocation(m_program, "Is")};
  GLint KaLoc{glGetUniformLocation(m_program, "Ka")};
  GLint KdLoc{glGetUniformLocation(m_program, "Kd")};
  GLint KsLoc{glGetUniformLocation(m_program, "Ks")};
  // Set uniform variables used by every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  glUniform4fv(lightDirLoc, 1.0f, &m_camera.m_at.x);
  glUniform1f(shininessLoc, m_shininess);
  glUniform4fv(IaLoc, 1, &m_Ia.x);
  glUniform4fv(IdLoc, 1, &m_Id.x);
  glUniform4fv(IsLoc, 1, &m_Is.x);
  glUniform4fv(KaLoc, 1, &m_Ka.x);
  glUniform4fv(KdLoc, 1, &m_Kd.x);
  glUniform4fv(KsLoc, 1, &m_Ks.x);
  // Set uniform variables of the current object
  glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);

  auto modelViewMatrix{glm::mat3(m_viewMatrix * m_modelMatrix)};
  glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);
  if (m_map) {
    // Mapa labirinto
    glm::mat4 model{1.0f};
    model = glm::mat4(1.0f);
    model = glm::translate(model, m_position_map);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.003f));

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  } else {
    // Labirinto
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(1.0f, 5.0f, 1.0f));

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
  }
  m_model.render(m_trianglesToDraw);

  glUseProgram(0);
}

void OpenGLWindow::paintUI() { m_model.setupVAO(m_program); }

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  m_camera.computeProjectionMatrix(width, height, m_projMatrix);
}

void OpenGLWindow::terminateGL() {
  glDeleteProgram(m_program);
  glDeleteBuffers(1, &m_model.m_EBO);
  glDeleteBuffers(1, &m_model.m_VBO);
  glDeleteVertexArrays(1, &m_model.m_VAO);
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime, m_viewMatrix);
  m_camera.truck(m_truckSpeed * deltaTime, m_viewMatrix);
  m_camera.pan(m_panSpeed * deltaTime, m_viewMatrix);

  m_position_map.x = m_camera.m_at.x;
  m_position_map.y = m_camera.m_at.y;
  m_position_map.z = m_camera.m_at.z;
}
