#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <imgui.h>

#include <random>

#include "abcg.hpp"
#include "paddle.hpp"
#include "gamedata.hpp"
#include "rectangle.hpp"
#include "ball.hpp"
#include "starlayers.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
protected:
    void handleEvent(SDL_Event& event) override;
    void initializeGL() override;
    void paintGL() override;
    void paintUI() override;
    void resizeGL(int width, int height) override;
    void terminateGL() override;
    void checkCollisions();
    void checkWinCondition();

private:
    GLuint m_starsProgram{};
    GLuint m_objectsProgram{};

    int m_viewportWidth{};
    int m_viewportHeight{};

    GameData m_gameData;

    Rectangle m_rectangle;
    Ball m_ball;
    Paddle m_paddle;
    StarLayers m_starLayers;

    abcg::ElapsedTimer m_restartWaitTimer;

    ImFont* m_font{};

    std::default_random_engine m_randomEngine;

    void restart();
    void update();
};



#endif