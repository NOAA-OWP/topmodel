#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#define DS                  "/"              // Directory separator
#define LOG_DIR_NGENCERF    "/ngencerf/data" // ngenCERF log directory string if environement var empty.
#define LOG_DIR_DEFAULT     "run-logs"
#define LOG_FILE_EXT        "log"
#define LOG_MODULE_NAME_LEN  8               // Width of module name for log entries
#define MODULE_NAME          "TopModel"

bool openedAppendMode = true;
bool loggerInitialized = false;
FILE *logFile = NULL;
char logFilePath[1024] = "";
LogLevel logLevel = INFO;
char moduleName[LOG_MODULE_NAME_LEN+1] = "";
bool envLogLevelLogged = false;

void TrimString(const char *input, char *output, size_t outputSize) {
    if (!input || !output || outputSize == 0) {
        if (output && outputSize > 0) output[0] = '\0';
        return;
    }

    // Skip leading whitespace
    while (isspace((unsigned char)*input)) {
        input++;
    }

    size_t len = strlen(input);
    if (len == 0) {
        output[0] = '\0';
        return;
    }

    // Find the end of the string and move backward past trailing whitespace
    const char *end = input + len - 1;
    while (end > input && isspace((unsigned char)*end)) {
        end--;
    }

    size_t trimmedLen = end - input + 1;
    if (trimmedLen >= outputSize) {
        trimmedLen = outputSize - 1;
    }

    strncpy(output, input, trimmedLen);
    output[trimmedLen] = '\0';
}

void SetLogModuleName(void) {

    // Copy MODULE_NAME to a string variable 
    char src[20];
    strncpy(src, MODULE_NAME, sizeof(src));
    src[sizeof(src) - 1] = '\0'; // ensure null termination

    // Convert to uppercase and copy, up to width or null terminator
    size_t i = 0;
    for (; i < LOG_MODULE_NAME_LEN && src[i] != '\0'; i++) {
        moduleName[i] = toupper((unsigned char)src[i]);
    }

    // Pad with spaces if needed
    for (; i < LOG_MODULE_NAME_LEN; i++) {
        moduleName[i] = ' ';
    }
    moduleName[LOG_MODULE_NAME_LEN] = '\0'; // null-terminate
}

void CreateTimestamp(char *buffer, size_t size, bool appendMS, bool iso) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm utc_tm;
    gmtime_r(&tv.tv_sec, &utc_tm);

    char base[32];
    if (iso) {
        strftime(base, sizeof(base), "%Y-%m-%dT%H:%M:%S", &utc_tm);
    } else {
        strftime(base, sizeof(base), "%Y%m%dT%H%M%S", &utc_tm);
    }

    if (appendMS) {
        snprintf(buffer, size, "%s.%03ld", base, tv.tv_usec / 1000);
    } else {
        snprintf(buffer, size, "%s", base);
    }
}

bool DirectoryExists(const char *path) {
    struct stat info;
    return (stat(path, &info) == 0 && (info.st_mode & S_IFDIR));
}

bool CreateDirectory(const char *path) {
    if (!DirectoryExists(path)) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", path);
        int status = system(cmd);
        if (status == -1 || (WIFEXITED(status) && WEXITSTATUS(status) != 0)) {
            fprintf(stderr, "Failed to create directory: %s\n", path);
            return false;
        }
    }
    return true;
}

bool LogFileReady(bool appendMode) {
    if (logFile && !ferror(logFile)) {
        fseek(logFile, 0, SEEK_END);
        return true;
    } else if (strlen(logFilePath) > 0) {
        logFile = fopen(logFilePath, appendMode ? "a" : "w");
        if (logFile != NULL) {
            openedAppendMode = appendMode;
            if (loggerInitialized) Log(INFO, "Opened log file %s in %s mode\n", logFilePath, (appendMode ? "Append" : "Truncate"));
            return true;
        }
        return false;
    }
    return false;
}

/**
 * Set the log file path name using the following pattern
 *  - Use the module log file if available (unset when first run by ngen), otherwise 
 *  - Use ngen log file if available, otherwise
 *  - Use /ngencerf/data/run-logs/<username>/<module>_<YYMMDDTHHMMSS> if available, othrewise
 *  - Use ~/run-logs/<YYYYMMDD>/<module>_<YYMMDDTHHMMSS>
 *  - Onced opened, save the full log path to the modules log environment variable so
 *    it is only opened once for each ngen run (vs for each catchment)
 */
void SetupLogFile(void) {
    logFilePath[0] = '\0';
    bool appendEntries = true;
    bool moduleLogEnvExists = false;

    const char *envVar = getenv("TOPMODEL_LOGFILEPATH");
    if (envVar && envVar[0] != '\0') {
        strncpy(logFilePath, envVar, sizeof(logFilePath) - 1);
        moduleLogEnvExists = true;
    } else {
        envVar = getenv("NGEN_LOG_FILE_PATH");
        if (envVar && envVar[0] != '\0') {
            strncpy(logFilePath, envVar, sizeof(logFilePath) - 1);
        } else {
            appendEntries = false;
            char logFileDir[512];

            if (DirectoryExists(LOG_DIR_NGENCERF)) {
                snprintf(logFileDir, sizeof(logFileDir), "%s%s%s", LOG_DIR_NGENCERF, DS, LOG_DIR_DEFAULT);
            } else {
                const char *home = getenv("HOME"); // Get users home directly pathname
                if (home) {
                    snprintf(logFileDir, sizeof(logFileDir), "%s%s%s", home, DS, LOG_DIR_DEFAULT);
                }
                else {
                    snprintf(logFileDir, sizeof(logFileDir), "~%s%s", DS, LOG_DIR_DEFAULT);
                }
            }

            if (CreateDirectory(logFileDir)) {
                const char *envUsername = getenv("USER");
                if (envUsername) {
                    strncat(logFileDir, DS, sizeof(logFileDir) - strlen(logFileDir) - 1);
                    strncat(logFileDir, envUsername, sizeof(logFileDir) - strlen(logFileDir) - 1);
                } else {
                    char dateStr[32];
                    CreateTimestamp(dateStr, sizeof(dateStr), 0, 0);
                    strncat(logFileDir, DS, sizeof(logFileDir) - strlen(logFileDir) - 1);
                    strncat(logFileDir, dateStr, sizeof(logFileDir) - strlen(logFileDir) - 1);
                }

                if (CreateDirectory(logFileDir)) {
                    char timestamp[32];
                    CreateTimestamp(timestamp, sizeof(timestamp), 0, 0);
                    snprintf(logFilePath, sizeof(logFilePath), "%s%s%s_%s.%s",
                             logFileDir, DS, MODULE_NAME, timestamp, LOG_FILE_EXT);
                }
            }
        }
    }

    if (LogFileReady(appendEntries)) {
        if (!moduleLogEnvExists) setenv("TOPMODEL_LOGFILEPATH", logFilePath, 1);
    } else {
        printf("Unable to open log file ");
        if (strlen(logFilePath) > 0) {
            printf("%s (Perhaps check permissions)\n", logFilePath);
        }
        printf("Log entries will be written to stdout\n");
    }
}

const char* ConvertLogLevelToString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        case FATAL: return "FATAL";
        default:    return "NONE";
    }
}

LogLevel ConvertStringToLogLevel(const char* str) {
    if (strcasecmp(str, "DEBUG") == 0) return DEBUG;
    if (strcasecmp(str, "INFO")  == 0) return INFO;
    if (strcasecmp(str, "WARN")  == 0) return WARN;
    if (strcasecmp(str, "ERROR") == 0) return ERROR;
    if (strcasecmp(str, "FATAL") == 0) return FATAL;
    return NONE;
}

bool CheckLogLevelEv(void) {
    const char* envLogLevel = getenv("TOPMODEL_LOGLEVEL");
    if (envLogLevel && envLogLevel[0] != '\0') {
        LogLevel envll = ConvertStringToLogLevel(envLogLevel);
        if (envll != logLevel) {
            logLevel = envll;

            char llStr[10];
            char llMsg[100];
            TrimString(ConvertLogLevelToString(logLevel), llStr, sizeof(llStr));
            snprintf(llMsg, sizeof(llMsg), "INFO ONLY: Log level changed to %s found in TOPMODEL_LOGLEVEL\n", llStr);
            if (loggerInitialized) Log(logLevel, llMsg);
        }
        return 1;
    }
    return 0;
}

void SetLogPreferences(void) {

    // Set the module name used in log entries
    SetLogModuleName();

    // Set the log file path name and open it
    SetupLogFile();
    loggerInitialized = true;
    printf("Module %s Log File: %s\n", MODULE_NAME, logFilePath);
    // This is an INFO message that always should be in the log but the
    // logLevel could be different than INFO. Therefore use logLevel to
    // ensure the message is recorded in the log
    char logMsg[1100];
    snprintf(logMsg, sizeof(logMsg), "Logfile: %s\n", logFilePath);
    Log(logLevel, logMsg);
    if (logFile != NULL) Log(INFO, "Opened log file %s in %s mode\n", logFilePath, (openedAppendMode ? "Append" : "Truncate"));

    // Check for the log level environment variable for a log level
    char llStr[10];
    char llMsg[80];
    TrimString(ConvertLogLevelToString(logLevel), llStr, sizeof(llStr));
    snprintf(llMsg,sizeof(llMsg), "INFO ONLY: Default log level is %s\n", llStr);
    Log(logLevel, llMsg);
    CheckLogLevelEv(); // Check for log level set in the environment variable
}

void TrimToOneNewline(char *str, size_t max_len) {
    size_t len = strlen(str);

    // Strip trailing whitespace including \n, \r, spaces, tabs
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }

    // Ensure room to add a newline
    if (len + 1 < max_len) {
        str[len] = '\n';
        str[len + 1] = '\0';
    }
}

void Log(LogLevel messageLevel, const char* message, ...) {
    if (!loggerInitialized) SetLogPreferences(); // Initialize logger on first call

    // Check for change in log level environment variable
    (void)CheckLogLevelEv();

    if (messageLevel < logLevel) return;

    va_list arglist;
    va_start(arglist, message);
    va_list arglist_copy;
    va_copy(arglist_copy, arglist);

    int length = vsnprintf(NULL, 0, message, arglist_copy);
    va_end(arglist_copy);

    char *buffer = malloc(length + 1);
    if (!buffer) return; // Always good to check

    vsnprintf(buffer, length + 1, message, arglist);
    va_end(arglist);

    const char* logType;
    switch (messageLevel) {
        case DEBUG: logType = "DEBUG"; break;
        case INFO:  logType = "INFO "; break;
        case WARN:  logType = "WARN "; break;
        case ERROR: logType = "ERROR"; break;
        case FATAL: logType = "FATAL"; break;
        default:    logType = "NONE "; break;
    }

    char timestamp[32];
    CreateTimestamp(timestamp, sizeof(timestamp), 1, 1);

    char logPrefix[128];
    snprintf(logPrefix, sizeof(logPrefix), "%s %s %s", timestamp, moduleName, logType);

    TrimToOneNewline(buffer, sizeof(buffer));
    char *line = strtok(buffer, "\n");
    if (LogFileReady(true)) {
        while (line != NULL) {
            fprintf(logFile, "%s %s\n", logPrefix, line);
            line = strtok(NULL, "\n");
        }
        fflush(logFile);
    } else {
        while (line != NULL) {
            printf("%s %s\n", logPrefix, line);
            line = strtok(NULL, "\n");
        }
        fflush(stdout);
    }

    free(buffer);
}

LogLevel GetLogLevel(const char* logLevel) {
    if (strcmp(logLevel, "DEBUG") == 0) return DEBUG;
    if (strcmp(logLevel, "INFO") == 0)  return INFO;
    if (strcmp(logLevel, "WARN") == 0)  return ERROR;
    if (strcmp(logLevel, "ERROR") == 0) return ERROR;
    if (strcmp(logLevel, "FATAL") == 0) return ERROR;
    return NONE;
}

void WriteLogTestMsgs() {

    if (!loggerInitialized) SetLogPreferences();
    LogLevel savedLogLevel = logLevel;
    logLevel = ERROR; Log(ERROR, "Sample Log for LogLevel::ERROR");
    logLevel = FATAL; Log(FATAL, "Sample Log for LogLevel::FATAL");
    logLevel = WARN; Log(WARN, "Sample Log for LogLevel::WARN");
    logLevel = INFO; Log(INFO, "Sample Log for LogLevel::INFO");
    logLevel = DEBUG; Log(DEBUG, "Sample Log for LogLevel::DEBUG");
    
    const char* multiline_log = 
        "First line of multiline log:\n"
        "Second line of multiline log\n"
        "Third line of multiline log\n"
        "Fourth line of multiline log";
    Log(DEBUG, multiline_log);
    
    logLevel = savedLogLevel;
}
