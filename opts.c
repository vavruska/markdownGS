#pragma noroot

#include <types.h>
#include <string.h>

#include <font.h>
#include <locator.h>
#include <memory.h>

#include "babelfish.h"

#include "opts.h"

extern word userID;
static opts options = DEFAULT_OPTS;
static opts optBackup;
static word id = 0;

word sisHeaderSizes[] = { 24, 18, 14, 12, 10, 9 };

void loadOptions(void) {
    BFLoadDataIn loadDataIn;
    BFLoadDataOut loadDataOut = { 0 };

    loadDataIn.userID = userID;
    SendRequest(BFLoadData,
                sendToName + stopAfterOne,
                (long)NAME_OF_BABELFISH,
                (long)&loadDataIn,
                (ptr)&loadDataOut);

    if (loadDataOut.bfResult == bfNoErr) {
        memcpy(&options, *(loadDataOut.dataHandle), sizeof(opts));
    }
}

void saveOptions(void) {
    BFSaveDataIn saveDataIn;
    BFSaveDataOut saveDataOut;

    saveDataIn.userID = userID;
    saveDataIn.dataHandle = NewHandle(sizeof(opts), userID, attrLocked, NULL);
    PtrToHand((ptr)&options, saveDataIn.dataHandle, sizeof(opts));
    HUnlock(saveDataIn.dataHandle);
    SendRequest(BFSaveData,
                sendToName + stopAfterOne,
                (long)NAME_OF_BABELFISH,
                (long)&saveDataIn,
                (ptr)&saveDataOut);
}

word getHeaderSize(word size) {
    word retSize = 12;
    if ((size >= 1) || (size <= 7)) {
        retSize = options.headerSize[size - 1];
        if (options.sisDefaults) {
            retSize = sisHeaderSizes[size - 1];
        }
    }
    return retSize;
}

void setHeaderSize(word size, word value) {
    if ((size >= 1) || (size <= 7)) {
        options.headerSize[size - 1] = value;
    }
}

word getTextSize(void) {
    word retVal = options.defaultSize;
    if (options.sisDefaults) {
        retVal = 9;
    }
    return retVal;
}

void setTextSize(word value) {
    options.defaultSize = value;
}

word getUseSis(void) {
    return options.sisDefaults;
}

void setUseSis(word value) {
    options.sisDefaults = value;
}

char *getDefaultFace(void) {
    return options.defaultFace;
}

void setDefaultFace(char *face) {
    memset(options.defaultFace, 0, MAX_FACE_NAME);
    memcpy(options.defaultFace, face, face[0] + 1 > 20 ? MAX_FACE_NAME : face[0] + 1);
    id = 0;
}

word getDefaultFaceID(void) {
    word retVal = 36866; //SIS-1 font ID
    if (!options.sisDefaults) {
        if (id == 0) {
            word famSpecs = 0, famNum;
            word pos = 1;
            char name[27];
            do {
                if ((famNum = FindFamily(famSpecs, pos, NULL)) != 0xFFFF) {
                    famNum = FindFamily(famSpecs, pos, (char *)name);
                    if ((name[0] == options.defaultFace[0]) &&
                        (strncmp(name + 1, options.defaultFace + 1, name[0]) == 0)) {
                        id = famNum;
                        break;
                    }
                }
                pos++;

            } while (famNum != 0xFFFF);
        }
        retVal = id;
    }
    return retVal;
}

word getMonoFaceID(void) {
    word retVal = courier;
    if (options.sisDefaults) {
        retVal = 36867; //SIS-2 font ID
    }
    return retVal;
}


