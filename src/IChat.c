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

/* {4DD3BAF4-7579-11D1-B1C6-006097176556} */
const GUID IID_IChat              = {0x4DD3BAF4,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};

/* {4DD3BAF6-7579-11D1-B1C6-006097176556} */
const GUID IID_IChatEvent         = {0x4DD3BAF6,0x7579,0x11D1,{0xB1,0xC6,0x00,0x60,0x97,0x17,0x65,0x56}};

IChat *IChatSingleton = NULL;

static HRESULT __stdcall _QueryInterface(IChat *this, REFIID riid, void **ppvObject)
{
    dprintf("IChat::QueryInterface(this=%p, riid={%s}, ppvObject=%p)\n", this, str_GUID(riid), ppvObject);

    if (IsEqualIID(riid, &IID_IConnectionPointContainer))
    {
        dprintf(" IConnectionPointContainer interface requested\n");
        *ppvObject = (void *)&IConnectionPointContainerSingleton;
        return S_OK;
    }

    if (IsEqualIID(riid, &IID_IChat))
    {
        dprintf(" IChat interface requested, returning self\n");
        *ppvObject = this;
        return S_OK;
    }

    return E_NOINTERFACE;
}

static ULONG __stdcall _AddRef(IChat *this)
{
    dprintf("IChat::AddRef(this=%p)\n", this);
    return ++this->ref;
}

static ULONG __stdcall _Release(IChat *this)
{
    dprintf("IChat::Release(this=%p)\n", this);
    this->ref--;

    if (this->ref == 0)
    {
        dprintf(" releasing memory\n");
        irc_close(this->irc);
        free(this);
        return 0;
    }

    return this->ref;
}

static HRESULT __stdcall _PumpMessages(IChat *this)
{
#ifdef _VERBOSE
    dprintf("IChat::PumpMessages(this=%p)\n", this);
#endif

    irc_pump(this->irc);

    return S_OK;
}

static HRESULT __stdcall _RequestServerList(IChat *this, unsigned long SKU, unsigned long current_version, LPSTR loginname, LPSTR password, int timeout)
{
    Server irc;
    Server lad;
    Server gam;

    dprintf("IChat::RequestServerList(this=%p, SKU=%08X, current_version=%08X, loginname=\"%s\", password=\"%s\", timeout=%d)\n", this, SKU, current_version, loginname, password, timeout);

    memset(&irc, 0, sizeof(Server));
    irc.gametype = SKU;
    strcpy(irc.name, "Live chat server");
    strcpy(irc.connlabel, "IRC");
    strcpy(irc.conndata, "TCP;irc.cncnet.org;5000");

    memset(&lad, 0, sizeof(Server));
    lad.gametype = SKU;
    strcpy(lad.name, "Ladder server");
    strcpy(lad.connlabel, "LAD");
    strcpy(lad.conndata, "TCP;unused;-1");

    memset(&gam, 0, sizeof(Server));
    gam.gametype = SKU;
    strcpy(gam.name, "Gameres server");
    strcpy(gam.connlabel, "GAM");
    strcpy(gam.conndata, "TCP;unused;-1");

    irc.next = &lad;
    lad.next = &gam;

    this->SKU = SKU;
    this->current_version = current_version;

    IChatEvent_OnServerList(this->ev, S_OK, &irc);

    return S_OK;
}

char *wol_apgar(const char *);

static HRESULT __stdcall _RequestConnection(IChat *this, Server* server, int timeout, int domangle)
{
    char host[128];
    int port;

    dprintf("IChat::RequestConnection(this=%p, server=%p, timeout=%d, domangle=%d)\n", this, server, timeout, domangle);

    dprintf("    name     : %s\n", server->name);
    dprintf("    connlabel: %s\n", server->connlabel);
    dprintf("    conndata : %s\n", server->conndata);
    dprintf("    login    : %s\n", server->login);
    dprintf("    password : %s\n", server->password);

    memset(host, 0, sizeof(host));

    if (sscanf(server->conndata, "TCP;%127[^;];%d", host, &port) != 2)
    {
        dprintf(" Error parsing connection data\n");
        return S_FALSE;
    }

    if (!irc_connect(this->irc, host, port))
    {
        dprintf(" Error connecting to server\n");
        return S_FALSE;
    }

    IChatEvent_OnNetStatus(this->ev, CHAT_S_CON_CONNECTED);

    irc_printf(this->irc, "CVERS %d %d", WOL_SKU, this->SKU);
    irc_printf(this->irc, "PASS %s", "supersecret");
    irc_printf(this->irc, "NICK %s", server->login);
    irc_printf(this->irc, "apgar %s 0", wol_apgar(server->password));
    irc_printf(this->irc, "");
    irc_printf(this->irc, "SERIAL %s", "");
    irc_printf(this->irc, "USER UserName HostName irc.westwood.com :RealName");
    irc_printf(this->irc, "verchk %d %d", WOL_VERSION, this->current_version);

    strcpy(this->user.name, server->login);

    return S_OK;
}

static HRESULT __stdcall _RequestChannelList(IChat *this, int channelType, int autoping)
{
    dprintf("IChat::RequestChannelList(this=%p, channelType=%d, autoping=%d)\n", this, channelType, autoping);

    irc_printf(this->irc, "LIST %d 21", channelType);

    return S_OK;
}

static HRESULT __stdcall _RequestChannelCreate(IChat *this, Channel* channel)
{
    dprintf("IChat::RequestChannelCreate(this=%p, channel=%p)\n", this, channel);

    dprintf(" Dumping channel %p\n", channel);
    dprintf("    type        : %d\n", channel->type);
    dprintf("    minUsers    : %d\n", channel->minUsers);
    dprintf("    maxUsers    : %d\n", channel->maxUsers);
    dprintf("    currentUsers: %d\n", channel->currentUsers);
    dprintf("    official    : %d\n", channel->official);
    dprintf("    tournament  : %d\n", channel->tournament);
    dprintf("    ingame      : %d\n", channel->ingame);
    dprintf("    flags       : %08X\n", channel->flags);
    dprintf("    reserved    : %08X\n", channel->reserved);
    dprintf("    ipaddr      : %u\n", channel->ipaddr);
    dprintf("    latency     : %d\n", channel->latency);
    dprintf("    hidden      : %d\n", channel->hidden);
    dprintf("    next        : %p\n", channel->next);
    dprintf("    name        : \"%s\"\n", channel->name);
    dprintf("    topic       : \"%s\"\n", channel->topic);
    dprintf("    location    : \"%s\"\n", channel->location);
    dprintf("    key         : \"%s\"\n", channel->key);
    dprintf("    exInfo      : \"%s\"\n", channel->exInfo);

    memcpy(&this->channel, channel, sizeof(Channel));

    irc_printf(this->irc, "JOINGAME #%s %d %d %d %d %d %d %u %s",
            channel->name, channel->minUsers, channel->maxUsers, channel->type, 3, 1, channel->tournament, channel->reserved, channel->key);

    return S_OK;
}

static HRESULT __stdcall _RequestChannelJoin(IChat *this, Channel* channel)
{
    dprintf("IChat::RequestChannelJoin(this=%p, channel=%p)\n", this, channel);

    if (channel->type == 0)
    {
        irc_printf(this->irc, "JOIN #%s %s", channel->name, channel->key);
    }
    else
    {
        irc_printf(this->irc, "JOINGAME #%s 1 %s", channel->name, channel->key);
    }

    return S_OK;
}

static HRESULT __stdcall _RequestChannelLeave(IChat *this)
{
    dprintf("IChat::RequestChannelLeave(this=%p)\n", this);

    irc_printf(this->irc, "PART #%s", this->channel.name);

    return S_OK;
}

static HRESULT __stdcall _RequestUserList(IChat *this)
{
    dprintf("IChat::RequestUserList(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestPublicMessage(IChat *this, LPSTR message)
{
    dprintf("IChat::RequestPublicMessage(this=%p, message=\"%s\")\n", this, message);

    irc_printf(this->irc, "PRIVMSG #%s :%s", this->channel.name, message);

    return S_OK;
}

static HRESULT __stdcall _RequestPrivateMessage(IChat *this, User* users, LPSTR message)
{
    dprintf("IChat::RequestPrivateMessage(this=%p, users=%p, message=\"%s\")\n", this, users, message);

    /* FIXME: handle multiple recipients */
    irc_printf(this->irc, "PRIVMSG %s :%s", users->name, message);

    return S_OK;
}

static HRESULT __stdcall _RequestLogout(IChat *this)
{
    dprintf("IChat::RequestLogout(this=%p)\n", this);

    irc_printf(this->irc, "QUIT");

    return S_OK;
}

void hook_gameopt(IChat *this, const char *prefix, const char *command, int argc, const char *argv[]);

static HRESULT __stdcall _RequestPrivateGameOptions(IChat *this, User* user, LPSTR options)
{
    dprintf("IChat::RequestPrivateGameOptions(this=%p, user=%p, options=\"%s\")\n", this, user, options);

    irc_printf(this->irc, "GAMEOPT %s :%s", user->name, options);

    if (this->SKU == SKU_RA303)
    {
        Gameopt *go;

        this->gameopt_sent = 1;

        WOL_LIST_FOREACH(this->gameopt_queue, go)
        {
            char *argv[2];
            argv[0] = go->argv[0];
            argv[1] = go->argv[1];
            hook_gameopt(this, go->prefix, go->command, 2, (const char **)argv);
        }

        WOL_LIST_FREE(this->gameopt_queue);
    }

    return S_OK;
}

static HRESULT __stdcall _RequestPublicGameOptions(IChat *this, LPSTR options)
{
    dprintf("IChat::RequestPublicGameOptions(this=%p, options=\"%s\")\n", this, options);

    irc_printf(this->irc, "GAMEOPT #%s :%s", this->channel.name, options);

    return S_OK;
}

static HRESULT __stdcall _RequestPublicAction(IChat *this, LPSTR action)
{
    dprintf("IChat::RequestPublicAction(this=%p, action=\"%s\")\n", this, action);

    irc_printf(this->irc, "PRIVMSG #%s :\001ACTION %s\001", this->channel.name, action);

    return S_OK;
}

static HRESULT __stdcall _RequestPrivateAction(IChat *this, User* users, LPSTR action)
{
    dprintf("IChat::RequestPrivateAction(this=%p, action=\"%s\")\n", this, action);

    /* FIXME: handle multiple recipients */
    irc_printf(this->irc, "PRIVMSG %s :\001ACTION %s\001", users->name, action);

    return S_OK;
}

static HRESULT __stdcall _RequestGameStart(IChat *this, User* users)
{
    User *user;
    char buf[128] = { 0 };

    dprintf("IChat::RequestGameStart(this=%p, users=%p)\n", this, users);

    WOL_LIST_FOREACH(users, user)
    {
        strcat(buf, user->name);
        if (user->next)
            strcat(buf, ",");
    }

    irc_printf(this->irc, "STARTG #%s %s", this->channel.name, buf);

    return S_OK;
}

static HRESULT __stdcall _RequestChannelTopic(IChat *this, LPSTR topic)
{
    dprintf("IChat::RequestChannelTopic(this=%p, topic=\"%s\")\n", this, topic);

    irc_printf(this->irc, "TOPIC #%s :%s", this->channel.name, topic);

    return S_OK;
}

static HRESULT __stdcall _GetVersion(IChat *this, unsigned long* version)
{
    dprintf("IChat::GetVersion(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestUserKick(IChat *this, User* User)
{
    dprintf("IChat::RequestUserKick(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestUserIP(IChat *this, User* user)
{
    dprintf("IChat::RequestUserIP(this=%p, user=%p)\n", this, user);
    dprintf(" name: %s\n", user->name);

    irc_printf(this->irc, "USERIP %s", user->name);

    return S_OK;
}

static HRESULT __stdcall _GetGametypeInfo(IChat *this, unsigned int gtype, int icon_size, unsigned char** bitmap, int* bmp_bytes, LPSTR* name, LPSTR* URL)
{
    unsigned char bmp[1222];

    dprintf("IChat::GetGametypeInfo(this=%p, gtype=%d, icon_size=%d, bitmap=%p, bmp_bytes=%p, name=%p, URL=%p)\n", this, gtype, icon_size, bitmap, bmp_bytes, name, URL);

    switch (gtype)
    {
        case -1:
            *name = "New Game";
            break;
        case 0:
            *name = "Chat channels";
            break;
        case 1:
            *name = "Command & Conquer Gold";
            break;
        case 2:
            *name = "C&C Red Alert";
            break;
        case 3:
            *name = "RA: Counterstrike";
            break;
        case 4:
            *name = "RA: Aftermath";
            break;
        case 5:
            *name = "C&C Sole Survivor";
            break;
        case 12:
            *name = "Commando";
            break;
        case 14:
            *name = "Dune 2000";
            break;
    }

    *URL = "http://www.westwood.com/";

    /* FIXME: handle icon correctly */
    memset(bmp, 0, sizeof(bmp));
    *bitmap = bmp;
    *bmp_bytes = 1222;

    return S_OK;
}

static HRESULT __stdcall _RequestFind(IChat *this, User* User)
{
    dprintf("IChat::RequestFind(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestPage(IChat *this, User* User, LPSTR message)
{
    dprintf("IChat::RequestPage(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _SetFindPage(IChat *this, int findOn, int pageOn)
{
    dprintf("IChat::SetFindPage(this=%p, findOn=%d, pageOn=%d)\n", this, findOn, pageOn);
    return S_OK;
}

static HRESULT __stdcall _SetSquelch(IChat *this, User* User, int squelch)
{
    dprintf("IChat::SetSquelch(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _GetSquelch(IChat *this, User* User)
{
    dprintf("IChat::GetSquelch(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _SetChannelFilter(IChat *this, int channelType)
{
    dprintf("IChat::SetChannelFilter(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestGameEnd(IChat *this)
{
    dprintf("IChat::RequestGameEnd(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _SetLangFilter(IChat *this, int onoff)
{
    dprintf("IChat::SetLangFilter(this=%p, onoff=%d)\n", this, onoff);
    return S_OK;
}

static HRESULT __stdcall _RequestChannelBan(IChat *this, LPSTR name, int ban)
{
    dprintf("IChat::RequestChannelBan(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _GetGametypeList(IChat *this, LPSTR* list)
{
    dprintf("IChat::GetGametypeList(this=%p, list=%p)\n", this, list);
    /* comma separated list of game ids, see GetGametypeInfo */
    *list = "1,2,3,4,5,12,14";
    return S_OK;
}

static HRESULT __stdcall _GetHelpURL(IChat *this, LPSTR* URL)
{
    dprintf("IChat::GetHelpURL(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _SetProductSKU(IChat *this, unsigned long SKU)
{
    dprintf("IChat::SetProductSKU(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _GetNick(IChat *this, int num, LPSTR* nick, LPSTR* pass)
{
    dprintf("IChat::GetNick(this=%p, num=%d, nick=%p, pass=%p)\n", this, num, nick, pass);
    return S_FALSE;
}

static HRESULT __stdcall _SetNick(IChat *this, int num, LPSTR nick, LPSTR pass, int domangle)
{
    dprintf("IChat::SetNick(this=%p, nick=\"%s\", pass=\"%s\", domangle=%d)\n", this, nick, pass, domangle);
    return S_OK;
}

static HRESULT __stdcall _GetLobbyCount(IChat *this, int* count)
{
    dprintf("IChat::GetLobbyCount(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestRawMessage(IChat *this, LPSTR ircmsg)
{
    dprintf("IChat::RequestRawMessage(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _GetAttributeValue(IChat *this, LPSTR attrib, LPSTR* value)
{
    dprintf("IChat::GetAttributeValue(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _SetAttributeValue(IChat *this, LPSTR attrib, LPSTR value)
{
    dprintf("IChat::SetAttributeValue(this=%p, attrib=\"%s\", value=\"%s\")\n", this, attrib, value);
    return S_OK;
}

static HRESULT __stdcall _SetChannelExInfo(IChat *this, LPSTR info)
{
    dprintf("IChat::SetChannelExInfo(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _StopAutoping(IChat *this)
{
    dprintf("IChat::StopAutoping(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall _RequestSquadInfo(IChat *this, unsigned long id)
{
    dprintf("IChat::RequestSquadInfo(this=%p, ...)\n", this);
    return S_OK;
}

static IChatVtbl Vtbl =
{
    /* IUnknown */
    _QueryInterface,
    _AddRef,
    _Release,

    /* IChat */
    _PumpMessages,
    _RequestServerList,
    _RequestConnection,
    _RequestChannelList,
    _RequestChannelCreate,
    _RequestChannelJoin,
    _RequestChannelLeave,
    _RequestUserList,
    _RequestPublicMessage,
    _RequestPrivateMessage,
    _RequestLogout,
    _RequestPrivateGameOptions,
    _RequestPublicGameOptions,
    _RequestPublicAction,
    _RequestPrivateAction,
    _RequestGameStart,
    _RequestChannelTopic,
    _GetVersion,
    _RequestUserKick,
    _RequestUserIP,
    _GetGametypeInfo,
    _RequestFind,
    _RequestPage,
    _SetFindPage,
    _SetSquelch,
    _GetSquelch,
    _SetChannelFilter,
    _RequestGameEnd,
    _SetLangFilter,
    _RequestChannelBan,
    _GetGametypeList,
    _GetHelpURL,
    _SetProductSKU,
    _GetNick,
    _SetNick,
    _GetLobbyCount,
    _RequestRawMessage,
    _GetAttributeValue,
    _SetAttributeValue,
    _SetChannelExInfo,
    _StopAutoping,
    _RequestSquadInfo
};

void hook_motdstart(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    this->motd[0] = '\0';

    if (argc < 2)
        return;

    strcpy(this->motd, argv[1]);
    strcat(this->motd, "\n");
}

void hook_motd(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc < 2)
        return;

    strcat(this->motd, argv[1]);
    strcat(this->motd, "\n");
}

void hook_connect(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    IChatEvent_OnConnection(this->ev, S_OK, this->motd);
}

void hook_badpass(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    IChatEvent_OnConnection(this->ev, CHAT_E_BADPASS, this->motd);
}

void hook_nicknameinuse(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    IChatEvent_OnConnection(this->ev, CHAT_E_NICKINUSE, this->motd);
}

void hook_liststart(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    WOL_LIST_FREE(this->channels);
}

void hook_listlobby(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc > 4 && argv[1][0] == '#')
    {
        Channel *channel = calloc(1, sizeof(Channel));
        strcpy(channel->name, argv[1] + 1);
        channel->currentUsers = atoi(argv[2]);
        channel->official = atoi(argv[3]);
        channel->flags = atoi(argv[4]);
        WOL_LIST_INSERT(this->channels, channel);
    }
}

void hook_listgame(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    Channel *channel;

    if (argc < 9)
        return;

    channel = calloc(1, sizeof(Channel));

    strcpy(channel->name, argv[1]+1);
    channel->currentUsers = atoi(argv[2]);
    channel->maxUsers = atoi(argv[3]);
    channel->type = atoi(argv[4]);
    channel->tournament = atoi(argv[5]);
    channel->reserved = atoi(argv[6]);
    channel->ipaddr = atol(argv[7]);

    sscanf(argv[8], "%u::%s", &channel->flags, channel->topic);

    /* RA hack: read in the topic variables mainly for maxUsers, I don't know how minUsers is used */
    sscanf(channel->topic, "%*1[gG]%1u%1u", &channel->minUsers, &channel->maxUsers);

    WOL_LIST_INSERT(this->channels, channel);
}

void hook_listend(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    IChatEvent_OnChannelList(this->ev, S_OK, this->channels);
}

void hook_join(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    Channel channel;
    User user;

    memset(&channel, 0, sizeof(Channel));
    memset(&user, 0, sizeof(User));

    irc_parse_prefix(prefix, user.name, NULL);

    if (argc > 0)
    {
        if (sscanf(argv[0], "%*f,%*f #%s", channel.name))
        {
            /* check if we joined */
            if (strcmp(user.name, this->user.name) == 0)
            {
                user.flags = CHAT_USER_MYSELF;
                strcpy(this->channel.name, channel.name);
            }

            IChatEvent_OnChannelJoin(this->ev, S_OK, &channel, &user);
        }
    }
}

void hook_part(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    Channel channel;
    User user;

    memset(&channel, 0, sizeof(Channel));
    memset(&user, 0, sizeof(User));

    if (argc > 0)
    {
        strcpy(channel.name, argv[0]+1);
        irc_parse_prefix(prefix, user.name, NULL);

        strcpy(this->channel.name, channel.name);

        if (strcmp(user.name, this->user.name) == 0)
        {
            user.flags = CHAT_USER_MYSELF;
            this->gameopt_sent = 0;
        }

        IChatEvent_OnChannelLeave(this->ev, S_OK, &channel, &user);
    }
}

void hook_error(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc > 0)
    {
        IChatEvent_OnServerError(this->ev, CHAT_E_UNKNOWNRESPONSE, (char *)argv[0]);
    }
}

void hook_ping(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc > 0)
    {
        irc_printf(this->irc, "PONG :%s", argv[0]);
    }
}

void hook_privmsg(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    User user;
    char message[512];
    int action = 0;

    if (argc < 2)
        return;

    if ((action = sscanf(argv[1], "\001ACTION %511[^\001]\001", message)) < 1)
    {
        strcpy(message, argv[1]);
    }

    memset(&user, 0, sizeof(User));
    irc_parse_prefix(prefix, user.name, NULL);

    if (argv[0][0] == '#')
    {
        Channel channel;
        memset(&channel, 0, sizeof(Channel));
        strcpy(this->channel.name, argv[0]+1);

        if (action)
        {
            IChatEvent_OnPublicAction(this->ev, S_OK, &channel, &user, message);
        }
        else
        {
            IChatEvent_OnPublicMessage(this->ev, S_OK, &channel, &user, message);
        }
    }
    else
    {
        if (action)
        {
            IChatEvent_OnPrivateAction(this->ev, S_OK, &user, message);
        }
        else
        {
            IChatEvent_OnPrivateMessage(this->ev, S_OK, &user, message);
        }
    }
}

void hook_userip(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    User user;

    if (argc < 2)
        return;

    memset(&user, 0, sizeof(User));
    strcpy(user.name, argv[0]);
    user.ipaddr = inet_addr(argv[1]);

    IChatEvent_OnUserIP(this->ev, S_OK, &user);
}

void hook_gameopt(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    User user;
    memset(&user, 0, sizeof(User));

    if (argc < 2)
        return;

    if (this->SKU == SKU_RA303 && !this->gameopt_sent)
    {
        Gameopt *go = WOL_LIST_NEW(Gameopt);

        strcpy(go->prefix, prefix);
        strcpy(go->command, command);
        strcpy(go->argv[0], argv[0]);
        strcpy(go->argv[1], argv[1]);

        WOL_LIST_INSERT(this->gameopt_queue, go);
        dprintf("hook_gameopt: no gameopt was yet sent so postponing this\n");
        return;
    }

    irc_parse_prefix(prefix, user.name, NULL);

    if (argv[0][0] == '#')
    {
        Channel channel;
        memset(&channel, 0, sizeof(Channel));
        strcpy(this->channel.name, argv[0]+1);
        IChatEvent_OnPublicGameOptions(this->ev, S_OK, &channel, &user, (char *)argv[1]);
    }
    else
    {
        IChatEvent_OnPrivateGameOptions(this->ev, S_OK, &user, (char *)argv[1]);
    }
}

void hook_joingame(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    User user;
    char buf[32];

    memset(&user, 0, sizeof(User));

    if (argc < 8)
        return;

    irc_parse_prefix(prefix, user.name, NULL);

    if (strcmp(user.name, this->user.name) == 0)
    {
        user.flags = CHAT_USER_MYSELF;
    }

    sprintf(buf, "#%s's_game", this->user.name);
    strcpy(this->channel.name, argv[7]+1);

    if (user.flags & CHAT_USER_MYSELF && strcmp(argv[7], buf) == 0)
    {
        this->channel.minUsers = atoi(argv[0]);
        this->channel.maxUsers = atoi(argv[1]);
        this->channel.currentUsers = 1;
        this->channel.type = atoi(argv[2]);
        this->channel.tournament = atoi(argv[3]);
        this->channel.ipaddr = atol(argv[5]);

        IChatEvent_OnChannelCreate(this->ev, S_OK, &this->channel);

        this->gameopt_sent = 1;

        /* RA hack, create our topic */
        sprintf(this->channel.topic, "%c%d%d%d", this->channel.ingame ? 'G' : 'g', this->channel.minUsers, this->channel.maxUsers, 0 /* FIXME */);
        IChat_RequestChannelTopic(this, this->channel.topic);
    }
    else
    {
        IChatEvent_OnChannelJoin(this->ev, S_OK, &this->channel, &user);
    }
}

void hook_namreply(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc > 3)
    {
        char *names = wol_strdup(argv[3]);
        char *ptr = strtok(names, " ");

        do
        {
            char name[16];
            unsigned int ipaddr;

            if (sscanf(ptr, "%15[^,],%*u,%u", name, &ipaddr))
            {
                User *user = WOL_LIST_NEW(User);

                if (name[0] == '@')
                {
                    strncpy(user->name, name+1, 10);
                    user->flags = CHAT_USER_CHANNELOWNER;
                }
                else
                    strncpy(user->name, name, 10);

                if (strcmp(user->name, this->user.name) == 0)
                {
                    user->flags |= CHAT_USER_MYSELF;
                }

                user->ipaddr = ipaddr;

                WOL_LIST_INSERT(this->users, user);
            }

            ptr = strtok(NULL, " ");
        } while(ptr);

        free(names);
    }
}

void hook_endofnames(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    IChatEvent_OnUserList(this->ev, S_OK, &this->channel, this->users);
    WOL_LIST_FREE(this->users);
}

void hook_notopic(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc < 2)
        return;

    IChatEvent_OnChannelTopic(this->ev, S_OK, &this->channel, "");
}

void hook_topic(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc < 2)
        return;

    IChatEvent_OnChannelTopic(this->ev, S_OK, &this->channel, (char *)argv[1]);
}

void hook_startg(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    int i,gameid,now;

    char buf[512];
    char users[512];
    char *p;

    User *list = NULL;
    User *user = NULL;

    if (argc < 2)
        return;

    strcpy(buf, argv[1]);

    memset(users, 0, sizeof(users));

    if (sscanf(buf, "%511[^:]:%d %d", users, &gameid, &now) < 3)
        return;

    p = strtok(users, " ");
    i = 0;
    do {
        if (strlen(p) < 2)
            break;

        if (i % 2 == 0)
        {
            user = WOL_LIST_NEW(User);
            WOL_LIST_INSERT(list, user);
            strcpy(user->name, p);
            if (strcmp(user->name, this->user.name) == 0)
            {
                user->flags |= CHAT_USER_MYSELF;
            }
            else
            {
                user->flags |= CHAT_USER_CHANNELOWNER;
            }
        }
        else
        {
            user->ipaddr = inet_addr(p);
        }

        p = strtok(NULL, " ");
        i++;
    } while(p);

    IChatEvent_OnGameStart(this->ev, S_OK, &this->channel, list, gameid);

    WOL_LIST_FREE(list);
}

void hook_debug(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    int i;
    dprintf("IRC <- %s, %s", prefix, command);
    for (i = 0; i < argc; i++)
        dprintf(", \"%s\"", argv[i]);
    dprintf("\n");
}

void hook_disconnect(IChat *this, const char *prefix, const char *command, int argc, char *argv[])
{
    dprintf("IRC disconnected\n");
    IChatEvent_OnNetStatus(this->ev, CHAT_S_CON_DISCONNECTED);
}

IChat *IChat_New()
{
    IChat *this = calloc(1, sizeof(IChat));
    WSADATA wsaData;
    WSAStartup(0x0101, &wsaData);

    this->lpVtbl = &Vtbl;
    dprintf("IChat::New()\n");
    _AddRef(this);

    /* set self flag, don't know the correct value yet */
    this->user.flags = CHAT_USER_MYSELF;
    this->irc = irc_create((void *)this);
    this->irc->disconnect = (irc_callback)hook_disconnect;

    irc_hook_add(this->irc, "*", (irc_callback)hook_debug);
    irc_hook_add(this->irc, WOL_RPL_LISTSTART, (irc_callback)hook_liststart);
    irc_hook_add(this->irc, WOL_RPL_LISTGAME, (irc_callback)hook_listgame);
    irc_hook_add(this->irc, WOL_RPL_LISTLOBBY, (irc_callback)hook_listlobby);
    irc_hook_add(this->irc, WOL_RPL_LISTEND, (irc_callback)hook_listend);
    irc_hook_add(this->irc, WOL_RPL_NAMREPLY, (irc_callback)hook_namreply);
    irc_hook_add(this->irc, WOL_RPL_ENDOFNAMES, (irc_callback)hook_endofnames);
    irc_hook_add(this->irc, WOL_RPL_NOTOPIC, (irc_callback)hook_notopic);
    irc_hook_add(this->irc, WOL_RPL_TOPIC, (irc_callback)hook_topic);
    irc_hook_add(this->irc, WOL_RPL_MOTDSTART, (irc_callback)hook_motdstart);
    irc_hook_add(this->irc, WOL_RPL_MOTD, (irc_callback)hook_motd);
    irc_hook_add(this->irc, WOL_RPL_ENDOFMOTD, (irc_callback)hook_connect);
    irc_hook_add(this->irc, WOL_RPL_BADPASS, (irc_callback)hook_badpass);
    irc_hook_add(this->irc, WOL_ERR_NOMOTD, (irc_callback)hook_connect);
    irc_hook_add(this->irc, WOL_ERR_NICKNAMEINUSE, (irc_callback)hook_nicknameinuse);
    irc_hook_add(this->irc, "ERROR", (irc_callback)hook_error);
    irc_hook_add(this->irc, "PING", (irc_callback)hook_ping);
    irc_hook_add(this->irc, "PRIVMSG", (irc_callback)hook_privmsg);
    irc_hook_add(this->irc, "JOIN", (irc_callback)hook_join);
    irc_hook_add(this->irc, "JOINGAME", (irc_callback)hook_joingame);
    irc_hook_add(this->irc, "PART", (irc_callback)hook_part);
    irc_hook_add(this->irc, "USERIP", (irc_callback)hook_userip);
    irc_hook_add(this->irc, "GAMEOPT", (irc_callback)hook_gameopt);
    irc_hook_add(this->irc, "STARTG", (irc_callback)hook_startg);

    /* to get our callback ev pointer */
    IChatSingleton = this;

    return this;
}
