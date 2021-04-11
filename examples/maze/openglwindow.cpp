#include "openglwindow.h"

#include <tiny_obj_loader.h>

#include <iostream>
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
    if (ev.key.keysym.sym == SDLK_q && m_truckSpeed < 0) m_truckSpeed = 0.0f;
    if (ev.key.keysym.sym == SDLK_e && m_truckSpeed > 0) m_truckSpeed = 0.0f;
  }
}

void OpenGLWindow::initializeGL() {
  glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  glEnable(GL_DEPTH_TEST);

  // Create program
  m_program = createProgramFromFile(getAssetsPath() + "lookat.vert",
                                    getAssetsPath() + "lookat.frag");
  // Load model
  m_model.loadFromFile(getAssetsPath() + "Maze.obj");
  m_model_map.loadFromFile(getAssetsPath() + "Maze.obj");
  m_model.setupVAO(m_program);
  m_model_map.setupVAO(m_program);

  resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  glUseProgram(m_program);
  glBindVertexArray(m_model.m_VAO);
  glBindVertexArray(m_model_map.m_VAO);

  // Get location of uniform variables (could be precomputed)
  GLint viewMatrixLoc{glGetUniformLocation(m_program, "viewMatrix")};
  GLint projMatrixLoc{glGetUniformLocation(m_program, "projMatrix")};
  GLint modelMatrixLoc{glGetUniformLocation(m_program, "modelMatrix")};
  GLint colorLoc{glGetUniformLocation(m_program, "color")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);

  if (m_map) {
    // Labirinto
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(1.0f));

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
    glUniform4f(colorLoc, 0.1f, 0.1f, 0.1f, 0.1f);
    glDrawElements(GL_TRIANGLES, m_model.m_indices.size(), GL_UNSIGNED_INT,
                   nullptr);

    // Mapa labirinto
    model = glm::mat4(1.0f);
    model = glm::translate(model, m_position_map);
    model =
        glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.003f));

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
    glUniform4f(colorLoc, 0.9f, 0.8f, 0.7f, 0.6f);
    glDrawElements(GL_TRIANGLES, m_model_map.m_indices.size(), GL_UNSIGNED_INT,
                   nullptr);
  } else {
    // Labirinto
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(1.0f));

    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &model[0][0]);
    glUniform4f(colorLoc, 1.0f, 0.9f, 0.7f, 0.6f);
    glDrawElements(GL_TRIANGLES, m_model.m_indices.size(), GL_UNSIGNED_INT,
                   nullptr);
  }

  glBindVertexArray(0);
  glUseProgram(0);
}

void OpenGLWindow::paintUI() {}

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
