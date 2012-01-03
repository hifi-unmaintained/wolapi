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

static HRESULT __stdcall _QueryInterface(IWOLConnectionPoint *this, REFIID riid, void **obj)
{
    dprintf("IWOLConnectionPoint::QueryInterface(this=%p, riid=%p, obj=%p)\n", this, riid, obj);
    return E_NOINTERFACE;
}

static ULONG __stdcall _AddRef(IWOLConnectionPoint *this)
{
    dprintf("IWOLConnectionPoint::AddRef(this=%p)\n", this);
    return ++this->ref;
}

static ULONG __stdcall _Release(IWOLConnectionPoint *this)
{
    dprintf("IWOLConnectionPoint::Release(this=%p)\n", this);
    if (--this->ref == 0)
    {
        dprintf(" releasing memory\n");
        free(this);
        return 0;
    }
    return this->ref;
}

static HRESULT __stdcall _Advise(IWOLConnectionPoint *this, IUnknown *pUnkSink, DWORD *pdwCookie)
{
    dprintf("IWOLConnectionPoint::Advise(this=%p, pUnkSink=%p, pdwCookie=%p)\n", this, pUnkSink, pdwCookie);

    if (IsEqualIID(&this->iid, &IID_IChatEvent))
    {
        dprintf(" IChatEvent callback registered\n");
        if (IChatSingleton)
        {
            IChatSingleton->ev = (IChatEvent *)pUnkSink;

            printf("    OnServerList         @ %08X\n", IChatSingleton->ev->lpVtbl->OnServerList);
            printf("    OnUpdateList         @ %08X\n", IChatSingleton->ev->lpVtbl->OnUpdateList);
            printf("    OnServerError        @ %08X\n", IChatSingleton->ev->lpVtbl->OnServerError);
            printf("    OnConnection         @ %08X\n", IChatSingleton->ev->lpVtbl->OnConnection);
            printf("    OnMessageOfTheDay    @ %08X\n", IChatSingleton->ev->lpVtbl->OnMessageOfTheDay);
            printf("    OnChannelList        @ %08X\n", IChatSingleton->ev->lpVtbl->OnChannelList);
            printf("    OnChannelCreate      @ %08X\n", IChatSingleton->ev->lpVtbl->OnChannelCreate);
            printf("    OnChannelJoin        @ %08X\n", IChatSingleton->ev->lpVtbl->OnChannelJoin);
            printf("    OnChannelLeave       @ %08X\n", IChatSingleton->ev->lpVtbl->OnChannelLeave);
            printf("    OnChannelTopic       @ %08X\n", IChatSingleton->ev->lpVtbl->OnChannelTopic);
            printf("    OnPrivateAction      @ %08X\n", IChatSingleton->ev->lpVtbl->OnPrivateAction);
            printf("    OnPublicAction       @ %08X\n", IChatSingleton->ev->lpVtbl->OnPublicAction);
            printf("    OnSystemMessage      @ %08X\n", IChatSingleton->ev->lpVtbl->OnSystemMessage);
            printf("    OnNetStatus          @ %08X\n", IChatSingleton->ev->lpVtbl->OnNetStatus);
            printf("    OnLogout             @ %08X\n", IChatSingleton->ev->lpVtbl->OnLogout);
            printf("    OnPrivateGameOptions @ %08X\n", IChatSingleton->ev->lpVtbl->OnPrivateGameOptions);
            printf("    OnPublicGameOptions  @ %08X\n", IChatSingleton->ev->lpVtbl->OnPublicGameOptions);
            printf("    OnGameStart          @ %08X\n", IChatSingleton->ev->lpVtbl->OnGameStart);
            printf("    OnUserKick           @ %08X\n", IChatSingleton->ev->lpVtbl->OnUserKick);
            printf("    OnUserIP             @ %08X\n", IChatSingleton->ev->lpVtbl->OnUserIP);
            printf("    OnFind               @ %08X\n", IChatSingleton->ev->lpVtbl->OnFind);
            printf("    OnPageSend           @ %08X\n", IChatSingleton->ev->lpVtbl->OnPageSend);
            printf("    OnPaged              @ %08X\n", IChatSingleton->ev->lpVtbl->OnPaged);
            printf("    OnServerBannedYou    @ %08X\n", IChatSingleton->ev->lpVtbl->OnServerBannedYou);
            printf("    OnUserFlags          @ %08X\n", IChatSingleton->ev->lpVtbl->OnUserFlags);
            printf("    OnChannelBan         @ %08X\n", IChatSingleton->ev->lpVtbl->OnChannelBan);
            printf("    OnSquadInfo          @ %08X\n", IChatSingleton->ev->lpVtbl->OnSquadInfo);
        }
        *pdwCookie = 1;
        return S_OK;
    }

    if (IsEqualIID(&this->iid, &IID_INetUtilEvent))
    {
        dprintf(" INetUtilEvent callback registered\n");
        if (INetUtilSingleton)
        {
            INetUtilSingleton->ev = (INetUtilEvent *)pUnkSink;
        }
        *pdwCookie = 1;
        return S_OK;
    }

    return S_OK;
}

static HRESULT __stdcall _EnumConnections(IWOLConnectionPoint *this, IEnumConnections **ppEnum)
{
    dprintf("IWOLConnectionPoint::EnumConnections(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _GetConnectionInterface(IWOLConnectionPoint *this, IID *pIID)
{
    dprintf("IWOLConnectionPoint::GetConnectionInterface(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _GetConnectionPointContainer(IWOLConnectionPoint *this, IConnectionPointContainer **ppCPC)
{
    dprintf("IWOLConnectionPoint::GetConnectionPointContainer(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _Unadvise(IWOLConnectionPoint *this, DWORD dwCookie)
{
    dprintf("IWOLConnectionPoint::Unadvise(this=%p, dwCookie=%08X)\n", this, dwCookie);

    if (IsEqualIID(&this->iid, &IID_IChatEvent))
    {
        dprintf(" IChatEvent callback unregistered\n");
        if (IChatSingleton)
        {
            IChatSingleton->ev = NULL;
        }
        return S_OK;
    }

    if (IsEqualIID(&this->iid, &IID_INetUtilEvent))
    {
        dprintf(" INetUtilEvent callback unregistered\n");
        if (INetUtilSingleton)
        {
            INetUtilSingleton->ev = NULL;
        }
        return S_OK;
    }

    return S_OK;
}

static IWOLConnectionPointVtbl Vtbl =
{
    /* IUnknown */
    _QueryInterface,
    _AddRef,
    _Release,

    /* IWOLConnectionPoint */
    _GetConnectionInterface,
    _GetConnectionPointContainer,
    _Advise,
    _Unadvise,
    _EnumConnections
};

IWOLConnectionPoint *IWOLConnectionPoint_New(REFIID riid)
{
    IWOLConnectionPoint *this = calloc(1, sizeof(IWOLConnectionPoint));
    this->lpVtbl = &Vtbl;
    memcpy(&this->iid, riid, sizeof(IID));
    dprintf("IWOLConnectionPoint::New({%s})\n", str_GUID(riid));
    _AddRef(this);
    return this;
}
