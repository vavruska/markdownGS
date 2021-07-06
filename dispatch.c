#pragma noroot

#include <types.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <memory.h>
#include <locator.h>
#include <font.h>
#include <gsos.h>
#include <orca.h>
#include <intmath.h>
#include <stdarg.h>

#include "babelfish.h"
#include "import.h"
#include "export.h"

extern word userID;
FILE *debugFd = NULL;
char dbgBuffer[256];
OpenRecGS dbgOpenRec = { 3, 0, 0, writeEnable };


word dispatch(word request, BFXferRecHndl dataIn, BFResultOutPtr dataOut) {
    int result = 0;

    switch (request) {
    case srqGoAway:      
        ((srqGoAwayOutPtr)dataOut)->resultID = MMStartUp();
        result = 0x8000;
        break;
    case TrStartUp:
        userID = MMStartUp();
        if (((*dataIn)->miscFlags) & bffExporting) {
            exportStartup(*dataIn, dataOut);
        } else {
            importStartup(*dataIn, dataOut);
        }
        result = 0x8000;
        break;
    case TrShutDown:
        if (((*dataIn)->miscFlags) & bffExporting) {
             exportCleanUp(*dataIn, dataOut);
        } else {
            importCleanUp();
        }
        result = 0x8000;
        break;
    case TrRead:
        trRead(*dataIn, dataOut);
        result = 0x8000;
        break;
    case TrWrite:
        trWrite(*dataIn, dataOut);
        result = 0x8000;
        break;
    }
    return result;
}

void createDebugFile(void) {
    GSString255 path = {10, "*:md.debug" };
    CreateRecGS     createRec =  {6, &path,0x00E3,0x0004,0L,1,0L};
    FileInfoRecGS   info = { 5, &path };
    ResultBuf255    option = { 255, 0 };
    NameRecGS destroy = { 1, &path };
    word err;

    //if the file exists then delete it and create a new one.
    //debugging is enabled by creating the file
    info.optionList = &option;
    GetFileInfoGS(&info);
    err = toolerror();
    if (toolerror() == 0) {
        DestroyGS(&destroy);
        if (toolerror() == 0) {
            CreateGS(&createRec);
            dbgOpenRec.pathname = &path;
            dbgOpenRec.optionList = 0;
            OpenGS(&dbgOpenRec);
        }
    }
}

void closeDebugFile(void) {
    if (dbgOpenRec.refNum) {
        RefNumRecGS     closeRec = { 1, dbgOpenRec.refNum };
        CloseGS(&closeRec);
    }
}

void debugPrint(const char *format, ...) {

    va_list arglist;
    extern char dbgBuffer[];

    va_start(arglist, format);
    vsprintf(dbgBuffer, format, arglist );
    va_end( arglist );
    if (dbgOpenRec.refNum) {
        IORecGS         writeRec = { 4, 0, 0, 0, 0 };
        RefNumRecGS     flushRec = { 1, dbgOpenRec.refNum };

        writeRec.refNum = dbgOpenRec.refNum;
        writeRec.dataBuffer = dbgBuffer;
        writeRec.requestCount = strlen(dbgBuffer);
        WriteGS(&writeRec);
        FlushGS(&flushRec);
    }
}

void debugWrite(char *text, size_t charSize, size_t size) {
    if (dbgOpenRec.refNum) {
        IORecGS writeRec = { 4, 0, 0, 0, 0 };
        RefNumRecGS flushRec = { 1, dbgOpenRec.refNum };

        writeRec.refNum = dbgOpenRec.refNum;
        writeRec.dataBuffer = text;
        writeRec.requestCount = size * charSize;
        WriteGS(&writeRec);
        FlushGS(&flushRec);
    }
}

