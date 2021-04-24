#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
using namespace std;

using namespace std;

void Camera::computeProjectionMatrix(int width, int height, glm::mat4 &m_projMatrix) {
  m_projMatrix = glm::mat4(1.0f);
  auto aspect{static_cast<float>(width) / static_cast<float>(height)};
  m_projMatrix = glm::perspective(glm::radians(90.0f), aspect, 0.1f, 15.0f);
}

void Camera::computeViewMatrix(glm::mat4 &m_viewMatrix) {
  m_viewMatrix = glm::lookAt(m_eye, m_at, m_up);
}

void Camera::dolly(float speed, glm::mat4 &m_viewMatrix) {
  // Compute forward vector (view direction)
  glm::vec3 forward = glm::normalize(m_at - m_eye);

  // Move eye and center forward (speed > 0) or backward (speed < 0)
  m_eye += forward * speed;
  m_at += forward * speed;

  computeViewMatrix(m_viewMatrix);
}

void Camera::truck(float speed, glm::mat4 &m_viewMatrix) {
  // Compute forward vector (view direction)
  glm::vec3 forward = glm::normalize(m_at - m_eye);
  // Compute vector to the left
  glm::vec3 left = glm::cross(m_up, forward);

  // Move eye and center to the left (speed < 0) or to the right (speed > 0)
  m_at -= left * speed;
  m_eye -= left * speed;

  computeViewMatrix(m_viewMatrix);
}

void Camera::pan(float speed, glm::mat4 &m_viewMatrix) {
  glm::mat4 transform{glm::mat4(1.0f)};

  // Rotate camera around its local y axis
  transform = glm::translate(transform, m_eye);
  transform = glm::rotate(transform, -speed, m_up);
  transform = glm::translate(transform, -m_eye);

  m_at = transform * glm::vec4(m_at, 1.0f);

  computeViewMatrix(m_viewMatrix);
}
