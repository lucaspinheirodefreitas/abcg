#include "model.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <filesystem>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Custom specialization of std::hash injected in namespace std
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    std::size_t h2{std::hash<glm::vec3>()(vertex.normal)};
    return h1 ^ h2;
  }
};
}  // namespace std

Model::~Model() {
  glDeleteBuffers(1, &m_EBO);
  glDeleteBuffers(1, &m_VBO);
  glDeleteVertexArrays(1, &m_VAO);
}

void Model::loadFromFile(std::string_view path) {
  auto basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath;

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::Exception{abcg::Exception::Runtime(
          fmt::format("Failed to load model {} ({})", path, reader.Error()))};
    }
    throw abcg::Exception{
        abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{reader.GetAttrib()};
  const auto& shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  m_hasNormals = false;

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (const auto& shape : shapes) {
    // Loop over faces(polygon)
    size_t indexOffset{0};
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      tinyobj::index_t index{shape.mesh.indices.at(offset)};

      // Vertex position
      std::size_t startIndex{static_cast<size_t>(3 * index.vertex_index)};
      float vx{attrib.vertices.at(startIndex + 0)};
      float vy{attrib.vertices.at(startIndex + 1)};
      float vz{attrib.vertices.at(startIndex + 2)};

      // Vertex normal
      float nx{};
      float ny{};
      float nz{};
      if (index.normal_index >= 0) {
        m_hasNormals = true;
        startIndex = 3 * index.normal_index;
        nx = attrib.normals.at(startIndex + 0);
        ny = attrib.normals.at(startIndex + 1);
        nz = attrib.normals.at(startIndex + 2);
      }

      Vertex vertex{};
      vertex.position = {vx, vy, vz};
      vertex.normal = {nx, ny, nz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }
  if (!m_hasNormals) {
    computeNormals();
  }
  createBuffers();
}

void Model::computeNormals() {
  // Clear previous vertex normals
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::zero<glm::vec3>();
  }

  // Compute face normals
  for (const auto offset : iter::range<int>(0, m_indices.size(), 3)) {
    // Get face vertices
    Vertex& a{m_vertices.at(m_indices.at(offset + 0))};
    Vertex& b{m_vertices.at(m_indices.at(offset + 1))};
    Vertex& c{m_vertices.at(m_indices.at(offset + 2))};

    // Compute normal
    const auto edge1{b.position - a.position};
    const auto edge2{c.position - b.position};
    glm::vec3 normal{glm::cross(edge1, edge2)};

    // Accumulate on vertices
    a.normal += normal;
    b.normal += normal;
    c.normal += normal;
  }

  // Normalize
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::normalize(vertex.normal);
  }

  m_hasNormals = true;
}

void Model::createBuffers() {
  // Delete previous buffers
  glDeleteBuffers(1, &m_EBO);
  glDeleteBuffers(1, &m_VBO);

  // VBO
  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
               m_vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // EBO
  glGenBuffers(1, &m_EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(),
               m_indices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Model::setupVAO(GLuint program) {
  // Release previous VAO
  glDeleteVertexArrays(1, &m_VAO);

  // Create VAO
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  // Bind EBO and VBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Bind vertex attributes
  GLint positionAttribute = glGetAttribLocation(program, "inPosition");
  if (positionAttribute >= 0) {
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), nullptr);
  }

  GLint normalAttribute = glGetAttribLocation(program, "inNormal");
  if (normalAttribute >= 0) {
    glEnableVertexAttribArray(normalAttribute);
    GLsizei offset{sizeof(glm::vec3)};
    glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offset));
  }

  // End of binding
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Model::render(int numTriangles, bool mapa) const {
  glBindVertexArray(m_VAO);

  GLsizei numIndices = (numTriangles < 0) ? m_indices.size() : numTriangles * 3;

  glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

  glBindVertexArray(0);
}