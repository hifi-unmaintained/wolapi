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

#ifndef _ICHATEVENT_H_
#define _ICHATEVENT_H_

/* IChatEvent Interface */

extern const GUID IID_IChatEvent;

typedef struct _IChatEvent      IChatEvent;
typedef struct _IChatEventVtbl  IChatEventVtbl;

struct _IChatEvent
{
    IChatEventVtbl  *lpVtbl;
};

struct _IChatEventVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)              (IChatEvent *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)                        (IChatEvent *);
    ULONG(__stdcall *Release)                       (IChatEvent *);

    /* IChatEvent */
    HRESULT(__stdcall *OnServerList)            (IChatEvent *, HRESULT res, Server* servers);
    HRESULT(__stdcall *OnUpdateList)            (IChatEvent *, HRESULT res, Update* updates);
    HRESULT(__stdcall *OnServerError)           (IChatEvent *, HRESULT res, LPSTR ircmsg);
    HRESULT(__stdcall *OnConnection)            (IChatEvent *, HRESULT res, LPSTR motd);
    HRESULT(__stdcall *OnMessageOfTheDay)       (IChatEvent *, HRESULT res, LPSTR motd);
    HRESULT(__stdcall *OnChannelList)           (IChatEvent *, HRESULT res, Channel* channels);
    HRESULT(__stdcall *OnChannelCreate)         (IChatEvent *, HRESULT res, Channel* Channel);
    HRESULT(__stdcall *OnChannelJoin)           (IChatEvent *, HRESULT res, Channel* Channel, User* User); 
    HRESULT(__stdcall *OnChannelLeave)          (IChatEvent *, HRESULT res, Channel* Channel, User* User);
    HRESULT(__stdcall *OnChannelTopic)          (IChatEvent *, HRESULT res, Channel* Channel, LPSTR topic);
    HRESULT(__stdcall *OnPrivateAction)         (IChatEvent *, HRESULT res, User* User, LPSTR action);
    HRESULT(__stdcall *OnPublicAction)          (IChatEvent *, HRESULT res, Channel* Channel, User* User, LPSTR action);
    HRESULT(__stdcall *OnUserList)              (IChatEvent *, HRESULT res, Channel* Channel, User* users);
    HRESULT(__stdcall *OnPublicMessage)         (IChatEvent *, HRESULT res, Channel* Channel, User* User, LPSTR message);
    HRESULT(__stdcall *OnPrivateMessage)        (IChatEvent *, HRESULT res, User* User, LPSTR message);
    HRESULT(__stdcall *OnSystemMessage)         (IChatEvent *, HRESULT res, LPSTR message); 
    HRESULT(__stdcall *OnNetStatus)             (IChatEvent *, HRESULT res);
    HRESULT(__stdcall *OnLogout)                (IChatEvent *, HRESULT status, User* User);
    HRESULT(__stdcall *OnPrivateGameOptions)    (IChatEvent *, HRESULT res, User* User, LPSTR options);
    HRESULT(__stdcall *OnPublicGameOptions)     (IChatEvent *, HRESULT res, Channel* Channel, User* User, LPSTR options);
    HRESULT(__stdcall *OnGameStart)             (IChatEvent *, HRESULT res, Channel* Channel, User* users, int gameid);
    HRESULT(__stdcall *OnUserKick)              (IChatEvent *, HRESULT res, Channel* Channel, User* kicked, User* kicker);
    HRESULT(__stdcall *OnUserIP)                (IChatEvent *, HRESULT res, User* User); 
    HRESULT(__stdcall *OnFind)                  (IChatEvent *, HRESULT res, Channel* chan);
    HRESULT(__stdcall *OnPageSend)              (IChatEvent *, HRESULT res);
    HRESULT(__stdcall *OnPaged)                 (IChatEvent *, HRESULT res, User* User, LPSTR message);
    HRESULT(__stdcall *OnServerBannedYou)       (IChatEvent *, HRESULT res, long bannedTill);
    HRESULT(__stdcall *OnUserFlags)             (IChatEvent *, HRESULT res, LPSTR name, unsigned int flags, unsigned int mask);
    HRESULT(__stdcall *OnChannelBan)            (IChatEvent *, HRESULT res, LPSTR name, int banned);
    HRESULT(__stdcall *OnSquadInfo)             (IChatEvent *, HRESULT res, unsigned long id, Squad* Squad);
};

#define IChatEvent_OnServerList(T,a,b) (T)->lpVtbl->OnServerList(T,a,b)
#define IChatEvent_OnUpdateList(T,a,b) (T)->lpVtbl->OnUpdateList(T,a,b)
#define IChatEvent_OnServerError(T,a,b) (T)->lpVtbl->OnServerError(T,a,b)
#define IChatEvent_OnConnection(T,a,b) (T)->lpVtbl->OnConnection(T,a,b)
#define IChatEvent_OnMessageOfTheDay(T,a,b) (T)->lpVtbl->OnMessageOfTheDay(T,a,b)
#define IChatEvent_OnChannelList(T,a,b) (T)->lpVtbl->OnChannelList(T,a,b)
#define IChatEvent_OnChannelCreate(T,a,b) (T)->lpVtbl->OnChannelCreate(T,a,b)
#define IChatEvent_OnChannelJoin(T,a,b,c) (T)->lpVtbl->OnChannelJoin(T,a,b,c)
#define IChatEvent_OnChannelLeave(T,a,b,c) (T)->lpVtbl->OnChannelLeave(T,a,b,c)
#define IChatEvent_OnChannelTopic(T,a,b,c) (T)->lpVtbl->OnChannelTopic(T,a,b,c)
#define IChatEvent_OnPublicAction(T,a,b,c,d) (T)->lpVtbl->OnPublicAction(T,a,b,c,d)
#define IChatEvent_OnUserList(T,a,b,c) (T)->lpVtbl->OnUserList(T,a,b,c)
#define IChatEvent_OnPublicMessage(T,a,b,c,d) (T)->lpVtbl->OnPublicMessage(T,a,b,c,d)
#define IChatEvent_OnPrivateMessage(T,a,b,c) (T)->lpVtbl->OnPrivateMessage(T,a,b,c)
#define IChatEvent_OnSystemMessage(T,a,b) (T)->lpVtbl->OnSystemMessage(T,a,b)
#define IChatEvent_OnPrivateGameOptions(T,a,b,c) (T)->lpVtbl->OnPrivateGameOptions(T,a,b,c)
#define IChatEvent_OnPublicGameOptions(T,a,b,c,d) (T)->lpVtbl->OnPublicGameOptions(T,a,b,c,d)

#define IChatEvent_OnNetStatus(T,a) (T)->lpVtbl->OnNetStatus(T,a)

#endif
