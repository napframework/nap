/*********************************************************************
 *
 * $Id: ykey.c 24249 2016-04-26 13:03:58Z seb $
 *
 * Implementation of standard key computations
 *
 * - - - - - - - - - License information: - - - - - - - - -
 *
 *  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
 *
 *  Yoctopuce Sarl (hereafter Licensor) grants to you a perpetual
 *  non-exclusive license to use, modify, copy and integrate this
 *  file into your software for the sole purpose of interfacing
 *  with Yoctopuce products.
 *
 *  You may reproduce and distribute copies of this file in
 *  source or object form, as long as the sole purpose of this
 *  code is to interface with Yoctopuce products. You must retain
 *  this notice in the distributed source file.
 *
 *  You should refer to Yoctopuce General Terms and Conditions
 *  for additional information regarding your rights and
 *  obligations.
 *
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 *  WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
 *  EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 *  COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR
 *  SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT
 *  LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
 *  CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
 *  BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
 *  WARRANTY, OR OTHERWISE.
 *
 *********************************************************************/
#define __FILE_ID__  "ydef"
#include "ykey.h"

#ifdef MICROCHIP_API
#include "Yocto/yocto.h"
#include "Yocto/yapi_ext.h"
#define ntohl(dw) swapl(dw)
#else
#include <string.h>
#include "yproto.h"
#endif

static char btohexa_low_high(u8 b)
{
    b >>= 4;
    return (b>9u) ? b+'a'-10 : b+'0';
}

static char btohexa_low_low(u8 b)
{
    b &= 0x0F;
    return (b>9u) ? b+'a'-10 : b+'0';
}

void bin2str(char *to, const u8 *p, u16 len, u8 addnull)
{
    for (; len--; p++) {
    	*to++ = btohexa_low_high(*p);
    	*to++ = btohexa_low_low(*p);
    }
    if(addnull) *to = '\0';
}

#if !defined(MICROCHIP_API) || defined(HTTP_ON_NET)

//#define DEBUG_HTTP_AUTHENTICATION

// compute the ha1 (in binary form)
void ComputeAuthHA1(u8 *ha1, const char *user, const char *pass, const char *realm)
{
    HASH_SUM ctx;

    MD5Initialize(&ctx);
    MD5AddData(&ctx, (u8*)user, YSTRLEN(user));
    MD5AddData(&ctx, (u8*)":", 1);
    MD5AddData(&ctx, (u8*)realm, YSTRLEN(realm));
    MD5AddData(&ctx, (u8*)":", 1);
    MD5AddData(&ctx, (u8*)pass, YSTRLEN(pass));
    MD5Calculate(&ctx, ha1);
#ifdef DEBUG_HTTP_AUTHENTICATION
    {
        char     tmpha[HTTP_AUTH_MD5_STRLEN + 1];
        bin2str(tmpha, ha1, HTTP_AUTH_MD5_SIZE, 1);
        dbglog("Compute HA1 u=%s r=%s p=%s -> %s\n", user, realm, pass, tmpha);
    }
#endif
}

// compute the ha2 (in binary form)
void ComputeAuthHA2(u8 *ha2, const char *method, const char *uri)
{
    HASH_SUM ctx;

    MD5Initialize(&ctx);
    MD5AddData(&ctx, (u8*)method, YSTRLEN(method));
    MD5AddData(&ctx, (u8*)":",  1);
    MD5AddData(&ctx, (u8*)uri,    YSTRLEN(uri));
    MD5Calculate(&ctx, ha2);
#ifdef DEBUG_HTTP_AUTHENTICATION
    {
        char     tmpha[HTTP_AUTH_MD5_STRLEN + 1];
        bin2str(tmpha, ha2, HTTP_AUTH_MD5_SIZE, 1);
        dbglog("Compute HA2 m=%s u=%s -> %s\n", method, uri, tmpha);
    }
#endif
}


// Return stringified MD5 hash for the specified parameters
void ComputeAuthResponse(char *buf, const u8 *ha1, const char *nonce, const char *nc,  const char *cnonce, const u8* ha2)
{
    u8       hash[HTTP_AUTH_MD5_SIZE];
    char     tmpha[HTTP_AUTH_MD5_STRLEN+1];
    HASH_SUM ctx;

    MD5Initialize(&ctx);
    // convert ha1 into str before using it
    bin2str(tmpha, ha1, HTTP_AUTH_MD5_SIZE,1);
    MD5AddData(&ctx, (u8*)tmpha,  HTTP_AUTH_MD5_STRLEN);
    MD5AddData(&ctx, (u8*)":",  1);
    MD5AddData(&ctx, (u8*)nonce,  YSTRLEN(nonce));
    MD5AddData(&ctx, (u8*)":",  1);
    if(nc && cnonce) {
        MD5AddData(&ctx, (u8*)nc,     YSTRLEN(nc));
        MD5AddData(&ctx, (u8*)":",  1);
        MD5AddData(&ctx, (u8*)cnonce, YSTRLEN(cnonce));
        MD5AddData(&ctx, (u8*)":auth:",  6);
    }
    bin2str(tmpha, ha2, HTTP_AUTH_MD5_SIZE,1);
    MD5AddData(&ctx, (u8*)tmpha, HTTP_AUTH_MD5_STRLEN);
    MD5Calculate(&ctx,hash);
    bin2str(buf, hash, HTTP_AUTH_MD5_SIZE,1);
#ifdef DEBUG_HTTP_AUTHENTICATION
    {
        char     tmpha1[HTTP_AUTH_MD5_STRLEN + 1];
        bin2str(tmpha1, ha1, HTTP_AUTH_MD5_SIZE, 1);
        if (nc && cnonce) {
            dbglog("Auth Resp ha1=%s nonce=%s nc=%s cnouce=%s ha2=%s -> %s\n",
                tmpha1, nonce, nc, cnonce, tmpha, buf);
        } else {
            dbglog("Auth Resp ha1=%s nonce=%s (no nc/cnounce) ha2=%s -> %s\n",
                tmpha1, nonce, tmpha, buf);
        }
    }
#endif
}


// Return stringified sha1 hash for the specified parameters
int CheckWSAuth(u32 nonce, const u8 *ha1, const u8 *to_verify, u8 *out)
{
    char     tmpbuff[HTTP_AUTH_MD5_STRLEN + 8 + 1];
    const u8 * sha1;
    int res;

    // convert ha1 into str before using it
    bin2str(tmpbuff, ha1, HTTP_AUTH_MD5_SIZE, 1);
#ifdef DEBUG_HTTP_AUTHENTICATION
    dbglog("ha1=%s\n", tmpbuff);
#endif
    // convert ha1 into str before using it
#ifdef CPU_BIG_ENDIAN
    nonce = INTEL_U32(nonce);
#endif
    bin2str(tmpbuff + HTTP_AUTH_MD5_STRLEN, (u8*) &nonce, 4, 1);
#ifdef DEBUG_HTTP_AUTHENTICATION
    dbglog("full=%s\n", tmpbuff);
#endif
    sha1 = ySHA1(tmpbuff);
    if (out) {
        memcpy(out, sha1, 20);
    }
    if (to_verify == NULL) {
        return 0;
    }
    res = memcmp(sha1, to_verify, 20)==0;
    return res;
}

// Parse a request header, return 0 if a valid WWW-Authenticate header and set args to corresponding fields
// - Request is patched in place to null-terminate each field.
// - If return value is 0, at least method,realm and qop are set to non-NULL value
// - qop is set to an empty string if not specified in thq authenticate header
int yParseWWWAuthenticate(char *replybuf, int replysize, char **method, char **realm, char **qop, char **nonce, char **opaque)
{
    int     pos = 0;
    char    *p=replybuf, *start;

    while(pos < replysize) {
        while(pos < replysize && replybuf[pos] != '\r') pos++;
        if(pos < replysize && replybuf[++pos] == '\n') pos++;
        // new line, look for authorization header (always at least 25 chars)
        if(pos+25 >= replysize) return -1;
        if(YSTRNICMP(replybuf+pos, "WWW-Authenticate:", 17) != 0) continue;
        // header found, keep a pointer to content and make sure it is complete
        pos += 17;
        p = replybuf + pos;
        while(pos < replysize && replybuf[pos] != '\r') pos++;
        break;
    }
    if(pos >= replysize) return -1;
    replybuf[pos] = 0;
    // we now have a full null-terminated authentication header to parse at p
    while(*p == ' ') p++;
    start = p;
    while(*p && *p != ' ') p++;
    // we expect at least a realm after the method name
    if(!*p) return -1;
    // method found, followed by something. Set default realm and qop to an empty string
    *method = start;
    *realm  = replybuf+pos;
    *qop    = replybuf+pos;
    // null-terminate the method name, and proceed to named parameters
    *p++ = 0;
    while(*p) {
        while(*p == ' ' || *p == ',') p++;
        if(!*p) break;
        if(YSTRNICMP(p,"realm=\"",7) == 0) {
            p += 7;
            start = p;
            while(*p && *p != '\"') p++;
            if(!*p) return -1;
            *p++ = 0; // replace quote by NUL
            *realm = start;
        } else if(YSTRNICMP(p,"qop=\"",5) == 0) {
            p += 5;
            start = p;
            while(*p && *p != '\"') p++;
            if(!*p) return -1;
            *p++ = 0; // replace quote by NUL
            *qop = start;
        } else if(YSTRNICMP(p,"nonce=\"",7) == 0) {
            p += 7;
            start = p;
            while(*p && *p != '\"') p++;
            if(!*p) return -1;
            *p++ = 0; // replace quote by NUL
            *nonce = start;
        } else if(YSTRNICMP(p,"opaque=\"",8) == 0) {
            p += 8;
            start = p;
            while(*p && *p != '\"') p++;
            if(!*p) return -1;
            *p++ = 0; // replace quote by NUL
            *opaque = start;
        } else {
            // skip unknown tag
            while(*p && *p != ',') p++;
        }
    }
    // if no non-empty realm has been specified, the authentication header is not valid
    if(!**realm) return -1;

    return 0;
}

extern u32 yapiGetCNonce(u32 nc);

// Write an authorization header in the buffer provided
// method and uri can be provided in the same memory zone as destination if needed
void yDigestAuthorization(char *buf, int bufsize, const char *user, const char *realm, const u8 *ha1,
                          const char *nonce, const char *opaque, u32 *nc, const char *method, const char *uri)
{
    u32     cnonce;
    char    ncbuf[9], cnoncebuf[9];
    u8      ha2[HTTP_AUTH_MD5_SIZE];
    int     len;

    ComputeAuthHA2(ha2, method, uri);
    YSTRCPY(buf, bufsize, "Authorization: Digest username=\"");
    YSTRCAT(buf, bufsize, user);
    YSTRCAT(buf, bufsize, "\", realm=\"");
    YSTRCAT(buf, bufsize, realm);
    YSTRCAT(buf, bufsize, "\", nonce=\"");
    YSTRCAT(buf, bufsize, nonce);
    YSTRCAT(buf, bufsize, "\", uri=\"");
    YSTRCAT(buf, bufsize, uri);
    if(nc) {
        (*nc)++;
        cnonce = yapiGetCNonce(*nc);
        yxtoa(*nc, ncbuf, sizeof(ncbuf)-1);
        yxtoa(cnonce, cnoncebuf, sizeof(cnoncebuf)-1);
        len = (int)strlen(buf);
        buf += len;
        bufsize -= len;
        YSTRCAT(buf, bufsize, "\", qop=auth, nc=");
        YSTRCAT(buf, bufsize, ncbuf);
        YSTRCAT(buf, bufsize, ", cnonce=\"");
        YSTRCAT(buf, bufsize, cnoncebuf);
    }
    YSTRCAT(buf, bufsize, "\", response=\"");
    len = (int)strlen(buf);
    buf += len;
    bufsize -= len;
    ComputeAuthResponse(buf, ha1, nonce, (nc?ncbuf:NULL), (nc?cnoncebuf:NULL), ha2);
    if(opaque) {
        len = (int)strlen(buf);
        buf += len;
        bufsize -= len;
        YSTRCAT(buf, bufsize, "\", opaque=\"");
        YSTRCAT(buf, bufsize, opaque);
    }
    YSTRCAT(buf, bufsize, "\"\r\n");
}

// State variables used during key computation
typedef struct {
    int iter;
    int pos;
    u32 inner[5];
    u32 outer[5];
    u32 shau[5];
    u32 shaw[80];
    u8  res[32];
} WPA_CALC_STATE;

static WPA_CALC_STATE wpak = { -1 };

const u32 sha1_init[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };

static void initshaw(const char *s, u16 ofs, u8 pad, u16 xinit)
{
    int ii, j = -1, k = 0;
    int n = (int)strlen(s);

    for(ii = 0; ii < 64; ii++) {
        int i = ofs + ii;
        u8  c = 0;
        if (i < n) {
            c = s[i];
        } else if (pad) {
            if (pad & 0x80) {
                if (i == n) c = pad;
            } else {
                if (i == n + 3) c = pad;
                else if (i == n + 4) c = 0x80;
            }
        }
        if (k == 0) {
            j++;
            wpak.shaw[j] = 0;
            k = 32;
        }
        k -= 8;
        wpak.shaw[j] |= ((u32)c << k);
    }
    if(pad) {
        if(pad == 0x80) {
            if(n <= ofs+55) {
                wpak.shaw[15] = 8 * n;
            }
        } else {
            wpak.shaw[15] = 8 * (n + 68);
        }
    }
    if(xinit) {
        u32 xdw = ((u32)xinit << 16) | xinit;
        for (j = 0; j < 16; j++) {
            wpak.shaw[j] ^= xdw;
        }
    }
}

static void itershaw(const u32 *s)
{
    u32 a, b, c, d, e, t;
    int k;

    a = s[0];
    b = s[1];
    c = s[2];
    d = s[3];
    e = s[4];
    for (k = 16; k < 80; k++) {
        t = wpak.shaw[k - 3] ^ wpak.shaw[k - 8] ^ wpak.shaw[k - 14] ^ wpak.shaw[k - 16];
        wpak.shaw[k] = (t << 1) | (t >> 31);
    }
    for (k = 0; k < 20; k++) {
        t = ((a << 5) | (a >> 27)) + e + wpak.shaw[k] + 0x5A827999 + ((b & c) | ((~b) & d));
        e = d;
        d = c;
        c = (b << 30) | (b >> 2);
        b = a;
        a = t;
    }
    for (k = 20; k < 40; k++) {
        t = ((a << 5) | (a >> 27)) + e + wpak.shaw[k] + 0x6ED9EBA1 + (b^c^d);
        e = d;
        d = c;
        c = (b << 30) | (b >> 2);
        b = a;
        a = t;
    }
    for (k = 40; k < 60; k++) {
        t = ((a << 5) | (a >> 27)) + e + wpak.shaw[k] + 0x8F1BBCDC + ((b & c) | (b & d) | (c & d));
        e = d;
        d = c;
        c = (b << 30) | (b >> 2);
        b = a;
        a = t;
    }
    for (k = 60; k < 80; k++) {
        t = ((a << 5) | (a >> 27)) + e + wpak.shaw[k] + 0xCA62C1D6 + (b^c^d);
        e = d;
        d = c;
        c = (b << 30) | (b >> 2);
        b = a;
        a = t;
    }
    wpak.shaw[0] = s[0] + a;
    wpak.shaw[1] = s[1] + b;
    wpak.shaw[2] = s[2] + c;
    wpak.shaw[3] = s[3] + d;
    wpak.shaw[4] = s[4] + e;
}

u8  *ySHA1(const char *text)
{
    int ofs = 0, n = (int)strlen(text);

    memcpy((u8 *)wpak.shau, (u8 *)sha1_init, sizeof(wpak.shau));
    do {
        initshaw(text, ofs, 0x80, 0);
        itershaw(wpak.shau);
        memcpy((u8 *)wpak.shau, (u8 *)wpak.shaw, sizeof(wpak.shau));
        ofs += 64;
    } while(n > ofs-9);
#ifndef CPU_BIG_ENDIAN
    for(ofs = 0; ofs < 5; ofs++) {
        wpak.shau[ofs] = ntohl(wpak.shau[ofs]);
    }
#endif

    return (u8 *)wpak.shau;
}

void yInitPsk(const char *pass, const char *ssid)
{
    // precompute part of sha used in the loops
    initshaw(pass, 0, 0, 0x3636);
    itershaw(sha1_init);
    memcpy(wpak.inner, wpak.shaw, sizeof(wpak.inner));

    initshaw(pass, 0, 0, 0x5c5c);
    itershaw(sha1_init);
    memcpy(wpak.outer, wpak.shaw, sizeof(wpak.outer));

    // prepare to compute first 20 bytes
    wpak.pos = 0;
    wpak.iter = 0;
    memset(wpak.shau, 0, sizeof(wpak.shau));
    initshaw(ssid, 0, 1, 0);
}

int yIterPsk(u8 *res, const char *ssid)
{
    int k;

    if(wpak.iter < 0) return -1;
    if(wpak.iter >= 8192) return 0;
    itershaw(wpak.inner);
    wpak.shaw[5] = 0x80000000;
    for (k = 6; k < 15; k++) {
        wpak.shaw[k] = 0;
    }
    wpak.shaw[15] = 8 * (64 + 20);
    itershaw(wpak.outer);
    wpak.shau[0] ^= wpak.shaw[0];
    wpak.shau[1] ^= wpak.shaw[1];
    wpak.shau[2] ^= wpak.shaw[2];
    wpak.shau[3] ^= wpak.shaw[3];
    wpak.shau[4] ^= wpak.shaw[4];
    wpak.iter++;
    // after 4096 loops, move to 2nd half of sha1
    if((wpak.iter & 4095) == 0) {
        for(k = 0; k < 5 && wpak.pos < 32; k++) {
            wpak.res[wpak.pos++] = (wpak.shau[k] >> 24) & 0xff;
            wpak.res[wpak.pos++] = (wpak.shau[k] >> 16) & 0xff;
            wpak.res[wpak.pos++] = (wpak.shau[k] >> 8) & 0xff;
            wpak.res[wpak.pos++] = wpak.shau[k] & 0xff;
        }
        if(wpak.iter == 4096) {
            memset(wpak.shau, 0, sizeof(wpak.shau));
            initshaw(ssid, 0,2, 0);
        } else {
            // done
            memcpy(res, wpak.res, 32);
            return 0;
        }
    }
    // return 1 to ask for more loops
    return 1;
}

#endif

#ifndef MICROCHIP_API
/*
 * MD5 implementation below is mostly from Sergey Lyubka, author of SHTTPD.
 * Any other implementation would do as well, but we like this one.
 * This code has been published by Sergey under his "THE BEER-WARE LICENSE" (Revision 42):
 *
 *   Sergey Lyubka wrote this software. As long as you retain this notice you
 *   can do whatever you want with this stuff. If we meet some day, and you think
 *   this stuff is worth it, you can buy me a beer in return.
 */

#ifndef CPU_BIG_ENDIAN
#define byteReverse(buf, len) // Do nothing
#else
static void byteReverse(unsigned char *buf, unsigned longs) {
    u32 t;
    do {
        t = (u32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
        ((unsigned) buf[1] << 8 | buf[0]);
        *(u32 *) buf = t;
        buf += 4;
    } while (--longs);
}
#endif

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, data, s) \
( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

// Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
// initialization constants.
void MD5Initialize(HASH_SUM *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

static void MD5Transform(u32 buf[4], u32 const in[16])
{
    register u32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

void MD5AddData(HASH_SUM *ctx,  const u8 *buf, u32 len)
{
    u32 t;

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((u32) len << 3)) < t)
        ctx->bits[1]++;
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;

    if (t) {
        unsigned char *p = (unsigned char *) ctx->in + t;

        t = 64 - t;
        if (len < t) {
            memcpy(p, buf, len);
            return;
        }
        memcpy(p, buf, t);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);
        buf += t;
        len -= t;
    }

    while (len >= 64) {
        memcpy(ctx->in, buf, 64);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);
        buf += 64;
        len -= 64;
    }

    memcpy(ctx->in, buf, len);
}

void MD5Calculate(HASH_SUM *ctx, u8 digest[16])
{
    unsigned count;
    unsigned char *p;

    count = (ctx->bits[0] >> 3) & 0x3F;

    p = ctx->in + count;
    *p++ = 0x80;
    count = 64 - 1 - count;
    if (count < 8) {
        memset(p, 0, count);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);
        memset(ctx->in, 0, 56);
    } else {
        memset(p, 0, count - 8);
    }
    byteReverse(ctx->in, 14);

    ctx->in32[14] = ctx->bits[0];
    ctx->in32[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (u32 *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset((char *) ctx, 0, sizeof(*ctx));
}
#endif

