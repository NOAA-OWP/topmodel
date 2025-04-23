#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h> // for variable args: va_list
#include <stdbool.h>


typedef enum {      
    NONE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
} LogLevel;

// Methods
void CreateTimestamp(char *buffer, size_t size, bool appendMS, bool iso);
bool CreateDirectory(const char *path);
bool DirectoryExists(const char *path);
LogLevel GetLogLevel(const char* logLevel);
void Log(LogLevel messageLevel, const char* message, ...);
bool LogFileReady(bool appendMode);
void SetLogModuleName(void);
void SetLogPreferences(void);
void SetupLogFile(void);
void TrimString(const char *input, char *output, size_t outputSize);
void TrimToOneNewline(char *str, size_t max_len);
void WriteLogTestMsgs(void);
#endif
