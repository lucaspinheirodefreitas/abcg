#ifndef OPENGLWINDOW_H_
#define OPENGLWINDOW_H_

#include "abcg.hpp"
#include "model.hpp"
#include "camera.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& ev) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  GLuint m_program{};

  int m_viewportWidth{};
  int m_viewportHeight{};

  Model m_model_map;
  bool m_map = false;
  glm::vec3 m_position_map{glm::vec3(0)};
  Model m_model;

  Camera m_camera;
  float m_dollySpeed{0.0f};
  float m_truckSpeed{0.0f};
  float m_panSpeed{0.0f};

  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  void update();
};

#endif