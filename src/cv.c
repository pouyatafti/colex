/*
 * cv.c -- draw-based image viewer for X.
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
 * $ 2006-01-01 $
 *
 */

#include <u.h>
#include <stdio.h>
#include <libc.h>
#include <bio.h>
#include <draw.h>
#include <event.h>

#define DEBUG       1

#include "colour.h"

#define EPS         1e-6
#define MAX_STR     256
#define WIDTH       500
#define HEIGHT      500
#define GAMMA       1.0
#define GAMMAUNITS  10

RGBc RGBBlack   = {0,0,0};
RGBc RGBRed     = {NLEVELS-1,0,0};
RGBc RGBGreen   = {0,NLEVELS-1,0};
RGBc RGBBlue    = {0,0,NLEVELS-1};

/* command keys */
typedef enum {
    Kr8g8b8a8 = 'r'+'8'+'g'+'8'+'b'+'8'+'a'+'8',
    Kred      = 'r'+'e'+'d',
    Kgrn      = 'g'+'r'+'n',
    Kblu      = 'b'+'l'+'u',
    Kwht      = 'w'+'h'+'t',
    KgammaR   = 'g'+'a'+'m'+'m'+'a'+'R',
    KgammaG   = 'g'+'a'+'m'+'m'+'a'+'G',
    KgammaB   = 'g'+'a'+'m'+'m'+'a'+'B',
    Kgamma    = 'g'+'a'+'m'+'m'+'a',
    Kredraw   = 'r'+'e'+'d'+'r'+'a'+'w',
    Kreset    = 'r'+'e'+'s'+'e'+'t',
    Kclear    = 'c'+'l'+'e'+'a'+'r',
    Kmouse    = 'm'+'o'+'u'+'s'+'e',  
    Kexit     = 'e'+'x'+'i'+'t'
} Cmd;

typedef struct CList CList;
struct CList {
    RGBc c;
    
    CList *prev;
    CList *next;
};
void killCList(CList *);
void printCList(CList *);
CList *addC(RGBc,CList **);
    
void eresized(int);

void usage();
void wexits(char *);
char *cmd(char *);
char *redraw();
void drawcl(CList *);

Point xyY2pt(CIExyY);
void printSCProf(SCProf *);

void putpx(Point,RGBc);
void putmark(Point,RGBc);

Biobuf bin, bout;
Mouse m;
/* menus */
char *mbtn[] = {"red","green","blue","white","gammaR","gammaG","gammaB","gamma",0};
char *rbtn[] = {"clear","redraw","reset","send","exit",0};
Menu mmenu = {mbtn};
Menu rmenu = {rbtn};

CList *cl=nil;
SCProf cpr;

void
main(int argc, char **argv)
{
    char b[MAX_STR],*msg;
    char *bp=b;
    int i;

    Point mpos;
    
    if(DEBUG) fprint(2,"initSCProf\n");
    initSCProf(&cpr,GAMMA);

    /* init i/o */
    Binit(&bin,0,OREAD);
    Binit(&bout,1,OWRITE);
    Bgetc(&bin); Bungetc(&bin); /* activate */

    /* init interface */
    if(DEBUG) fprint(2,"init interface\n");
    if(initdraw(0,0,"cv")==0) {
        fprint(2,"cv: initdraw failed: %r\n");
        wexits("initdraw");
    }
    einit(Emouse);
    /* the next line should be replaced by a write to /dev/wctl in Plan 9. */
    drawresizewindow(Rect(0,0,WIDTH,HEIGHT));
    eresized(0);
    
    /*
    if((cl=(CList *)malloc(sizeof(CList)))==nil)
        wexits("memory");
     */
    
    /* main loop */
    i=0;
    for(;;) {
        /* read cmd */
        if(DEBUG) fprint(2,"read cmd: ");
        while(read(0,bp,1)>0) { 
            /* *bp = Bgetc(&bin); */
            if(*bp=='\n' || i>=MAX_STR-1) {
                *bp=0; bp=b; i=0;
                if(DEBUG) fprint(2,"cmd: %s\n",b);
                if(DEBUG && (msg=cmd(b))!=nil)
                    fprint(2,"cmd: failure: %s",msg);
            }
            else {
                bp++; i++;
            }
            if(DEBUG) fprint(2,".");
            /* if(DEBUG) fprint(2,"%d ",Bbuffered(&bin)); */
        }
        /* } while(Bbuffered(&bin)>0); */
        if(DEBUG) fprint(2,"\n");

        if(DEBUG) fprint(2,"get mouse\n");
        m = emouse();

        if(m.buttons == 2) { /* mmenu */
            mpos = m.xy;
            if(DEBUG) fprint(2,"mmenu: %d,%d\n",mpos.x,mpos.y);
            switch(emenuhit(2, &m, &mmenu)) {
            case 0: /* red    */
                cpr.red.x=(double)mpos.x/WIDTH;
                cpr.red.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 1: /* green  */
                cpr.grn.x=(double)mpos.x/WIDTH;
                cpr.grn.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 2: /* blue   */
                cpr.blu.x=(double)mpos.x/WIDTH;
                cpr.blu.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 3: /* white  */
                cpr.wht.x=(double)mpos.x/WIDTH;
                cpr.wht.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 4: /* gammaR */
                setgamma(&cpr,(double)mpos.x/(WIDTH/GAMMAUNITS),-1,-1);
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 5: /* gammaG */
                setgamma(&cpr,-1,(double)mpos.x/(WIDTH/GAMMAUNITS),-1);
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 6: /* gammaB */
                setgamma(&cpr,-1,-1,(double)mpos.x/(WIDTH/GAMMAUNITS));
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 7: /* gamma */
                setgamma(&cpr,(double)mpos.x/(WIDTH/GAMMAUNITS),
                              (double)mpos.x/(WIDTH/GAMMAUNITS),
                              (double)mpos.x/(WIDTH/GAMMAUNITS));
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            }
        }

        if(m.buttons == 4) { /* rmenu */
            mpos = m.xy;
            if(DEBUG) fprint(2,"rmenu: %d,%d\n",mpos.x,mpos.y);
            switch(emenuhit(3, &m, &rmenu)) {
            case 0: /* clear  */
                killCList(cl);
                cl=nil;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 1: /* redraw */
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 2: /* reset  */
                initSCProf(&cpr,GAMMA);
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 3: /* send   */
                printSCProf(&cpr);
                printCList(cl);
            case 4: /* exit   */
                wexits(0);
            }
        }
    }
    
    wexits(0);
}

char *
cmd(char *b)
{
    char *t=b,*msg;
    int kee=0;
    double A,B,C;
    RGBc c;
    CIExyY xyY;
    Point mpos;

    do kee+=*(t++); while(*t != 0 && *t != ' ');
    t += (*t == 0) ? 0 : 1;

    switch(kee) {
    case Kr8g8b8a8:
        if(sscanf(t,"%d %d %d %lf",&(c.R),&(c.G),&(c.B),&A) < 4)
            return "incomplete";
        A /= NLEVELS-1;
        c.R *= A; c.G *= A; c.B *= A;
        drawcl(addC(c,&cl));
        /*
        addC(c,&cl);
        return redraw();
        */
        break;
    case Kred:
        if(sscanf(t,"%lf %lf %lf",&(xyY.x),&(xyY.y),&(xyY.Y)) < 3)
            return "incomplete";
        cpr.red = xyY;
        return redraw();
    case Kgrn:
        if(sscanf(t,"%lf %lf %lf",&(xyY.x),&(xyY.y),&(xyY.Y)) < 3)
            return "incomplete";
        cpr.grn = xyY;
        return redraw();
    case Kblu:
        if(sscanf(t,"%lf %lf %lf",&(xyY.x),&(xyY.y),&(xyY.Y)) < 3)
            return "incomplete";
        cpr.blu = xyY;
        return redraw();
    case Kwht:
        if(sscanf(t,"%lf %lf %lf",&(xyY.x),&(xyY.y),&(xyY.Y)) < 3)
            return "incomplete";
        cpr.wht = xyY;
        return redraw();
    case KgammaR:
        if(sscanf(t,"%lf",&A) < 1)
            return "incomplete";
        setgamma(&cpr,A,-1,-1);
        return redraw();
    case KgammaG:
        if(sscanf(t,"%lf",&A) < 1)
            return "incomplete";
        setgamma(&cpr,-1,A,-1);
        return redraw();
    case KgammaB:
        if(sscanf(t,"%lf",&A) < 1)
            return "incomplete";
        setgamma(&cpr,-1,-1,A);
        return redraw();
    case Kgamma:
        if(sscanf(t,"%lf %lf %lf",&A,&B,&C) < 3)
            return "incomplete";
        setgamma(&cpr,A,B,C);
        return redraw();
    case Kredraw:
        return redraw();
    case Kreset:
        initSCProf(&cpr,GAMMA);
        return redraw();
    case Kclear:
        killCList(cl);
        cl=nil;
        return redraw();
    case Kmouse:
        if(DEBUG) fprint(2,"get mouse\n");
        do m = emouse(); while(m.buttons != 2 && m.buttons != 4);

        if(m.buttons == 2) { /* mmenu */
            mpos = m.xy;
            if(DEBUG) fprint(2,"mmenu: %d,%d\n",mpos.x,mpos.y);
            switch(emenuhit(2, &m, &mmenu)) {
            case 0: /* red    */
                cpr.red.x=(double)mpos.x/WIDTH;
                cpr.red.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 1: /* green  */
                cpr.grn.x=(double)mpos.x/WIDTH;
                cpr.grn.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 2: /* blue   */
                cpr.blu.x=(double)mpos.x/WIDTH;
                cpr.blu.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 3: /* white  */
                cpr.wht.x=(double)mpos.x/WIDTH;
                cpr.wht.y=(double)mpos.y/HEIGHT;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 4: /* gammaR */
                setgamma(&cpr,(double)mpos.x/(WIDTH/GAMMAUNITS),-1,-1);
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 5: /* gammaG */
                setgamma(&cpr,-1,(double)mpos.x/(WIDTH/GAMMAUNITS),-1);
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 6: /* gammaB */
                setgamma(&cpr,-1,-1,(double)mpos.x/(WIDTH/GAMMAUNITS));
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 7: /* gamma */
                setgamma(&cpr,(double)mpos.x/(WIDTH/GAMMAUNITS),
                              (double)mpos.x/(WIDTH/GAMMAUNITS),
                              (double)mpos.x/(WIDTH/GAMMAUNITS));
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            }
        }

        if(m.buttons == 4) { /* rmenu */
            mpos = m.xy;
            if(DEBUG) fprint(2,"rmenu: %d,%d\n",mpos.x,mpos.y);
            switch(emenuhit(3, &m, &rmenu)) {
            case 0: /* clear  */
                killCList(cl);
                cl=nil;
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 1: /* redraw */
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 2: /* reset  */
                initSCProf(&cpr,GAMMA);
                if((msg=redraw())!=nil)
                    wexits(msg);
                break;
            case 3: /* send   */
                printSCProf(&cpr);
                printCList(cl);
            case 4: /* exit   */
                wexits(0);
            }
        }
        break;
    case Kexit:
        wexits(0);
    default:
        return "unsupported";
    }
        
    return nil;
}

void
eresized(int new)
{
    if(new && getwindow(display, Refnone) < 0) {
        fprint(2, "iv: could not reattach to window: %r\n");
        wexits("resized");
    }
    redraw();
    flushimage(display, 1);
}

char *
redraw()
{
    CList *nd=cl;
    char s[MAX_STR];
    int i;
    CIExyY xyY;
    /* FIXME for debugging */ Point pt;

    draw(screen, screen->r, display->white, nil, ZP);

    /* gamma bar */
    for(i=0;i<WIDTH;i+=(WIDTH/50)) {
        if(!(i%(WIDTH/GAMMAUNITS))) {
            putpx(Pt(i,HEIGHT-21),RGBBlack);
            putpx(Pt(i,HEIGHT-19),RGBBlack);
            snprint(s,MAX_STR,"% 2d",i*GAMMAUNITS/WIDTH);
            string(screen,Pt(i,HEIGHT-19),display->black,ZP,display->defaultfont,s);
        }
        putpx(Pt(i,HEIGHT-20),RGBBlack);
    }
    putmark(Pt(cpr.gR*WIDTH/GAMMAUNITS,HEIGHT-20),RGBRed);
    putmark(Pt(cpr.gG*WIDTH/GAMMAUNITS,HEIGHT-20),RGBGreen);
    putmark(Pt(cpr.gB*WIDTH/GAMMAUNITS,HEIGHT-20),RGBBlue);

    /* colour triangle */
    line(screen,xyY2pt(cpr.red),xyY2pt(cpr.grn),Enddisc,Enddisc,0,display->black,xyY2pt(cpr.red));
    line(screen,xyY2pt(cpr.grn),xyY2pt(cpr.blu),Enddisc,Enddisc,0,display->black,xyY2pt(cpr.grn));
    line(screen,xyY2pt(cpr.blu),xyY2pt(cpr.red),Enddisc,Enddisc,0,display->black,xyY2pt(cpr.blu));
    putmark(xyY2pt(cpr.red),RGBRed);
    string(screen,xyY2pt(cpr.red),display->black,ZP,display->defaultfont,"R");
    putmark(xyY2pt(cpr.grn),RGBGreen);
    string(screen,xyY2pt(cpr.grn),display->black,ZP,display->defaultfont,"G");
    putmark(xyY2pt(cpr.blu),RGBBlue);
    string(screen,xyY2pt(cpr.blu),display->black,ZP,display->defaultfont,"B");
    putmark(xyY2pt(cpr.wht),RGBBlack);
    string(screen,xyY2pt(cpr.wht),display->black,ZP,display->defaultfont,"W");
    
    while(nd != nil) {
        xyY = RGB2xyY(nd->c,&cpr);
        if(DEBUG) { pt=xyY2pt(xyY); fprint(2,"putpx: % 1.3lf % 1.3lf\n",pt.x,pt.y); }
        if(xyY.Y > EPS) putpx(xyY2pt(xyY), nd->c);
        nd=nd->next;
    }

    flushimage(display, 1);

    return nil;
}

void
drawcl(CList *cln)
{
    CIExyY xyY;

    if(DEBUG) fprint(2,"drawcl\n");
    
    if(cln != nil) {
        xyY = RGB2xyY(cln->c, &cpr);
        if(xyY.Y > EPS) putpx(xyY2pt(xyY), cln->c);
    }

    flushimage(display, 1);
}

void
putpx(Point pt,RGBc c)
{
    /* TODO add colour support */
    if(ptinrect(pt,screen->r))
        draw(screen,Rect(pt.x,pt.y,pt.x+1,pt.y+1),display->black,0,ZP);
    if(ptinrect(Pt(pt.x-1,pt.y),screen->r))
        draw(screen,Rect(pt.x-1,pt.y,pt.x,pt.y+1),display->black,0,ZP);
    if(ptinrect(Pt(pt.x+1,pt.y),screen->r))
        draw(screen,Rect(pt.x+1,pt.y,pt.x+2,pt.y+1),display->black,0,ZP);
    if(ptinrect(Pt(pt.x,pt.y-1),screen->r))
        draw(screen,Rect(pt.x,pt.y-1,pt.x+1,pt.y),display->black,0,ZP);
    if(ptinrect(Pt(pt.x,pt.y+1),screen->r))
        draw(screen,Rect(pt.x,pt.y+1,pt.x+1,pt.y+2),display->black,0,ZP);
}

void
putmark(Point pt,RGBc c)
{
    /* TODO add colour support */
    line(screen,pt,pt,Enddisc,Enddisc,3,display->black,pt);
}

Point xyY2pt(CIExyY c)
{
    Point pt;

    pt.x=c.x*(WIDTH-1);
    pt.y=c.y*(HEIGHT-1);

    if(DEBUG) fprint(2,"xy2pt: % 1.3lf % 1.3lf % 1.3lf -> % 4d % 4d\n",c.x,c.y,c.Y,pt.x,pt.y);

    return pt;
}
    

void
printSCProf(SCProf *cp)
{/* TODO */}


CList
*addC(RGBc c,CList **clst)
{
    CList *nd=*clst;

    if(DEBUG) fprint(2,"addC: %d,%d,%d\n",c.R,c.G,c.B);

    if(nd==nil) {
        if((*clst = malloc(sizeof(CList))) == nil)
            return nil;
        (*clst)->c=c;
        (*clst)->next=nil;
        (*clst)->prev=nil;

        return nd;
    }

    while(nd->next != nil) nd=nd->next;

    if((nd->next = malloc(sizeof(CList))) == nil)
        return nil;
    nd->next->prev=nd;
    nd=nd->next;
    nd->c=c;
    nd->next=nil;

    return nd;
}
    
void
killCList(CList *clst)
{
    CList *nd=clst;

    if(nd==nil) return;

    while(nd->next!=nil) {
        nd=nd->next;
        free(nd->prev);
    }
    free(nd);
}

void
printCList(CList *clst)
{/* TODO */}

void
wexits(char *msg)
{
    killCList(cl);

    if(msg!=nil)
        fprint(2,"%s: %r\n",msg);

    exits(msg);
}
