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

void user_list_add(User **list, User *user)
{
    User *current = *list;

    if (*list == NULL)
    {
        *list = user;
        return;
    }

    while (current->next)
    {
        current = current->next;
    }

    current->next = user;
}

void user_list_free(User **list)
{
    User *current = *list;
    while (current)
    {
        User *tmp = current;
        current = current->next;
        free(tmp);
    }
    *list = NULL;
}

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

    struct timeval tv = { 0, 0 };
    fd_set in_set;
    fd_set out_set;
    int maxfd = 0;

    FD_ZERO(&in_set);
    FD_ZERO(&out_set);

    irc_add_select_descriptors(this->irc_session, &in_set, &out_set, &maxfd);

    if (select(maxfd, &in_set, &out_set, NULL, &tv) > 0)
    {
        irc_process_select_descriptors(this->irc_session, &in_set, &out_set);
    }

    return S_OK;
}

static HRESULT __stdcall IChat_RequestServerList(IChat *this, unsigned long SKU, unsigned long current_version, LPSTR loginname, LPSTR password, int timeout)
{
    dprintf("IChat::RequestServerList(this=%p, SKU=%08X, current_version=%08X, loginname=\"%s\", password=\"%s\", timeout=%d)\n", this, SKU, current_version, loginname, password, timeout);

    Server srv;
    srv.gametype = SKU;
    strcpy(srv.name, "WOLAPI stub");
    strcpy(srv.connlabel, "IRC");
    strcpy(srv.conndata, "TCP:localhost:6667");
    IChatEvent_OnServerList(this->ev, S_OK, &srv);

    this->SKU = SKU; /* for debugging only */

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

    strcpy(this->name, "RA-");
    strcat(this->name, server->login);

    this->lobby.flags = 0x84010000;
    this->lobby.latency = -1;
    strcpy(this->lobby.name, "Lob_21_0");

    if (irc_connect(this->irc_session, "localhost", 6667, 0, this->name, 0, 0))
    {
        return S_FALSE;
    }

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelList(IChat *this, int channelType, int autoping)
{
    dprintf("IChat::RequestChannelList(this=%p, channelType=%d, autoping=%d)\n", this, channelType, autoping);

    if (channelType == 0)
    {
        IChatEvent_OnChannelList(this->ev, S_OK, &this->lobby);
    }

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelCreate(IChat *this, Channel* channel)
{
    dprintf("IChat::RequestChannelCreate(this=%p, channel=%p)\n", this, channel);

    dprintf("    type      : %d\n", channel->type);
    dprintf("    minUsers  : %d\n", channel->minUsers);
    dprintf("    maxUsers  : %d\n", channel->maxUsers);
    dprintf("    tournament: %d\n", channel->tournament);
    dprintf("    flags     : %08X\n", channel->flags);
    dprintf("    reserved  : %08X\n", channel->reserved);
    dprintf("    name      : %s\n", channel->name);
    dprintf("    topic     : %s\n", channel->topic);
    dprintf("    key       : %s\n", channel->key);
    dprintf("    exInfo    : %s\n", channel->exInfo);

    memcpy(&this->game, channel, sizeof(Channel));
    this->game.currentUsers = 1;

    IChatEvent_OnChannelCreate(this->ev, S_OK, &this->game);
    IChatEvent_OnUserList(this->ev, S_OK, &this->game, &this->user);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelJoin(IChat *this, Channel* channel)
{
    dprintf("IChat::RequestChannelJoin(this=%p, channel=%p)\n", this, channel);

    /* clear old users list always */
    user_list_free(&this->lobby_users);

    /* if we are already in the lobby, just emulate join */
    if (this->channel.name[0])
    {
        irc_cmd_names(this->irc_session, this->channel.name);
        return S_OK;
    }

    memcpy(&this->channel, channel, sizeof(Channel));

    /* force game clients to main channel, use GameServ service for game channels */
    strcpy(this->channel.name, "#cncnet");
    this->channel.key[0] = '\0';

    irc_cmd_join(this->irc_session, this->channel.name, this->channel.key);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelLeave(IChat *this)
{
    dprintf("IChat::RequestChannelLeave(this=%p)\n", this);

    if (this->game.type)
    {
        IChatEvent_OnChannelLeave(this->ev, S_OK, &this->game, &this->user);
        memset(&this->game, 0, sizeof(Channel));
        return S_OK;
    }

    /* you don't leave the lobby channel, ever */
    IChatEvent_OnChannelLeave(this->ev, S_OK, &this->channel, &this->user);

    /*irc_cmd_part(this->irc_session, this->channel.name);*/

    return S_OK;
}

static HRESULT __stdcall IChat_RequestUserList(IChat *this)
{
    dprintf("IChat::RequestUserList(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicMessage(IChat *this, LPSTR message)
{
    dprintf("IChat::RequestPublicMessage(this=%p, message=\"%s\")\n", this, message);

    irc_cmd_msg(this->irc_session, this->channel.name, message);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateMessage(IChat *this, User* users, LPSTR message)
{
    dprintf("IChat::RequestPrivateMessage(this=%p, users=%p, message=\"%s\")\n", this, users, message);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestLogout(IChat *this)
{
    dprintf("IChat::RequestLogout(this=%p)\n", this);

    irc_cmd_quit(this->irc_session, NULL);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateGameOptions(IChat *this, User* users, LPSTR options)
{
    dprintf("IChat::RequestPrivateGameOptions(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicGameOptions(IChat *this, LPSTR options)
{
    dprintf("IChat::RequestPublicGameOptions(this=%p, options=\"%s\")\n", this, options);

    IChatEvent_OnPublicGameOptions(this->ev, S_OK, &this->game, &this->user, options);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicAction(IChat *this, LPSTR action)
{
    dprintf("IChat::RequestPublicAction(this=%p, action=\"%s\")\n", this, action);

    irc_cmd_me(this->irc_session, this->channel.name, action);

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
        *nick = "WOLGuest";
        *pass = "guest";
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

void irc_split_origin(const char *in, char *nick, char *host)
{
    int i,dpos = 0;
    char *dst = nick;
    printf("irc_split_origin(\"%s\", \"%s\", \"%s\"\n", in, nick, host);
    for (i = 0; i < strlen(in); i++)
    {
        if (in[i] == '!')
        {
            dst = host;
            dpos = 0;
        }
        else if (dst)
        {
            dst[dpos++] = in[i];
        }
    }
}

void irc_dump_event(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{                                                                                                                          
    char buf[512] = { 0 };
    int cnt;

    for (cnt = 0; cnt < count; cnt++)
    {
        if (cnt)
            strcat (buf, "|");

        strcat (buf, params[cnt]);
    }

    printf("IRC event \"%s\", origin: \"%s\", params: %d [%s]\n", event, origin ? origin : "NULL", cnt, buf);
} 

void irc_event_numeric(irc_session_t * session, unsigned int event, const char * origin, const char ** params, unsigned int count)
{
    IChat *this = irc_get_ctx(session);

    char buf[512] = { 0 };
    int cnt;

    for (cnt = 0; cnt < count; cnt++)
    {
        if (cnt)
            strcat (buf, "|");

        strcat (buf, params[cnt]);
    }

    if (event == LIBIRC_RFC_ERR_NICKNAMEINUSE)
    {
        IChatEvent_OnConnection(this->ev, CHAT_E_NICKINUSE, NULL);
        irc_cmd_quit(session, NULL);
        return;
    }

    if (event == LIBIRC_RFC_RPL_MOTD)
    {
        strncat(this->motd, params[1], sizeof(this->motd)-1);
        strncat(this->motd, "\r\n", sizeof(this->motd)-1);
    }

    if (event == LIBIRC_RFC_RPL_ENDOFMOTD)
    {
        printf(this->motd);
        IChatEvent_OnConnection(this->ev, S_OK, this->motd);
    }

    if (event == LIBIRC_RFC_RPL_NAMREPLY)
    {
        strcpy(buf, params[3]);
        char *name = strtok(buf, " ");
        do {
            if (name)
            {
                User *user = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(User));
                if (name[0] == '@' || name[0] == '+')
                    name++;
                strcpy(user->name, name);
                user_list_add(&this->lobby_users, user);
            }
        } while((name = strtok(NULL, " ")));
    }

    if (event == LIBIRC_RFC_RPL_ENDOFNAMES)
    {
        IChatEvent_OnChannelJoin(this->ev, S_OK, &this->lobby, &this->user);
        IChatEvent_OnUserList(this->ev, S_OK, &this->lobby, this->lobby_users);
    }

    printf("IRC event \"%03d\", origin: \"%s\", params: %d [%s]\n", event, origin ? origin : "NULL", cnt, buf);
}

void irc_event_join(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    IChat *this = irc_get_ctx(session);
    User user;
    memset(&user, 0, sizeof(User));

    if (this->game.type)
    {
        return;
    }

    irc_split_origin(origin, user.name, NULL);

    if (strcmp(user.name, this->name) == 0)
    {
        /* set user flags, don't know which is "me" */
        user.flags = 0xFFFFFFF;
    }

    IChatEvent_OnChannelJoin(this->ev, S_OK, &this->channel, &user);
}

void irc_event_part(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    IChat *this = irc_get_ctx(session);
    User user;

    if (this->game.type)
    {
        return;
    }

    irc_split_origin(origin, user.name, NULL);

    if (strcmp(user.name, this->name) == 0)
    {
        /* set user flags, don't know which is "me" */
        user.flags = 0xFFFFFFF;
    }

    IChatEvent_OnChannelLeave(this->ev, S_OK, &this->channel, &user);
}

void irc_event_channel(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    IChat *this = irc_get_ctx(session);

    if (this->game.type)
    {
        return;
    }

    if (strcmp(params[0], this->channel.name) == 0)
    {
        User user;
        memset(user.name, 0, sizeof(user.name));
        irc_split_origin(origin, user.name, NULL);
        IChatEvent_OnPublicMessage(this->ev, S_OK, &this->channel, &user, (LPSTR)params[1]);
    }
}

void irc_event_action(irc_session_t * session, const char * event, const char * origin, const char ** params, unsigned int count)
{
    IChat *this = irc_get_ctx(session);

    if (this->game.type)
    {
        return;
    }

    if (strcmp(params[0], this->channel.name) == 0)
    {
        User user;
        memset(user.name, 0, sizeof(user.name));
        irc_split_origin(origin, user.name, NULL);
        IChatEvent_OnPublicAction(this->ev, S_OK, &this->channel, &user, (LPSTR)params[1]);
    }
}

IChat *IChat_New()
{
    dprintf("IChat::New()\n");

    WSADATA wsaData;
    WSAStartup(0x0101, &wsaData);

    IChat *this = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IChat));
    this->lpVtbl = &Vtbl;
    IChat_AddRef(this);

    /* set self flag, don't know the correct value yet */
    this->user.flags = 0xFFFFFFF;

    /*this->irc_callbacks.event_connect = irc_event_connect;*/
    this->irc_callbacks.event_join = irc_event_join;
    this->irc_callbacks.event_nick = irc_dump_event;
    this->irc_callbacks.event_quit = irc_dump_event;
    this->irc_callbacks.event_part = irc_event_part;
    this->irc_callbacks.event_mode = irc_dump_event;
    this->irc_callbacks.event_topic = irc_dump_event;
    this->irc_callbacks.event_kick = irc_dump_event;
    this->irc_callbacks.event_channel = irc_event_channel;
    this->irc_callbacks.event_privmsg = irc_dump_event;
    this->irc_callbacks.event_notice = irc_dump_event;
    this->irc_callbacks.event_invite = irc_dump_event;
    this->irc_callbacks.event_umode = irc_dump_event;
    this->irc_callbacks.event_ctcp_rep = irc_dump_event;
    this->irc_callbacks.event_ctcp_action = irc_event_action;
    this->irc_callbacks.event_unknown = irc_dump_event;
    this->irc_callbacks.event_numeric = irc_event_numeric;

    this->irc_session = irc_create_session(&this->irc_callbacks);
    irc_set_ctx(this->irc_session, this);

    /* to get our callback ev pointer */
    IChatSingleton = this;

    return this;
}
