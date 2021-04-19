#ifndef MODEL_HPP_
#define MODEL_HPP_

#include "abcg.hpp"

struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};

  bool operator==(const Vertex& other) const noexcept {
    static const auto epsilon{std::numeric_limits<float>::epsilon()};
    return glm::all(glm::epsilonEqual(position, other.position, epsilon)) &&
           glm::all(glm::epsilonEqual(normal, other.normal, epsilon));
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
  [[nodiscard]] int getNumTriangles() const {
    return static_cast<int>(m_indices.size());
  }
  void render(int numTriangles = -1, bool mapa = false) const;
  void loadFromFile(std::string_view path);
  void setupVAO(GLuint program);
  void loadDiffuseTexture(std::string_view path);
  void loadNormalTexture(std::string_view path);

  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};

  bool m_hasNormals{false};
  void createBuffers();
  void computeNormals();

  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;
};

#endif