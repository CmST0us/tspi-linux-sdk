// Copyright 2013 Google Inc. All Rights Reserved.
//
// Log - Platform independent interface for a Logging class
//
#ifndef update_engine_CORE_LOG_H_
#define update_engine_CORE_LOG_H_

// Simple logging class. The implementation is platform dependent.

typedef enum {
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG,
  LOG_VERBOSE
} LogPriority;

// Enable/disable verbose logging (LOGV).
// This function is supplied for cases where the system layer does not
// initialize logging.  This is also needed to initialize logging in
// unit tests.
void InitLogging(int argc, const char* const* argv);

void Log(const char* file, int line, LogPriority level, const char* fmt, ...);

// Log APIs
#define LOGE(...) Log(__FILE__, __LINE__, LOG_ERROR, __VA_ARGS__)
#define LOGW(...) Log(__FILE__, __LINE__, LOG_WARN, __VA_ARGS__)
#define LOGI(...) Log(__FILE__, __LINE__, LOG_INFO, __VA_ARGS__)
#define LOGD(...) Log(__FILE__, __LINE__, LOG_DEBUG, __VA_ARGS__)
#define LOGV(...) Log(__FILE__, __LINE__, LOG_VERBOSE, __VA_ARGS__)

#endif  // update_engine_CORE_LOG_H_
