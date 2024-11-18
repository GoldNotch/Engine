#pragma once
#include <string>
#include <utility>

#include "LoggingAPI.h"


#ifdef LOGGING_IMPLEMENTATION
namespace io::p_detail
{
LogFunc __st_logFunc__ = nullptr;
wLogFunc __st_wlogFunc__ = nullptr;
} // namespace io::p_detail

#define IMPLEMENT_LOGGING_API(ModuleName, ApiCall)                                                 \
  ApiCall void ModuleName##_SetLoggingFunc(LogFunc func)                                           \
  {                                                                                                \
    io::p_detail::__st_logFunc__ = func;                                                           \
  }                                                                                                \
  ApiCall void ModuleName##_SetLoggingWFunc(wLogFunc func)                                         \
  {                                                                                                \
    io::p_detail::__st_wlogFunc__ = func;                                                          \
  }
#else
namespace io
{
namespace p_detail
{
extern LogFunc __st_logFunc__;
extern wLogFunc __st_wlogFunc__;
} // namespace p_detail


} // namespace io
#endif //LOGGING_IMPLEMENTATION


namespace io
{
template<typename... Args>
void Log(LogStatus status, int code, const char * format, Args &&... args)
{
  if (p_detail::__st_logFunc__)
  {
    thread_local std::string buffer(1024, '\0');
    std::sprintf(const_cast<char *>(buffer.c_str()), format, args...);
    p_detail::__st_logFunc__(status, code, buffer.c_str());
  }
}

template<typename... Args>
void Log(LogStatus status, int code, const wchar_t * format, Args &&... args)
{
  if (p_detail::__st_wlogFunc__)
  {
    thread_local std::wstring wbuffer(1024, '\0');
    std::swprintf(const_cast<wchar_t *>(wbuffer.c_str()), format, args...);
    p_detail::__st_wlogFunc__(status, code, wbuffer.c_str());
  }
}
} // namespace io
