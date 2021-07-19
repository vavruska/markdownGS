#pragma noroot

#include <types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <font.h>
#include <gsos.h>
#include <memory.h>
#include <orca.h>

#include "babelfish.h"

#include "defs.h"
#include "opts.h"
#include "export.h"
#include "exporter.h"
#include "dispatch.h"

#define OUT_ALLOC_BUF_SIZE 4096

//externs
extern word userID;
extern tEntity entities[];

//locals
static word column = 0;
static bool inHeading = false;
static word curStyle = 0;
static long skipOffset = 0;
static bool inCode = false;
static bool inCodeBlock = false;

void newLine(char *func, char *data, long *pos, char *output, long *len, long dataLen) {
    //fix for line ending
    switch (getLineEndingFormat()) {
    case OPTS_LINE_APPLE:
        output[*len] = '\r';
        (*len)++;
        break;
    case OPTS_LINE_LINUX:
        output[*len] = '\n';
        (*len)++;
        break;
    case OPTS_LINE_WINDOWS:
        output[*len] = '\r';
        (*len)++;
        output[*len] = '\n';
        (*len)++;
        break;
    }
    debugPrint("[NL %s]\r", func);

    column = 0;
    inHeading = false;
    //if there were a \n after a \r then skip it as it was part of a Windows line ending
    if ((*pos < dataLen) && ((data[*pos] == '\r') && (data[(*pos)+1] == '\n'))) {
        (*pos)++;
    }
}

void insertEntity(char *val, long *pos, char *output, long *len) {
    word entityNum = 0;
    while (entities[entityNum].entityChar != 0) {
        if (*val == entities[entityNum].entityChar) {
            strcpy(output + *len, entities[entityNum].entityString);
            *len += strlen(entities[entityNum].entityString);
            break;
        }
        entityNum++;
    }
    //if we didnt find a matching entity write the hex code
    if (entities[entityNum].entityChar == 0) {
        *len += sprintf(output + *len, "&x%d", *val);
    }
}

static void setExportStyle(word style, char *out, long *len) {
    word count = 0;
    if (!inHeading) {
        word newStyle = curStyle;
        //set the styles
        if ((style & STYLE_TEXT_MASK_STRONG) &&
            (!(curStyle & STYLE_TEXT_MASK_STRONG))) {
            debugPrint("[B]");
            count += 2;
            newStyle |= STYLE_TEXT_MASK_STRONG;
        }
        if ((style & STYLE_TEXT_MASK_EMPHASIZED) && 
            (!(curStyle & STYLE_TEXT_MASK_EMPHASIZED))) {
            debugPrint("[I]");
            count += 1;
            newStyle |= STYLE_TEXT_MASK_EMPHASIZED;
        }
        //unset the styles
        if ((!(style & STYLE_TEXT_MASK_STRONG)) &&
            (curStyle & STYLE_TEXT_MASK_STRONG)) {
            debugPrint("[\B]");
            count += 2;
            newStyle &= ~STYLE_TEXT_MASK_STRONG;
        }
        if ((!(style & STYLE_TEXT_MASK_EMPHASIZED)) &&
            (curStyle & STYLE_TEXT_MASK_EMPHASIZED)) {
            debugPrint("[\I]");
            count += 1;
            newStyle &= ~STYLE_TEXT_MASK_EMPHASIZED;
        }
        curStyle = newStyle;
        for (int i = 0; i < count; i++) {
            out[*len] = '*';
            (*len)++;
        }
    }
}

bool headerCheck(char data, TextData *td, char *out, long *len) {
    word headerSize = 0;
    int i;

    for (i = 1; i < 7; i++) {
        if (td->fontSize == getExportHeaderSize(i)) {
            headerSize = i;
            break;
        }
    }
    if (headerSize) {
        debugPrint("\r[Header Size: %d]", headerSize);

        for (i = 0; i < headerSize; i++) {
            out[*len] = '#';
            (*len)++;
        }
        if (data != ' ') {
            out[*len] = ' ';
            (*len)++;
        }
    }
    return (headerSize != 0);
}

bool horizontalRuleCheck(char *data, long *pos, char *output, long *len, long dataLen) {
    if (dataLen - *pos > 31) {
        //check to make sure the whole line is just a horizontal line
        if (strncmp(data + *pos, HORIZONTAL_LINE"\r", strlen(HORIZONTAL_LINE) + 1) == 0) {
            debugPrint("\r[Horizontal Line]");
            //newLine(__FUNC__, data, pos, output, len, dataLen);
            strcpy(output + *len, "---");
            *len += 3;
            *pos += (strlen(HORIZONTAL_LINE) + 1);
            newLine("horizontalRuleCheck", data, pos, output, len, dataLen);
            column = 0;
            return true;
        }
    }
    return false;
}

bool listCheck(char *data, long *pos, char *output, long *len, long dataLen) {
    long curPos = *pos;
    char *d = data + curPos;
    word numbers = 0;
    bool inList = false;

    //list are determined by multiples of spaces (or tabs) followed
    //by a bullet char or a number and a period. 
    if ((*d == ' ') || (*d == '\t') || (*d == LIST_BULLET) || isdigit(*d)) {
        int count = 0;
        while ((strncmp(d, SPACES, getExportIndent()) == 0) || (*d == '\t')) {
            if (*d == '\t') {
                curPos++;
            } else {
                curPos += getExportIndent();
            }
            count++;
            d = data + curPos;
        }

        if (*d != LIST_BULLET) {
            char *n = d;
            while (isdigit(*n)) {
                numbers++;
                n++;
            }
            if (numbers && *n != '.') {
                numbers = 0;
            }
        }
        if ((*d == LIST_BULLET) || numbers) {
            debugPrint("\r[List indent %d]", count);

            if (*d == LIST_BULLET) {
                *d = '-';
            }
            for (int indents = 0; indents < count; indents++) {
                strncpy(output + *len, SPACES, MARKDOWN_INDENT);
                (*len) += MARKDOWN_INDENT;
            }

            //skip over the indent from the source data
            *pos = curPos;
            inList = true;
            column++;
        }
    }
    return inList;
}

bool codeBlockCheck(char *data, long *pos, TextData *td, char *output, long *len) {
    if ((inCodeBlock == false) &&
        ((td->bfFontID == courier) || (td->bfFontID == SISMONO)) && (td->backColor == 0)) {
        //check to see if text ends with a newline. if so then it is a codeBlock otherwise
        //we treat it as code.
        if (data[td->textStreamLen-1] = '\r') {
            debugPrint("\r[+CodeBlock]");
            *len += sprintf(output + *len, "```");
            newLine("codeBlockCheck", data, pos, output, len, td->textStreamLen);
            inCodeBlock = true;
        } else {
            debugPrint("[%cCode]", inCode ? '-'  : '+');
            output[*len] =  '`';
            (*len)++;
            inCode = !inCode;
        }
    } else if ((td->bfFontID != courier) && (td->bfFontID != SISMONO) && (inCodeBlock == true)) {
        //end of the codeBlock
        debugPrint("\r[-CodeBlock]");
        *len += sprintf(output + *len, "```");
        newLine("codeBlockCheck", data, pos, output, len, td->textStreamLen);
        inCodeBlock = false;
    }

    return inCodeBlock;
}

void codeCheck(TextData *td, char *output, long *len) {
    if ((((td->bfFontID  == courier) || (td->bfFontID == SISMONO)) 
         && (td->backColor == 0)) || inCode) {
        debugPrint("[%cCode]", inCode ? '-'  : '+');

        output[*len] =  '`';
        (*len)++;
        inCode = !inCode;
    }
}

bool blockQuoteCheck(char *data, long *pos, TextData *td, char *output, long *len) {
    word count = 0;
    long curPos = *pos;
    char *d = data + curPos;

    if (td->backColor == 0xEEEE) {
        while ((strncmp(d, SPACES, getExportIndent()) == 0) || (*d == '\t')) {
            if (*d == '\t') {
                curPos++;
            } else {
                curPos += getExportIndent();
            }
            count++;
            d = data + curPos;
        }

        //check for a blank line inside a quote block
        if ((curPos == *pos) && (*d == '\r')) {
            count++;
        }

        if (count) {
            debugPrint("[blockQuote index %d]", count);
            for (int x = 0; x < count; x++) {
                output[*len] = '>';
                (*len)++;
            }
            column++;
            output[*len] = ' ';
            (*len)++;
            *pos = curPos;
        }
    }
    return (count != 0);
}

word writeMarkdown(word refNum, char *data, TextData *td) {
    Handle outHndl = NewHandle(OUT_ALLOC_BUF_SIZE, userID, attrFixed + attrLocked, NULL);
    char *output = *outHndl;
    long len = 0;
    long pos;
    long outBufSize = OUT_ALLOC_BUF_SIZE;
    word result = 0;
    bool firstPass = true;
    char *d = NULL;
    long startOffset = skipOffset;

    if (skipOffset > td->textStreamLen) {
        skipOffset -= td->textStreamLen;
    } else {
        skipOffset = 0;
    }

    if (column != 0) {
        codeCheck(td, output, &len);
    }
    for (pos = startOffset; pos < td->textStreamLen; pos++) {
        d = data + pos;
        if (outBufSize - pos < 50) {
            //resize the handle since we are getting close to the end
            HUnlock(outHndl);
            SetHandleSize(outBufSize + OUT_ALLOC_BUF_SIZE, outHndl);
            if (toolerror()) {
                word err = toolerror();
                DisposeHandle(outHndl);
                return err;
            }
            outBufSize = GetHandleSize(outHndl);
            HLock(outHndl);
            output = *outHndl;
        }

        if (column == 0) {
            if (!codeBlockCheck(data, &pos, td, output, &len)) {
                if (blockQuoteCheck(data, &pos, td, output, &len)) {
                    if (pos > td->textStreamLen) {
                        skipOffset = pos - td->textStreamLen;
                        continue;
                    }
                }

                if (!(inHeading = headerCheck(data[pos], td, output, &len))) {
                    if (horizontalRuleCheck(data, &pos, output, &len, outBufSize)) {
                        pos--;
                        continue;
                    }
                    if (listCheck(data, &pos, output, &len, outBufSize)) {
                        if (pos > td->textStreamLen) {
                            skipOffset = pos - td->textStreamLen;
                            continue;
                        }
                    }
                }
            } 
        } 
        if (pos >= td->textStreamLen) {
            continue;
        }
        if (strchr("\r\n", data[pos])) {
            //clear the font style after each newline
            firstPass = true;
            setExportStyle(0, output, &len);
            // if we are in a code and we hit a newline then we need to stop the inline 
            // code and continue it after the newline....might get a little messy if
            if (inCode) {
                output[len++] = '`';
                debugPrint("[Code end]");
            }
            newLine("writeMarkdown", data, &pos, output, &len, outBufSize);
            if (inCode) {
                output[len++] = '`';
                debugPrint("\r[Code start]");                
            }
        } else if (!isprint(data[pos])) {
            insertEntity(data + pos, &pos, output, &len);
            //in case insertEntity moved the handle. Deref it again.
            output = *outHndl;
            outBufSize = GetHandleSize(outHndl);
            column++;
        } else {
            if (firstPass  && !inHeading && !inCode && !inCodeBlock) {
                setExportStyle(td->fontStyle, output, &len);
                firstPass = false;
            }
            output[len] = data[pos];
            debugPrint("%c", data[pos]);
            len++;
            column++;
        }
    }
    if ((data[pos-1] = '\r') && inCode) {
        output[len++] = '`';
        inCode = false;
    }
    if (len) {
        output[len] = 0;
        result = writeData(refNum, output);
    }

    DisposeHandle(outHndl);

    return result;
}
