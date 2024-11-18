#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

  /// @brief Types of log messages
  typedef enum
  {
    US_LOG_INFO,       ///< use for information, debugging
    US_LOG_WARNING,    ///< use for strange situations which are should be kept in eye
    US_LOG_ERROR,      ///< logic and algorithm errors
    US_LOG_FATAL_ERROR ///< errors which are lead to abort
  } LogStatus;

  /// @brief callback to process char output
  typedef void (*LogFunc)(LogStatus, int, const char *);
  /// @brief callback to process wide char output
  typedef void (*wLogFunc)(LogStatus, int, const wchar_t *);


#define DECLARE_LOGGING_API(ModuleName, ApiCall)                                                   \
  ApiCall void ModuleName##_SetLoggingFunc(LogFunc func);                                          \
  ApiCall void ModuleName##_SetLoggingWFunc(wLogFunc func);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
