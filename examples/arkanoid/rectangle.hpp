#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include <list>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Rectangle {
 public:
  void initializeGL(GLuint program, int quantity);
  void paintGL();
  void terminateGL();

  void update(float deltaTime);

 private:
  friend OpenGLWindow;

  GLuint m_program{};
  GLint m_colorLoc{};
  GLint m_rotationLoc{};
  GLint m_translationLoc{};
  GLint m_scaleLoc{};

  struct Rectangles {
    GLuint m_vao{};
    GLuint m_vbo{};
    GLuint m_ebo{};

    float m_angularVelocity{};
    glm::vec4 m_color{1};
    bool m_hit{false};
    glm::vec2 m_translation{glm::vec2(0)};
    glm::vec2 m_velocity{glm::vec2(0)};
  };
  float m_scale = 0.1f;
  float m_rotation = 0.0f;
  std::list<Rectangles> m_rectangles;

  std::default_random_engine m_randomEngine;

  Rectangle::Rectangles createAsteroid(int, int);
};

#endif  // ABCG_RECTANGLE_HPP
