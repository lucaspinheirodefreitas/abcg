//
// Created by mohammed on 12/03/2021.
//

#include "paddle.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
using namespace std;

void Paddle::initializeGL(GLuint program) {
    terminateGL();

    m_program = program;
    m_colorLoc = glGetUniformLocation(m_program, "color");
    m_rotationLoc = glGetUniformLocation(m_program, "rotation");
    m_scaleLoc = glGetUniformLocation(m_program, "scale");
    m_translationLoc = glGetUniformLocation(m_program, "translation");

    m_rotation = 0.0f;
    m_translation = glm::vec2(0, -0.9);

    std::array<glm::vec2, 4> positions{
            // Paddle body
            glm::vec2{-12.0f, 0.0f},
            glm::vec2{-12.0f, 0.2f},
            glm::vec2{+12.0f, 0.0f},
            glm::vec2{+12.0f, 0.2f},
    };

    // Normalize
    for (auto &position :positions) {
        position /= glm::vec2{10.0f, 1.0f};
    }

    std::array indices{0, 1,
                       2, 3,
                       0, 2,
                       1, 3};
    // Generate VBO
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate EBO
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

    // End of binding to current VAO
    glBindVertexArray(0);
}

void Paddle::paintGL(const GameData &gameData) {
    if (gameData.m_state != State::Playing) return;

    glUseProgram(m_program);

    glBindVertexArray(m_vao);

    glUniform1f(m_scaleLoc, m_scale);
    glUniform1f(m_rotationLoc, m_rotation);
    glUniform2fv(m_translationLoc, 1, &m_translation.x);

    // Restart thruster blink timer every 100 ms
    if (m_trailBlinkTimer.elapsed() > 100.0 / 1000.0) m_trailBlinkTimer.restart();

    if (gameData.m_input[static_cast<size_t>(Input::Up)]) {
        // Show thruster trail during 50 ms
        if (m_trailBlinkTimer.elapsed() < 50.0 / 1000.0) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // 50% transparent
            glUniform4f(m_colorLoc, 1, 1, 1, 0.5f);

            glDrawElements(GL_TRIANGLES, 14 * 3, GL_UNSIGNED_INT, nullptr);

            glDisable(GL_BLEND);
        }
    }

    glUniform4fv(m_colorLoc, 1, &m_color.r);
    glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

    glUseProgram(0);
}

void Paddle::terminateGL() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}

void Paddle::update(const GameData &gameData) {

    // Rotate
    //m_translation.y = -10.5;
    if (m_bulletCoolDownTimer.elapsed() > 50.0 / 1000.0) {
        m_bulletCoolDownTimer.restart();
        if (gameData.m_input[static_cast<size_t>(Input::Left)]) {
            // Wrap-around
            if (m_translation.x - 0.12f >= -1.0f) m_translation.x -= 0.12f;
            if (m_translation.x - 0.12f < -1.0f) m_translation.x = -0.88f;
            //cout << "Esquerda: " << m_translation.x << endl;
            //cout << "Y: " << m_translation.y << endl;
        }

        if (gameData.m_input[static_cast<size_t>(Input::Right)]) {
            if (m_translation.x + 0.12f <= +1.0f) m_translation.x += 0.12f;
            if (m_translation.x + 0.12f > 1.0f) m_translation.x = 0.88f;
            //cout << "Direita: " << m_translation.x << endl;
        }
    }
}