/* ================================================ */
/* Babelfish APW/C header file                      */
/* by Bill Tudor                                    */
/* Copyright (c) 1993-94 Seven Hills Software Corp. */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* Version: 1.0b13                                  */
/* Updated:  7/10/94                                */
/* Heavily Modified 12/08/2019 Chris Vavruska       */
/* ================================================ */

/* NOTE: THIS IS A PRELIMINARY FILE; USE WITH CAUTION
 * ~~~~
 * Comments/Suggestions/etc are welcome.
*/

#ifndef __BABELFISH__
#define __BABELFISH__

#include <types.h>

#define NAME_OF_BABELFISH "\pSeven Hills~Babelfish~"

/* Babelfish Error Codes */
#define bfNoErr            0x0000
#define bfNotStarted       0x5300
#define bfBFBusy           0x5301
#define bfMissingTools     0x5302
#define bfNoTransErr       0x5303
#define bfTransBusy        0x5304
#define bfNotSupported     0x5305
#define bfSupportNotFound  0x5306
#define bfBadUserID        0x5307

/* Babelfish Status Codes */
#define bfContinue     0x0000
#define bfDone         0x8000
#define bfUserAbort    0x8001
#define bfBadFileErr   0x8002
#define bfReadErr      0x8003
#define bfWriteErr     0x8004
#define bfMemErr       0x8005
#define bfAbortErr     0x80FF

/* Babelfish Import/Export Action and Response Codes */
#define bfOK           0x00
#define bfSkipThat     0x80
#define bfContinuing   0x00

/* Text Data */
#define bfContinuing   0x00
#define bfGetSettings  0x01
#define bfFirstHeader  0x02
#define bfOddHeader    0x03
#define bfEvenHeader   0x04
#define bfFirstFooter  0x05
#define bfOddFooter    0x06
#define bfEvenFooter   0x07
#define bfBodyText     0x08
#define bfFootNotes    0x09

/* Sound Data */
#define bfChannel1     0x0101
#define bfChannel2     0x0102
#define bfChannel3     0x0103
#define bfChannel4     0x0104

/* Progress Action Codes */
#define bfNewMercury   0x01
#define bfSetScale     0x02
#define bfNewMessage   0x04
#define bfUserMsg      0x08
#define bfStandardMsg  0x10

/* Translator Request Codes */
#define TrStartUp  0x9101
#define TrShutDown 0x9102
#define TrRead     0x9103
#define TrWrite    0x9104

/* Babelfish Request Codes */
#define BFStartUp      0x9001
#define BFShutDown     0x9002
#define BFVersion      0x9004
#define BFChooseTrans  0x9011
#define BFGetFile      0x9012
#define BFImportThis   0x9013
#define BFRead         0x9014
#define BFPutFile      0x9015
#define BFExportThis   0x9016
#define BFWrite        0x9017
#define BFProgress     0x9018
#define BFMatchKinds   0x9021
#define BFMatchFile    0x9022
#define BFTransNum2Name 0x9023
#define BFTransName2Num 0x9024
#define BFGetTrData    0x9025
#define BFShowInfo     0x9026
#define BFDoOptions    0x9027
#define BFLoadData     0x9028
#define BFSaveData     0x9029
#define BFLoadSupport  0x902A
#define BFUnloadSupport 0x902B

/* Babelfish Flag Bits */
#define bffShowProgress    1
#define bffShowErrors      2
#define bffLoose           0
#define bffStandard        4
#define bffImporting       0
#define bffExporting       8
#define bffSilentMode      16

#define bffNoData          0   /* No Bits Set */
#define bffText            1   /* Bit 0 set   */
#define bffPixelMap        2
#define bffTrueColor       4
#define bffPict            8
#define bffFont           16
#define bffSound          32
#define bffAnimation      64
#define bffPublishing    128

#define bffFilterNo         0
#define bffFilterGeneric1   1 
#define bffFilterGeneric2   2
#define bffFilterSpecific1  3
#define bffFilterSpecific2  4
#define nffFilterCheck2     0x8000
#define bffFilterStage2     0x80000000L

/* MiscFlags Bits*/
#define bfShowProgess       0x0001
#define bfShowErrors        0x0002
#define bfMatching          0x0004
#define bfDirection         0x0008
#define bfSilentMode        0x0010

/* Text Format response Codes*/
#define bfTFReponseOK       0x00
#define bfTFReponseSkip     0x80

/* Babelfish Data Structures */
typedef struct DFlags {
   byte flag1;
   byte flag2;
   byte flag3;
   byte flag4;
   byte flag5;
   byte flag6;
   byte flag7;
   byte flag8;
} DFlags;

typedef struct BFXferRec {
   word    pCount;
   word    status;
   word    miscFlags;
   DFlags  dataKinds;
   word    transNum;
   word    userID;
   word    progressAction;
   word    fullTherm;
   word    currentTherm;
   char    *msgPtr;
   Pointer dataRecordPtr;
   GSString255Ptr    filePathPtr;
   char    *fileNamePtr;
} BFXferRec, *BFXferRecPtr,**BFXferRecHndl;


typedef struct BFDataIn {
   BFXferRecPtr xferRecPtr;
} BFDataIn, *BFDataInPtr, **BFDataInHndl;

typedef struct BFResultOut {
   word recvCount;
   word bfResult;
} BFResultOut, *BFResultOutPtr, **BFResultOutHndl;

// $9001    BFStartUp
// dataIn   BFStartUpInPtr
// dataOut  BFStartUpOutPtr

typedef struct BFStartIn {
   word userID;
} BFStartUpIn, *BFStartUpInPtr, **BFStartUpInHndl;

typedef struct BFStartUpOut {
   word recvCount;
   word bfResult;
   word xferRecSize;
} BFStartUpOut, *BFStartUpOutPtr, **BFStartUpOutHndl;

// $9002    BFShutDown
// dataIn   BFShutDownInPtr
// dataOut  BFShutDownOutPtr

typedef struct BFShutDownIn {
   word userID;
} BFShutDownIn, *BFShutDownInPtr, **BFShutDownInHndl;

typedef struct BFResultOut BFShutDownOut, *BFShutDownOutPtr, **BFShutDownOutHndl;

// $9004    BFVersion
// dataIn   Nil
// dataOut  BFVersionOutPtr

typedef struct BFVersionOut {
   word recvCount;
   word bfResult;
   long version;
} BFVersionOut, *BFVersionOutPtr, **BFVersionOutHndl;

// $9011    BFChooseTrans
// dataIn   BFCustomChooseTransInPtr
// dataOut  BFCustomChooseTransOutPtr

typedef struct BFCustomChooseTrans {
   word pCount;
   char *promptPtr;
} BFCustomChooseTrans, *BFCustomChooseTransPtr, **BFCustomChooseTransHndl;

typedef struct BFChooseTransIn {
   BFXferRecPtr xferRecPtr;
   BFCustomChooseTransPtr customPtr;
} BFChooseTransIn, *BFChooseTransInPtr, **BFChooseTransInHndl;

typedef struct BFResultOut BFCustomChooseTransOut, *BFCustomChooseTransOutPtr, **BFCustomChooseTransOutHndl;

// $9012    BFGetFile
// dataIn   BFGetFileInPtr
// dataOut  BFGetFileOutPtr

typedef struct BFCustomGetFile {
   word pCount;
   char *promptPtr;
   char *importButtonPtr;
} BFCustomGetFile, *BFCustomGetFilePtr, **BFCustomGetFileHndl;

typedef struct BFGetFileIn {
   BFXferRecPtr xferRecPtr;
   BFCustomGetFilePtr customPtr;
   long replyPtr;
} BFGetFileIn, *BFGetFileInPtr, **BFGetFileInHndl;

typedef struct BFResultOut BFGetFileOut, *BFGetFileOutPtr, **BFGetFileOutHndl;

// $9013    BFImportThis
// dataIn   BFImportThisInPtr
// dataOut  BFImportThisOutPtr

typedef struct BFDataIn BFImportThisIn, *BFImportThisInPtr, **BFImportThisInHndl;
typedef struct BFResultOut BFImportThisOut, *BFImportThisOutPtr, **BFImportThisOutHndl;

// $9014    BFRead
// dataIn   BFReadInPtr
// dataOut  BFReadOutPtr

typedef struct BFDataIn BFReadIn, *BFReadInPtr, **BFReadInHndl;
typedef struct BFResultOut BFReadOut, *BFReadOutPtr, **BFReadOutHndl;

// $9015    BFPutFile
// dataIn   BFCustomPutFilePtr
// dataOut  BFPutFileOutPtr

typedef struct BFCustomPutFile {
   word pCount;
   char *promptPtr;
   char *exportButtonPtr;
   GSString255Ptr importButtonPtr;
} BFCustomPutFile, *BFCustomPutFilePtr, **BFCustomPutFileHndl;

typedef struct BFPutFileIn {
   BFXferRecPtr xferRecPtr;
   BFCustomPutFilePtr customPtr;
   long replyPtr;
} BFPutFileIn, *BFPutFileInPtr, **BFPutFileInHndl;

typedef struct BFResultOut BFPutFileOut, *BFPutFileOutPtr, **BFPutFileOutHndl;

// $9016    BFExportThis
// dataIn   BFExportThisInPtr
// dataOut  BFExportThisOutPtr

typedef struct BFDataIn BFExportThisIn, *BFExportThisInPtr, **BFExportThisnHndl;
typedef struct BFResultOut BFExportThisOut, *BFExportThisOutPtr, **BFExportThisOutHndl;

// $9017    BFWrite
// dataIn   BFWriteInPtr
// dataOut  BFWriteOutPtr

typedef struct BFDataIn BFWriteIn, *BFWriteInPtr, **BFWriteInHndl;
typedef struct BFResultOut BFWriteOut, *BFWriteOutPtr, **BFWriteOutHndl;

// $9018    BFProgress
// dataIn   BFProgressInPtr
// dataOut  BFProgressOutPtr

typedef struct BFDataIn BFProgressIn, *BFProgressInPtr, **BFProgressInHndl;
typedef struct BFResultOut BFProgressOut, *BFProgressOutPtr, **BFProgressOutHndl;

// $9021    BFMatchKinds
// dataIn   BFMatchKindsInPtr
// dataOut  BFMatchKindsOutPtr

typedef struct BFDataIn BFMatchKindsIn, *BFMatchKindsInPtr, **BFMatchKindsInHndl;

typedef struct BFTransListKinds {
    word transCount;
    word transArray[1];
} BFTransListKinds, *BFTransListKindsPtr, **BFTransListKindsHndl;

typedef struct BFMatchKindsOut {
   word recvCount;
   word bfResult;
   BFTransListKindsHndl transListHndl;
} BFMatchKindsOut, *BFMatchKindsOutPtr, **BFMatchKindsOutHndl;

// $9022    BFMatchFile
// dataIn   BFMatchFileInPtr
// dataOut  BFMatchFileOutPtr

typedef struct BFDataIn BFMatchFileIn, *BFMatchFileInPtr, **BFMatchFileInHndl;

typedef struct BFTransNumArray {
    word transNum;
    word ranking;
} BFTransNumArray;

typedef struct BFTransListNum {
    word transCount;
    BFTransNumArray transArray[1];
} BFTransListNum, *BFTransListNumPtr, **BFTransListNumHndl;

typedef struct BFMatchFileOut {
   word recvCount;
   word bfResult;
   BFTransListNumHndl transListHndl;
} BFMatchFileOut, *BFMatchFileOutPtr, **BFMatchFileOutHndl;

// $9023    BFTransNum2Name
// dataIn   BFTransNum2NameInPtr
// dataOut  BFTransNum2NameOutPtr

typedef struct BFDataIn BFTransNum2NameIn, *BFTransNum2NameInPtr, **BFTransNum2NameInHndl;

typedef struct BFTransNum2NameOut {
    word recvCount;
    word bfResult;
    Handle trNameHndl;
} BFTransNum2NameOut, *BFTransNum2NameOutPtr, **BFTransNum2NameOutHndl;

// $9024    BFTransName2Num
// dataIn   BFTransName2NumInPtr
// dataOut  BFTransName2NumOutPtr

typedef struct BFTransName2NumIn {
   BFXferRecPtr xferRecPtr;
   char *namePtr;
} BFTransName2NumIn, *BFTransName2NumInPtr, **BFTransName2NumInHndl;

typedef struct BFResultOut BFTransName2NumOut, *BFTransName2NumOutPtr, **BFTransName2NumOutHndl;

// $9025    BFGetTrData
// dataIn   BFGetTRDataInPtr
// dataOut  BFGetTRDataOutPtr

typedef struct BFDataIn BFGetTRDataIn, *BFGetTRDataInPtr, **BFGetTRDataInHndl;

typedef struct TrData {
    word trRevision;
    word trGeneralFlags;
    word trImportFlags;
    char trImportKinds[8];
    word trExportFlags;
    char trExportKinds[8];
} TrData, *TrDataPtr, **TrDataHndl;

typedef struct BFGetTRDataOut {
    word    recvCount;
    word    bfResult;
    TrDataHndl  trDataHndl; 
} BFGetTRDataOut, *BFGetTRDataOutPtr, **BFGetTRDataOutHndl;

// $9026    BFShowInfo
// dataIn   BFShowInfoInPtr
// dataOut  BFShowInfoOutPtr

typedef struct BFDataIn BFShowInfoIn, *BFShowInfoInPtr, **BFShowInfoInHndl;
typedef struct BFResultOut BFShowInfoOut, *BFShowInfoOutPtr, **BFShowInfoOutHndl;

// $9027    BFDoOptions
// dataIn   BFDoOptionsInPtr
// dataOut  BFDoOptionsOutPtr

typedef struct BFDataIn BFDoOptionsIn, *BFDoOptionsInPtr, **BFDoOptionsInHndl;
typedef struct BFResultOut BFDoOptionsOut, *BFDoOptionsOutPtr, **BFDoOptionsOutHndl;

// $9028    BFLoadData
// dataIn   BFLoadDataInPtr
// dataOut  BFLoadDataOutPtr

typedef struct BFLoadDataIn {
   word userID;
} BFLoadDataIn, *BFLoadDataInPtr, **BFLoadDataInHndl;

typedef struct BFLoadDataOut {
   word recvCount;
   word bfResult;
   Handle dataHandle;
} BFLoadDataOut, *BFLoadDataOutPtr, **BFLoadDataOutHndl;

// $9029    BFSaveData
// dataIn   BFSaveDataInPtr
// dataOut  BFSaveDataOutPtr

typedef struct BFSaveDataIn {
   word userID;
   Handle dataHandle;
} BFSaveDataIn, *BFSaveDataInPtr, **BFSaveDataInHndl;

typedef struct BFResultOut BFSaveDataOut, *BFSaveDataOutOutPtr, **BFSaveDataOutOutHndl;

// $902A    BFLoadSupport
// dataIn   BFLoadSupportInPtr
// dataOut  BFLoadSupportOutPtr

typedef struct BFLoadSupportIn {
   char    *namePtr;
} BFLoadSupportIn, *BFLoadSupportInPtr, **BFLoadSupportInHndl;

typedef struct BFLoadSupportOut {
   word recvCount;
   word bfResult;
   word SupportID;
} BFLoadSupportOut, *BFLoadSupportOutPtr, **BFLoadSupportOutHndl;

// $902B    BFUnloadSupport
// dataIn   BFUnloadSupportInPtr
// dataOut  BFUnloadSupportOutPtr

typedef struct BFnloadSupportIn {
   char    *namePtr;
} BFUnloadSupportIn, *BFUnloadSupportInPtr, **BFUnloadSupportInHndl;


/* Translator Data Structures */

/* Text Data Record (Format 1) */
struct TextData {
   word    pCount;
   word    actionCode;
   word    responseCode;
   long    textStreamLen;
   Pointer textStreamPtr;
   Handle  textStreamHndl;
   word    bfFontID;               /* Cannot use FontID because QD uses it */
   word    fontSize;
   word    fontStyle;
   word    foreColor;
   word    backColor;
   word    postion;
   word    charSpacing;
   word    lineSpacing;
   word    spaceBefore;
   word    spaceAfter;
   word    firstIndent;
   word    leftIndent;
   word    rightIndent;
   word    tabs[32];
   word    options;
   long    border;
   word    pageLength;
   word    pageWidth;
   Rect    rect;
   word    columns;
   word    gutter;
   Handle  pictHandle;
};
typedef struct TextData TextData, *TextDataPtr, **TextDataHndl;

/* Pixel Map Data Record (Format 2) */
struct PixelData {
   word    ParmCount;              /* Must be 8                            */
   long    LocInfoRecPtr;          /* Pointer to LocInfo Record            */
   long    SCBsArrayPtr;           /* Pointer to SCB Array                 */
   long    PalettesPtr;            /* Pointer to array of color pallettes  */
   long    ImageHnd;               /* Handle to image data                 */
   long    SCBsArrayHnd;           /* Handle to SCB Array                  */
   long    PalettesHnd;            /* Handle to array of color pallettes   */
   long    ResolutionH;            /* Horizontal resolution                */
   long    ResolutionV;            /* Vertical resolution                  */
};
typedef struct PixelData PixelData, *PixelDataPtr, **PixelDataHndl;

/* Quickdraw II Picture Data Record (Format 4) */
struct PictData {
   word    ParmCount;              /* Must be 3                            */
   long    PicHandle;              /* Handle to Quickdraw II Picture       */
   long    ResolutionH;            /* Horizontal resolution                */
   long    ResolutionV;            /* Vertical resolution                  */
};
typedef struct PictData PictData, *PictDataPtr, **PictDataHndl;

/* Font Data Record (Format 5) */
struct FontData {
   word    ParmCount;              /* Must be 2                            */
   long    FamilyName;             /* Pointer to pString name              */
   long    FontDataHandle;         /* Handle to IIgs Font Data             */
};
typedef struct FontData FontData, *FontDataPtr, **FontDataHndl;

/* Sound Data Record (Format 6) */
struct SoundData {
   word    ParmCount;              /* 7, 8, 9, or 10                       */
   word    ActionCode;             /* 1 - Sender order for rec             */
   word    ResponseCode;           /* 2 - Receiver response                */
   long    SoundDataLength;        /* 3 - Sound Data length                */
   long    SoundDataPointer;       /* 4 - Sound Data pointer               */
   long    SoundDataHandle;        /* 5 - Sound Data Handle (or 0)         */
   long    channelLength;          /* 6 - Total length of this channel     */
   word    frequency;              /* 7 - Frequency in Hz (0-65535Hz)      */
   word    volume;                 /* 8 - Volume (1-15 or 0 for max        */
   word    relPitch;               /* 9 - relative pitch (from rSound)     */
                                   /*     OR zero if "none/not provided"   */
   char    name[34];               /*10 - pString name (zero length=none   */
};
typedef struct SoundData SoundData, *SoundDataPtr, **SoundDataHndl;
#endif
