#pragma noroot

#include <ctype.h>
#include <gsos.h>
#include <orca.h>
#include <string.h>
#include "babelfish.h"

// Forward Declarations
word stage2(GSString255Ptr pathname);
int strncasecmp(const char *s1, const char *s2, size_t n);

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static const unsigned char charmap[] = {
        '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
        '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
        '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
        '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
        '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
        '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
        '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
        '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
        '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
        '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
        '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
        '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
        '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
        '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
        '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
        '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
        '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
        '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
        '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
        '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

 
#pragma toolparms 1
#pragma databank 1
pascal word Filter(DirEntryRecGS *entry) {

    if ((long) entry & bffFilterStage2) {
        GSString255Ptr path;
        ResultBuf255Ptr buf;
        entry = (DirEntryRecGS *) ((long)entry & ~bffFilterStage2);
        buf = entry->name;
        path = (GSString255Ptr) &buf->bufString;
        return stage2(path);
    } else {
        if (entry && (entry->eof != 0)) {
            GSString255Ptr path;
            ResultBuf255Ptr buf = entry->name;
            path = (GSString255Ptr) &buf->bufString;
            //if the file end with .md then make the markdown translator more favorable
            if ((path->length > 3) && (strncasecmp(&path->text[path->length-5], ".md", 3) == 0)) {
                return bffFilterSpecific2;
            }
            //if the file is a text file then tell babelfish to do a deeper dive
            if (entry->fileType == 0x04) {
                return bffFilterSpecific1 | nffFilterCheck2;
            }
        }
    }
    return bffFilterNo;
}
#pragma toolparms 0
#pragma databank 0

word stage2(GSString255Ptr pathname) {
    char data[256] = {0};
    OpenRecGS open = {3, 0, 0, readEnable};
    RefNumRecGS close = {1, 0};
    IORecGS read = {4, 0, 0, 0, 0};
    word score = 0;
    char *match;
    word ret = bffFilterSpecific1;

    open.pathname = pathname;
    OpenGS(&open);
    if (toolerror() == 0) {
        close.refNum = open.refNum;
        read.refNum = open.refNum;
        read.dataBuffer = data;
        read.requestCount = sizeof(data) - 1;
        ReadGS(&read);
        CloseGS(&close);
        if (toolerror() == 0) {
            char *pos = data;
            while ((pos < data + read.transferCount)) {
                //search for markdown special characters
                if (match = strchr("#*_", *pos)) {
                    pos++;
                    score++;
                    //skip over duplicates of the matched character
                    while ((*pos == *match) && (pos < data + read.transferCount)) {
                        pos++;
                    }
                } else {
                    pos++;
                }
            }
        }
    }

    //if we have at least 2 special character then make the markdown translator
    //more preferable in the translation list.
    if (score > 2) {
        ret = bffFilterSpecific2;
    }
    return ret;
}

int strncasecmp(const char *s1, const char *s2, size_t n) {

    unsigned char u1, u2;

    for (; n != 0; --n) {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (charmap[u1] != charmap[u2]) {
            return charmap[u1] - charmap[u2];
        }
        if (u1 == '\0') {
            return 0;
        }
    }
    return 0;
}


