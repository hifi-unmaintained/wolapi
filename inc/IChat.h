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
#include <stdio.h>

#ifndef _ICHAT_H_
#define _ICHAT_H_

/* IChat Interface */

#define CHAT_E_NICKINUSE            0x80040064
#define CHAT_E_BADPASS              0x80040065
#define CHAT_E_CON_DOWN             0x80040067
#define CHAT_E_CON_LOOKUP_FAILED    0x80040068
#define CHAT_E_CON_ERROR            0x80040069 /* guess */
#define CHAT_E_TIMEOUT              0x8004006A
#define CHAT_E_MUSTPATCH            0x8004006C
#define CHAT_E_UNKNOWNRESPONSE      0x8004006D
#define CHAT_E_JOINCHANNEL          0x800401F5

#define CHAT_S_CON_CONNECTING       0x0004012C /* guess */
#define CHAT_S_CON_CONNECTED        0x0004012D
#define CHAT_S_CON_DISCONNECTING    0x0004012E /* guess */
#define CHAT_S_CON_DISCONNECTED     0x0004012F

#define CHAT_E_DISABLED             E_FAIL
#define CHAT_E_SERIALBANNED         E_FAIL
#define CHAT_E_SERIALDUP            E_FAIL

#define CHAT_USER_CHANNELOWNER      (1 << 0)    /* @ */
#define CHAT_USER_VOICE             (1 << 1)    /* + */
#define CHAT_USER_SQUELCHED         (1 << 2)
#define CHAT_USER_MYSELF            (1 << 15)

#define CHAN_MODE_KEY               (1 << 8)

#define SKU_RA303                   0x00001500

typedef struct Gameopt
{
    char            prefix[128];
    char            command[128];
    char            argv[2][512];
    struct Gameopt* next;
} Gameopt;

extern const GUID IID_IChat;

typedef struct _IChat       IChat;
typedef struct _IChatVtbl   IChatVtbl;

struct _IChat
{
    IChatVtbl       *lpVtbl;

    char            name[18];
    int             ref;
    IChatEvent      *ev;
    unsigned long   SKU;
    unsigned long   current_version;
    Server          server;
    User            user;
    User            *users;
    Channel         channel;
    Channel         *channels;
    char            motd[4096];
    irc_session     *irc;
    int             gameopt_sent; /* RA 3.03 hack */
    Gameopt         *gameopt_queue;
};

struct _IChatVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)              (IChat *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)                        (IChat *);
    ULONG(__stdcall *Release)                       (IChat *);

    /* IChat */
    HRESULT(__stdcall *PumpMessages)                (IChat *);
    HRESULT(__stdcall *RequestServerList)           (IChat *, unsigned long SKU, unsigned long current_version, LPSTR loginname, LPSTR password, int timeout);
    HRESULT(__stdcall *RequestConnection)           (IChat *, Server* Server, int timeout, int domangle);
    HRESULT(__stdcall *RequestChannelList)          (IChat *, int channelType, int autoping);
    HRESULT(__stdcall *RequestChannelCreate)        (IChat *, Channel* Channel);
    HRESULT(__stdcall *RequestChannelJoin)          (IChat *, Channel* Channel);
    HRESULT(__stdcall *RequestChannelLeave)         (IChat *);
    HRESULT(__stdcall *RequestUserList)             (IChat *);
    HRESULT(__stdcall *RequestPublicMessage)        (IChat *, LPSTR message);
    HRESULT(__stdcall *RequestPrivateMessage)       (IChat *, User* users, LPSTR message);
    HRESULT(__stdcall *RequestLogout)               (IChat *);
    HRESULT(__stdcall *RequestPrivateGameOptions)   (IChat *, User* users, LPSTR options);
    HRESULT(__stdcall *RequestPublicGameOptions)    (IChat *, LPSTR options);
    HRESULT(__stdcall *RequestPublicAction)         (IChat *, LPSTR action);
    HRESULT(__stdcall *RequestPrivateAction)        (IChat *, User* users, LPSTR action);
    HRESULT(__stdcall *RequestGameStart)            (IChat *, User* users);
    HRESULT(__stdcall *RequestChannelTopic)         (IChat *, LPSTR topic);
    HRESULT(__stdcall *GetVersion)                  (IChat *, unsigned long* version);
    HRESULT(__stdcall *RequestUserKick)             (IChat *, User* User);
    HRESULT(__stdcall *RequestUserIP)               (IChat *, User* User);
    HRESULT(__stdcall *GetGametypeInfo)             (IChat *, unsigned int gtype, int icon_size, unsigned char** bitmap, int* bmp_bytes, LPSTR* name, LPSTR* URL);
    HRESULT(__stdcall *RequestFind)                 (IChat *, User* User);
    HRESULT(__stdcall *RequestPage)                 (IChat *, User* User, LPSTR message);
    HRESULT(__stdcall *SetFindPage)                 (IChat *, int findOn, int pageOn);
    HRESULT(__stdcall *SetSquelch)                  (IChat *, User* User, int squelch);
    HRESULT(__stdcall *GetSquelch)                  (IChat *, User* User);
    HRESULT(__stdcall *SetChannelFilter)            (IChat *, int channelType);
    HRESULT(__stdcall *RequestGameEnd)              (IChat *);
    HRESULT(__stdcall *SetLangFilter)               (IChat *, int onoff);
    HRESULT(__stdcall *RequestChannelBan)           (IChat *, LPSTR name, int ban);
    HRESULT(__stdcall *GetGametypeList)             (IChat *, LPSTR* list);
    HRESULT(__stdcall *GetHelpURL)                  (IChat *, LPSTR* URL);
    HRESULT(__stdcall *SetProductSKU)               (IChat *, unsigned long SKU);
    HRESULT(__stdcall *GetNick)                     (IChat *, int num, LPSTR* nick, LPSTR* pass);
    HRESULT(__stdcall *SetNick)                     (IChat *, int num, LPSTR nick, LPSTR pass, int domangle);
    HRESULT(__stdcall *GetLobbyCount)               (IChat *, int* count);
    HRESULT(__stdcall *RequestRawMessage)           (IChat *, LPSTR ircmsg);
    HRESULT(__stdcall *GetAttributeValue)           (IChat *, LPSTR attrib, LPSTR* value);
    HRESULT(__stdcall *SetAttributeValue)           (IChat *, LPSTR attrib, LPSTR value);
    HRESULT(__stdcall *SetChannelExInfo)            (IChat *, LPSTR info);
    HRESULT(__stdcall *StopAutoping)                (IChat *);
    HRESULT(__stdcall *RequestSquadInfo)            (IChat *, unsigned long id);
};

#define IChat_QueryInterface(T,a,b) (T)->lpVtbl->QueryInterface(T,a,b)

#define IChat_PumpMessages(T) (T)->lpVtbl->PumpMessages(T)
#define IChat_RequestServerList(T,a,b,c,d,e) (T)->lpVtbl->RequestServerList(T,a,b,c,d,e)
#define IChat_RequestConnection(T,a,b,c) (T)->lpVtbl->RequestConnection(T,a,b,c)
#define IChat_RequestChannelList(T,a,b) (T)->lpVtbl->RequestChannelList(T,a,b)
#define IChat_RequestChannelCreate(T,a) (T)->lpVtbl->RequestChannelCreate(T,a)
#define IChat_RequestChannelJoin(T,a) (T)->lpVtbl->RequestChannelJoin(T,a)
#define IChat_RequestChannelLeave(T) (T)->lpVtbl->RequestChannelLeave(T)
#define IChat_RequestLogout(T) (T)->lpVtbl->RequestLogout(T)
#define IChat_RequestChannelTopic(T,a) (T)->lpVtbl->RequestChannelTopic(T,a)
#define IChat_GetGametypeInfo(T,a,b,c,d,e,f) (T)->lpVtbl->GetGametypeInfo(T,a,b,c,d,e,f)
#define IChat_GetGametypeList(T,a) (T)->lpVtbl->GetGametypeList(T,a)
#define IChat_GetAttributeValue(T,a,b) (T)->lpVtbl->GetAttributeValue(T,a,b)
#define IChat_SetAttributeValue(T,a,b) (T)->lpVtbl->SetAttributeValue(T,a,b)

extern IChat *IChatSingleton;
IChat* IChat_New();

#endif
