#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h> // for variable args: va_list

typedef enum {      
    NONE = 0,
    DEBUG,
    INFO,
    FATAL,
    WARN,
    ERROR,
} LogLevel;

typedef enum {
    NGEN,
    NOAHOWP,
    SNOW17,
    UEB,
    CFE,
    SACSMA,
    LASAM,
    SMP,
    SFT,
    TROUTE,
    SCHISM,
    SFINCS,
    TOPMODEL,
    TOPOFLOW,
    MODULE_COUNT
} LoggingModule;

static const char* module_name[MODULE_COUNT] = {
    "NGEN    ",
    "NOAHOWP ",
    "SNOW17  ",
    "UEB     ",
    "CFE     ",
    "SACSMA  ",
    "LASAM   ",
    "SMP     ",
    "SFT     ",
    "TROUTE  ",
    "SCHISM  ",
    "SFINCS  ",
    "TOPMODEL",
    "TOPOFLOW"
};

typedef struct {
    LogLevel logLevel;
    FILE* logFile;
} Logger;

Logger* GetInstance();
void SetLogPreferences(Logger* logger);
void Log(LogLevel messageLevel, const char* message, ...);
LogLevel GetLogLevel(const char* logLevel);
char* createTimestamp();
void setup_logger(void);

#endif
