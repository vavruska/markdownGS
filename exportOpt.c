#pragma noroot

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <types.h>

#include <control.h>
#include <lineedit.h>
#include <locator.h>
#include <memory.h>
#include <menu.h>
#include <misctool.h>
#include <orca.h>
#include <qdaux.h>
#include <quickdraw.h>
#include <resources.h>
#include <window.h>

#include "babelfish.h"

#include "opts.h"

#define WINDOW 0x00000FF9
#define VERSION 0x00000002
#define CANCEL 0x00000009
#define ACCEPT 0x0000000A
#define USEIMPORT 0x00000024
#define HEADER1 0x0000002D
#define HEADER2 0x0000002E
#define HEADER3 0x0000002F
#define HEADER4 0x00000030
#define HEADER5 0x00000031
#define HEADER6 0x00000032
#define ENDCHOICEMENU 0x00000025
#define INDENTMENU 0x00000035
#define SHOWWINDOW 0x00000036


typedef struct fontItem {
    word            famNum;
    word            itemID;
    char            name[27];
    struct fontItem **next;
} fontItem, *fontItemPtr, **fontItemHndl;

//forward declarations
void DrawWindow(void);
bool validateFontSize(WindowPtr wind, long controlID);
void setCtls(WindowPtr wind);

word userID;

#pragma databank 1
void exportOpt(BFXferRecPtr xfer) {
    WindowPtr     myWindow;
    EventRecord   CurrentEvent;
    word          controlID = 0;
    CtlRecHndl ctl;
    versionHndl   ver;
    bool done = false;
    int j = 0;
    word value = 0;
    bool update = false;
    char verStr[10] = { 0 };

    userID = MMStartUp();

    //load the options
    loadOptions();

    myWindow = NewWindow2(NULL, 0, &DrawWindow, NULL, refIsResource, WINDOW, rWindParam1);
    SetPort(myWindow);

    //version
    ver = (versionHndl)LoadResource(rVersion, 1);
    VersionString(0, (*ver)->version, verStr);
    ctl = GetCtlHandleFromID(myWindow, VERSION);
    (*ctl)->ctlMoreFlags = 0x1000;
    memmove(verStr + 2, verStr + 1, 8);
    verStr[1] = 'v';
    (*ctl)->ctlData = (long)(verStr + 1);
    (*ctl)->ctlValue = strlen(verStr + 1);

    //initialize the values
    SetCtlValueByID(getUseImportSizes(), myWindow, USEIMPORT);
    SetCtlValueByID(getLineEndingFormat(), myWindow, ENDCHOICEMENU);
    SetCtlValueByID(getExportIndent(), myWindow, INDENTMENU);
    SetCtlValueByID(getShowExportWindow(), myWindow, SHOWWINDOW);
    setCtls(myWindow);

    ShowWindow(myWindow);
    InitCursor();

    done = false;
    while (!done) {
        controlID = DoModalWindow(&CurrentEvent, NULL,
                                  (VoidProcPtr)0x80000000L, NULL, 0x0);

        switch (controlID) {
        case ACCEPT:
        case CANCEL:
            done = true;
            break;
        case USEIMPORT:
            ctl = GetCtlHandleFromID(myWindow, USEIMPORT);
            setUseImportSizes(GetCtlValue(ctl));
            setCtls(myWindow);
            break;
        default:
            break;
        }
    }

    if (controlID == ACCEPT) {
        value = GetCtlValueByID(myWindow, INDENTMENU);
        setLineEndingFormat(GetCtlValueByID(myWindow, ENDCHOICEMENU));
        setExportIndent(GetCtlValueByID(myWindow, INDENTMENU));
        setShowExportWindow(GetCtlValueByID(myWindow, SHOWWINDOW));
        for (long i = 0; i < 6; i++) {
            if (validateFontSize(myWindow, HEADER1 + i)) {
                Str32 txt;
                word fontSize;

                GetLETextByID(myWindow, HEADER1 + i, (StringPtr) &txt);
                fontSize = atoi(txt.text);
                if ((fontSize > 1) && (fontSize <= 255)) {
                    setExportHeaderSize(i + 1, fontSize);
                }
            }
        }
        saveOptions();
    }

    CloseWindow(myWindow);
}
#pragma databank 0

void DrawWindow(void) {
    DrawControls(GetPort());
}

bool validateFontSize(WindowPtr wind, long controlID) {
    CtlRecHndl ctl = GetCtlHandleFromID(wind, controlID);
    LERecHndl le = (LERecHndl) (*ctl)->ctlData;
    Handle leHand = LEGetTextHand(le);
    word leLen = LEGetTextLen(le);
    char *leData = (char *) *leHand;
    bool valid = true;
    int i;

    for (i = 0; i < leLen; i++) {
        if (!isdigit(leData[i])) {
            valid = false;
            break;
        }
    }

    if (valid && (leLen < 5)) {
        char data[5] = { 0 };
        word value;

        strncpy(data, leData, leLen);
        value = atoi(data);
        if ((value >=  1) && (value <= 255))  {
            //check to make sure fontsize exists
            valid = true;
        } else {
            valid = false;
        }
    } else {
        valid = false;
    }
    return valid;
}

void setLETextFromWord(WindowPtr wind, CtlRecHndl ctl, word val) {
    if (ctl) {
        LERecHndl le;
        le = (LERecHndl) (*ctl)->ctlData;
        if (le) {
            char data[10] = { 0 };

            snprintf(data, 10, "%d", val);
            LESetText(data, strlen(data), le);
            InvalRect(&(*ctl)->ctlRect);
        }
    }
}

void setCtls(WindowPtr wind) {
    word hilite = getUseImportSizes() ?  inactiveHilite : noHilite;
    CtlRecHndl ctl;

    for (int i = HEADER1; i < HEADER6 + 1; i++) {
        ctl = GetCtlHandleFromID(wind, i);
        HiliteControl(hilite, ctl);
        setLETextFromWord(wind, ctl, getExportHeaderSize(i - HEADER1 + 1));
    }
}

