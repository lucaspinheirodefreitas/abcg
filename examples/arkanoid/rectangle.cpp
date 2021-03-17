#include "rectangle.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

#include <iostream>

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

    for (auto &asteroid : m_rectangles) {
        asteroid = createAsteroid();
        // Make sure the asteroid won't collide with the ship
        int vezes;
        bool distancia;

        do {
            distancia=false;
            vezes=0;
            asteroid.m_translation = {m_randomDistX(m_randomEngine),
                                      m_randomDistY(m_randomEngine)};
            for(auto &poligono : m_rectangles) {
                if((poligono.m_translation.x + 0.25 > asteroid.m_translation.x  && poligono.m_translation.x - 0.25 < asteroid.m_translation.x
                && poligono.m_translation.y + 0.25 > asteroid.m_translation.y  && poligono.m_translation.y - 0.25 < asteroid.m_translation.y)
                || (poligono.m_translation.x + 0.25 < asteroid.m_translation.x  && poligono.m_translation.x - 0.25 > asteroid.m_translation.x
                   && poligono.m_translation.y + 0.25 < asteroid.m_translation.y  && poligono.m_translation.y - 0.25 > asteroid.m_translation.y)) {
                    distancia = true;
                } else {
                    vezes++;
                }
            }
        } while (distancia && vezes<=1);

    }
}

void Rectangle::paintGL() {
    glUseProgram(m_program);

    for (auto &rectangles : m_rectangles) {
        glBindVertexArray(rectangles.m_vao);

        glUniform4fv(m_colorLoc, 1, &rectangles.m_color.r);
        glUniform1f(m_scaleLoc, rectangles.m_scale);
        glUniform1f(m_rotationLoc, rectangles.m_rotation);

        for (auto i : {(1/5), 1}) {
            for (auto j : {(-1), 0, (1)}) {
                glUniform2f(m_translationLoc, rectangles.m_translation.x + j,
                            rectangles.m_translation.y + i);
                glDrawArrays(GL_TRIANGLE_FAN, 0, rectangles.m_polygonSides + 2);
            }
        }
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
        rectangles.m_translation -= 0.5f * deltaTime;
        rectangles.m_rotation = glm::wrapAngle(
                rectangles.m_rotation + rectangles.m_angularVelocity * deltaTime);
        rectangles.m_translation += rectangles.m_velocity * deltaTime;

        // Wrap-around
        if (rectangles.m_translation.x < -1.0f) rectangles.m_translation.x += 2.0f;
        if (rectangles.m_translation.x > +1.0f) rectangles.m_translation.x -= 2.0f;
        if (rectangles.m_translation.y < -1.0f) rectangles.m_translation.y += 2.0f;
        if (rectangles.m_translation.y > +1.0f) rectangles.m_translation.y -= 2.0f;
    }
}

Rectangle::Rectangles Rectangle::createAsteroid(glm::vec2 translation,
                                                float scale) {
    Rectangles rectangle;

    auto &re{m_randomEngine};  // Shortcut

    // Randomly choose the number of sides
    std::uniform_int_distribution<int> randomSides(4, 4);
    rectangle.m_polygonSides = randomSides(re);


    // Choose a random color (actually, a grayscale)
    std::uniform_real_distribution<float> randomIntensity(0.5f, 1.0f);
    rectangle.m_color = glm::vec4(1) * randomIntensity(re);

    rectangle.m_color.a = 1.0f;
    rectangle.m_rotation = 0.0f;
    rectangle.m_scale = scale;
    rectangle.m_translation = translation;
    // Choose a random angular velocity
    rectangle.m_angularVelocity = m_randomDist(re);

    // Choose a random direction
    glm::vec2 direction{m_randomDist(re), m_randomDist(re)};
    rectangle.m_velocity = glm::normalize(direction) / 7.0f;

    // Create geometry
    std::vector<glm::vec2> positions(0);
    positions.emplace_back(0, 0);
    auto step{M_PI * 2 / rectangle.m_polygonSides};
    std::uniform_real_distribution<float> randomRadius(0.8f, 1.0f);
    for (auto angle : iter::range(0.0, M_PI * 2, step)) {
        auto radius{randomRadius(re)};
        positions.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
    }
    positions.push_back(positions.at(1));

    // Generate VBO
    glGenBuffers(1, &rectangle.m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rectangle.m_vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                 positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Get location of attributes in the program
    GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};

    // Create VAO
    glGenVertexArrays(1, &rectangle.m_vao);

    // Bind vertex attributes to current VAO
    glBindVertexArray(rectangle.m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, rectangle.m_vbo);
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // End of binding to current VAO
    glBindVertexArray(0);

    return rectangle;
}