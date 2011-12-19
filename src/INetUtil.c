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

/* {B832B0AA-A7D3-11D1-97C3-00609706FA0C} */
const GUID IID_INetUtil           = {0xB832B0AA,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};
    
/* {B832B0AC-A7D3-11D1-97C3-00609706FA0C} */
const GUID IID_INetUtilEvent      = {0xB832B0AC,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};

INetUtil *INetUtilSingleton = NULL;

static HRESULT __stdcall INetUtil_QueryInterface(INetUtil *this, REFIID riid, void **ppvObject)
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

static ULONG __stdcall INetUtil_AddRef(INetUtil *this)
{
    dprintf("INetUtil::AddRef(this=%p)\n", this);
    return ++this->ref;
}

static ULONG __stdcall INetUtil_Release(INetUtil *this)
{
    dprintf("INetUtil::Release(this=%p)\n", this);
    return --this->ref;
}

static HRESULT __stdcall INetUtil_RequestGameresSend(INetUtil *this, LPSTR host, int port, unsigned char* data, int length)
{
    dprintf("INetUtil::RequestGameresSend(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall INetUtil_RequestLadderSearch(INetUtil *this, LPSTR host, int port, LPSTR key, unsigned long SKU, int team, int cond, int sort, int number, int leading)
{
    dprintf("INetUtil::RequestLadderSearch(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall INetUtil_RequestLadderList(INetUtil *this, LPSTR host, int port, LPSTR keys, unsigned long SKU, int team, int cond, int sort)
{
    dprintf("INetUtil::RequestLadderList(this=%p, host=\"%s\", port=%d, keys=\"%s\", SKU=%d, team=%d, cond=%d, sort=%d)\n", this, host, port, keys, SKU, team, cond, sort);
    return S_OK;
}

static HRESULT __stdcall INetUtil_RequestPing(INetUtil *this, LPSTR host, int timeout, int* handle)
{
    dprintf("INetUtil::RequestPing(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall INetUtil_PumpMessages(INetUtil *this)
{
#ifdef _VERBOSE
    dprintf("INetUtil::PumpMessages(this=%p, ...)\n", this);
#endif
    return S_OK;
}

static HRESULT __stdcall INetUtil_GetAvgPing(INetUtil *this, unsigned long ip, int* avg)
{
    dprintf("INetUtil::GetAvgPing(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall INetUtil_RequestNewNick(INetUtil *this, LPSTR nick, LPSTR pass, LPSTR email, LPSTR parentEmail, int newsletter)
{
    dprintf("INetUtil::RequestNewNick(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall INetUtil_RequestAgeCheck(INetUtil *this, int month, int day, int year, LPSTR email)
{
    dprintf("INetUtil::RequestAgeCheck(this=%p, ...)\n", this);
    return S_OK;
}

static INetUtilVtbl Vtbl =
{
    /* IUnknown */
    INetUtil_QueryInterface,
    INetUtil_AddRef,
    INetUtil_Release,

    /* INetUtil */
    INetUtil_RequestGameresSend,
    INetUtil_RequestLadderSearch,
    INetUtil_RequestLadderList,
    INetUtil_RequestPing,
    INetUtil_PumpMessages,
    INetUtil_GetAvgPing,
    INetUtil_RequestNewNick,
    INetUtil_RequestAgeCheck
};

INetUtil *INetUtil_New()
{
    dprintf("INetUtil::New()\n");

    INetUtil *this = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(INetUtil));
    this->lpVtbl = &Vtbl;
    INetUtil_AddRef(this);
    return this;
}
