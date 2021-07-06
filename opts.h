#ifndef __OPTS_H__
#define __OPTS_H__

#define SIS 36866
#define SISMONO 36866
#define MAX_FACE_NAME 20

typedef struct _version {
    long version;
    word country;
    char pName[14];
    //copyright info after name
} version, *versionPtr, **versionHndl;

typedef struct _opts {
    word version;
    char defaultFace[MAX_FACE_NAME];
    word defaultSize;
    word sisDefaults;
    word indentStyle;
    word headerSize[6];
    word useImportFonts;
    word exportHeaderSize[6];
    word endLineFormat;
    word exportIndent;
    word showExportWindow;
} opts, *optsPtr, **optsHndl;

#define OPTS_VERSION 2
#define OPTS_FACE "\pHelvetica"
// these sizes might be better for the GS
#define OPTS_FONTSIZES {30, 27, 24, 20, 18, 14 }
// original sizes used by md2teach {36, 30, 27, 24, 20, 18}
#define OPTS_SIZE 12
#define OPTS_SIS 0
#define OPTS_INDENTSTYLE 8
#define OPTS_USEIMPORT 1
#define OPTS_LINE_APPLE 1
#define OPTS_LINE_LINUX 2
#define OPTS_LINE_WINDOWS 3
#define OPTS_EXPORT_INDENT 8
#define OPTS_SHOWEXPORTWINDOW 1

#define DEFAULT_OPTS {   \
    OPTS_VERSION,        \
    OPTS_FACE,           \
    OPTS_SIZE,           \
    OPTS_SIS,            \
    OPTS_INDENTSTYLE,    \
    OPTS_FONTSIZES,      \
    OPTS_USEIMPORT,      \
    OPTS_FONTSIZES,      \
    OPTS_LINE_APPLE,     \
    OPTS_EXPORT_INDENT,  \
    OPTS_SHOWEXPORTWINDOW\
    }

void loadOptions(void);
void saveOptions(void);
word getHeaderSize(word size);
void setHeaderSize(word size, word value);
word getTextSize(void);
void setTextSize(word value);
word getUseSis(void);
void setUseSis(word value);
word getIndentStyle(void);
void setIndentStyle(word value);
char *getDefaultFace(void);
void setDefaultFace(char *face);
word getDefaultFaceID(void);
word getMonoFaceID(void);
word getUseImportSizes(void);
void setUseImportSizes(word value);
word getExportHeaderSize(word size);
void setExportHeaderSize(word size, word value);
word getLineEndingFormat(void);
void setLineEndingFormat(word value);
word getExportIndent(void);
void setExportIndent(word value);
word getShowExportWindow(void);
void setShowExportWindow(word value);

#endif
