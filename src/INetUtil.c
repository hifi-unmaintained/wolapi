/*
 * Copyright (c) 2011, 2012 Toni Spets <toni.spets@iki.fi>
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

/* {B832B0AA-A7D3-11D1-97C3-00609706FA0C} */
const GUID IID_INetUtil           = {0xB832B0AA,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};
    
/* {B832B0AC-A7D3-11D1-97C3-00609706FA0C} */
const GUID IID_INetUtilEvent      = {0xB832B0AC,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};

INetUtil *INetUtilSingleton = NULL;

static HRESULT __stdcall _QueryInterface(INetUtil *this, REFIID riid, void **ppvObject)
{
    dprintf("INetUtil::QueryInterface(this=%p, riid={%s}, ppvObject=%p)\n", this, str_GUID(riid), ppvObject);

    if (IsEqualIID(riid, &IID_IConnectionPointContainer))
    {
        dprintf(" IConnectionPointContainer interface requested\n");
        *ppvObject = (void *)&IConnectionPointContainerSingleton;
        return S_OK;
    }

    if (IsEqualIID(riid, &IID_INetUtil))
    {
        dprintf(" INetUtil interface requested, returning self\n");
        *ppvObject = this;
        return S_OK;
    }

    return E_NOINTERFACE;
}

static ULONG __stdcall _AddRef(INetUtil *this)
{
    dprintf("INetUtil::AddRef(this=%p)\n", this);
    return ++this->ref;
}

static ULONG __stdcall _Release(INetUtil *this)
{
    dprintf("INetUtil::Release(this=%p)\n", this);
    return --this->ref;
}

static HRESULT __stdcall _RequestGameresSend(INetUtil *this, LPSTR host, int port, unsigned char* data, int length)
{
    dprintf("INetUtil::RequestGameresSend(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestLadderSearch(INetUtil *this, LPSTR host, int port, LPSTR key, unsigned long SKU, int team, int cond, int sort, int number, int leading)
{
    dprintf("INetUtil::RequestLadderSearch(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestLadderList(INetUtil *this, LPSTR host, int port, LPSTR keys, unsigned long SKU, int team, int cond, int sort)
{
    dprintf("INetUtil::RequestLadderList(this=%p, host=\"%s\", port=%d, keys=\"%s\", SKU=%d, team=%d, cond=%d, sort=%d)\n", this, host, port, keys, SKU, team, cond, sort);
    return S_FALSE;
}

static HRESULT __stdcall _RequestPing(INetUtil *this, LPSTR host, int timeout, int* handle)
{
#ifdef _VERBOSE
    dprintf("INetUtil::RequestPing(this=%p, host=\"%s\", timeout=%d, handle=%p)\n", this, host, timeout, handle);
#endif
    return S_FALSE;
}

static HRESULT __stdcall _PumpMessages(INetUtil *this)
{
#ifdef _VERBOSE
    dprintf("INetUtil::PumpMessages(this=%p, ...)\n", this);
#endif

    if (this->have_ladder > -1 && --this->have_ladder == 0)
    {
        INetUtilEvent_OnLadderList(this->ev, S_OK, &this->ladder, 1, time(NULL)-3600, -1);
    }

    return S_OK;
}

static HRESULT __stdcall _GetAvgPing(INetUtil *this, unsigned long ip, int* avg)
{
#ifdef _VERBOSE
    dprintf("INetUtil::GetAvgPing(this=%p, ip=%d, avg=%p)\n", this, ip, avg);
#endif
    return S_FALSE;
}

static HRESULT __stdcall _RequestNewNick(INetUtil *this, LPSTR nick, LPSTR pass, LPSTR email, LPSTR parentEmail, int newsletter)
{
    dprintf("INetUtil::RequestNewNick(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestAgeCheck(INetUtil *this, int month, int day, int year, LPSTR email)
{
    dprintf("INetUtil::RequestAgeCheck(this=%p, ...)\n", this);
    return S_OK;
}

static INetUtilVtbl Vtbl =
{
    /* IUnknown */
    _QueryInterface,
    _AddRef,
    _Release,

    /* INetUtil */
    _RequestGameresSend,
    _RequestLadderSearch,
    _RequestLadderList,
    _RequestPing,
    _PumpMessages,
    _GetAvgPing,
    _RequestNewNick,
    _RequestAgeCheck
};

INetUtil *INetUtil_New()
{
    INetUtil *this = calloc(1, sizeof(INetUtil));
    this->lpVtbl = &Vtbl;
    dprintf("INetUtil::New()\n");
    _AddRef(this);
    return this;
}
