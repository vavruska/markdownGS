#pragma noroot

#include <types.h>
#include <string.h>
#include <gsos.h>
#include <intmath.h>
#include <locator.h>
#include <memory.h>
#include <orca.h>
#include <quickdraw.h>

#include "babelfish.h"

#include "import.h"
#include "importer.h"
#include "dispatch.h"
#include "translate.h"
#include "opts.h"

#define CLEAR_DATARECORD(d, p, a) \
    d.pCount = p; \
    d.textStreamHndl = 0L; \
    d.textStreamPtr = 0L; \
    d.textStreamLen = 0L; \
    d.actionCode = a

FileInfoRecGS info = { 12, 0 };
OpenRecGS openRec = { 12, 0, 0, readEnable };
RefNumRecGS closeRec = { 1, 0 };
IORecGS readRec = { 4, 0, 0, 0, 0 };
extern word userID;
word readStep = 0;
Handle dataHandle = 0;
Handle textHandle = 0;
char *dataPtr;
long ratio, lastRatio;
TextData dataRecord;
char importMsg[] = "\pPhase 1: Importing markdown...";
char translateMsg[] = "\pPhase 1: Importing markdown...\rPhase 2: Translating markdown...";
long pos,size;
BFXferRecPtr globalXfer;

void trRead(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    BFProgressIn progressIn;
    BFProgressOut progressOut;

    dataPtr = *dataHandle;
    globalXfer = dataIn;
    progressIn.xferRecPtr = dataIn;
    if (dataIn->status == 0) {
        switch (readStep) {
        case 1: //get settings
            dataIn->dataRecordPtr = (Pointer)&dataRecord;
            CLEAR_DATARECORD(dataRecord, 5, bfGetSettings);
            dataOut->bfResult = bfNoErr;
            readStep++;
            break;
        case 2: //read data
            CLEAR_DATARECORD(dataRecord, 5, bfBodyText);
            dataOut->bfResult = bfNoErr;

            readRec.pCount = 4;
            readRec.requestCount = openRec.eof;
            ReadGS(&readRec);

            if (!toolerror()) {
                pos = size = 0;
                if (dataIn->miscFlags & bffShowProgress) {
                    dataIn->fullTherm = 255;
                    dataIn->currentTherm = 0;
                    dataIn->msgPtr = importMsg;
                    ratio = FixDiv(readRec.transferCount, 255L);
                    dataIn->progressAction = bfNewMercury + bfSetScale + bfNewMessage + bfUserMsg;
                    SendRequest(BFProgress,                 /* Request  */
                                sendToName + stopAfterOne,    /* Flags    */
                                (long)NAME_OF_BABELFISH,          /* Name     */
                                (long)&progressIn,                     /* Data In  */
                                (ptr)&progressOut);            /* Data Out */
                }
                //call md4c
                if (mdImport(*dataHandle, readRec.transferCount, dataIn->miscFlags & bffShowProgress) != -1) {
                    //we no longer need the data from the file
                    DisposeHandle(dataHandle);
                    if (dataIn->status == bfContinue && dataIn->miscFlags & bffShowProgress) {
                        dataIn->currentTherm = 255;
                        dataIn->progressAction = bfNewMercury + bfUserMsg;
                        ratio = FixDiv(size, 255L);
                        if (ratio == 0) {
                            ratio = 1;
                            dataIn->fullTherm = size;
                        } 
                        SendRequest(BFProgress,                 /* Request  */
                                    sendToName + stopAfterOne,  /* Flags    */
                                    (long)NAME_OF_BABELFISH,    /* Name     */
                                    (long)&progressIn,          /* Data In  */
                                    (ptr)&progressOut);         /* Data Out */
                    }
                } else {
                    dataIn->status = bfReadErr;
                    dataOut->bfResult = _toolErr;
                }
            } else if (_toolErr == 0x004C) {
                dataIn->status = bfDone;
                dataOut->bfResult = bfNoErr;
            } else {
                dataIn->status = bfReadErr;
                dataOut->bfResult = _toolErr;
            }
            CloseGS(&closeRec);
            readStep++;
            break;
        case 3:
            dataIn->msgPtr = NULL;
            CLEAR_DATARECORD(dataRecord, 10, bfBodyText);
            dataOut->bfResult = bfNoErr;
            lastRatio = 0;
            readStep++;
            break;
        case 4:
            if (textHandle) {
                DisposeHandle(textHandle);
            }
            textHandle = translate();

            if (textHandle) {
                dataIn->dataRecordPtr = (Pointer) &dataRecord;
                dataIn->status = bfContinuing;
                dataPtr = *textHandle;
                pos += GetHandleSize(textHandle);
                CLEAR_DATARECORD(dataRecord, 10, bfContinuing);
                dataRecord.textStreamPtr = *textHandle;
                dataRecord.textStreamLen = GetHandleSize(textHandle);
                if (dataIn->miscFlags & bffShowProgress) {
                    dataIn->currentTherm = FixDiv(pos,ratio);
                    dataIn->progressAction = bfNewMercury + bfUserMsg;
                    dataIn->msgPtr = translateMsg;
                    if (lastRatio == 0) {
                        dataIn->progressAction |= bfNewMessage;
                    }
                    if (lastRatio != dataIn->currentTherm) {
                        SendRequest(BFProgress,                 /* Request  */
                                    sendToName + stopAfterOne,  /* Flags    */
                                    (long)NAME_OF_BABELFISH,    /* Name     */
                                    (long)&progressIn,          /* Data In  */
                                    (ptr)&progressOut);         /* Data Out */
                        lastRatio = dataIn->currentTherm;
                    }
                }
            } else {
                dataIn->status = bfContinue;
                readStep++;
                if (dataIn->miscFlags & bffShowProgress) {
                    dataIn->currentTherm = 255;
                    dataIn->progressAction =  bfNewMercury + bfUserMsg;
                    SendRequest(BFProgress,                 /* Request  */
                                sendToName + stopAfterOne,  /* Flags    */
                                (long)NAME_OF_BABELFISH,    /* Name     */
                                (long)&progressIn,          /* Data In  */
                                (ptr)&progressOut);         /* Data Out */
                }
            }
            dataOut->bfResult = bfNoErr;
            break;
        case 5:
            if (textHandle) {
                DisposeHandle(textHandle);
            }
            CLEAR_DATARECORD(dataRecord, 10, bfDone);
            dataIn->status = bfDone;
            dataOut->bfResult = bfNoErr;
            break;
        }
    } else {
        CLEAR_DATARECORD(dataRecord, 10, bfDone);
        dataIn->status = bfDone;
        dataOut->bfResult = bfNoErr;
    }
}

void importStartup(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {

    readStep = 1;

    createDebugFile();

    loadOptions();

    openRec.pathname = dataIn->filePathPtr;
    openRec.optionList = 0;
    OpenGS(&openRec);
    if (!toolerror()) {
        closeRec.refNum = openRec.refNum;
        readRec.refNum = openRec.refNum;

        dataHandle = NewHandle(openRec.eof, userID, attrFixed + attrLocked, NULL);
        dataPtr = *dataHandle;
        readRec.requestCount = openRec.eof;
        if (toolerror()) {
            dataIn->status = bfMemErr;
            return;
        }

        readRec.dataBuffer = *dataHandle;
        memset(&dataIn->dataKinds, 0, sizeof(dataIn->dataKinds));
        dataIn->dataKinds.flag1 = bffText;
    } else {
        dataIn->status = bfAbortErr;
        dataOut->bfResult = toolerror();
    }
}

void importCleanUp(void) {
}
