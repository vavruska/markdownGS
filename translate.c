#pragma noroot

#include <types.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include <memory.h>
#include <orca.h>
#include <font.h>

#include "babelfish.h"
#include "translate.h"
#include "opts.h"

extern word userID;

extern TextData dataRecord;

typedef struct dataNode {
    TextData textData;
    Handle data;
    struct dataNode **next;
} dataNode, *dataNodePtr, **dataNodeHndl;

dataNodeHndl dataListHead = NULL;
dataNodeHndl dataListTail = NULL;
long err = 0;
long cnt = 0;


void writeString(char *str, size_t len) {
    extern TextData dataRecord;
    char *p;
    cnt++;
    dataNodeHndl node = (dataNodeHndl)NewHandle(sizeof(dataNode), userID, 0, NULL);
    if (toolerror() == 0) {
        (*node)->data = NewHandle(len, userID, attrFixed, NULL);
        if (toolerror() == 0) {
            PtrToHand(str,(*node)->data, len);
            memcpy(&(*node)->textData, &dataRecord, sizeof(TextData));
            //convert \n to \r
            p = *((*node)->data);
            while (p < *((*node)->data) + len) {
                if (*p == '\n') {
                    *p = '\r';
                }
                p++;
            }
            (*node)->next = NULL;

            if (dataListHead != NULL) {
                (*dataListTail)->next = node;
            } else {
                dataListHead = node;
            }
            dataListTail = node;
        } else {
            err++;
        }
    } else {
        err++;
    }
}

void writeChar(char c) {
    writeString(&c, 1);
}

Handle translate(void) {
    Handle hand = NULL;
    char *data;
    extern dataNodeHndl dataListHead;

    if (dataListHead != NULL) {
        dataNodeHndl node = dataListHead;
        dataListHead = (*node)->next;
        hand = (*node)->data;
        data = *hand;
        memcpy(&dataRecord, &(*node)->textData, sizeof(TextData));
        DisposeHandle((Handle) node);
    }

    return hand;
}

void setStyle(tStyleType styleType, uint16_t textMask, uint16_t headerSize) {
extern TextData dataRecord;
    dataRecord.bfFontID = getDefaultFaceID();

    switch (styleType) {
    case STYLE_TYPE_HEADER:
        dataRecord.backColor = 0xFFFF;
        dataRecord.foreColor = 0x0;
        dataRecord.fontSize = getHeaderSize(headerSize);
        dataRecord.fontStyle = textMask;
        break;

    default:
    case STYLE_TYPE_TEXT:
        dataRecord.backColor = 0xFFFF;
        dataRecord.foreColor = 0x0;
        dataRecord.fontSize = getTextSize();
        dataRecord.fontStyle = textMask;
        break;

    case STYLE_TYPE_QUOTE:
        dataRecord.backColor = 0xEEEE;
        dataRecord.foreColor = 0x0;
        dataRecord.fontSize = getTextSize();
        dataRecord.fontStyle = textMask;
        break;

    case STYLE_TYPE_CODE:
        dataRecord.bfFontID = getMonoFaceID();
        dataRecord.backColor = 0xEEEE;
        dataRecord.foreColor = 0x0;
        dataRecord.fontSize = getTextSize();
        dataRecord.fontStyle = textMask;
        break;
    }
}
#pragma debug 0
