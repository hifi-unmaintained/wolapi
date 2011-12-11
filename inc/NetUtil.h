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

#ifndef _NETUTIL_H_
#define _NETUTIL_H_

/* NetUtil Class */
extern const GUID CLSID_NetUtil;

typedef struct _NetUtil     NetUtil;
typedef struct _NetUtilVtbl NetUtilVtbl;

struct _NetUtil
{
    NetUtilVtbl *lpVtbl;

    int         ref;
};

struct _NetUtilVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)              (NetUtil *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)                        (NetUtil *);
    ULONG(__stdcall *Release)                       (NetUtil *);

    /* IClassFactory */
    HRESULT(__stdcall *CreateInstance)              (NetUtil *, IUnknown *, REFIID, void **);
    HRESULT(__stdcall *LockServer)                  (NetUtil *, BOOL);
};

NetUtil* NetUtil_New();

#endif
