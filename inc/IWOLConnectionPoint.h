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

#ifndef _IWOLCONNECTIONPOINT_H_
#define _IWOLCONNECTIONPOINT_H_

/* IWOLConnectionPoint Interface */
typedef struct _IWOLConnectionPoint     IWOLConnectionPoint;
typedef struct _IWOLConnectionPointVtbl IWOLConnectionPointVtbl;

struct _IWOLConnectionPoint
{
    IWOLConnectionPointVtbl   *lpVtbl;

    int         ref;
    IID         iid;
};

struct _IWOLConnectionPointVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)              (IWOLConnectionPoint *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)                        (IWOLConnectionPoint *);
    ULONG(__stdcall *Release)                       (IWOLConnectionPoint *);

    /* IConnectionPoint */
    HRESULT(__stdcall *GetConnectionInterface)      (IWOLConnectionPoint *, IID *pIID);
    HRESULT(__stdcall *GetConnectionPointContainer) (IWOLConnectionPoint *, IConnectionPointContainer **ppCPC);
    HRESULT(__stdcall *Advise)                      (IWOLConnectionPoint *, IUnknown *pUnkSink, DWORD *pdwCookie);
    HRESULT(__stdcall *Unadvise)                    (IWOLConnectionPoint *, DWORD dwCookie);
    HRESULT(__stdcall *EnumConnections)             (IWOLConnectionPoint *, IEnumConnections **ppEnum);
};

IWOLConnectionPoint *IWOLConnectionPoint_New();

#endif
