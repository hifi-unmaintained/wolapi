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

#ifndef _IRTPATCHER_H_
#define _IRTPATCHER_H_

/* IRTPatcher Interface */

extern const GUID IID_IRTPatcher;

typedef struct _IRTPatcher      IRTPatcher;
typedef struct _IRTPatcherVtbl  IRTPatcherVtbl;

struct _IRTPatcher
{
    IRTPatcherVtbl  *lpVtbl;
};

struct _IRTPatcherVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)  (IRTPatcher *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)            (IRTPatcher *);
    ULONG(__stdcall *Release)           (IRTPatcher *);

    /* IRTPatcher */
    HRESULT(__stdcall *ApplyPatch)      (IRTPatcher *, LPSTR destpath, LPSTR filename);
    HRESULT(__stdcall *PumpMessages)    (IRTPatcher *);
};

#endif
