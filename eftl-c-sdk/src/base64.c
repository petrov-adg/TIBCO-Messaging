/*
 * Copyright (c) 2009-$Date: 2017-06-16 15:38:34 -0500 (Fri, 16 Jun 2017) $ TIBCO Software Inc.
 * Licensed under a BSD-style license. Refer to [LICENSE]
 * For more information, please contact:
 * TIBCO Software Inc., Palo Alto, California, USA
 *
 * $Id: base64.c 94080 2017-06-16 20:38:34Z bpeterse $
 *
 */

#include "base64.h"

static const char* base64chars_rfc2045 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char char_to_num_rfc2045[] = {
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x3e,  0x00,  0x00,  0x00,  0x3f, 
    0x34,  0x35,  0x36,  0x37,  0x38,  0x39,  0x3a,  0x3b,  0x3c,  0x3d,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0a,  0x0b,  0x0c,  0x0d,  0x0e, 
    0x0f,  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x1a,  0x1b,  0x1c,  0x1d,  0x1e,  0x1f,  0x20,  0x21,  0x22,  0x23,  0x24,  0x25,  0x26,  0x27,  0x28, 
    0x29,  0x2a,  0x2b,  0x2c,  0x2d,  0x2e,  0x2f,  0x30,  0x31,  0x32,  0x33,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00, 
    0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00
};

int
base64_encode(
    const unsigned char*    source,
    int                     slen,
    char*                   output,
    int                     outlen)
{
    int                     olen = 0;
    int                     buff3;   // Contains 3 bytes that will be separated into 4 6-bit numbers

    if (outlen < BASE64_ENC_MAX_LEN(slen)) {
        return -1;
    }
    
    while (slen > 0)
    {
        buff3 = *source++ << 16;
        
        if (slen > 1)
            buff3 |= *source++ << 8;
        
        if (slen > 2)
            buff3 |= *source++;
        
        // We have a three byte block in buff3.  Now generate the 4 chars.
        *output++ = base64chars_rfc2045[(buff3 >> 18) & 0x3f];
        *output++ = base64chars_rfc2045[(buff3 >> 12) & 0x3f];

        if (slen > 1)
            *output++ = base64chars_rfc2045[(buff3 >> 6) & 0x3f];
        else
            *output++ = '=';
        
        if (slen > 2)
            *output++ = base64chars_rfc2045[buff3 & 0x3f];
        else
            *output++ = '=';
        
        olen +=4;
        slen -= 3;
    }
    
    *output='\0';
    return olen;
}

int
base64_decode(
    unsigned char*          output,
    int                     outlen,
    const char*             source,
    int                     slen)
{
    int                     i, k, pad = 0;
    unsigned int            n;
    
    if (outlen < BASE64_DEC_MAX_LEN(slen)) {
        return -1;
    }

    outlen=0;
    
    while(slen>0) {
        n = 0;
        k = 0;

        while(slen > 0) {
            n <<= 6;
            n |= (((unsigned int)(char_to_num_rfc2045[(int)*source])) & 0x003f); 
            if (*source == '=')
                pad++;
            source++; slen--;
            k++;
            if (k >= 4) 
                break;
        }

        for (i=k-2; i>=0; i--) {
            *output=(n>>(8*i))&0x00ff;
            output++; outlen++;
        }
    }

    return outlen - pad;
}