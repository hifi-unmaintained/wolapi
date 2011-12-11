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

#ifndef _ICHAT2_H_
#define _ICHAT2_H_

/* IChat2 Interface */

extern const GUID IID_IChat2;

typedef struct _IChat2      IChat2;
typedef struct _IChat2Vtbl  IChat2Vtbl;

struct _IChat2
{
    IChat2Vtbl  *lpVtbl;
};

struct _IChat2Vtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)          (IChat2 *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)                    (IChat2 *);
    ULONG(__stdcall *Release)                   (IChat2 *);

    /* IChat2 */
    HRESULT(__stdcall *PumpMessages)            (IChat2 *);
    HRESULT(__stdcall *RequestConnection)       (IChat2 *, Server* Server, int timeout);
    HRESULT(__stdcall *RequestMessage)          (IChat2 *, unsigned long who, LPSTR message);
    HRESULT(__stdcall *GetTypeFromGID)          (IChat2 *, unsigned long id, GTYPE_* type);
    HRESULT(__stdcall *RequestChannelList)      (IChat2 *);
    HRESULT(__stdcall *RequestChannelJoin)      (IChat2 *, LPSTR name);
    HRESULT(__stdcall *RequestChannelLeave)     (IChat2 *, Channel* chan);
    HRESULT(__stdcall *RequestUserList)         (IChat2 *, Channel* chan);
    HRESULT(__stdcall *RequestLogout)           (IChat2 *);
    HRESULT(__stdcall *RequestChannelCreate)    (IChat2 *, Channel* chan);
    HRESULT(__stdcall *RequestRawCmd)           (IChat2 *, LPSTR cmd);
};

#endif
