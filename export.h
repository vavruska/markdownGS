#ifndef __EXPORT_H__
#define __EXPORT_H__

void trWrite(BFXferRecPtr dataIn, BFResultOutPtr dataOut);
word writeData(word refNum, char *data);
void exportStartup(BFXferRecPtr dataIn, BFResultOutPtr dataOut);
void exportCleanUp(BFXferRecPtr dataIn, BFResultOutPtr dataOut);

#endif
