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

#ifndef _ICHAT2EVENT_H_
#define _ICHAT2EVENT_H_

/* IChat2Event Interface */

extern const GUID IID_IChat2Event;

typedef struct _IChat2Event      IChat2Event;
typedef struct _IChat2EventVtbl  IChat2EventVtbl;

struct _IChat2Event
{
    IChat2EventVtbl  *lpVtbl;
};

struct _IChat2EventVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)  (IChat2Event *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)            (IChat2Event *);
    ULONG(__stdcall *Release)           (IChat2Event *);

    /* IChat2Event */
    HRESULT(__stdcall *OnNetStatus)     (IChat2Event *, HRESULT res);
    HRESULT(__stdcall *OnMessage)       (IChat2Event *, HRESULT res, User* User, LPSTR message);
    HRESULT(__stdcall *OnChannelList)   (IChat2Event *, HRESULT res, Channel* list);
    HRESULT(__stdcall *OnChannelJoin)   (IChat2Event *, HRESULT res, Channel* chan, User* User);
    HRESULT(__stdcall *OnLogin)         (IChat2Event *, HRESULT res);
    HRESULT(__stdcall *OnUserList)      (IChat2Event *, HRESULT res, Channel* chan, User* users);
    HRESULT(__stdcall *OnChannelLeave)  (IChat2Event *, HRESULT res, Channel* chan, User* User);
    HRESULT(__stdcall *OnChannelCreate) (IChat2Event *, HRESULT res, Channel* chan);
    HRESULT(__stdcall *OnUnknownLine)   (IChat2Event *, HRESULT res, LPSTR line);
};

#endif
