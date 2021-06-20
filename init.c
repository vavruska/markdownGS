#pragma noroot

#include <types.h>
#include <stdio.h>
#include <string.h>

#include <memory.h>
#include <locator.h>
#include <gsos.h>
#include <orca.h>
#include <resources.h>

#include "babelfish.h"

typedef void (*DISPATCH_CB)(word, BFXferRecHndl, BFResultOutPtr);

char ipcName[32];
word userID;
//DISPATCH_CB dispatch = NULL;
Handle dispatchHandle;

pascal int myRequest(word request, BFXferRecHndl dataIn, BFResultOutPtr dataOut);
extern word dispatch(word request, BFXferRecHndl dataIn, BFResultOutPtr dataOut);


#pragma toolparms 1
#pragma databank 1
void init(void) {
    userID = MMStartUp();

    sprintf(ipcName, "xBabelfish~CCV~markdown%x", userID);
    ipcName[0] = strlen(ipcName + 1);

    AcceptRequests(ipcName, userID, (WordProcPtr) &myRequest);
}
#pragma databank 0
#pragma toolparms 0


#pragma toolparms 1
#pragma databank 1
pascal int myRequest(word request, BFXferRecHndl dataIn, BFResultOutPtr dataOut) {
    int result = 0;

    switch (request) {
    case srqGoAway:
        ((srqGoAwayOutPtr)dataOut)->resultID = MMStartUp();
        result = 0x8000;
        break;
    case TrStartUp:
        dispatch(request, dataIn, dataOut);
        result = 0x8000;
        break;
    case TrShutDown:
        dispatch(request, dataIn, dataOut);
        result = 0x8000;
        break;
    default:
        dispatch(request, dataIn, dataOut);
        result = 0x8000;
        break;
    }
    return result;
}
#pragma toolparms 0
#pragma databank 0

