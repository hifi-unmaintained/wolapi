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

static HRESULT __stdcall _QueryInterface(IConnectionPointContainer *this, REFIID riid, void **obj)
{
    dprintf("IConnectionPointContainer::QueryInterface(this=%p, riid=%p, obj=%p)\n", this, riid, obj);
    return E_NOINTERFACE;
}

static ULONG __stdcall _AddRef(IConnectionPointContainer *this)
{
    dprintf("IConnectionPointContainer::AddRef(this=%p)\n", this);
    return 1;
}

static ULONG __stdcall _Release(IConnectionPointContainer *this)
{
    dprintf("IConnectionPointContainer::Release(this=%p)\n", this);
    return 1;
}

static HRESULT __stdcall _EnumConnectionPoints(IConnectionPointContainer *this, IEnumConnectionPoints **ppEnum)
{
    dprintf("IConnectionPointContainer::EnumConnectionPoints(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _FindConnectionPoint(IConnectionPointContainer *this, REFIID riid, IConnectionPoint **ppCP)
{
    dprintf("IConnectionPointContainer::EnumConnectionPoints(this=%p, riid={%s}, ppCP=%p)\n", this, str_GUID(riid), ppCP);

    if (IsEqualIID(riid, &IID_IChatEvent))
    {
        dprintf(" IChatEvent IConnectionPoint requested\n");
        *ppCP = (IConnectionPoint *)IWOLConnectionPoint_New(riid);
        return S_OK;
    }

    if (IsEqualIID(riid, &IID_INetUtilEvent))
    {
        dprintf(" INetUtilEvent IConnectionPoint requested\n");
        *ppCP = (IConnectionPoint *)IWOLConnectionPoint_New(riid);
        return S_OK;
    }

    return E_NOINTERFACE;
}

static IConnectionPointContainerVtbl Vtbl =
{
    /* IUnknown */
    _QueryInterface,
    _AddRef,
    _Release,

    /* IConnectionPointContainer */
    _EnumConnectionPoints,
    _FindConnectionPoint
};

IConnectionPointContainer IConnectionPointContainerSingleton = { &Vtbl };
