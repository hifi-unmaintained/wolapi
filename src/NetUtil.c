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

/* {B832B0AB-A7D3-11D1-97C3-00609706FA0C} */
const GUID CLSID_NetUtil          = {0xB832B0AB,0xA7D3,0x11D1,{0x97,0xC3,0x00,0x60,0x97,0x06,0xFA,0x0C}};

static HRESULT __stdcall _QueryInterface(NetUtil *this, REFIID riid, void **obj)
{
    dprintf("NetUtil::QueryInterface(this=%p, riid=%p, obj=%p)\n", this, riid, obj);
    return S_OK;
}

static ULONG __stdcall _AddRef(NetUtil *this)
{
    dprintf("NetUtil::AddRef(this=%p)\n", this);
    return ++this->ref;
}

static ULONG __stdcall _Release(NetUtil *this)
{
    dprintf("NetUtil::Release(this=%p)\n", this);
    return --this->ref;
}

static HRESULT __stdcall _CreateInstance(NetUtil *this, IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
    dprintf("NetUtil::CreateInstance(this=%p, pUnkOuter=%p, riid={%s}, ppvObject=%p)\n", this, pUnkOuter, str_GUID(riid), ppvObject);

    if (IsEqualIID(riid, &IID_INetUtil))
    {
        dprintf(" INetUtil interface requested\n");
        *ppvObject = INetUtil_New();
        return S_OK;
    }

    return E_NOINTERFACE;
}

static HRESULT __stdcall _LockServer(NetUtil *this, BOOL fLock)
{
    dprintf("NetUtil::LockServer(this=%p, fLock=%s)\n", this, fLock ? "true" : "false");
    return S_OK;
}

static NetUtilVtbl Vtbl =
{
    /* IUnknown */
    _QueryInterface,
    _AddRef,
    _Release,

    /* IClassFactory */
    _CreateInstance,
    _LockServer
};

NetUtil* NetUtil_New()
{
    NetUtil *this = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(NetUtil));
    this->lpVtbl = &Vtbl;
    dprintf("NetUtil::New()\n");
    _AddRef(this);
    return this;
}
