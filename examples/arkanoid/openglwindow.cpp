#include "openglwindow.hpp"

#include <imgui.h>

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
  m_starsProgram = createProgramFromFile(getAssetsPath() + "stars.vert",
                                         getAssetsPath() + "stars.frag");
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",
                                           getAssetsPath() + "objects.frag");

  glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2],
               m_clearColor[3]);

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
  m_rectangle.initializeGL(m_objectsProgram, 42);
  m_ball.initializeGL(m_objectsProgram);
}

void OpenGLWindow::update() {
  if (m_gameData.m_state != State::Playing &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }

  m_paddle.update(m_gameData);
  m_ball.update(m_paddle, m_gameData);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
    checkLossCondition();
  }
}

void OpenGLWindow::paintGL() {
  update();
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_paddle.paintGL(m_gameData);
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
}

void OpenGLWindow::checkCollisions() {
  for (auto &rectangle : m_rectangle.m_rectangles) {
    if (rectangle.m_translation.x - 0.15 <= m_ball.m_translation.x &&
        rectangle.m_translation.x + 0.15 >= m_ball.m_translation.x &&
        rectangle.m_translation.y + 0.03f >= m_ball.m_translation.y &&
        rectangle.m_translation.y - 0.03f <= m_ball.m_translation.y) {
      rectangle.m_dead = true;
    }
  }
  m_rectangle.m_rectangles.remove_if(
      [](const Rectangle::Rectangles &a) { return a.m_dead; });
}

void OpenGLWindow::checkWinCondition() {
  if (m_rectangle.m_rectangles.empty()) {
    m_gameData.m_state = State::Win;
    m_restartWaitTimer.restart();
  }
}

void OpenGLWindow::checkLossCondition() {
  if (m_ball.m_translation.y <= m_paddle.m_translation.y) {
    m_gameData.m_state = State::GameOver;
    m_restartWaitTimer.restart();
  }
}
