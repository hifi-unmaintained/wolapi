/*
 * Copyright (c) 2011 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <wolapi.h>

#ifndef _INETUTIL_H_
#define _INETUTIL_H_

/* INetUtil Interface */

extern const GUID IID_INetUtil;

typedef struct _INetUtil        INetUtil;
typedef struct _INetUtilVtbl    INetUtilVtbl;

struct _INetUtil
{
    INetUtilVtbl    *lpVtbl;

    int             ref;
    INetUtilEvent   *ev;
    int             have_ladder;
    Ladder          ladder;
};

struct _INetUtilVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)  (INetUtil *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)            (INetUtil *);
    ULONG(__stdcall *Release)           (INetUtil *);

    /* INetUtil */
    HRESULT(__stdcall *RequestGameresSend)  (INetUtil *, LPSTR host, int port, unsigned char* data, int length);
    HRESULT(__stdcall *RequestLadderSearch) (INetUtil *, LPSTR host, int port, LPSTR key, unsigned long SKU, int team, int cond, int sort, int number, int leading);
    HRESULT(__stdcall *RequestLadderList)   (INetUtil *, LPSTR host, int port, LPSTR keys, unsigned long SKU, int team, int cond, int sort);
    HRESULT(__stdcall *RequestPing)         (INetUtil *, LPSTR host, int timeout, int* handle);
    HRESULT(__stdcall *PumpMessages)        (INetUtil *);
    HRESULT(__stdcall *GetAvgPing)          (INetUtil *, unsigned long ip, int* avg);
    HRESULT(__stdcall *RequestNewNick)      (INetUtil *, LPSTR nick, LPSTR pass, LPSTR email, LPSTR parentEmail, int newsletter);
    HRESULT(__stdcall *RequestAgeCheck)     (INetUtil *, int month, int day, int year, LPSTR email);
};

extern INetUtil *INetUtilSingleton;
INetUtil *INetUtil_New();

#endif
