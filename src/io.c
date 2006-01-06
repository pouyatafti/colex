/*
 * io.c -- part of the colex package
 *
 * Copyright (c) 2006 Pouya D. Tafti
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
 * 
 * author: Pouya D. Tafti
 *
 * $ 2006-01-06 $
 *
 */

#include <u.h>
#include <libc.h>
#include <bio.h> /* FIXME for debugging */
#include <draw.h>
#include <thread.h>
#include <mouse.h>

#include "io.h"

#define DEBUG   1

uchar got;
uchar block;

Biobuf bin; /* FIXME for debugging */

Channel *stdinc;

void
initio()
{
    if(DEBUG) fprint(2,"initio: mouse\n");

    mctl = initmouse(nil,display->image);
    if(mctl == nil){
        fprint(2, "initio: mouse: %r\n");
        threadexitsall("mouse");
    }
    
    if(DEBUG) fprint(2,"initio: stdinproc\n");

    stdinc = chancreate(sizeof(char),0);
    proccreate(stdinproc,stdinc,8192);
    
    Binit(&bin,0,OREAD); /* FIXME for debugging */

    got=block=0;
}

/*
 * the following function is a bad implementation of Rob Pike's design for i/o
 * control in samterm, part of the sam editor.
 */
void
recvio()
{
    Alt alts[Nres+1];
    ulong ev;

    alts[Rmouse].c  = mctl->c;
    alts[Rmouse].v  = &mctl->m;
    alts[Rmouse].op = CHANRCV;
    if(isset(block,Rmouse))
        alts[Rmouse].op = CHANNOP;
    
    alts[Rstdin].c  = stdinc;
    alts[Rstdin].v  = &nextch;
    alts[Rstdin].op = CHANRCV;
    if(isset(got,Rstdin))
        alts[Rstdin].op = CHANNOP;
    
    alts[Rresize].c  = mctl->resizec;
    alts[Rresize].v  = nil;
    alts[Rresize].op = CHANRCV;
    if(isset(block,Rresize))
        alts[Rresize].op = CHANNOP;

    alts[Nres].op = CHANEND;

    if(DEBUG) fprint(2,"recvio %c%c%c\n",
        "m-"[alts[Rmouse].op == CHANNOP],
        "s-"[alts[Rstdin].op == CHANNOP],
        "r-"[alts[Rresize].op == CHANNOP]);

    if(got) return;

    ev = alt(alts);
    switch(ev) {
    case Rmouse:
        break;
    case Rstdin:
        /* block |= (1<<Rstdin); */ /* FIXME is block needed? */
        break;
    case Rresize:
        break;
    }

    got |= (1<<ev);

    if(DEBUG) fprint(2,"got %c%c%c  ",
        "m-"[!isset(got,Rmouse)],
        "s-"[!isset(got,Rstdin)],
        "r-"[!isset(got,Rresize)]);
    if(DEBUG) fprint(2,"block %c%c%c\n",
        "m-"[!isset(block,Rmouse)],
        "s-"[!isset(block,Rstdin)],
        "r-"[!isset(block,Rresize)]);
}

void
stdinproc(void *v)
{
    Channel *cnl=v;
    char ch;

    Bflush(&bin);
    for(;;) {
        //block |= (1<<Rstdin);
        ch=Bgetc(&bin);
        if(DEBUG) fprint(2,"stdinproc: %c\n",ch);
        send(cnl,&ch);
        block |= (1<<Rstdin);
        if(DEBUG) fprint(2,"Rstdin blocked\n");
    }

    if(DEBUG) fprint(2,"stdinproc ended\n");
    threadexits(0);
}

Mouse
rdmouse()
{
    got &= !(1<<Rmouse);
    return mctl->m;
}

char
rdchar()
{
    /* PDT 2006-01-02 0111 -0500 */
    /* TODO change rdchar to read from stdinc -- rewrite or remove readio */

    /*
    if(nbrecv(stdinc,&nextch) !
    got &= !(1<<Rstdin);
    return ;
    */
    got &= !(1<<Rstdin);
    block &= !(1<<Rstdin);
    if(DEBUG) fprint(2,"Rstdin unblocked\n");
    return nextch;
}

void
closeio()
{
    closemouse(mctl);
    chanfree(stdinc);
}
