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

#include <windows.h>
#include <objbase.h>
#include <ocidl.h>
#include <string.h>
#include <time.h>

#ifdef _DEBUG
    #include <stdio.h>
    #define dprintf printf
#else
    #define dprintf(...)
#endif

#ifndef _WOLAPI_H_
#define _WOLAPI_H_

const char *str_GUID(REFGUID);

typedef enum
{
    GT_Server  = 0,
    GT_Channel = 1,
    GT_CLIENT  = 2
} GTYPE_;

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

char *wol_strdup(const char *in);

/* linked list handlers, could use a define macro wrapper */
void user_list_add(User **list, User *user);
void user_list_free(User **list);
void channel_list_add(Channel **list, Channel *channel);
void channel_list_free(Channel **list);

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
