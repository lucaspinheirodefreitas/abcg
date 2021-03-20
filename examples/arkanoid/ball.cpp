#include "ball.hpp"

#include <cppitertools/itertools.hpp>
#include <iostream>
#include <random>

using namespace std;

void Ball::initializeGL(GLuint program) {
  terminateGL();
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);
  std::uniform_int_distribution<int> intDistribution(0, 1);
  intDistribution(m_randomEngine) == 1 ? m_esquerda = true : m_direita = true;

  m_program = program;
  m_colorLoc = glGetUniformLocation(m_program, "color");
  m_scaleLoc = glGetUniformLocation(m_program, "scale");
  m_translationLoc = glGetUniformLocation(m_program, "translation");
  m_velocity = glm::vec2(2.0f / 7.0f);

  // Choose a random direction

  m_translation = glm::vec2(0);

  // Create regular polygon
  auto sides{10};

  std::vector<glm::vec2> positions(0);
  positions.emplace_back(0, 0);
  auto step{M_PI * 2 / sides};
  for (auto angle : iter::range(0.0, M_PI * 2, step)) {
    positions.emplace_back(std::cos(angle), std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // Generate VBO of positions
  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
               positions.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  glBindVertexArray(m_vao);

  glEnableVertexAttribArray(positionAttribute);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  glBindVertexArray(0);
}

void Ball::paintGL() {
  glUseProgram(m_program);

  glBindVertexArray(m_vao);
  glUniform4f(m_colorLoc, 1, 1, 1, 0);
  glUniform1f(m_rotationLoc, 0);
  glUniform1f(m_scaleLoc, m_scale);

  glUniform2f(m_translationLoc, m_translation.x, m_translation.y);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 12);

  glBindVertexArray(0);

  glUseProgram(0);
}

void Ball::terminateGL() {
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

bool Ball::verifyPosition(Paddle &paddle) {
  return paddle.m_translation.x - 0.12 <= m_translation.x &&
         paddle.m_translation.x + 0.12 >= m_translation.x;
}

void Ball::update(Paddle &paddle, const GameData &gameData) {
  if (gameData.m_state == State::Playing) {
    if (m_ballTimer.elapsed() > 50.0 / 1000.0) {
      m_ballTimer.restart();

      if (m_translation.x >= 0.99f) {
        m_esquerda = true;
        m_direita = false;
      }
      if (m_translation.x <= -0.99f) {
        m_esquerda = false;
        m_direita = true;
      }
      if (m_translation.y >= 0.99f) {
        m_baixo = true;
        m_cima = false;
      }
      if (verifyPosition(paddle) && m_translation.y <= -0.87f) {
        m_baixo = false;
        m_cima = true;
        cout << "Defense - Y : " << m_translation.y << endl;
      }

      if (m_esquerda) {
        if (m_translation.x < -0.0f) {
          m_translation.x - 0.05f < -0.99f ? m_translation.x = -0.99f
                                           : m_translation.x -= 0.05f;
        } else {
          m_translation.x -= 0.05;
        }

      } else if (m_direita) {
        if (m_translation.x < -0.0f) {
          m_translation.x += 0.05;
        } else {
          m_translation.x + 0.05f < 0.99f ? m_translation.x += 0.05f
                                          : m_translation.x = 0.99f;
        }
      }
      if (m_baixo) {
        if (m_translation.y < -0.0f) {
          m_translation.y - 0.05f > -0.87f
              ? m_translation.y -= 0.05
              : (verifyPosition(paddle) ? m_translation.y = -0.87f
                                        : m_translation.y -= 0.05);
        } else {
          m_translation.y -= 0.05;
        }
      } else if (m_cima) {
        if (m_translation.y < -0.0f) {
          m_translation.y += 0.05f;
        } else {
          m_translation.y + 0.05f < 0.99f ? m_translation.y += 0.05f
                                          : m_translation.y = 0.99f;
        }
      }
    }
  }
}