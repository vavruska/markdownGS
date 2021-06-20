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
    word headerSize[6];
} opts, *optsPtr, **optsHndl;

#define OPTS_VERSION 1
#define OPTS_FACE "\pHelvetica"
#define OPTS_SIZE 12
#define OPTS_SIS 0
#define DEFAULT_OPTS {   \
    OPTS_VERSION,        \
    OPTS_FACE,           \
    OPTS_SIZE,           \
    OPTS_SIS,            \
    {36, 30, 27, 24, 20, 18} \
    }

void loadOptions(void);
void saveOptions(void);
word getHeaderSize(word size);
void setHeaderSize(word size, word value);
word getTextSize(void);
void setTextSize(word value);
word getUseSis(void);
void setUseSis(word value);
char *getDefaultFace(void);
void setDefaultFace(char *face);
word getDefaultFaceID(void);
word getMonoFaceID(void);

#endif
