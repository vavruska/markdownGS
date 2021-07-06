#ifndef __DEFS_H__
#define __DEFS_H__

// Defines

#define STYLE_TEXT_PLAIN 0u
#define STYLE_TEXT_MASK_STRONG 1u
#define STYLE_TEXT_MASK_EMPHASIZED 2u

#define HORIZONTAL_LINE "__________________________________________________________________________"
#define SPACES "                                                                "
#define TABS "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"

#define MARKDOWN_INDENT 4
#define MAX_TABS 8

#define LIST_BULLET 0xa5

#define SISMONO 36866

// Typedefs

typedef enum tStyleType {
    STYLE_TYPE_HEADER,
    STYLE_TYPE_TEXT,
    STYLE_TYPE_QUOTE,
    STYLE_TYPE_CODE
} tStyleType;

typedef struct tEntity
{
    const char * entityString;
    char entityChar;
    long unicodeChar;
} tEntity;

#endif
