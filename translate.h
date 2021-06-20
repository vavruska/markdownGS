
#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__


// Defines

#define STYLE_TEXT_PLAIN 0u
#define STYLE_TEXT_MASK_STRONG 1u
#define STYLE_TEXT_MASK_EMPHASIZED 2u


// Typedefs

typedef enum tStyleType {
    STYLE_TYPE_HEADER,
    STYLE_TYPE_TEXT,
    STYLE_TYPE_QUOTE,
    STYLE_TYPE_CODE
} tStyleType;


// API

extern void setStyle(tStyleType styleType, uint16_t textMask, uint16_t headerSize);
void writeChar(char c);
void writeString(char *str, size_t len);
Handle translate(void);

#endif /* define _GUARD_PROJECTmd2teach_FILEstyle_ */
