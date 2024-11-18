#ifndef APP_H
#define APP_H

#ifdef _WIN32
#if defined(APP_STATIC_BUILD)
#define USAPP_API
#elif defined(APP_BUILD)
#define USAPP_API __declspec(dllexport)
#else
#define USAPP_API __declspec(dllimport)
#endif
#else
#define USAPP_API
#endif

#include "LoggingAPI.h"
DECLARE_LOGGING_API(App, USAPP_API)

#ifdef __cplusplus
extern "C"
{
#endif

#include <Graphics.h>


  /// @brief initialize application module
  USAPP_API void usApp_Init();
  /// @brief application tick process
  USAPP_API void usApp_Tick();
  /// @brief terminate application module
  USAPP_API void usApp_Terminate();
  /// @brief collects and fills frame data
  /// @param handler - identifier of frame data
  USAPP_API void usApp_InitRenderableScene(const RenderSceneHandler scene);

#ifdef __cplusplus
}
#endif

#endif // APP_H
