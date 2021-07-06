#pragma noroot

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <types.h>

#include <control.h>
#include <font.h>
#include <lineedit.h>
#include <list.h>
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

#define WINDOW 0x00000FFA
#define VERSION 0x00000002
#define CANCEL 0x00000009
#define ACCEPT 0x0000000A
#define FONTLIST 000000004
#define FONTSIZE 0x00000008
#define SISDEFAULTS 0x00000003
#define HEADER1 0x00000012
#define HEADER2 0x00000013
#define HEADER3 0x00000014
#define HEADER4 0x00000015
#define HEADER5 0x00000016
#define HEADER6 0x00000017
#define WARNFONTSIZE 0x00000019
#define WARNHEADER1 0x0000001A
#define WARNHEADER2 0x0000001B
#define WARNHEADER3 0x0000001C
#define WARNHEADER4 0x0000001D
#define WARNHEADER5 0x0000001E
#define WARNHEADER6 0x0000001F
#define WARNICON 0x00000020
#define WARNTEXT 0x00000021
#define INDENTMENU 0x00000034

typedef struct fontItem {
    word            famNum;
    word            itemID;
    char            name[27];
    struct fontItem **next;
} fontItem, *fontItemPtr, **fontItemHndl;

//forward declarations
void DrawWindow(void);
void setCtls(WindowPtr wind);
void buildFontList(void);
void addFontsToMenu(CtlRecHndl ctl, char *defaultFont);
void DisposeMenuMemory(void);
bool checkForSIS(void);
void validateFontSizes(WindowPtr wind);

WindowPtr     myWindow;
EventRecord   CurrentEvent;
word          controlID = 0;
versionHndl   ver;
word userID;
CtlRecHndl ctl;
MemRecHndl myListHndl;
fontItemHndl firstFont = NULL;
MenuItemTemplate **GenericFont = NULL;

#pragma databank 1
void importOpt(BFXferRecPtr xfer) {
    bool done = 0;
    int j = 0;
    word idx;
    word value = 0;
    bool update = false;
    char verStr[10] = {0};

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
    if (!checkForSIS()) {
        //SIS Font not present then clear the useSis and disable the checkbox. 
        setUseSis(false);
        HiliteCtlByID(inactiveHilite, myWindow, SISDEFAULTS);
    }
    SetCtlValueByID(getUseSis(), myWindow, SISDEFAULTS);
    setCtls(myWindow);
    validateFontSizes(myWindow);
    buildFontList();
    addFontsToMenu(GetCtlHandleFromID(myWindow, FONTLIST), getDefaultFace());
    SetCtlValueByID(getIndentStyle(), myWindow, INDENTMENU);
    CalcMenuSize(0, 0, 0);

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
        case SISDEFAULTS:
            ctl = GetCtlHandleFromID(myWindow, SISDEFAULTS);
            setUseSis(GetCtlValue(ctl));
            setCtls(myWindow);
        case HEADER1:
        case HEADER2:
        case HEADER3:
        case HEADER4:
        case HEADER5:
        case HEADER6:
        case FONTSIZE:
            //validate sizes
            validateFontSizes(myWindow);
            break;
        case FONTLIST: 
            {
                fontItemHndl curItem;
                word value = GetCtlValueByID(myWindow, FONTLIST);
                curItem = firstFont;
                while (curItem) {
                    if ((*curItem)->itemID == value) {
                        setDefaultFace((*curItem)->name);
                        validateFontSizes(myWindow);
                        break;
                    }
                    curItem = (*curItem)->next;
                }
                break;
            }
        default:
            break;
        }
    }

    if (controlID == ACCEPT) {
        fontItemHndl curItem = firstFont;

        value = GetCtlValueByID(myWindow, FONTLIST);
        while (curItem) {
            if ((*curItem)->itemID == value) {
                setDefaultFace((*curItem)->name);
                break;
            }
            curItem = (*curItem)->next;
        }
        if (curItem == NULL) {
            setDefaultFace("\pHelvetica");
        }
        setIndentStyle(GetCtlValueByID(myWindow, INDENTMENU));
        saveOptions();
    }

    CloseWindow(myWindow);
    DisposeMenuMemory();
}
#pragma databank 0

void DrawWindow(void) {
    DrawControls(GetPort());
}

void setLETextFromWord(WindowPtr wind, CtlRecHndl ctl, word val) {
    if (ctl) {
        char data[10] = { 0 };
        LERecHndl le;
        le = (LERecHndl) (*ctl)->ctlData;
        if (le) {
            snprintf(data, 10, "%d", val);
            LESetText(data, strlen(data), le);
            InvalRect(&(*ctl)->ctlRect);
        }
    }

}

void setCtls(WindowPtr wind) {
    word hilite = getUseSis() ?  inactiveHilite : noHilite;
    CtlRecHndl ctl;
    LERecHndl le;

    HiliteCtlByID(hilite, wind, FONTLIST);
    ctl = GetCtlHandleFromID(wind, FONTSIZE);
    setLETextFromWord(wind, ctl, getTextSize());
    HiliteControl(hilite, ctl);

    for (int i = HEADER1; i < HEADER6 + 1; i++) {
        HiliteControl(hilite, ctl);
        ctl = GetCtlHandleFromID(wind, i);
        setLETextFromWord(wind, ctl, getHeaderSize(i - HEADER1 + 1));
    }
}

void buildFontList(void) {
    fontItemHndl currentFont = NULL;
    word famNum, famSpecs = 0;
    word positionNum = 1;
    Handle h = NULL;
    word value = 0;
    word selectedFam = 0;
    fontItemHndl curItem, prevItem;
    MenuBarRecHndl oldMenu;

    do {
        famNum = FindFamily(famSpecs, positionNum, NULL);

        if (famNum != 0xFFFF) {
            h = NewHandle(sizeof(fontItem), userID, attrFixed + attrLocked, NULL);
            currentFont = (fontItem **)h;
            (*currentFont)->next = NULL;
            (*currentFont)->famNum = FindFamily(famSpecs, positionNum, (char *)(*currentFont)->name);
            (*currentFont)->name[(*currentFont)->name[0] + 1] = 0;

            if (firstFont == NULL) {
                firstFont = currentFont;
            } else {
                curItem = firstFont;
                prevItem = NULL;

                while (curItem && strcmp((*currentFont)->name + 1, (*curItem)->name + 1) > 0) {
                    prevItem = curItem;
                    curItem = (*curItem)->next;
                }

                if (curItem == NULL && prevItem != NULL) {   /* Insert at end of list */
                    (*prevItem)->next = currentFont;
                } else if (curItem == firstFont) { /* Insert at the front of the list */
                    (*currentFont)->next = firstFont;
                    firstFont = currentFont;
                } else {                      /* Insert in the 'middle' of the list */
                    (*currentFont)->next = curItem;
                    if (prevItem != NULL) {
                        (*prevItem)->next = currentFont;
                    }
                }
            }
        }
        positionNum++;
    } while (famNum != 0xFFFF);

    GenericFont = (MenuItemTemplate **) NewHandle(sizeof(MenuItemTemplate) * positionNum, userID, attrFixed + attrLocked, NULL);
    currentFont = firstFont;
    positionNum = 1;

    while (currentFont != NULL) {
        memset(&(*GenericFont)[positionNum -1], 0, sizeof(MenuItemTemplate));
        (*currentFont)->itemID = 4000 + positionNum;
        (*GenericFont)[positionNum - 1].itemID = 4000 + positionNum;
        (*GenericFont)[positionNum - 1].itemFlag = 0;
        (*GenericFont)[positionNum - 1].itemTitleRef = (Ref)(*currentFont)->name;

        currentFont = (*currentFont)->next;
        positionNum++;
    }
}

void addFontsToMenu(CtlRecHndl ctl, char *defaultFont) {
    fontItemHndl currentFont = firstFont;
    word value = 0, positionNum = 1;
    MenuBarRecHndl oldMenu;

    oldMenu = GetMenuBar();
    SetMenuBar(ctl);
    DeleteMItem(0x00000001); //remove the placeholder MItem

    while (currentFont != NULL) {
        if (((*currentFont)->name[0] == defaultFont[0]) &&
            (strncmp(&(*currentFont)->name[1], &defaultFont[1], (*currentFont)->name[0])) == 0) {
            value = (*currentFont)->itemID;
        }
        InsertMItem2(refIsPointer, (long)&(*GenericFont)[positionNum - 1], 0xFFFF, 0x0001);

        currentFont = (*currentFont)->next;
        positionNum++;
    }
    SetMenuBar(oldMenu);
    if (value) {
        SetCtlValue(value, ctl);
    }
}

void DisposeMenuMemory(void) {
    fontItemHndl currentFont = NULL;
    Handle h;

    currentFont = firstFont;
    while (currentFont != NULL) {
        h = (handle) currentFont;
        currentFont = (*currentFont)->next;
        DisposeHandle(h);
    }
    DisposeHandle((Handle) GenericFont);
}

bool checkForSIS(void) {
    FontID fid;
    FontStatRec fontRec;
    word fontSizes[] = { 9, 10, 12, 14, 18, 24 };
    bool retVal = true;

    fid.fidLong = 0L;
    for (int x = 0; x < 2; x++) {
        fid.fidRec.famNum = 36866 + x;
        for (int y = 0; y < 6; y++) {
            fid.fidRec.fontSize = fontSizes[y];
            FindFontStats(fid, realOnlyBit , 0x0001, &fontRec);
            if (fontRec.resultStats & notFoundBit) {
                retVal =  false;
                break;
            }
        }
    }
    return retVal;
}

bool verifyFontSize(word size) {
    FontID fid;
    FontStatRec fontRec;
    bool retVal = true;

    fid.fidLong = 0L;
    fid.fidRec.famNum = getDefaultFaceID();
    fid.fidRec.fontSize = size;
    FindFontStats(fid, realOnlyBit , 0x0001, &fontRec);
    if (fontRec.resultStats & notFoundBit) {
        retVal = false;
    }

    return retVal;
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
            valid = verifyFontSize(value);
        } else {
            valid = false;
        }
    } else {
        valid = false;
    }
    return valid;
}

void validateFontSizes(WindowPtr wind) {
    bool valid = true;
    long i;

    for (i = HEADER1; i < HEADER6 + 1; i++) {
        if (!validateFontSize(wind, i)) {
            valid = false;
            ShowControl(GetCtlHandleFromID(wind, WARNHEADER1 + (i - HEADER1)));
        } else {
            HideControl(GetCtlHandleFromID(wind, WARNHEADER1 + (i - HEADER1)));
        }
    }

    if (!validateFontSize(wind, FONTSIZE)) {
        valid = false;
        ShowControl(GetCtlHandleFromID(wind, WARNFONTSIZE));
    } else {
        HideControl(GetCtlHandleFromID(wind, WARNFONTSIZE));
    }

    if (valid) {
        HideControl(GetCtlHandleFromID(wind, WARNICON));
        HideControl(GetCtlHandleFromID(wind, WARNTEXT));
    } else {
        ShowControl(GetCtlHandleFromID(wind, WARNICON));
        ShowControl(GetCtlHandleFromID(wind, WARNTEXT));
    }
}

