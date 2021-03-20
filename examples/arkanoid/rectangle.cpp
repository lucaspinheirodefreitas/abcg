#include "rectangle.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

using namespace std;

void Rectangle::initializeGL(GLuint program, int quantity) {
  terminateGL();

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  m_program = program;
  m_colorLoc = glGetUniformLocation(m_program, "color");
  m_rotationLoc = glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = glGetUniformLocation(m_program, "scale");
  m_translationLoc = glGetUniformLocation(m_program, "translation");

  // Create arkanoid
  m_rectangles.clear();
  m_rectangles.resize(quantity);
  int indiceX = 0, indiceY = 1;

  for (auto &asteroid : m_rectangles) {
    if(indiceX<6) {
      indiceX++;
      asteroid = createAsteroid(indiceX, indiceY);
    } else {
      indiceY++;
      indiceX=1;
      asteroid = createAsteroid(indiceX, indiceY);
    }
  }
}

void Rectangle::paintGL() {
  glUseProgram(m_program);

  for (auto &rectangles : m_rectangles) {

    glBindVertexArray(rectangles.m_vao);

    glUniform1f(m_scaleLoc, m_scale);
    glUniform1f(m_rotationLoc, m_rotation);
    glUniform2fv(m_translationLoc, 1, &rectangles.m_translation.x);

    // Restart thruster blink timer every 100 ms

    glUniform4fv(m_colorLoc, 1, &rectangles.m_color.r);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
  }
  glUseProgram(0);
}

void Rectangle::terminateGL() {
  for (auto rectangles : m_rectangles) {
    glDeleteBuffers(1, &rectangles.m_vbo);
    glDeleteVertexArrays(1, &rectangles.m_vao);
  }
}

void Rectangle::update(float deltaTime) {
  for (auto &rectangles : m_rectangles) {

  }
}

Rectangle::Rectangles Rectangle::createAsteroid(int indiceX, int indiceY) {
  Rectangles rectangle;

  rectangle.m_translation.x = indiceX==1 ? -0.75f : -1+(indiceX*0.30f)-0.05f;
  rectangle.m_translation.y = indiceY==1 ?  0.97f : 1-(indiceY*0.03f);

  std::array<glm::vec2, 4> positions{
      // Paddle body
      glm::vec2{-15.0f, 0.0f},
      glm::vec2{-15.0f, 0.3f},
      glm::vec2{+15.0f, 0.0f},
      glm::vec2{+15.0f, 0.3f},
  };

  // Normalize
  for (auto &position : positions) {
    position /= glm::vec2{10.0f, 1.0f};
  }

  std::array indices{0, 1, 2, 3, 0, 2, 1, 3};

  std::uniform_real_distribution<float> rd(0.15f, 0.85f);
  glm::vec4 color{rd(m_randomEngine), rd(m_randomEngine), rd(m_randomEngine), rd(m_randomEngine)};
  rectangle.m_color = color;

  // Generate VBO
  glGenBuffers(1, &rectangle.m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, rectangle.m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  glGenBuffers(1, &rectangle.m_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectangle.m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &rectangle.m_vao);

  // Bind vertex attributes to current VAO
  glBindVertexArray(rectangle.m_vao);

  glEnableVertexAttribArray(positionAttribute);
  glBindBuffer(GL_ARRAY_BUFFER, rectangle.m_vbo);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectangle.m_ebo);

  // End of binding to current VAO
  glBindVertexArray(0);

  return rectangle;
}