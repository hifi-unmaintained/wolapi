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

#ifndef _IDOWNLOADEVENT_H_
#define _IDOWNLOADEVENT_H_

/* IDownloadEvent Interface */

extern const GUID IID_IDownloadEvent;

typedef struct _IDownloadEvent      IDownloadEvent;
typedef struct _IDownloadEventVtbl  IDownloadEventVtbl;

struct _IDownloadEvent
{
    IDownloadEventVtbl  *lpVtbl;
};

struct _IDownloadEventVtbl
{
    /* IUnknown */
    HRESULT(__stdcall *QueryInterface)      (IDownloadEvent *, const IID* const riid, LPVOID * ppvObj);
    ULONG(__stdcall *AddRef)                (IDownloadEvent *);
    ULONG(__stdcall *Release)               (IDownloadEvent *);

    /* IDownloadEvent */
    HRESULT(__stdcall *OnEnd)               (IDownloadEvent *);
    HRESULT(__stdcall *OnError)             (IDownloadEvent *, int error);
    HRESULT(__stdcall *OnProgressUpdate)    (IDownloadEvent *, int bytesread, int totalsize, int timetaken, int timeleft);
    HRESULT(__stdcall *OnQueryResume)       (IDownloadEvent *);
    HRESULT(__stdcall *OnStatusUpdate)      (IDownloadEvent *, int status);
};

#endif
