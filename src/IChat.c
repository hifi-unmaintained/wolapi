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

    if (IsEqualIID(riid, &IID_IChat))
    {
        dprintf(" IChat interface requested, returning self\n");
        *ppvObject = this;
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

    irc_pump(this->irc);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestServerList(IChat *this, unsigned long SKU, unsigned long current_version, LPSTR loginname, LPSTR password, int timeout)
{
    dprintf("IChat::RequestServerList(this=%p, SKU=%08X, current_version=%08X, loginname=\"%s\", password=\"%s\", timeout=%d)\n", this, SKU, current_version, loginname, password, timeout);

    Server srv;
    memset(&srv, 0, sizeof(Server));
    srv.gametype = SKU;
    strcpy(srv.name, "WOLAPI stub");
    strcpy(srv.connlabel, "IRC");
    strcpy(srv.conndata, "TCP:localhost:4000");
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

    char host[128];
    int port;
    memset(host, 0, sizeof(host));

    if (sscanf(server->conndata, "TCP:%127[^:]:%d", host, &port) != 2)
    {
        dprintf(" Error parsing connection data\n");
        return S_FALSE;
    }

    if (!irc_connect(this->irc, host, port))
    {
        dprintf(" Error connecting to server\n");
        return S_FALSE;
    }

    irc_printf(this->irc, "CVERS %d %d", 11020, 5376);
    irc_printf(this->irc, "PASS %s", "supersecret");
    irc_printf(this->irc, "NICK %s", server->login);
    irc_printf(this->irc, "apgar %s 0", "abcdefg");
    irc_printf(this->irc, "");
    irc_printf(this->irc, "SERIAL %s", "");
    irc_printf(this->irc, "USER UserName HostName irc.westwood.com :RealName");
    irc_printf(this->irc, "verchk %d %d", 32512, 270916);

    strcpy(this->user.name, server->login);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelList(IChat *this, int channelType, int autoping)
{
    dprintf("IChat::RequestChannelList(this=%p, channelType=%d, autoping=%d)\n", this, channelType, autoping);

    irc_printf(this->irc, "LIST %d 21", channelType);

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

    memcpy(&this->channel, channel, sizeof(Channel));

    this->creating_channel = 1;

    irc_printf(this->irc, "JOINGAME #%s %d %d %d %d %d %d %u %s",
            channel->name, channel->minUsers, channel->maxUsers, channel->type, 3, 1, channel->tournament, channel->reserved, channel->key);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelJoin(IChat *this, Channel* channel)
{
    dprintf("IChat::RequestChannelJoin(this=%p, channel=%p)\n", this, channel);

    irc_printf(this->irc, "JOINGAME #%s 1 %s", channel->name, channel->key);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelLeave(IChat *this)
{
    dprintf("IChat::RequestChannelLeave(this=%p)\n", this);

    irc_printf(this->irc, "PART #%s", this->channel.name);

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

    irc_printf(this->irc, "PRIVMSG #%s :%s", this->channel.name, message);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateMessage(IChat *this, User* users, LPSTR message)
{
    dprintf("IChat::RequestPrivateMessage(this=%p, users=%p, message=\"%s\")\n", this, users, message);

    /* FIXME: handle multiple recipients */
    irc_printf(this->irc, "PRIVMSG %s :%s", users->name, message);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestLogout(IChat *this)
{
    dprintf("IChat::RequestLogout(this=%p)\n", this);

    irc_printf(this->irc, "QUIT");

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateGameOptions(IChat *this, User* user, LPSTR options)
{
    dprintf("IChat::RequestPrivateGameOptions(this=%p, user=%p, options=\"%s\")\n", this, user, options);

    irc_printf(this->irc, "GAMEOPT %s :%s", user->name, options);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicGameOptions(IChat *this, LPSTR options)
{
    dprintf("IChat::RequestPublicGameOptions(this=%p, options=\"%s\")\n", this, options);

    irc_printf(this->irc, "GAMEOPT #%s :%s", this->channel.name, options);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPublicAction(IChat *this, LPSTR action)
{
    dprintf("IChat::RequestPublicAction(this=%p, action=\"%s\")\n", this, action);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateAction(IChat *this, User* users, LPSTR action)
{
    dprintf("IChat::RequestPrivateAction(this=%p, ...)\n", this);
    return S_OK;
}

static HRESULT __stdcall IChat_RequestGameStart(IChat *this, User* users)
{
    dprintf("IChat::RequestGameStart(this=%p, users=%p)\n", this, users);

    User *user = users;
    char buf[128] = { 0 };

    while (user)
    {
        strcat(buf, user->name);
        user = user->next;
        if (user)
            strcat(buf, ",");
    }

    irc_printf(this->irc, "STARTG #%s %s", this->channel.name, buf);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelTopic(IChat *this, LPSTR topic)
{
    dprintf("IChat::RequestChannelTopic(this=%p, topic=\"%s\")\n", this);

    irc_printf(this->irc, "TOPIC #%s :%s", this->channel.name, topic);

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

static HRESULT __stdcall IChat_RequestUserIP(IChat *this, User* user)
{
    dprintf("IChat::RequestUserIP(this=%p, user=%p)\n", this, user);
    dprintf(" name: %s\n", user->name);

    irc_printf(this->irc, "USERIP %s", user->name);

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

void hook_connect(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    dprintf("IChatEvent_OnConnection(%p, ...)\n", this->ev);
    IChatEvent_OnConnection(this->ev, S_OK, "Connected.");
}

void hook_liststart(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    channel_list_free(&this->channels);
}

void hook_list(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc > 2 && argv[1][0] == '#')
    {
        Channel *channel = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Channel));
        strcpy(channel->name, argv[1] + 1);
        channel->currentUsers = atoi(argv[2]);
        channel_list_add(&this->channels, channel);
    }
}

void hook_listgame(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc < 9)
        return;

    Channel *channel = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Channel));

    strcpy(channel->name, argv[1]+1);
    channel->currentUsers = atoi(argv[2]);
    channel->maxUsers = atoi(argv[3]);
    channel->type = atoi(argv[4]);
    channel->tournament = atoi(argv[5]);
    channel->reserved = atoi(argv[6]);
    channel->ipaddr = atol(argv[7]);

    sscanf(argv[8], "%u::%s", &channel->flags, channel->topic);

    channel_list_add(&this->channels, channel);
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
        }

        IChatEvent_OnChannelLeave(this->ev, S_OK, &channel, &user);
    }
}

void hook_ping(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    if (argc > 0)
    {
        irc_printf(this->irc, "PONG :%s", argv[0]);
    }
}

void hook_userip(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    /* currently as of 2011-12-27 PvPGN is faulty and returns only the ip, not the user */
    if (argc < 2)
        return;

    User user;
    memset(&user, 0, sizeof(User));
    strcpy(user.name, argv[0]);
    user.ipaddr = atoi(argv[1]);

    //IChatEvent_OnUserIP(this->ev, S_OK, &user);
}

void hook_gameopt(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    User user;
    memset(&user, 0, sizeof(User));

    if (argc < 2)
        return;

    irc_parse_prefix(prefix, user.name, NULL);

    if (argv[0][0] == '#')
    {
        Channel channel;
        memset(&channel, 0, sizeof(Channel));
        strcpy(channel.name, argv[0]+1);
        dprintf("public options from %s to %s: %s\n", user.name, channel.name, argv[1]);
        IChatEvent_OnPublicGameOptions(this->ev, S_OK, &channel, &user, (char *)argv[1]);
    }
    else
    {
        dprintf("private options from %s to %s: %s\n", user.name, argv[0], argv[1]);
        IChatEvent_OnPrivateGameOptions(this->ev, S_OK, &user, (char *)argv[1]);
    }
}

void hook_joingame(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    User user;
    memset(&user, 0, sizeof(User));

    if (argc < 8)
        return;

    irc_parse_prefix(prefix, user.name, NULL);

    if (strcmp(user.name, this->user.name) == 0)
    {
        user.flags = CHAT_USER_MYSELF;

        this->channel.minUsers = atoi(argv[0]);
        this->channel.maxUsers = atoi(argv[1]);
        this->channel.type = atoi(argv[2]);
        this->channel.tournament = atoi(argv[3]);
        this->channel.ipaddr = atol(argv[5]);
        strcpy(this->channel.name, argv[7]+1);
    }

    if (this->creating_channel)
    {
        IChatEvent_OnChannelCreate(this->ev, S_OK, &this->channel);
    }
    else
    {
        IChatEvent_OnChannelJoin(this->ev, S_OK, &this->channel, &user);
    }

    this->creating_channel = 0;
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
                User *user = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(User));

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

                user_list_add(&this->users, user);
            }

            ptr = strtok(NULL, " ");
        } while(ptr);

        free(names);
    }
}

void hook_endofnames(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    IChatEvent_OnUserList(this->ev, S_OK, &this->channel, this->users);
    user_list_free(&this->users);
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

    User *list = NULL;
    User *user = NULL;

    if (argc < 2)
        return;

    strcpy(buf, argv[1]);

    memset(users, 0, sizeof(users));

    if (sscanf(buf, "%511[^:]:%d %d", users, &gameid, &now) < 3)
        return;

    dprintf("gameid: %d, now: %d, users: \"%s\"\n", gameid, now, users);

    char *p = strtok(users, " ");
    i = 0;
    do {
        if (strlen(p) < 2)
            break;

        if (i % 2 == 0)
        {
            user = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(User));
            user_list_add(&list, user);
            strcpy(user->name, p);
            dprintf("new user name: \"%s\"\n", p);
            if (strcmp(user->name, this->user.name) == 0)
            {
                dprintf("setting self flag\n");
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
            dprintf("and his ip: \"%s\" (%ul)\n", p, inet_addr(p));
        }

        p = strtok(NULL, " ");
        i++;
    } while(p);

    dprintf("calling ongamestart\n");

    printf("list: %p, first: %s, second: %s, third: %p\n", list, list->name, list->next->name, list->next->next);

    IChatEvent_OnGameStart(this->ev, S_OK, &this->channel, list, gameid);

    user_list_free(&list);
}

void hook_debug(IChat *this, const char *prefix, const char *command, int argc, const char *argv[])
{
    int i;
    dprintf("IRC <- %s, %s", prefix, command);
    for (i = 0; i < argc; i++)
        dprintf(", \"%s\"", argv[i]);
    dprintf("\n");
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
    this->user.flags = CHAT_USER_MYSELF;
    this->irc = irc_create((void *)this);

    irc_hook_add(this->irc, "*", (irc_callback)hook_debug);
    irc_hook_add(this->irc, WOL_RPL_ENDOFMOTD, (irc_callback)hook_connect);
    irc_hook_add(this->irc, WOL_RPL_LISTSTART, (irc_callback)hook_liststart);
    irc_hook_add(this->irc, WOL_RPL_LISTGAME, (irc_callback)hook_listgame);
    irc_hook_add(this->irc, WOL_RPL_LIST, (irc_callback)hook_list);
    irc_hook_add(this->irc, WOL_RPL_LISTEND, (irc_callback)hook_listend);
    irc_hook_add(this->irc, WOL_RPL_NAMREPLY, (irc_callback)hook_namreply);
    irc_hook_add(this->irc, WOL_RPL_ENDOFNAMES, (irc_callback)hook_endofnames);
    irc_hook_add(this->irc, WOL_RPL_NOTOPIC, (irc_callback)hook_notopic);
    irc_hook_add(this->irc, WOL_RPL_TOPIC, (irc_callback)hook_topic);
    irc_hook_add(this->irc, "PING", (irc_callback)hook_ping);
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
