#include "log.h"
#include <malloc.h>
#include <string.h>
#include <unistd.h>
char *logPath;
char buf[1024];
int inited = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static const char *levelColors[] = {"\033[94m", "\033[36m", "\033[32m", "\033[33m", "\033[31m", "\033[35m", "\033[35m"};

static const char *levelStrings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

int mkdirs(const char *path, const mode_t mode, const int fail_on_exist)
{
    int result = 0;
    char *dir;

    do
    {
        if (NULL == path)
        {
            errno = EINVAL;
            result = -1;
            break;
        }
        char *s_path=(char*)malloc(strlen(path)+1);
        strcpy(s_path, path);
        free(s_path);
        if ((dir = strrchr(s_path, '/')))
        {
            *dir = '\0';
            result = mkdirs(path, mode, fail_on_exist);
            *dir = '/';

            if (result)
                break;
        }

        if (strlen(path))
        {
            if ((result = mkdir(path, mode)))
            {
                if ((EEXIST == result) && (0 == fail_on_exist))
                    result = 0;
                else
                    break;
            }
        }
    } while (0);

    return result;
}

void logInit(const char *setLogPath)
{

    logPath = (char *)malloc(strlen(setLogPath) + 1);
    strcpy(logPath, setLogPath);
    if (access(setLogPath, F_OK))
    {
        if (mkdirs(logPath, S_IRWXU, 0) == -1)
        {
            printf("%s\n", strerror(errno));
        }
    }

    extrapidLog(LOG_INFO, "libLog", "Init libLog");
    inited = 1;
}

void extrapidLog(const int logLevel, const char *moduleName, const char *fmt, ...)
{
    pthread_mutex_lock(&lock);

    time_t timep;
    time(&timep);
    struct tm *nowTime;
    nowTime = localtime(&timep);

    char timeString[22] = {0};
    snprintf(timeString, 22, "[%d-%s%d-%s%d_%s%d:%s%d:%s%d]", 1900 + nowTime->tm_year, nowTime->tm_mon < 9 ? "0" : "", 1 + nowTime->tm_mon, nowTime->tm_mday < 10 ? "0" : "", nowTime->tm_mday, nowTime->tm_hour < 10 ? "0" : "", nowTime->tm_hour, nowTime->tm_min < 10 ? "0" : "", nowTime->tm_min, nowTime->tm_sec < 10 ? "0" : "", nowTime->tm_sec);

    char logTimeLevelLib[128] = {0};
    char logToFile[128] = {0};
    sprintf(logTimeLevelLib, "%s %s%-5s\033[0m[%s] ", timeString, levelColors[logLevel], levelStrings[logLevel], moduleName);
    sprintf(logToFile, "%s %-5s[%s] ", timeString, levelStrings[logLevel], moduleName);

    memset(buf, 0, sizeof(buf));

    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    printf("%s%s\n", logTimeLevelLib, buf);

    if (inited == 1)
    {
        FILE *fp;
        char fileName[1024];
        sprintf(fileName, "%s/%d-%d-%d.log", logPath, 1900 + nowTime->tm_year, 1 + nowTime->tm_mon, nowTime->tm_mday);
        if ((fp = fopen(fileName, "a")) == NULL)
            printf("%s%s\n", logTimeLevelLib, strerror(errno));
        fprintf(fp, "%s%s\n", logToFile, buf);
        fclose(fp);
    }
    pthread_mutex_unlock(&lock);
}

