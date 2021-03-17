//
// Created by mohammed on 12/03/2021.
//

#ifndef BALL_HPP
#define BALL_HPP


#include "abcg.hpp"
#include "gamedata.hpp"
#include "paddle.hpp"
class OpenGLWindow;

class Ball {
   public:
    void initializeGL(GLuint program);
    void paintGL();
    void terminateGL();
    bool verifyPosition(Paddle &paddle);
    void update(Paddle &paddle, const GameData &gameData);

private:
    friend OpenGLWindow;

    GLuint m_program{};
    GLint m_colorLoc{};
    GLint m_rotationLoc{};
    GLint m_translationLoc{};
    GLint m_scaleLoc{};
    glm::vec2 m_translation{glm::vec2(0)};
    glm::vec2 m_velocity{glm::vec2(0)};

    GLuint m_vao{};
    GLuint m_vbo{};
    bool m_start = true;
    bool m_esquerda=true, m_direita=false, m_cima=false, m_baixo=true;
    float m_scale{0.01f};
    abcg::ElapsedTimer m_bulletCoolDownTimer;
};


#endif //ABCG_BALL_HPP
