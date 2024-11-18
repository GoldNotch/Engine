#ifndef SCENE_H
#define SCENE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

  typedef struct
  {
    float x, y;
  } glVec2;

  typedef struct
  {
    float x, y, z;
  } glVec3;

  typedef struct
  {
    float x, y, z, w;
  } glVec4;


  typedef struct StaticMeshData
  {
    size_t vertices_count;
    const glVec2 * vertices;
    const glVec3 * colors;

    size_t indices_count;
    const unsigned int * indices;
  } StaticMesh;

  typedef void * RenderSceneHandler;


#ifdef __cplusplus
}
#endif

#endif
