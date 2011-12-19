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
        HeapFree(GetProcessHeap(), 0, tmp);
    }
    *list = NULL;
}

void channel_list_add(Channel **list, Channel *channel)
{
    Channel *current = *list;

    if (*list == NULL)
    {
        *list = channel;
        return;
    }

    while (current->next)
    {
        current = current->next;
    }

    current->next = channel;
}

void channel_list_free(Channel **list)
{
    Channel *current = *list;
    while (current)
    {
        Channel *tmp = current;
        current = current->next;
        HeapFree(GetProcessHeap(), 0, tmp);
    }
    *list = NULL;
}

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

void irc_printf(IChat *this, const char *fmt, ...)
{
    va_list args;
    char buf[512];

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    printf("-> %s\n", buf);

    strncat(buf, "\r\n", sizeof(buf));
    send(this->s, buf, strlen(buf), 0);
}

void irc_process_line(IChat *this, const char *line)
{
    char origin[128];
    char cmd[32];
    char target[128];
    char params[512];

    memset(origin, 0, sizeof(origin));
    memset(cmd, 0, sizeof(cmd));
    memset(target, 0, sizeof(target));
    memset(params, 0, sizeof(params));

    if (sscanf(line, ":%127s %31s %127s %511c", origin, cmd, target, params) == 4
        || sscanf(line, ":%127s %31s %511c", origin, cmd, params) == 3
        || sscanf(line, "%31s :%511c", cmd, params) == 2)
    {
        int icmd = atoi(cmd);

        if (icmd == WOL_RPL_ENDOFMOTD)
        {
            dprintf("IChatEvent_OnConnection(%p, ...)\n", this->ev);
            IChatEvent_OnConnection(this->ev, S_OK, "Connected.");
        }
        else if (icmd == WOL_RPL_LISTSTART)
        {
            channel_list_free(&this->channels);
        }
        else if (icmd == WOL_RPL_LIST)
        {
            Channel *channel = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Channel));
            if (sscanf(params, "#%s %*d %*d %*d", channel->name))
            {
                channel_list_add(&this->channels, channel);
            }
            else
            {
                HeapFree(GetProcessHeap(), 0, channel);
            }
        }
        else if (icmd == WOL_RPL_LISTGAME)
        {
            printf("WOL_RPL_LISTGAME: %s\n", params);
            Channel *channel = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Channel));
            if (sscanf(params, "#%s %u %u %d %u %lu %*u %*d::%80c", channel->name, &channel->currentUsers, &channel->maxUsers, &channel->type, &channel->tournament, &channel->reserved, channel->topic))
            {
                channel_list_add(&this->channels, channel);
            }
            else
            {
                HeapFree(GetProcessHeap(), 0, channel);
            }
        }
        else if (icmd == WOL_RPL_LISTEND)
        {
            IChatEvent_OnChannelList(this->ev, S_OK, this->channels);
        }
        else if (icmd == WOL_RPL_TOPIC)
        {
            /* ignore TOPIC */
        }
        else if (icmd == WOL_RPL_NAMREPLY)
        {
            char buf[512];
            memset(buf, 0, sizeof(buf));
            if (sscanf(params, "%*c #%*s :%511c", buf) == 1)
            {
                char *p = strtok(buf, " ");
                while (p)
                {
                    char name[16];
                    memset(name, 0, sizeof(name));

                    User *user = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(User));
                    if (sscanf(p, "%15[^,],%*u,%*u", name))
                    {
                        if (name[0] == '@')
                            strncpy(user->name, name+1, 10);
                        else
                            strncpy(user->name, name, 10);

                        /* FIXME: use correct self flag */
                        if (strcmp(user->name, this->user.name) == 0)
                        {
                            user->flags = 0xFFFFFFFF;
                        }

                        user_list_add(&this->users, user);
                    }
                    else
                    {
                        HeapFree(GetProcessHeap(), 0, user);
                    }
                    p = strtok(NULL, " ");
                }
            }
        }
        else if (icmd == WOL_RPL_ENDOFNAMES)
        {
            IChatEvent_OnUserList(this->ev, S_OK, &this->channel, this->users);
            user_list_free(&this->users);
        }
        else if (strcmp(cmd, "JOINGAME") == 0)
        {
            this->channel.minUsers = atoi(target);
            if (sscanf(params, "%u %d %*d %*d %*d %u :#%16s", &this->channel.maxUsers, &this->channel.type, &this->channel.tournament, this->channel.name))
            {
                IChatEvent_OnChannelCreate(this->ev, S_OK, &this->channel);
            }
        }
        else if (strcmp(cmd, "PART") == 0)
        {
            Channel channel;
            User user;

            memset(&channel, 0, sizeof(Channel));
            memset(&user, 0, sizeof(User));

            strcpy(channel.name, params+1);
            irc_split_origin(origin, user.name, NULL);

            /* FIXME: use correct self flags */
            if (strcmp(user.name, this->user.name) == 0)
            {
                user.flags = 0xFFFFFFFF;
            }

            IChatEvent_OnChannelLeave(this->ev, S_OK, &channel, &user);
        }
        else if (strcmp(cmd, "JOIN") == 0)
        {
            Channel channel;
            User user;

            memset(&channel, 0, sizeof(Channel));
            memset(&user, 0, sizeof(User));

            strcpy(channel.name, params+1);
            irc_split_origin(origin, user.name, NULL);

            strcpy(this->channel.name, channel.name);

            /* FIXME: use correct self flags */
            if (strcmp(user.name, this->user.name) == 0)
            {
                user.flags = 0xFFFFFFFF;
            }

            IChatEvent_OnChannelJoin(this->ev, S_OK, &channel, &user);
        }
        else if (strcmp(cmd, "PING") == 0)
        {
            irc_printf(this, "PONG :%s", params);
        }
        else
        {
            dprintf("IRC: Unhandled cmd \"%s\" from \"%s\" to \"%s\" with params \"%s\"\n", cmd, origin, target, params);
        }
    }
    else
    {
        dprintf("IRC: Failed to process line: %s\n", line);
    }
}

void irc_process_input(IChat *this, const char *buf)
{
    static char tbuf[1024] = { 0 };
    int i,len;
    char *ptr = tbuf;

    strncat(tbuf, buf, sizeof(tbuf));
    len = strlen(tbuf);

    for (i = 0; i < len; i++)
    {
        if (tbuf[i] == '\n')
        {
            if (i > 0)
                tbuf[i-1] = '\0'; /* be RFC compliant or gtfo */
            irc_process_line(this, ptr);
            ptr = tbuf+i+1;
        }
    }

    strncpy(tbuf, ptr, sizeof(tbuf));
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
    struct timeval tv = { 0, 0 };
    fd_set in_set;

    FD_ZERO(&in_set);

    FD_SET(this->s, &in_set);

    if (select(this->s + 1, &in_set, NULL, NULL, &tv) > 0)
    {
        char buf[512];
        int len = recv(this->s, buf, sizeof(buf)-1, 0);
        buf[len] = '\0';
        irc_process_input(this, buf);
    }

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

    struct sockaddr_in addr;
    struct hostent *hent;
    hent = gethostbyname(host);

    if (!hent)
    {
        dprintf(" Error resolving server host\n");
        return S_FALSE;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *(int *)hent->h_addr_list[0];
    addr.sin_port = htons(port);

    this->s = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(this->s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0)
    {
        dprintf(" Error connecting to server\n");
        return S_FALSE;
    }

    irc_printf(this, "CVERS %d %d", 11020, 5376);
    irc_printf(this, "PASS %s", "supersecret");
    irc_printf(this, "NICK %s", server->login);
    irc_printf(this, "apgar %s 0", "abcdefg");
    irc_printf(this, "");
    irc_printf(this, "SERIAL %s", "");
    irc_printf(this, "USER UserName HostName irc.westwood.com :RealName");
    irc_printf(this, "verchk %d %d", 32512, 270916);

    strcpy(this->user.name, server->login);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelList(IChat *this, int channelType, int autoping)
{
    dprintf("IChat::RequestChannelList(this=%p, channelType=%d, autoping=%d)\n", this, channelType, autoping);

    irc_printf(this, "LIST %d 21", channelType);

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

    irc_printf(this, "JOINGAME #%s %d %d %d %d %d %d %u %s",
            channel->name, channel->minUsers, channel->maxUsers, channel->type, 0, 0, channel->tournament, channel->reserved, channel->key);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelJoin(IChat *this, Channel* channel)
{
    dprintf("IChat::RequestChannelJoin(this=%p, channel=%p)\n", this, channel);

    /* FIXME: game channel join is in different format */
    irc_printf(this, "JOINGAME #%s 1 zotclot9", channel->name);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestChannelLeave(IChat *this)
{
    dprintf("IChat::RequestChannelLeave(this=%p)\n", this);

    irc_printf(this, "PART #%s", this->channel.name);

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

    irc_printf(this, "PRIVMSG #%s :%s", this->channel.name, message);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestPrivateMessage(IChat *this, User* users, LPSTR message)
{
    dprintf("IChat::RequestPrivateMessage(this=%p, users=%p, message=\"%s\")\n", this, users, message);

    /* FIXME: handle multiple recipients */
    irc_printf(this, "PRIVMSG %s :%s", users->name, message);

    return S_OK;
}

static HRESULT __stdcall IChat_RequestLogout(IChat *this)
{
    dprintf("IChat::RequestLogout(this=%p)\n", this);

    irc_printf(this, "QUIT");

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

static HRESULT __stdcall IChat_RequestUserIP(IChat *this, User* user)
{
    dprintf("IChat::RequestUserIp(this=%p, user=%p)\n", this, user);
    dprintf(" name: %s\n", user->name);
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

    /* to get our callback ev pointer */
    IChatSingleton = this;

    return this;
}
