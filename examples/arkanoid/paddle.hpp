//
// Created by mohammed on 12/03/2021.
//

#ifndef PADDLE_HPP
#define PADDLE_HPP


#include "abcg.hpp"
#include "gamedata.hpp"

class Rectangle;
class Ball;
class OpenGLWindow;

class Paddle {
public:
    void initializeGL(GLuint program);
    void paintGL(const GameData &gameData);
    void terminateGL();
    void update(const GameData &gameData);

private:
    friend Rectangle;
    friend Ball;
    friend OpenGLWindow;

    GLuint m_program{};
    GLint m_translationLoc{};
    GLint m_colorLoc{};
    GLint m_scaleLoc{};
    GLint m_rotationLoc{};

    GLuint m_vao{};
    GLuint m_vbo{};
    GLuint m_ebo{};

    glm::vec4 m_color{1};
    float m_rotation{};
    float m_scale{0.1f};
    glm::vec2 m_translation{glm::vec2(0)};

    abcg::ElapsedTimer m_trailBlinkTimer;
    abcg::ElapsedTimer m_paddleTimer;
};

#endif //ABCG_PADDLE_HPP
