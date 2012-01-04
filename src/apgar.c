#include <string.h>
#include <math.h>

/* algorithm source: http://triggsolutions.com/wiki/wol-apgar-routine */
/* original author: 0x90 */

/* ported to C and modified to work with <8 length passwords */
char *wol_apgar(const char *in)
{
    const char *lookup = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    size_t ilen = strlen(in);
    static char out[9];
    int i;

    for (i = 0; i < 8; i++)
    {
        unsigned char left = in[i];
        unsigned char right = i < ilen ? in[ilen - i] : 0;
        out[i] = lookup[((left & 1) > 0 ? (left << 1) & right : left ^ right) & 63];
    }

    out[8] = '\0';

    return out;
}
