#pragma noroot

#include <types.h>
#include <stdio.h>
#include <string.h>

#include <memory.h>
#include <locator.h>
#include <font.h>
#include <gsos.h>
#include <orca.h>
#include <intmath.h>
#include <event.h>
#include <window.h>
#include <quickdraw.h>
#include <resources.h>
#include <control.h>
#include <misctool.h>

#include "babelfish.h"

#include "opts.h"
#include "exporter.h"
#include "export.h"
#include "dispatch.h"

#define mdWindowId 0x00000FFB
#define thermoId 0x00000023

typedef struct tdList {
    TextData td;
    struct tdList **next;
} tdList, *tdListPtr, **tdListHndl;


extern word userID;

Handle newData = NULL;
static long pos = 0;
static long dataSize = 0;
static GSString255 exportFile;
static tdListHndl tdFront = NULL;
static tdListHndl tdBack = NULL;

void DrawWindow(void) {
    DrawControls(GetPort());
}

void gatherText(BFXferRecPtr dataIn) {
    TextData *td = (TextData *) dataIn->dataRecordPtr;
    tdListHndl node;
    extern tdListHndl tdFront = NULL;

    if (dataIn->status == bfContinue) {
        if (td->textStreamHndl || td->textStreamLen) {
            node = (tdListHndl)NewHandle(sizeof(tdList), userID, 0, NULL);
            (*node)->next = NULL;
            //put the new node at the back of the list
            (*tdBack)->next = node;
            tdBack = node;
            memcpy(&(*node)->td, td, sizeof(TextData));
            HLock(newData);
            if (td->textStreamHndl != NULL) {
                if (pos + GetHandleSize(td->textStreamHndl) > GetHandleSize(newData)) {
                    HUnlock(newData);
                    (*node)->td.textStreamLen = GetHandleSize(td->textStreamHndl);
                    (*node)->td.textStreamHndl = 0;
                    SetHandleSize(pos + GetHandleSize(td->textStreamHndl) + (32L * 1024L), newData);
                    if (toolerror()) {
                        dataIn->status = bfMemErr;
                    }
                    HLock(newData);
                }
                HandToPtr(td->textStreamHndl, *newData + pos, GetHandleSize(td->textStreamHndl));
                pos += GetHandleSize(td->textStreamHndl);
                DisposeHandle((Handle)td->textStreamHndl);
            } else {
                if (pos + td->textStreamLen > GetHandleSize(newData)) {
                    HUnlock(newData);
                    SetHandleSize(pos +  td->textStreamLen + (32L * 1024L), newData);
                    if (toolerror()) {
                        dataIn->status = bfMemErr;
                    }
                    HLock(newData);
                }
                memcpy(*newData + pos, td->textStreamPtr, td->textStreamLen);
                pos += td->textStreamLen;
            }
            HUnlock(newData);
        }
    }
}

word writeData(word refNum, char *data) {
    IORecGS       writeRec = { 4 };

    writeRec.refNum = refNum;
    writeRec.dataBuffer = data;
    writeRec.requestCount = strlen(data);
    WriteGS(&writeRec);

    return toolerror();
}

void exportData(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    CreateRecGS     createRec =  {6, &exportFile,0x00E3,0x0004,0L,1,0L};
    OpenRecGS       openRec = {3, 0, &exportFile, writeEnable };
    RefNumRecGS     closeRec = { 1 };
    NameRecGS       destroyRec = { 1, &exportFile };
    tdListHndl      node = NULL;
    long            position = 0;
    WindowPtr       mdWin;
    word            ratio = pos/255; 
    CtlRecHndl      thermo;
    EventRecord     currentEvent;
    extern long pos;

    dataIn->status = bfNoErr;

    //destroy
    DestroyGS(&destroyRec);
    if (toolerror() && toolerror() != fileNotFound) {
        dataIn->status = bfWriteErr;
        return;
    }
    //create
    CreateGS(&createRec);
    if (toolerror()) {
        dataIn->status = bfWriteErr;
        return;
    }

    //open
    OpenGS(&openRec);
    if (toolerror()) {
        dataIn->status = bfWriteErr;
        return;
    }
   
    if (ratio > 4 && 
        ((dataIn->progressAction & bffShowProgress) || getShowExportWindow())) {
        mdWin = NewWindow2(NULL, NULL, &DrawWindow, NULL,
                             refIsResource, mdWindowId, rWindParam1);
        SetPort(mdWin);
        ShowWindow(mdWin);
    }

    thermo = GetCtlHandleFromID(mdWin,thermoId);
    currentEvent.wmTaskMask = 0x001FFFFE;

    while ((dataIn->status == bfNoErr) && (node = tdFront)) {
        tdFront = (*tdFront)->next;
        TaskMaster(everyEvent, &currentEvent);
        TextData *td = (TextData *)&(*node)->td;
        if (writeMarkdown(openRec.refNum, *newData + position, &(*node)->td)) {
            DisposeHandle((Handle)node);
            break;
        }
        position += td->textStreamLen;
        DisposeHandle((Handle)node);
        if (ratio > 4 && 
            ((dataIn->progressAction & bffShowProgress) || getShowExportWindow())) {
            SetCtlValue((word)(position / ratio), thermo);
        }
    }

    if (ratio > 4 && 
        ((dataIn->progressAction & bffShowProgress) || getShowExportWindow())) {
        CloseWindow(mdWin);
    }

    //close
    closeRec.refNum = openRec.refNum;
    CloseGS(&closeRec);
}


void exportStartup(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    dataSize = 32L * 1024L;
    newData = NewHandle(dataSize, userID, attrFixed + attrLocked, NULL);
    pos = 0;
    tdFront = (tdListHndl) NewHandle(sizeof(tdList), userID, 0, NULL);
    memset(*tdFront, 0, sizeof(tdList));
    tdBack = tdFront;

    createDebugFile();

    loadOptions();

}

void exportCleanUp(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    tdListHndl next;

    //do the hard work
    exportData(dataIn, dataOut);

    while (tdFront) {
        next = (*tdFront)->next;
        DisposeHandle((Handle)tdFront);
        tdFront = next;
    }
}

void trWrite(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    TextData *td = (TextData *) dataIn->dataRecordPtr;

    if (dataIn->status == bfContinue) {
        switch (td->actionCode) {
        case bfGetSettings:
            memcpy(&exportFile, dataIn->filePathPtr, sizeof(GSString255));
            td->responseCode = bfTFReponseOK;
            break;
        case bfContinuing:
        case bfBodyText:
            gatherText(dataIn);
            td->responseCode = bfTFReponseOK;
            break;
        default:
            td->responseCode = bfTFReponseSkip;
            break;
        }
    }
}

