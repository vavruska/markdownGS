/*
 *  importer.c
 *
 * Based on the work that Jeremy Rand did for his md2teach utility. Only
 * a few modifications made for my babelfish translator.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <types.h>
#include <gsos.h>
#include <loader.h>
#include <locator.h>
#include <memory.h>
#include <orca.h>
#include <resources.h>

#include "babelfish.h"

#include "md4c.h"
#include "dispatch.h"
#include "translate.h"

#pragma noroot

// Typedefs

typedef struct tEntity
{
    const char * entityString;
    char entityChar;
    uint32_t unicodeChar;
} tEntity;

typedef struct tBlockListItem
{
    MD_BLOCKTYPE type;
    union {
        MD_BLOCK_UL_DETAIL ulDetail;
        MD_BLOCK_OL_DETAIL olDetail;
        MD_BLOCK_H_DETAIL hDetail;
        MD_BLOCK_CODE_DETAIL codeDetail;
    } u;
    int numTabs;
    tStyleType styleType;

    struct tBlockListItem **next;
} tBlockListItem;

typedef long (*PARSER_CALLBACK)(const MD_CHAR* text, MD_SIZE size, const MD_PARSER* parser, void* userdata);

typedef struct {
    Word pCount;
    Word refNum;
    Word access;
    ResultBuf255Ptr pathname;
} PathInfo;

// Forward declarations

static int enterBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata);
static int leaveBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata);
static int enterSpanHook(MD_SPANTYPE type, void * detail, void * userdata);
static int leaveSpanHook(MD_SPANTYPE type, void * detail, void * userdata);
static int textHook(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE size, void * userdata);
static void debugLogHook(const char * message, void * userdata);

// Externs
extern word userID;
extern long size;

// Globals
word debugIndentLevel = 0;

//statics
static long lastTherm = 0;

static MD_PARSER parser = {
    0, // abi_version
    MD_FLAG_NOHTMLBLOCKS | MD_FLAG_NOHTMLSPANS, // flags
    enterBlockHook,
    leaveBlockHook,
    enterSpanHook,
    leaveSpanHook,
    textHook,
    debugLogHook,
    NULL // syntax
};

static tBlockListItem ** blockList = NULL;

static uint16_t textStyleMask = STYLE_TEXT_PLAIN;

static tEntity entities[] = {
    { "&Tab;", 0x9, 0x9 },
    { "&NewLine;", 0x13, 0x10 },
    { "&excl;", 0x21, 0x21 },
    { "&quot;", 0x22, 0x22 },
    { "&QUOT;", 0x22, 0x22 },
    { "&num;", 0x23, 0x23 },
    { "&dollar;", 0x24, 0x24 },
    { "&percnt;", 0x25, 0x25 },
    { "&amp;", 0x26, 0x26 },
    { "&apos;", 0x27, 0x27 },
    { "&lpar;", 0x28, 0x28 },
    { "&rpar;", 0x29, 0x29 },
    { "&ast;", 0x2a, 0x2a },
    { "&midast;", 0x2a, 0x2a },
    { "&plus;", 0x2b, 0x2b },
    { "&comma;", 0x2c, 0x2c },
    { "&period;", 0x2e, 0x2e },
    { "&sol;", 0x2f, 0x2f },
    { "&colon;", 0x3a, 0x3a },
    { "&semi;", 0x3b, 0x3b },
    { "&lt;", 0x3c, 0x3c },
    { "&LT;", 0x3c, 0x3c },
    { "&equals;", 0x3d, 0x3d },
    { "&gt;", 0x3e, 0x3e },
    { "&GT;", 0x3e, 0x3e },
    { "&quest;", 0x3f, 0x3f },
    { "&commat;", 0x40, 0x40 },
    { "&lsqb;", 0x5b, 0x5b },
    { "&lbrack;", 0x5b, 0x5b },
    { "&bsol;", 0x5c, 0x5c },
    { "&rsqb;", 0x5d, 0x5d },
    { "&rbrack;", 0x5d, 0x5d },
    { "&Hat;", 0x5e, 0x5e },
    { "&lowbar;", 0x5f, 0x5f },
    { "&grave;", 0x60, 0x60 },
    { "&DiacriticalGrave;", 0x60, 0x60 },
    { "&lcub;", 0x7b, 0x7b },
    { "&lbrace;", 0x7b, 0x7b },
    { "&verbar;", 0x7c, 0x7c },
    { "&vert;", 0x7c, 0x7c },
    { "&VerticalLine;", 0x7c, 0x7c },
    { "&rcub;", 0x7d, 0x7d },
    { "&rbrace;", 0x7d, 0x7d },
    { "&nbsp;", 0xca, 0xa0 },
    { "&NonBreakingSpace;", 0xca, 0xa0 },
    { "&iexcl;", 0xc1, 0xa1 },
    { "&cent;", 0xa2, 0xa2 },
    { "&pound;", 0xa3, 0xa3 },
    { "&curren;", 0xdb, 0xa4 },
    { "&yen;", 0xb4, 0xa5 },
    { "&sect;", 0xa4, 0xa7 },
    { "&Dot;", 0xac, 0xa8 },
    { "&die;", 0xac, 0xa8 },
    { "&DoubleDot;", 0xac, 0xa8 },
    { "&uml;", 0xac, 0xa8 },
    { "&copy;", 0xa9, 0xa9 },
    { "&COPY;", 0xa9, 0xa9 },
    { "&ordf;", 0xbb, 0xaa },
    { "&laquo;", 0xc7, 0xab },
    { "&not;", 0xc2, 0xac },
    { "&reg;", 0xa8, 0xae },
    { "&circleR;", 0xa8, 0xae },
    { "&REG;", 0xa8, 0xae },
    { "&macr;", 0xf8, 0xaf },
    { "&OverBar;", 0xf8, 0xaf },
    { "&strns;", 0xf8, 0xaf },
    { "&deg;", 0xa1, 0xb0 },
    { "&plusmn;", 0xb1, 0xb1 },
    { "&pm;", 0xb1, 0xb1 },
    { "&PlusMinus;", 0xb1, 0xb1 },
    { "&acute;", 0xab, 0xb4 },
    { "&DiacriticalAcute;", 0xab, 0xb4 },
    { "&micro;", 0xb5, 0xb5 },
    { "&para;", 0xa6, 0xb6 },
    { "&middot;", 0xe1, 0xb7 },
    { "&centerdot;", 0xe1, 0xb7 },
    { "&CenterDot;", 0xe1, 0xb7 },
    { "&cedil;", 0xfc, 0xb8 },
    { "&Cedilla;", 0xfc, 0xb8 },
    { "&ordm;", 0xbc, 0xba },
    { "&raquo;", 0xc8, 0xbb },
    { "&iquest;", 0xc0, 0xbf },
    { "&Agrave;", 0xcb, 0xc0 },
    { "&Aacute;", 0xe7, 0xc1 },
    { "&Acirc;", 0xe5, 0xc2 },
    { "&Atilde;", 0xcc, 0xc3 },
    { "&Auml;", 0x80, 0xc4 },
    { "&Aring;", 0x81, 0xc5 },
    { "&AElig;", 0xae, 0xc6 },
    { "&Ccedil;", 0x82, 0xc7 },
    { "&Egrave;", 0xe9, 0xc8 },
    { "&Eacute;", 0x83, 0xc9 },
    { "&Ecirc;", 0xe6, 0xca },
    { "&Euml;", 0xe8, 0xcb },
    { "&Igrave;", 0xed, 0xcc },
    { "&Iacute;", 0xea, 0xcd },
    { "&Icirc;", 0xeb, 0xce },
    { "&Iuml;", 0xec, 0xcf },
    { "&Ntilde;", 0x84, 0xd1 },
    { "&Ograve;", 0xf1, 0xd2 },
    { "&Oacute;", 0xee, 0xd3 },
    { "&Ocirc;", 0xef, 0xd4 },
    { "&Otilde;", 0xcd, 0xd5 },
    { "&Ouml;", 0x85, 0xd6 },
    { "&Oslash;", 0xaf, 0xd8 },
    { "&Ugrave;", 0xf4, 0xd9 },
    { "&Uacute;", 0xf2, 0xda },
    { "&Ucirc;", 0xf3, 0xdb },
    { "&Uuml;", 0x86, 0xdc },
    { "&szlig;", 0xa7, 0xdf },
    { "&agrave;", 0x88, 0xe0 },
    { "&aacute;", 0x87, 0xe1 },
    { "&acirc;", 0x89, 0xe2 },
    { "&atilde;", 0x8b, 0xe3 },
    { "&auml;", 0x8a, 0xe4 },
    { "&aring;", 0x8c, 0xe5 },
    { "&aelig;", 0xbe, 0xe6 },
    { "&ccedil;", 0x8d, 0xe7 },
    { "&egrave;", 0x8f, 0xe8 },
    { "&eacute;", 0x8e, 0xe9 },
    { "&ecirc;", 0x90, 0xea },
    { "&euml;", 0x91, 0xeb },
    { "&igrave;", 0x93, 0xec },
    { "&iacute;", 0x92, 0xed },
    { "&icirc;", 0x94, 0xee },
    { "&iuml;", 0x95, 0xef },
    { "&ntilde;", 0x96, 0xf1 },
    { "&ograve;", 0x98, 0xf2 },
    { "&oacute;", 0x97, 0xf3 },
    { "&ocirc;", 0x99, 0xf4 },
    { "&otilde;", 0x9b, 0xf5 },
    { "&ouml;", 0x9a, 0xf6 },
    { "&divide;", 0xd6, 0xf7 },
    { "&div;", 0xd6, 0xf7 },
    { "&oslash;", 0xbf, 0xf8 },
    { "&ugrave;", 0x9d, 0xf9 },
    { "&uacute;", 0x9c, 0xfa },
    { "&ucirc;", 0x9e, 0xfb },
    { "&uuml;", 0x9f, 0xfc },
    { "&yuml;", 0xd8, 0xff },
    { "&dagger;", 0xa0, 0x2020 },
    { "&bull;", 0xa5, 0x2022 },
    { "&bullet;", 0xa5, 0x2022 },
    { "&trade;", 0xaa, 0x2122 },
    { "&TRADE;", 0xaa, 0x2122 },
    { "&ne;", 0xad, 0x2260 },
    { "&NotEqual;", 0xad, 0x2260 },
    { "&infin;", 0xb0, 0x221e },
    { "&le;", 0xb2, 0x2264 },
    { "&leq;", 0xb2, 0x2264 },
    { "&LessEqual;", 0xb2, 0x2264 },
    { "&ge;", 0xb3, 0x2265 },
    { "&geq;", 0xb3, 0x2265 },
    { "&GreaterEqual;", 0xb3, 0x2265 },
    { "&part;", 0xb6, 0x2202 },
    { "&PartialD;", 0xb6, 0x2202 },
    { "&sum;", 0xb7, 0x2211 },
    { "&Sum;", 0xb7, 0x2211 },
    { "&prod;", 0xb8, 0x220f },
    { "&Product;", 0xb8, 0x220f },
    { "&pi;", 0xb9, 0x3c0 },
    { "&int;", 0xba, 0x222b },
    { "&Integral;", 0xba, 0x222b },
    { "&Omega;", 0xbd, 0x3a9 },
    { "&radic;", 0xc3, 0x221a },
    { "&Sqrt;", 0xc3, 0x221a },
    { "&fnof;", 0xc4, 0x192 },
    { "&asymp;", 0xc5, 0x2248 },
    { "&ap;", 0xc5, 0x2248 },
    { "&TildeTilde;", 0xc5, 0x2248 },
    { "&approx;", 0xc5, 0x2248 },
    { "&thkap;", 0xc5, 0x2248 },
    { "&thickapprox;", 0xc5, 0x2248 },
    { "&Delta;", 0xc6, 0x394 },
    { "&hellip;", 0xc9, 0x2026 },
    { "&mldr;", 0xc9, 0x2026 },
    { "&OElig;", 0xce, 0x152 },
    { "&oelig;", 0xcf, 0x153 },
    { "&ndash;", 0xd0, 0x2013 },
    { "&mdash;", 0xd1, 0x2014 },
    { "&ldquo;", 0xd2, 0x201c },
    { "&OpenCurlyDoubleQuote;", 0xd2, 0x201c },
    { "&rdquo;", 0xd3, 0x201d },
    { "&rdquor;", 0xd3, 0x201d },
    { "&CloseCurlyDoubleQuote;", 0xd3, 0x201d },
    { "&lsquo;", 0xd4, 0x2018 },
    { "&OpenCurlyQuote;", 0xd4, 0x2018 },
    { "&rsquo;", 0xd5, 0x2019 },
    { "&rsquor;", 0xd5, 0x2019 },
    { "&CloseCurlyQuote;", 0xd5, 0x2019 },
    { "&loz;", 0xd7, 0x25ca },
    { "&lozenge;", 0xd7, 0x25ca },
    { "&Yuml;", 0xd9, 0x178 },
    { "&frasl;", 0xda, 0x2044 },
    { "&lsaquo;", 0xdc, 0x2039 },
    { "&rsaquo;", 0xdd, 0x203a },
    { "&filig;", 0xde, 0xfb01 },
    { "&fllig;", 0xdf, 0xfb02 },
    { "&Dagger;", 0xe0, 0x2021 },
    { "&ddagger;", 0xe0, 0x2021 },
    { "&lsquor;", 0xe2, 0x201a },
    { "&sbquo;", 0xe2, 0x201a },
    { "&ldquor;", 0xe3, 0x201e },
    { "&bdquo;", 0xe3, 0x201e },
    { "&permil;", 0xe4, 0x2030 },
    { "", 0xf0, 0xf8ff },
    { "&imath;", 0xf5, 0x131 },
    { "&inodot;", 0xf5, 0x131 },
    { "&circ;", 0xf6, 0x2c6 },
    { "&tilde;", 0xf7, 0x2dc },
    { "&DiacriticalTilde;", 0xf7, 0x2dc },
    { "&breve;", 0xf9, 0x2d8 },
    { "&Breve;", 0xf9, 0x2d8 },
    { "&dot;", 0xfa, 0x2d9 },
    { "&DiacriticalDot;", 0xfa, 0x2d9 },
    { "&ring;", 0xfb, 0x2da },
    { "&dblac;", 0xfd, 0x2dd },
    { "&DiacriticalDoubleAcute;", 0xfd, 0x2dd },
    { "&ogon;", 0xfe, 0x2db },
    { "&caron;", 0xff, 0x2c7 },
    { "&Hacek;", 0xff, 0x2c7 },
};

// Implementation

#pragma databank 1
static int enterBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    static int isFirstNonDocumentBlock = 1;
    int shouldInsertCR = 1;
    uint16_t headerSize = 0;
    tBlockListItem ** newBlock = (tBlockListItem **) NewHandle(sizeof(tBlockListItem), userID, attrFixed, NULL);
    
    if (newBlock == NULL) {
        debugPrint("Out of memory");
        return 1;
    }
    
    (*newBlock)->type = type;
    if (blockList == NULL) {
        (*newBlock)->numTabs = 0;
        (*newBlock)->styleType = STYLE_TYPE_TEXT;
    } else {
        (*newBlock)->numTabs = (*blockList)->numTabs;
        if ((*blockList)->styleType == STYLE_TYPE_QUOTE) {
            (*newBlock)->styleType = STYLE_TYPE_QUOTE;
        } else {
            (*newBlock)->styleType = STYLE_TYPE_TEXT;
        }
    }
    (*newBlock)->next = blockList;
    blockList = newBlock;
    
    switch (type) {
        case MD_BLOCK_DOC:
            debugPrint("%*sDOC {\r", debugIndentLevel, "");
            (*newBlock)->styleType = STYLE_TYPE_TEXT;
            shouldInsertCR = 0;
            break;
            
        case MD_BLOCK_QUOTE:
            debugPrint("%*sQUOTE {\r", debugIndentLevel, "");
            (*newBlock)->styleType = STYLE_TYPE_QUOTE;
            shouldInsertCR = 0;
            break;
            
        case MD_BLOCK_UL: {
            MD_BLOCK_UL_DETAIL * ulDetail = (MD_BLOCK_UL_DETAIL *)detail;
            debugPrint("%*sUL (is_tight=%d, mark=%c) {\r", debugIndentLevel, "", ulDetail->is_tight, ulDetail->mark);
            
            memcpy(&((*newBlock)->u.ulDetail), ulDetail, sizeof(*ulDetail));
            (*newBlock)->numTabs++;
            break;
        }
            
        case MD_BLOCK_OL: {
            MD_BLOCK_OL_DETAIL * olDetail = (MD_BLOCK_OL_DETAIL *)detail;
            debugPrint("%*sOL (start=%u, is_tight=%d, mark_delimiter=%c) {\r", debugIndentLevel, "", olDetail->start, olDetail->is_tight, olDetail->mark_delimiter);
            
            memcpy(&((*newBlock)->u.olDetail), olDetail, sizeof(*olDetail));
            (*newBlock)->numTabs++;
            break;
        }
            
        case MD_BLOCK_LI: {
            int i;
            tBlockListItem ** enclosingBlock = (*newBlock)->next;
            
            debugPrint("%*sLI {\r", debugIndentLevel, "");
            
            if (enclosingBlock == NULL) {
                debugPrint("Got a list item block without an enclosing block\r");
                return 1;
            }
            
            if ((*enclosingBlock)->type == MD_BLOCK_OL) {
                shouldInsertCR = !(*enclosingBlock)->u.olDetail.is_tight;
            } else if ((*enclosingBlock)->type == MD_BLOCK_UL) {
                shouldInsertCR = !(*enclosingBlock)->u.ulDetail.is_tight;
            }
            
            break;
        }
            
        case MD_BLOCK_HR:
            debugPrint("%*sHR {\r", debugIndentLevel, "");
            break;
            
        case MD_BLOCK_H: {
            MD_BLOCK_H_DETAIL * hDetail = (MD_BLOCK_H_DETAIL *)detail;
            debugPrint("%*sH (level=%u) {\r", debugIndentLevel, "", hDetail->level);
            
            memcpy(&((*newBlock)->u.hDetail), hDetail, sizeof(*hDetail));
            setStyle(STYLE_TYPE_TEXT, textStyleMask, headerSize);
            if (!isFirstNonDocumentBlock) {
                writeChar('\r');
                size++;
            }
            headerSize = hDetail->level;
            shouldInsertCR = 0;
            (*newBlock)->styleType = STYLE_TYPE_HEADER;
            break;
        }
            
        case MD_BLOCK_CODE: {
            MD_BLOCK_CODE_DETAIL * codeDetail = (MD_BLOCK_CODE_DETAIL *)detail;
            debugPrint("%*sCODE ", debugIndentLevel, "");
            if (codeDetail->fence_char != '\0') {
                debugPrint("(fence_char=%c) ", codeDetail->fence_char);
            }
            debugPrint("{\r");
            
            memcpy(&((*newBlock)->u.codeDetail), codeDetail, sizeof(*codeDetail));
            (*newBlock)->styleType = STYLE_TYPE_CODE;
            break;
        }
            
        case MD_BLOCK_P:
            debugPrint("%*sP {\r", debugIndentLevel, "");
            break;
            
        default:
            debugPrint("Invalid block type (%d)\r", (int)type);
            return 1;
            break;
    }
    
    setStyle((*newBlock)->styleType, textStyleMask, headerSize);
    if ((!isFirstNonDocumentBlock) && (shouldInsertCR)) {
        writeChar('\r');
        size++;
    }
    
    switch (type) {
        case MD_BLOCK_LI: {
            tBlockListItem ** enclosingBlock = (*newBlock)->next;
            static char str[16];

            //allow up to 15 tabs..even that is a bit ridiculous
            writeString("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t", 
                        (*newBlock)->numTabs > 15 ? 15 : (*newBlock)->numTabs);
            size += 15;
            
            if ((*enclosingBlock)->type == MD_BLOCK_OL) {
                sprintf(str, "%u%c ", (*enclosingBlock)->u.olDetail.start, (*enclosingBlock)->u.olDetail.mark_delimiter);
                (*enclosingBlock)->u.olDetail.start++;
            } else {
                sprintf(str, "%c ", 0xa5);    // 0xa5 is a bullet character
            }
            writeString(str, strlen(str));
            size+= strlen(str);
            break;
        }
            
        case MD_BLOCK_HR:             
            writeString("______________________________", 30);
            size += 30;
            break;
            
        case MD_BLOCK_DOC:
        case MD_BLOCK_QUOTE:
        case MD_BLOCK_UL:
        case MD_BLOCK_OL:
        case MD_BLOCK_H:
        case MD_BLOCK_CODE:
        case MD_BLOCK_P:
            break;
            
        default:
            debugPrint("Invalid block type (%d)\r", (int)type);
            return 1;
            break;
    }
        
    if (type != MD_BLOCK_DOC) {
        isFirstNonDocumentBlock = 0;
    }
            
    debugIndentLevel+=2;
    return 0;
}
#pragma databank 0

#pragma databank 1
static int leaveBlockHook(MD_BLOCKTYPE type, void * detail, void * userdata)
{
    tBlockListItem ** oldBlock = blockList;
    
    if (oldBlock == NULL) {
        debugPrint("Block list is empty but leaving block of type %d\r", (int)type);
        return 1;
    }
    
    if ((*oldBlock)->type != type) {
        debugPrint("Expected to leave block of type %d but got type %d\r", (int)(*oldBlock)->type, (int)type);
        return 1;
    }
    
    blockList = (*oldBlock)->next;
    DisposeHandle((Handle) oldBlock);
    
    switch (type) {
        case MD_BLOCK_DOC:
            break;
            
        case MD_BLOCK_QUOTE:
            break;
            
        case MD_BLOCK_UL:
            break;
            
        case MD_BLOCK_OL:
            break;
            
        case MD_BLOCK_LI:
            writeChar('\r');
            size++;
            break;
            
        case MD_BLOCK_HR:
            writeChar('\r');
            size++;
            break;
            
        case MD_BLOCK_H:
            if (blockList != NULL) {
                setStyle((*blockList)->styleType, textStyleMask, 0);
            }
            writeChar('\r');
            size++;
            break;
            
        case MD_BLOCK_CODE:
            break;
            
        case MD_BLOCK_P:
            writeChar('\r');
            size++;
            break;
            
        default:
            debugPrint("Invalid block type (%d)\r", (int)type);
            return 1;
            break;
    }
    
    if (blockList != NULL) {
        setStyle((*blockList)->styleType, textStyleMask, 0);
    }
    
    debugIndentLevel-=2;
    debugPrint("%*s}\r", debugIndentLevel, "");
    
    return 0;
}
#pragma databank 0

#pragma databank 1
static int enterSpanHook(MD_SPANTYPE type, void * detail, void * userdata)
{
    switch (type) {
        case MD_SPAN_EM:
            debugPrint("%*sEM {\r", debugIndentLevel, "");
            
            textStyleMask |= STYLE_TEXT_MASK_EMPHASIZED;
            setStyle((*blockList)->styleType, textStyleMask, (*blockList)->u.hDetail.level);
            break;
            
        case MD_SPAN_STRONG:
            debugPrint("%*sSTRONG {\r", debugIndentLevel, "");
            
            textStyleMask |= STYLE_TEXT_MASK_STRONG;
            setStyle((*blockList)->styleType, textStyleMask, (*blockList)->u.hDetail.level);
            break;
            
        case MD_SPAN_A:
            debugPrint("%*sA {\r", debugIndentLevel, "");
            break;
            
        case MD_SPAN_IMG:
            debugPrint("%*sIMG {\r", debugIndentLevel, "");
            break;
            
        case MD_SPAN_CODE:
            debugPrint("%*sCODE {\r", debugIndentLevel, "");
            setStyle(STYLE_TYPE_CODE, STYLE_TEXT_PLAIN, 0);
            break;
            
        default:
            debugPrint("Invalid span type (%d)\r", (int)type);
            return 1;
            break;
    }
    
    debugIndentLevel+=2;
    return 0;
}
#pragma databank 0

#pragma databank 1
static int leaveSpanHook(MD_SPANTYPE type, void * detail, void * userdata)
{
    switch (type) {
        case MD_SPAN_EM:
            textStyleMask &= ~STYLE_TEXT_MASK_EMPHASIZED;
            setStyle((*blockList)->styleType, textStyleMask, (*blockList)->u.hDetail.level);
            break;
            
        case MD_SPAN_STRONG:
            textStyleMask &= ~STYLE_TEXT_MASK_STRONG;
            setStyle((*blockList)->styleType, textStyleMask, (*blockList)->u.hDetail.level);
            break;
            
        case MD_SPAN_A:
            break;
            
        case MD_SPAN_IMG:
            break;
            
        case MD_SPAN_CODE:
            setStyle((*blockList)->styleType, textStyleMask, (*blockList)->u.hDetail.level);
            break;
            
        default:
            debugPrint("Invalid span type (%d)\r", (int)type);
            return 1;
            break;
    }
    
    debugIndentLevel-=2;
    debugPrint("%*s}\r", debugIndentLevel, "");
    
    return 0;
}
#pragma databank 0

static word printEntity(const MD_CHAR * text, MD_SIZE size)
{
    int entityNum;
    uint32_t unicodeChar = 0;
    
    if (size < 4) {
        return 0;
    }
    
    if (text[0] != '&') {
        return 0;
    }
    
    if (text[size - 1] != ';') {
        return 0;
    }
    
    if (text[1] == '#') {
        char * end;
        unicodeChar = strtoul(text + 2, &end, 10);
        if (end != text + size - 1)
            unicodeChar = 0;
        if ((unicodeChar > 0) &&
            (unicodeChar < 128)) {
            writeChar(unicodeChar);
            return 1;
        }
    }
    
    if (text[1] == 'x') {
        char * end;
        unicodeChar = strtoul(text + 2, &end, 16);
        if (end != text + size - 1) {
            unicodeChar = 0;
        }
        if ((unicodeChar > 0) &&
            (unicodeChar < 128)) {
            writeChar(unicodeChar);
            return 1;
        }
    }
    
    for (entityNum = 0; entityNum < (sizeof(entities) / sizeof(entities[0])); entityNum++) {
        if ((unicodeChar == entities[entityNum].unicodeChar) ||
            (strncmp(entities[entityNum].entityString, text, size) == 0)) {
            writeChar(entities[entityNum].entityChar);
            return 1;
        }
    }
}

#pragma databank 1
static int textHook(MD_TEXTTYPE type, const MD_CHAR * text, MD_SIZE mdSize, void * userdata)
{
    BFProgressIn progressIn;
    BFProgressOut progressOut;
    long stats[2];
    long therm;
    extern BFXferRecPtr globalXfer;

    memcpy(&stats, userdata, sizeof(stats));

    switch (type) {
        case MD_TEXT_NORMAL:
            debugPrint("%*sText: \"", debugIndentLevel, "");
            break;
        
        case MD_TEXT_NULLCHAR:
            debugPrint("Null character encountered on input\r");
            return 1;
            
        case MD_TEXT_BR:
            debugPrint("%*sBR\r", debugIndentLevel, "");
            writeChar('\r');
            size++;
            return 0;
            
        case MD_TEXT_SOFTBR:
            debugPrint("%*sSOFT BR\r", debugIndentLevel, "");
            return 0;
            
        case MD_TEXT_ENTITY:
            debugPrint("%*sEntity: \"", debugIndentLevel, "");
            debugWrite(text, sizeof(MD_CHAR), mdSize);
            
            size += printEntity(text, mdSize);
            text = "";
            mdSize = 0;
            break;
            
        case MD_TEXT_CODE:
            debugPrint("%*sCode: \"", debugIndentLevel, "");
            break;
            
        default:
            debugPrint("Invalid text type (%d)\r", (int)type);
            return 1;
            break;
    }
    
    debugWrite(text, sizeof(MD_CHAR), mdSize);
    debugPrint("\"\r");
    
    size += mdSize;
    if (mdSize > 0) {
        writeString(text, mdSize);
    }

    if (userdata && stats[0] && stats[1]) {
        size = stats[1];
        progressIn.xferRecPtr = globalXfer;
        therm = ((double) stats[0] / (double) stats[1]) * 255L;
        if (therm != lastTherm) {
            lastTherm = therm;
            globalXfer->currentTherm = therm;
            globalXfer->progressAction = bfNewMercury + bfUserMsg;
            SendRequest(BFProgress,                 /* Request  */
                        sendToName + stopAfterOne,    /* Flags    */
                        (long)NAME_OF_BABELFISH,          /* Name     */
                        (long)&progressIn,                     /* Data In  */
                        (ptr)&progressOut);            /* Data Out */
        }
    }

    
    return 0;
}
#pragma databank 0


#pragma databank 1
static void debugLogHook(const char * message, void * userdata)
{
    debugPrint("DEBUG: %s\r", message);
}
#pragma databank 0

void loadParser(long *codeHandle, word *userID) {
    PathInfo pathInfo;
    ResultBuf255 resultBuf;
    InitialLoadOutputRec loadInfo;
    word err;

    resultBuf.bufSize = 255;
    pathInfo.pCount = 3;
    pathInfo.refNum = GetOpenFileRefNum (GetCurResourceFile());
    pathInfo.pathname = &resultBuf;
    GetRefInfoGS(&pathInfo);

    memset(&loadInfo, 0, sizeof(InitialLoadOutputRec));
    loadInfo = InitialLoad2(0x5000, (ptr) &resultBuf.bufString, 1, 1);
    err = toolerror();
    *codeHandle = (long) loadInfo.startAddr;
    *userID = loadInfo.userID;
}

int mdImport(const MD_CHAR* text, MD_SIZE size, bool showProgress) {
    int result = -1;
    long codeHandle;
    word codeID;
    PARSER_CALLBACK mdParser;

    loadParser(&codeHandle, &codeID);
    if (codeHandle) {
        mdParser = (PARSER_CALLBACK)codeHandle;
        // This is a bit of a hack, I am using the "userdata" param to
        // tell md4c to pass offset information to the importer functions.
        result = mdParser(text, size, &parser, (void *) showProgress);
    }

    DisposeAll(codeID);
    
    return result;
}
