
#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__

extern void setStyle(tStyleType styleType, uint16_t textMask, uint16_t headerSize, uint16_t bgColor);
void writeChar(char c);
void writeString(char *str, size_t len);
Handle translate(void);

#endif
