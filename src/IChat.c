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

/* {4DD3BAF4-7579-11D1-B1C6-006097176556} */
const GUID IID_IChat              = {0x4DD3BAF4,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};

/* {4DD3BAF6-7579-11D1-B1C6-006097176556} */
const GUID IID_IChatEvent         = {0x4DD3BAF6,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};

IChat *IChatSingleton = NULL;

static HRESULT __stdcall IChat_QueryInterface(IChat *this, REFIID riid, void **ppvObject)
{
    dprintf("IChat::QueryInterface(this=%p, riid={%s}, ppvObject=%p)\n", this, str_GUID(riid), ppvObject);

    if (IsEqualIID(riid, &IID_IConnectionPointContainer))
    {
        dprintf(" IConnectionPointContainer interface requested\n");
        *ppvObject = (void *)&IConnectionPointContainerSingleton;
        return S_OK;
    }

    return E_NOINTERFACE;
}

static ULONG __stdcall IChat_AddRef(IChat *this)
{
    dprintf("IChat::AddRef(this=%p)\n", this);
    return ++this->ref;
}

static ULONG __stdcall IChat_Release(IChat *this)
{
    dprintf("IChat::Release(this=%p)\n", this);
    return --this->ref;
}

static HRESULT __stdcall IChat_PumpMessages(IChat *this)
{
#ifdef _VERBOSE
    dprintf("IChat::PumpMessages(this=%p)\n", this);
#endif
    return S_OK;
}

static HRESULT __stdcall IChat_RequestServerList(IChat *this, unsigned long SKU, unsigned long current_version, LPSTR loginname, LPSTR password, int timeout)
{
    dprintf("IChat::RequestServerList(this=%p, SKU=%08X, current_version=%08X, loginname=\"%s\", password=\"%s\", timeout=%d)\n", this, SKU, current_version, loginname, password, timeout);

    static Server srv;
    srv.gametype = SKU;
    strcpy(srv.name, "WOLAPI stub");
    strcpy(srv.connlabel, "IRC");
    strcpy(srv.conndata, "TCP:localhost:6667");

    this->SKU = SKU; /* for debugging only */

    IChatEvent_OnServerList(this->ev, S_OK, &srv);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestConnection(IChat *this, Server* server, int timeout, int domangle)
{
    dprintf("IChat::RequestConnection(this=%p, server=%p, timeout=%d, domangle=%d)\n", this, server, timeout, domangle);

    dprintf("    name     : %s\n", server->name);
    dprintf("    connlabel: %s\n", server->connlabel);
    dprintf("    conndata : %s\n", server->conndata);
    dprintf("    login    : %s\n", server->login);
    dprintf("    password : %s\n", server->password);

    IChatEvent_OnConnection(this->ev, S_OK, "Welcome to Westwood Online stub implementation!\r\n");

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelList(IChat *this, int channelType, int autoping)
{
    dprintf("IChat::RequestChannelList(this=%p, channelType=%d, autoping=%d)\n", this, channelType, autoping);

    static Channel chan;
    chan.type       = 0;
    chan.official   = 1;
    chan.latency    = -1;
    strcpy(chan.name, "Stub");
    strcpy(chan.topic, "The default channel stub");

    IChatEvent_OnChannelList(this->ev, S_OK, &chan);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelCreate(IChat *this, Channel* Channel)
{
    dprintf("IChat::RequestChannelCreate(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelJoin(IChat *this, Channel* Channel)
{
    dprintf("IChat::RequestChannelJoin(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelLeave(IChat *this)
{
    dprintf("IChat::RequestChannelLeave(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestUserList(IChat *this)
{
    dprintf("IChat::RequestUserList(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicMessage(IChat *this, LPSTR message)
{
    dprintf("IChat::RequestPublicMessage(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateMessage(IChat *this, User* users, LPSTR message)
{
    dprintf("IChat::RequestPrivateMessage(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestLogout(IChat *this)
{
    dprintf("IChat::RequestLogout(this=%p)\n", this);
    IChatEvent_OnNetStatus(this->ev, CHAT_S_CON_DISCONNECTED);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateGameOptions(IChat *this, User* users, LPSTR options)
{
    dprintf("IChat::RequestPrivateGameOptions(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicGameOptions(IChat *this, LPSTR options)
{
    dprintf("IChat::RequestPublicGameOptions(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicAction(IChat *this, LPSTR action)
{
    dprintf("IChat::RequestPublicAction(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateAction(IChat *this, User* users, LPSTR action)
{
    dprintf("IChat::RequestPrivateAction(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestGameStart(IChat *this, User* users)
{
    dprintf("IChat::RequestGameStart(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelTopic(IChat *this, LPSTR topic)
{
    dprintf("IChat::RequestChannelTopic(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_GetVersion(IChat *this, unsigned long* version)
{
    dprintf("IChat::GetVersion(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestUserKick(IChat *this, User* User)
{
    dprintf("IChat::RequestUserKick(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestUserIP(IChat *this, User* User)
{
    dprintf("IChat::RequestUserIp(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_GetGametypeInfo(IChat *this, unsigned int gtype, int icon_size, unsigned char** bitmap, int* bmp_bytes, LPSTR* name, LPSTR* URL)
{
    dprintf("IChat::GetGametypeInfo(this=%p, gtype=%d, icon_size=%d, bitmap=%p, bmp_bytes=%p, name=%p, URL=%p)\n", this, gtype, icon_size, bitmap, bmp_bytes, name, URL);
    return S_FALSE;
}

static HRESULT __stdcall IChat_RequestFind(IChat *this, User* User)
{
    dprintf("IChat::RequestFind(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPage(IChat *this, User* User, LPSTR message)
{
    dprintf("IChat::RequestPage(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_SetFindPage(IChat *this, int findOn, int pageOn)
{
    dprintf("IChat::SetFindPage(this=%p, findOn=%d, pageOn=%d)\n", this, findOn, pageOn);
    return S_OK;
}

static HRESULT __stdcall IChat_SetSquelch(IChat *this, User* User, int squelch)
{
    dprintf("IChat::SetSquelch(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_GetSquelch(IChat *this, User* User)
{
    dprintf("IChat::GetSquelch(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_SetChannelFilter(IChat *this, int channelType)
{
    dprintf("IChat::SetChannelFilter(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestGameEnd(IChat *this)
{
    dprintf("IChat::RequestGameEnd(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_SetLangFilter(IChat *this, int onoff)
{
    dprintf("IChat::SetLangFilter(this=%p, onoff=%d)\n", this, onoff);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelBan(IChat *this, LPSTR name, int ban)
{
    dprintf("IChat::RequestChannelBan(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_GetGametypeList(IChat *this, LPSTR* list)
{
    dprintf("IChat::GetGametypeList(this=%p, list=%p)\n", this, list);
    /* comma separated list of integers, what are they? */
    *list = "";
    return S_OK;
}

static HRESULT __stdcall IChat_GetHelpURL(IChat *this, LPSTR* URL)
{
    dprintf("IChat::GetHelpURL(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_SetProductSKU(IChat *this, unsigned long SKU)
{
    dprintf("IChat::SetProductSKU(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_GetNick(IChat *this, int num, LPSTR* nick, LPSTR* pass)
{
    dprintf("IChat::GetNick(this=%p, num=%d, nick=%p, pass=%p)\n", this, num, nick, pass);

    /* just throw something back so the game won't ask for registration */
    if (num == 1)
    {
        *nick = "Guest";
        *pass = "Guest";
        return S_OK;
    }

    return S_FALSE;
}

static HRESULT __stdcall IChat_SetNick(IChat *this, int num, LPSTR nick, LPSTR pass, int domangle)
{
    dprintf("IChat::SetNick(this=%p, nick=\"%s\", pass=\"%s\", domangle=%d)\n", this, nick, pass, domangle);
    return S_OK;
}

static HRESULT __stdcall IChat_GetLobbyCount(IChat *this, int* count)
{
    dprintf("IChat::GetLobbyCount(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestRawMessage(IChat *this, LPSTR ircmsg)
{
    dprintf("IChat::RequestRawMessage(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_GetAttributeValue(IChat *this, LPSTR attrib, LPSTR* value)
{
    dprintf("IChat::GetAttributeValue(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_SetAttributeValue(IChat *this, LPSTR attrib, LPSTR value)
{
    dprintf("IChat::SetAttributeValue(this=%p, attrib=\"%s\", value=\"%s\")\n", this, attrib, value);
    return S_OK;
}

static HRESULT __stdcall IChat_SetChannelExInfo(IChat *this, LPSTR info)
{
    dprintf("IChat::SetChannelExInfo(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_StopAutoping(IChat *this)
{
    dprintf("IChat::StopAutoping(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestSquadInfo(IChat *this, unsigned long id)
{
    dprintf("IChat::RequestSquadInfo(this=%p, ...)\n", this);
    return S_OK;
}

static IChatVtbl Vtbl =
{
    /* IUnknown */
    IChat_QueryInterface,
    IChat_AddRef,
    IChat_Release,

    /* IChat */
    IChat_PumpMessages,
    IChat_RequestServerList,
    IChat_RequestConnection,
    IChat_RequestChannelList,
    IChat_RequestChannelCreate,
    IChat_RequestChannelJoin,
    IChat_RequestChannelLeave,
    IChat_RequestUserList,
    IChat_RequestPublicMessage,
    IChat_RequestPrivateMessage,
    IChat_RequestLogout,
    IChat_RequestPrivateGameOptions,
    IChat_RequestPublicGameOptions,
    IChat_RequestPublicAction,
    IChat_RequestPrivateAction,
    IChat_RequestGameStart,
    IChat_RequestChannelTopic,
    IChat_GetVersion,
    IChat_RequestUserKick,
    IChat_RequestUserIP,
    IChat_GetGametypeInfo,
    IChat_RequestFind,
    IChat_RequestPage,
    IChat_SetFindPage,
    IChat_SetSquelch,
    IChat_GetSquelch,
    IChat_SetChannelFilter,
    IChat_RequestGameEnd,
    IChat_SetLangFilter,
    IChat_RequestChannelBan,
    IChat_GetGametypeList,
    IChat_GetHelpURL,
    IChat_SetProductSKU,
    IChat_GetNick,
    IChat_SetNick,
    IChat_GetLobbyCount,
    IChat_RequestRawMessage,
    IChat_GetAttributeValue,
    IChat_SetAttributeValue,
    IChat_SetChannelExInfo,
    IChat_StopAutoping,
    IChat_RequestSquadInfo
};

IChat *IChat_New()
{
    dprintf("IChat::New()\n");

    IChat *this = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IChat));
    this->lpVtbl = &Vtbl;
    IChat_AddRef(this);

    /* to get our callback ev pointer */
    IChatSingleton = this;

    return this;
}
