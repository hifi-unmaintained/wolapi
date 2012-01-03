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

#include <windows.h>
#include <objbase.h>
#include <ocidl.h>
#include <string.h>
#include <time.h>

#if _DEBUG
    #include <stdio.h>
    #define dprintf printf
#else
    int dprintf(const char *, ...);
#endif

#ifndef _WOLAPI_H_
#define _WOLAPI_H_

typedef struct Server
{
    int             gametype;
    int             chattype;
    int             timezone;
    float           longitude;
    float           lattitude;
    struct Server*  next;
    char            name[71];
    char            connlabel[5];
    char            conndata[128];
    char            login[10];
    char            password[10];
} Server;

typedef struct Channel
{
    int             type;
    unsigned int    minUsers;
    unsigned int    maxUsers;
    unsigned int    currentUsers;
    unsigned int    official;
    unsigned int    tournament;
    unsigned int    ingame;
    unsigned int    flags;
    unsigned long   reserved;
    unsigned long   ipaddr;
    int             latency;
    int             hidden;
    struct Channel* next;
    char            name[17];
    char            topic[81];
    char            location[65];
    char            key[9];
    char            exInfo[41];
} Channel;

typedef struct User
{
    unsigned int    flags;
    int             group;
    unsigned long   reserved;
    unsigned long   reserved2;
    unsigned long   reserved3;
    unsigned long   squadID;
    unsigned long   ipaddr;
    unsigned long   squad_icon;
    struct User*    next;
    char            name[10];
    char            squadname[41];
} User;

typedef struct Update
{
    unsigned long   SKU;
    unsigned long   version;
    int             required;
    struct Update*  next;
    char            Server[65];
    char            patchpath[256];
    char            patchfile[33];
    char            login[33];
    char            password[65];
    char            localpath[256];
} Update;

typedef struct Squad
{
    unsigned long   id;
    int             SKU;
    int             members;
    struct Squad*   next;
    char            URL[129];
    char            name[41];
} Squad;

typedef struct Ladder
{
    unsigned int    SKU;
    unsigned int    team_no;
    unsigned int    wins;
    unsigned int    losses;
    unsigned int    points;
    unsigned int    kills;
    unsigned int    rank;
    unsigned int    rung;
    unsigned int    disconnects;
    unsigned int    team_rung;
    unsigned int    provisional;
    unsigned int    last_game_date;
    unsigned int    win_streak;
    unsigned int    reserved1;
    unsigned int    reserved2;
    struct Ladder*  next;
    char            login_name[40];
} Ladder;

#define WOL_ALLOC(size)                                     \
    calloc(1, size)

#define WOL_FREE(ptr)                                       \
    free(ptr)

/* linked list handling macros for the structures above */

#define WOL_LIST_NEW(type)                                  \
    WOL_ALLOC(sizeof(type))

#define WOL_LIST_FREE(el)                                   \
    while (el)                                              \
    {                                                       \
        void *_eltmp = (el);                                \
        (el) = (el)->next;                                  \
        free(_eltmp);                                       \
    }                                                       \
    (el) = NULL

#define WOL_LIST_INSERT(list, el)                           \
    if ((list) == NULL)                                     \
    {                                                       \
        (list) = (el);                                      \
    }                                                       \
    else                                                    \
    {                                                       \
        void *_eltmp = (el);                                \
        WOL_LIST_FOREACH(list, el)                          \
        {                                                   \
            if ((el)->next == NULL)                         \
            {                                               \
                (el)->next = _eltmp;                        \
                (el) = _eltmp;                              \
                break;                                      \
            }                                               \
        }                                                   \
    }

#define WOL_LIST_FOREACH(list, el)                          \
    for ((el) = (list); (el) != NULL; (el) = (el)->next)

#define WOL_LIST_REMOVE(list, el)                           \
    if ((list) == (el))                                     \
    {                                                       \
        (list) = NULL;                                      \
    }                                                       \
    else                                                    \
    {                                                       \
        void *_eltmp = (el);                                \
        (el) = (list);                                      \
        do {                                                \
            if ((el)->next == _eltmp)                       \
            {                                               \
                (el)->next = (el)->next->next;              \
                (el) = _eltmp;                              \
                break;                                      \
            }                                               \
            (el) = (el)->next;                              \
        } while(el);                                        \
    }

char *wol_strdup(const char *in);
const char *str_GUID(REFGUID);

#include "irc.h"
#include "IRTPatcherEvent.h"
#include "IRTPatcher.h"
#include "IDownloadEvent.h"
#include "IDownload.h"
#include "IChat2Event.h"
#include "IChat2.h"
#include "IChatEvent.h"
#include "IChat.h"
#include "Chat.h"
#include "INetUtilEvent.h"
#include "INetUtil.h"
#include "NetUtil.h"
#include "IConnectionPointContainer.h"
#include "IWOLConnectionPoint.h"

#endif
