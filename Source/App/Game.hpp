#pragma once
#include <vector>

#include "App.h"



struct TriangleObject final
{
  ~TriangleObject() = default;
  void Tick() {}

  const glVec2 * GetVerticesData() const { return vertices; }
  const glVec3 * GetColorsData() const { return colors; }
  const size_t VerticesCount() const { return 6; }
  const uint32_t * GetIndicesData() const { return indices; }
  const size_t IndicesCount() const { return 6; }

private:
  static constexpr glVec2 vertices[] = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {-0.5f, 0.5f}, {0.5f, 0.5f}};
  static constexpr glVec3 colors[] = {{0, 1, 1}, {1, 0, 0.6647}, {1, 1, 0}, {0, 0, 0}};
  static constexpr uint32_t indices[] = {0, 1, 2, 2, 1, 3};
};

struct Game final
{
public:
  Game() { triangles.emplace_back(); }
  ~Game() = default;

  void Tick()
  {
    for (auto && triangle : triangles)
      triangle.Tick();
  }

  void InitRenderableScene(const RenderSceneHandler scene)
  {
    StaticMesh mesh{};
    mesh.vertices = obj.GetVerticesData();
    mesh.vertices_count = obj.VerticesCount();
    mesh.colors = obj.GetColorsData();
    mesh.indices = obj.GetIndicesData();
    mesh.indices_count = obj.IndicesCount();
    RenderScene_PushStaticMesh(scene, mesh);
  }

private:
  TriangleObject obj;
  std::vector<TriangleObject> triangles;

private:
  Game(const Game &) = delete;
  Game & operator=(const Game &) = delete;
};
