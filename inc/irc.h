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
#include <unistd.h>

#ifndef _IRC_H_
#define _IRC_H_

#define IRC_BUFSIZ 513
#define IRC_IBUFSIZ (IRC_BUFSIZ * 2)
#define IRC_MAXPARAM 32

#define WOL_RPL_LISTSTART           "321"
#define WOL_RPL_LISTGAME            "326"
#define WOL_RPL_LISTLOBBY           "327"
#define WOL_RPL_LISTEND             "323"
#define WOL_RPL_NOTOPIC             "331"
#define WOL_RPL_TOPIC               "332"
#define WOL_RPL_NAMREPLY            "353"
#define WOL_RPL_ENDOFNAMES          "366"
#define WOL_RPL_ENDOFMOTD           "376"
#define WOL_RPL_MOTDSTART           "375"
#define WOL_RPL_MOTD                "372"

#define WOL_ERR_NOMOTD              "422"

typedef struct _irc_session irc_session;
typedef struct _irc_event irc_event;

struct _irc_session
{
    int s;
    char buf[IRC_IBUFSIZ];
    irc_event *events;
    void *ctx;
};

typedef void (*irc_callback)(void *ctx, const char *prefix, const char *command, int argc, const char *argv[]);

struct _irc_event
{
    char command[32];
    irc_callback func; 
    irc_event *next;
};

irc_event *irc_hook_add(irc_session *this, const char *command, irc_callback func);
int irc_hook_remove(irc_session *this, irc_event *hook);
void irc_parse_prefix(const char *in, char *nick, char *host);
int irc_printf(irc_session *this, const char *fmt, ...);
void irc_pump(irc_session *this);
irc_session *irc_create(void *ctx);
int irc_connect(irc_session *this, const char *host, int port);
void irc_close(irc_session *this);

#endif
