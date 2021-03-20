#include "openglwindow.hpp"

#include <imgui.h>

#include <iostream>

#include "abcg.hpp"
#include "gamedata.hpp"

using namespace std;
void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_SPACE)
      m_gameData.m_input.set(static_cast<size_t>(Input::Start));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.set(static_cast<size_t>(Input::Right));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_SPACE)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Start));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
  }
}

void OpenGLWindow::initializeGL() {
  // Load a new font
  ImGuiIO &io{ImGui::GetIO()};
  auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 60.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }
  m_starsProgram = createProgramFromFile(getAssetsPath() + "stars.vert",getAssetsPath() + "stars.frag");
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",getAssetsPath() + "objects.frag");

  //glClearColor(0, 0, 0, 1);

  glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2],m_clearColor[3]);

#if !defined(__EMSCRIPTEN__)
  glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  restart();
}

void OpenGLWindow::restart() {
  m_gameData.m_state = State::Playing;

  m_paddle.initializeGL(m_objectsProgram);
  // m_starLayers.initializeGL(m_starsProgram, 25);
  m_rectangle.initializeGL(m_objectsProgram, 30);
  m_ball.initializeGL(m_objectsProgram);
}

void OpenGLWindow::update() {
  /*
      float deltaTime{static_cast<float>(getDeltaTime())};
  */

  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }
  m_paddle.update(m_gameData);
  // m_starLayers.update(m_paddle, deltaTime);
  // m_rectangle.update(deltaTime);
  m_ball.update(m_paddle, m_gameData);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }
}

void OpenGLWindow::paintGL() {
  update();
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_paddle.paintGL(m_gameData);
  // m_starLayers.paintGL();
  m_rectangle.paintGL();
  m_ball.paintGL();
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();
  {
    auto size{ImVec2(300, 85)};
    auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                         (m_viewportHeight - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    // ColorEdit to change the clear color
    //ImGui::ColorEdit3("clear color", m_clearColor.data());
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::GameOver) {
      ImGui::Text("Game Over!");
    } else if (m_gameData.m_state == State::Win) {
      ImGui::Text("*You Win!*");
    }

    ImGui::PopFont();
    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  glDeleteProgram(m_starsProgram);
  glDeleteProgram(m_objectsProgram);

  m_rectangle.terminateGL();
  m_ball.terminateGL();
  m_paddle.terminateGL();
  // m_starLayers.terminateGL();
}

/*void OpenGLWindow::checkDefense() {
    if (m_ball.m_translation.y<-0.88f + 0.01f + 0.04f) {
        if(m_ball.m_translation.x >= m_paddle.m_translation.x - 0.12f &&
m_ball.m_translation.x <= m_paddle.m_translation.x + 0.12f) { m_paddle.m_defense
= true; cout << "defense: " << m_ball.m_translation.y << endl;
        }
    }
}*/

void OpenGLWindow::checkCollisions() {
  // Check collision between ship and arkanoid

  /*    auto paddleTranslation{m_paddle.m_translation.y};
      auto distance{glm::distance(m_ball.m_translation.y, paddleTranslation)};*/
  /*
      cout << "Distancia: " << distance << endl;
  */
  /*cout << "Paddle Y: " << m_paddle.m_translation.y << endl;
  cout << "Ball Y: " << m_translation.y << endl;*/

  if (m_ball.m_translation.y <= m_paddle.m_translation.y) {
    m_gameData.m_state = State::GameOver;
    m_restartWaitTimer.restart();
  }

  /*
  // Check collision between bullets and arkanoid
  for (auto &bullet : m_bullets.m_bullets) {
      if (bullet.m_dead) continue;

      for (auto &asteroid : m_asteroids.m_asteroids) {
          for (auto i : {-2, 0, 2}) {
              for (auto j : {-2, 0, 2}) {
                  auto asteroidTranslation{asteroid.m_translation + glm::vec2(i,
  j)}; auto distance{ glm::distance(bullet.m_translation, asteroidTranslation)};

                  if (distance < m_bullets.m_scale + asteroid.m_scale * 0.85f) {
                      asteroid.m_hit = true;
                      bullet.m_dead = true;
                  }
              }
          }
      }

      // Break arkanoid marked as hit
      for (auto &asteroid : m_asteroids.m_asteroids) {
          if (asteroid.m_hit && asteroid.m_scale > 0.10f) {
              std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};
              std::generate_n(std::back_inserter(m_asteroids.m_asteroids), 3,
  [&]() { glm::vec2 offset{m_randomDist(m_randomEngine),
                                   m_randomDist(m_randomEngine)};
                  return m_asteroids.createAsteroid(
                          asteroid.m_translation + offset * asteroid.m_scale *
  0.5f, asteroid.m_scale * 0.5f);
              });
          }
      }

      m_asteroids.m_asteroids.remove_if(
              [](const Asteroids::Asteroid &a) { return a.m_hit; });
  }*/
}

void OpenGLWindow::checkWinCondition() {
  /*if (m_rectangle.m_rectangle.empty()) {
      m_gameData.m_state = State::Win;
      m_restartWaitTimer.restart();
  }*/
}
