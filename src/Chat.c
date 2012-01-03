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

/* {4DD3BAF5-7579-11D1-B1C6-006097176556} */
const GUID CLSID_Chat               = {0x4DD3BAF5,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};

static HRESULT __stdcall _QueryInterface(Chat *this, REFIID riid, void **obj)
{
    dprintf("Chat::QueryInterface(this=%p, riid=%p, obj=%p)\n", this, riid, obj);
    return E_NOINTERFACE;
}

static ULONG __stdcall _AddRef(Chat *this)
{
    dprintf("Chat::AddRef(this=%p)\n", this);
    return ++this->ref;
}

static ULONG __stdcall _Release(Chat *this)
{
    dprintf("Chat::Release(this=%p)\n", this);
    return --this->ref;
}

static HRESULT __stdcall _CreateInstance(Chat *this, IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
    dprintf("Chat::CreateInstance(this=%p, pUnkOuter=%p, riid={%s}, ppvObject=%p)\n", this, pUnkOuter, str_GUID(riid), ppvObject);

    if (IsEqualIID(riid, &IID_IChat))
    {
        dprintf(" IChat interface requested, returning a new one\n");
        *ppvObject = IChat_New();
        return S_OK;
    }

    return E_NOINTERFACE;
}

static HRESULT __stdcall _LockServer(Chat *this, BOOL fLock)
{
    dprintf("Chat::LockServer(this=%p, fLock=%s)\n", this, fLock ? "true" : "false");
    return S_OK;
}

static ChatVtbl Vtbl =
{
    /* IUnknown */
    _QueryInterface,
    _AddRef,
    _Release,

    /* Chat */
    _CreateInstance,
    _LockServer
};

Chat* Chat_New()
{
    Chat *this = calloc(1, sizeof(Chat));
    this->lpVtbl = &Vtbl;
    dprintf("Chat::New()\n");
    _AddRef(this);
    return this;
}
