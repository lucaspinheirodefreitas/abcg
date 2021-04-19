#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class OpenGLWindow;

class Camera {
 public:
  void computeViewMatrix(glm::mat4 &m_viewMatrix);
  void computeProjectionMatrix(int width, int height, glm::mat4 &m_projMatrix);

  void dolly(float speed, glm::mat4 &m_viewMatrix);
  void truck(float speed, glm::mat4 &m_viewMatrix);
  void pan(float speed, glm::mat4 &m_viewMatrix);
  [[nodiscard]] glm::mat4 getRotation();

 private:
  friend OpenGLWindow;
  glm::mat4 m_rotation{1.0f};

  glm::vec3 m_eye{glm::vec3(0.0f, 0.5f, 0.5f)};  // Camera position
  glm::vec3 m_at{glm::vec3(0.0f, 0.5f, 0.0f)};   // Look-at point
  glm::vec3 m_up{glm::vec3(0.0f, 0.1f, 0.0f)};   // "up" direction
};


#endif