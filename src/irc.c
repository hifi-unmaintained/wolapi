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

irc_event *irc_hook_add(irc_session *this, const char *command, irc_callback func)
{
    irc_event **ev = &this->events;

    dprintf("irc_hook_add(this=%p, command=\"%s\", func=%p)\n", this, command, func);

    while (*ev)
    {
        ev = &(*ev)->next;
    }

    *ev = calloc(1, sizeof(irc_event));

    if (!*ev)
        return 0;

    strcpy((*ev)->command, command);
    (*ev)->func = func;
    return *ev;
}

int irc_hook_remove(irc_session *this, irc_event *hook)
{
    irc_event **ev = &this->events;
    irc_event **prev = NULL;

    dprintf("irc_hook_remove(this=%p, hook=%p)\n", this, hook);

    while (*ev)
    {
        if (*ev == hook)
        {
            if (*prev != NULL)
            {
                (*prev)->next = (*ev)->next;
            }

            free(*ev);
            ev = NULL;

            return 1;
        }

        prev = ev;
        ev = &(*ev)->next;
    }

    return 0;
}

int irc_printf(irc_session *this, const char *fmt, ...)
{
    va_list args;
    char buf[512];

    if (!this->s)
    {
        this->disconnect(this->ctx, NULL, NULL, 0, NULL);
        return 0;
    }

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    dprintf("IRC -> %s\n", buf);

    strncat(buf, "\r\n", sizeof(buf));
    return send(this->s, buf, strlen(buf), 0);
}

void irc_parse_prefix(const char *in, char *nick, char *host)
{
    int i,dpos = 0;
    char *dst = nick;

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

static void _irc_line(irc_session *this, const char *line)
{
    char prefix[IRC_BUFSIZ] = { 0 };
    char command[IRC_BUFSIZ];
    char params[IRC_BUFSIZ];

    memset(params, 0, sizeof(params));

    if (sscanf(line, ":%*1[ ]%512s %512c", command, params) == 2
           || sscanf(line, ":%512s %512s %512c", prefix, command, params) == 3
           || sscanf(line, "%512s %512c", command, params) == 2)
    {
        irc_event *ev = this->events;

        int argc = 0;
        char *argv[IRC_MAXPARAM];
        char *p = params;
        int i,len = strlen(params);

        for (i = 0; i < len; i++)
        {
            if (argc == IRC_MAXPARAM)
            {
                p = NULL;
                break;
            }

            if (params[i] == ':' && (i == 0 || params[i-1] == '\0'))
            {
                argv[argc++] = p + 1;
                p = NULL;
                break;
            }

            if (params[i] == ' ')
            {
                argv[argc++] = p;
                params[i] = '\0';
                p = params + i + 1;
            }
        }

        if (p)
        {
            argv[argc++] = p;
        }

        while (ev)
        {
            if (ev->command[0] == '*' || strcmp(ev->command, command) == 0)
            {
                ev->func(this->ctx, prefix, command, argc, (const char **)argv);
            }

            ev = ev->next;
        }
    }
    else
    {
        dprintf("IRC <- %s\n", line);
    }
}

void irc_pump(irc_session *this)
{
    int i,len;
    char ibuf[IRC_IBUFSIZ] = { 0 };
    char *ptr = ibuf;
    char buf[IRC_BUFSIZ];
    struct timeval tv = { 0, 0 };
    fd_set in_set;

    FD_ZERO(&in_set);
    FD_SET(this->s, &in_set);

    /* FIXME: validate that this is actually safe what I'm doin' */
    if (this->s && select(this->s + 1, &in_set, NULL, NULL, &tv) > 0)
    {
        len = recv(this->s, buf, IRC_BUFSIZ-1, 0);

        if (len == 0)
        {
            close(this->s);
            this->s = 0;
            this->disconnect(this->ctx, NULL, NULL, 0, NULL);
            return;
        }

        buf[len] = '\0';
        buf[IRC_BUFSIZ-1] = '\0';

        strncpy(ibuf, this->buf, IRC_IBUFSIZ-1);
        strncat(ibuf, buf, IRC_IBUFSIZ-1);

        len = strlen(ibuf);

        for (i = 0; i < len; i++)
        {
            if (ibuf[i] == '\n')
            {
                if (i > 0)
                    ibuf[i-1] = '\0'; /* be RFC compliant or be cut */
                _irc_line(this, ptr);
                ptr = ibuf+i+1;
            }
        }

        strncpy(this->buf, ptr, IRC_IBUFSIZ-1);
    }
}

irc_session *irc_create(void *ctx)
{
    irc_session *this = calloc(1, sizeof(irc_session));
    WSADATA wsaData;
    WSAStartup(MAKEWORD(1, 1), &wsaData);

    this->ctx = ctx;
    return this;
}

int irc_connect(irc_session *this, const char *host, int port)
{
    struct sockaddr_in addr;
    struct hostent *hent = gethostbyname(host);

    dprintf("irc_connect(host=\"%s\", port=%d)\n", host, port);

    if (this->s)
    {
        close(this->s);
    }

    this->s = socket(AF_INET, SOCK_STREAM, 0);

    if (!hent)
    {
        dprintf(" Error resolving server host\n");
        return 0;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *(int *)hent->h_addr_list[0];
    addr.sin_port = htons(port);

    if (connect(this->s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0)
    {
        dprintf(" Error connecting to server\n");
        return 0;
    }

    return 1;
}

void irc_close(irc_session *this)
{
    if (this)
    {
        irc_event *ev = this->events;

        while (ev)
        {
            irc_event *cur = ev;
            ev = ev->next;
            free(cur);
        }

        if (this->s)
        {
            close(this->s);
        }

        free(this);
    }
}
