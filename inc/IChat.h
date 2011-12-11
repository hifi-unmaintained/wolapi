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

#define CHAT_S_CON_CONNECTING       S_OK
#define CHAT_S_CON_CONNECTED        S_OK
#define CHAT_S_CON_DISCONNECTING    0x0004012E
#define CHAT_S_CON_DISCONNECTED     /*0x00040130*/ -1 /* seems to work with RA 3.03 */

#define CHAT_E_DISABLED             E_FAIL
#define CHAT_E_SERIALBANNED         E_FAIL
#define CHAT_E_SERIALDUP            E_FAIL

extern const GUID IID_IChat;

typedef struct _IChat       IChat;
typedef struct _IChatVtbl   IChatVtbl;

struct _IChat
{
    IChatVtbl       *lpVtbl;

    int             ref;
    IChatEvent      *ev;
    unsigned long   SKU;
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

extern IChat *IChatSingleton;
IChat* IChat_New();

#endif
