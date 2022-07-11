#include <malloc.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#include "value.h"
#include "native_utils.h"

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define strcasecmp stricmp
#endif


NATIVE_FUNCTION(fileRead)
{
    CHECK_ARG_COUNT("fileRead", 1);

    FILE* file = fopen(AS_CSTRING(args[0]), "r");

    if(file == NULL)
    {
        nativeError(line, "fileRead", "File '%s' does not exist.", AS_CSTRING(args[0]));
        return NULL_VAL;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        nativeError(line, "fileRead", "Not enough memory to read file '%s'.", AS_CSTRING(args[0]));
        return NULL_VAL;
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

    if (bytesRead < fileSize)
    {
        nativeError(line, "fileRead", "Failed to read file '%s'.", AS_CSTRING(args[0]));
        return NULL_VAL;
    }
    buffer[bytesRead] = '\0';

    fclose(file);

    return OBJ_VAL(copyString(buffer, strlen(buffer)));
}

NATIVE_FUNCTION(fileWrite)
{
    CHECK_ARG_COUNT("fileWrite", 2);

    FILE* file = fopen(AS_CSTRING(args[0]), "w");

    if (file == NULL)
    {
        nativeError(line, "fileRead", "File '%s' does not exist.", AS_CSTRING(args[0]));
        return NULL_VAL;
    }

    fprintf(file, "%s", AS_CSTRING(args[1]));

    fclose(file);

    return NULL_VAL;
}

NATIVE_FUNCTION(fileCreate)
{
    CHECK_ARG_COUNT("fileCreate", 1);

    FILE* file = fopen(AS_CSTRING(args[0]), "w");

    if(file == NULL)
    {
        nativeError(line, "fileCreate", "Failed to create file at '%s'.", args[0]);
    }
    else
    {
        fclose(file);
    }

    return NULL_VAL;
}

NATIVE_FUNCTION(fileRemove)
{
    CHECK_ARG_COUNT("fileRemove", 1);

    if (remove(AS_CSTRING(args[0])) != 0)
    {
        nativeError(line, "fileCreate", "Failed to remove file at '%s'.", args[0]);
    }

    return NULL_VAL;
}

NATIVE_FUNCTION(directoryCreate)
{
    CHECK_ARG_COUNT("directoryCreate", 1);

    if (mkdir(AS_CSTRING(args[0]), S_IRWXU | S_IRWXG | S_IRWXO) == -1)
    {
        nativeError(line, "directoryCreate", strerror(errno));
    }

    return NULL_VAL;
}

NATIVE_FUNCTION(directoryRemove)
{
    CHECK_ARG_COUNT("directoryCreate", 1);

    if (rmdir(AS_CSTRING(args[0])) == -1)
    {
        nativeError(line, "directoryCreate", strerror(errno));
    }

    return NULL_VAL;
}

NATIVE_FUNCTION(directoryExists)
{
    CHECK_ARG_COUNT("directoryExists", 1);

    DIR* dir = opendir(AS_CSTRING(args[0]));

    if (dir)
    {
        closedir(dir);
        return TRUE_VAL;
    }
    else
    {
        return FALSE_VAL;
    }

}

NATIVE_FUNCTION(fileExists)
{
    CHECK_ARG_COUNT("fileExists", 1);

    struct stat path_stat;
    stat(AS_CSTRING(args[0]), &path_stat);

    if(S_ISREG(path_stat.st_mode))
    {
        return TRUE_VAL;
    }
    else
    {
        return FALSE_VAL;
    }

}

NATIVE_FUNCTION(inputString)
{
    CHECK_ARG_COUNT("inputString", 1);

    int bufferLength = (int)AS_NUMBER(args[0]);

    char buffer[bufferLength];

    if (fgets(buffer, bufferLength, stdin) != NULL)
    {
        return OBJ_VAL(copyString(buffer, strlen(buffer) - 1));
    }
    else
    {
        return NULL_VAL;
    }

}

NATIVE_FUNCTION(inputYesNo)
{
    CHECK_ARG_COUNT("inputYesNo", 0);

    char str[4];

    if (fgets(str, 4, stdin) != NULL)
    {
        if(strcasecmp(str, "yes") == 0 ||
           (strlen(str) == 2 && (str[0] == 'Y' || str[0] == 'y')))
        {
            return TRUE_VAL;
        }
        // For whatever reason I had to add \n here, but not in yes
        else if(strcasecmp(str, "no\n") == 0 ||
                (strlen(str) == 2 && (str[0] == 'N' || str[0] == 'n')))
        {
            return FALSE_VAL;
        }
        else
        {
            return NULL_VAL;
        }
    }

    return NULL_VAL;

}

NATIVE_FUNCTION(exit)
{
    CHECK_ARG_COUNT("exit", 1);

    exit(AS_NUMBER(args[0]));
}

NATIVE_FUNCTION(getDate)
{
    CHECK_ARG_COUNT("getDate", 0);

    time_t t = time(NULL);
    struct tm* timeData = localtime(&t);

    char str[64];
    strftime(str, sizeof(str), "%c", timeData);

    return OBJ_VAL(copyString(str, strlen(str)));
}

void defineOS(Table* table)
{
    ObjClass* os = newClass(copyString("os", 2));

    #define DEFINE_OS_METHOD(name, method) defineNativeFunction(os->methods, name, method)

    DEFINE_OS_METHOD("getDate",         getDateNative);
    DEFINE_OS_METHOD("inputYesNo",      inputYesNoNative);
    DEFINE_OS_METHOD("inputString",     inputStringNative);
    DEFINE_OS_METHOD("fileExists",      fileExistsNative);
    DEFINE_OS_METHOD("directoryExists", directoryExistsNative);
    DEFINE_OS_METHOD("directoryRemove", directoryRemoveNative);
    DEFINE_OS_METHOD("directoryCreate", directoryCreateNative);
    DEFINE_OS_METHOD("fileRemove",      fileRemoveNative);
    DEFINE_OS_METHOD("fileCreate",      fileCreateNative);
    DEFINE_OS_METHOD("fileWrite",       fileWriteNative);
    DEFINE_OS_METHOD("fileRead",        fileReadNative);
    DEFINE_OS_METHOD("exit",            exitNative);

    #undef DEFINE_OS_METHOD

    char pathSeparator;

    #ifdef WIN32
    pathSeparator = '\';
    #else
    pathSeparator = '/';
    #endif

    ObjInstance* instance = newInstance(os);
    tableDefineEntry(instance->fields, copyString("pathSeparator", 13),
                     OBJ_VAL(copyString(&pathSeparator, 1)));

    tableDefineEntry(
            table,
            os->name,
            OBJ_VAL(instance)
    );

}

