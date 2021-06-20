#ifndef __DISPATCH_H__
#define __DISPATCH_H__

void createDebugFile(void);
void closeDebugFile(void);
void debugPrint(const char *format, ...);
void debugWrite(char *text, size_t charSize, size_t size);

#endif
