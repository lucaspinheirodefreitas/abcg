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

        float m_angularVelocity{};
        glm::vec4 m_color{1};
        bool m_hit{false};
        int m_polygonSides{};
        float m_rotation{};
        float m_scale{};
        glm::vec2 m_translation{glm::vec2(0)};
        glm::vec2 m_velocity{glm::vec2(0)};
    };
    float m_scale = 0.1f;
    std::list<Rectangles> m_rectangles;

    std::default_random_engine m_randomEngine;
    std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};

    std::uniform_real_distribution<float> m_randomDistX{-0.85f, 0.85f};
    std::uniform_real_distribution<float> m_randomDistY{0.0f, 0.85f};


    Rectangle::Rectangles createAsteroid(glm::vec2 translation = glm::vec2(0),float scale = 0.1f);
};


#endif //ABCG_RECTANGLE_HPP
