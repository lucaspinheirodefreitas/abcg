//
// Created by mohammed on 31/03/2021.
//

#ifndef ABCG_MODEL_H
#define ABCG_MODEL_H

#include "abcg.hpp"

struct Vertex {
  glm::vec3 position{};

  bool operator==(const Vertex& other) const noexcept {
    return position == other.position;
  }
};

class Model {
 public:
  Model() = default;
  virtual ~Model();

  Model(const Model&) = delete;
  Model(Model&&) = default;
  Model& operator=(const Model&) = delete;
  Model& operator=(Model&&) = default;

  void loadFromFile(std::string_view path, bool standardize = true);
  void render(int numTriangles = -1) const;
  void setupVAO(GLuint program);

  [[nodiscard]] int getNumTriangles() const {
    return static_cast<int>(m_indices.size()) / 3;
  }
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;
  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};
 private:
  //std::vector<Vertex> m_vertices;
  //std::vector<GLuint> m_indices;

  void createBuffers();
  void standardize();
};
#endif  // ABCG_MODEL_H
