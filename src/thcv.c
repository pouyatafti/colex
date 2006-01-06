/*
 * thcv.c -- (threaded cv.c) part of the colex package
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
#include <stdio.h>
#include <libc.h>
#include <bio.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>

#define DEBUG       1

#include "colour.h"
#include "io.h"

#define EPS         1e-6
#define MAX_STR     256
#define WIDTH       500
#define HEIGHT      500
#define GAMMAUNITS  10

RGBc RGBBlack   = {0,0,0};
RGBc RGBRed     = {NLEVELS-1,0,0};
RGBc RGBGreen   = {0,NLEVELS-1,0};
RGBc RGBBlue    = {0,0,NLEVELS-1};
RGBc RGBWhite   = {NLEVELS-1,NLEVELS-1,NLEVELS-1};

Image *pximg;

typedef enum {
    Icolour,    /* colour/mono          */
    Itriangle,  /* colour trangle       */
    Itongue,    /* colour tongue        */
    Igammabar,  /* gamma bar            */
    Idomwlen,   /* dominant wave-length */
    IhueLuv,    /* L*u*v* hue           */
    IhueHSV,    /* HSV hue              */
    Ipoints,    /* colour points        */
    Itext       /* labels               */
} ObjInvis;

u32int hide=0;
u32int updated=1;

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
    Kexit     = 'e'+'x'+'i'+'t'
} Cmd;

/* pixel types */
typedef enum {
    Ppoint,
    Pcross,
    Pdisc,
    Psqr,
    Plwedge,
    Prwedge,
    Pltri,
    Prtri,
    Putri,
    Pdtri,
    Plrtri,
    Pdiamond
} PxShape;

typedef struct CList CList;
struct CList {
    RGBc c;
    
    CList *prev;
    CList *next;
};
void killCList(CList *);
void printCList(CList *);
CList *addC(RGBc,CList **);
    
void resize(int);

void usage();
void wexits(char *);
char *cmd(char *);
char *redraw();
void drawcl(CList *);

Point xyY2pt(CIExyY);
Point xyY2Luv2pt(CIExyY);
void printSCProf(SCProf *);

void putpx(Point,RGBc,PxShape);

/* FIXME FIXME FIXME PDT 2006-01-05 1515-0500 */

Mouse m;

/* menus */
char *lbtn[] = {"CIExyY (x,y)","CIEL*u*v* (u*,v*)",0};
char *mbtn[] = {"red","green","blue","white","gammaR","gammaG","gammaB","gamma",0};
char *rbtn[] = {"clear","redraw","reset","send","exit","~~~","-colour","-triangle","-dom wlen","-hueLuv","-hueHSV",0};
Menu lmenu = {lbtn};
Menu mmenu = {mbtn};
Menu rmenu = {rbtn};

CList *cl=nil;
SCProf cpr;

extern uchar got;
extern uchar block;

Point (*topt)(CIExyY) = &xyY2pt;

void
threadmain(int argc, char **argv)
{
    char b[MAX_STR],*msg;
    char *bp=b;
    int i;
    CIExyY tmpxyY; /* FIXME for white point */

    if(DEBUG) fprint(2,"initSCProf\n");
    initcolour(&cpr);


    /* init interface */
    if(DEBUG) fprint(2,"initdraw\n");
    if(initdraw(0,0,"cv")==0) {
        fprint(2,"cv: initdraw failed: %r\n");
        wexits("initdraw");
    }
    /*
    if(screen->chan != RGBA32) {
        fprint(2,"bug: output channel type not supported yet\n");
        wexits("colourchannel");
    }
    */
    if(DEBUG) fprint(2,"screen channel: %s\n",chantostr(b,screen->chan));

    if((pximg=allocimage(display,Rect(0,0,1,1),RGB24,1,0)) == 0) {
        fprint(2,"cv: allocimage failed: %r\n");
        wexits("allocimage");
    }
    
    /* the next line should be replaced by a write to /dev/wctl in Plan 9. */
    if(DEBUG) fprint(2,"resize\n");
    drawresizewindow(Rect(0,0,WIDTH,HEIGHT));
    resize(0);
    
    /* init i/o: must be after call to initdraw() */
    if(DEBUG) fprint(2,"initio\n");
    initio();
    
    /*
    if((cl=(CList *)malloc(sizeof(CList)))==nil)
        wexits("memory");
     */
    
    /* main loop */
    if(DEBUG) fprint(2,"main loop\n");
    i=0;
    for(;;) {
        recvio();
        if(DEBUG) fprint(2,"io event:");

        if(isset(got,Rresize)) {
            if(DEBUG) fprint(2,"Rresize\n");
            resize(1);
            got &= !(1<<Rresize);
        }
        
        if(isset(got,Rstdin)) {
            if(DEBUG) fprint(2,"Rstdin\n");
            if((*bp = rdchar()) != 0) {
                if(*bp=='\n' || i>=MAX_STR-1) {
                    *bp=0; bp=b; i=0;
                    if(DEBUG) fprint(2,"cmd: %s\n",b);
                    if(DEBUG && (msg=cmd(b))!=nil)
                        fprint(2,"cmd: %s: failure: %s",b,msg);
                }
                else {
                    bp++; i++;
                }
                if(DEBUG) fprint(2,"%c[%d]",*(bp-1),i-1);
                /* if(DEBUG) fprint(2,"%d ",Bbuffered(&bin)); */
            }
        }

        if(isset(got,Rmouse)) {
            m=rdmouse();
            if(DEBUG) fprint(2,"Rmouse %d,%d b%d\n",m.xy.x,m.xy.y,m.buttons);
            
            if(m.buttons == 1) { /* lmenu */
                if(DEBUG) fprint(2,"lmenu: %d,%d\n",m.xy.x,m.xy.y);
                switch(menuhit(1, mctl, &lmenu,screen)) {
                case 0: /* CIE xyY      */
                    topt = &xyY2pt;
                    hide &= !(1<<Itongue);
                    updated = 1;
                    break;
                case 1: /* CIE L*u*v*   */
                    topt = &xyY2Luv2pt;
                    hide |=  (1<<Itongue);
                    updated = 1;
                    break;
                }
            }
                
            if(m.buttons == 2) { /* mmenu */
                if(DEBUG) fprint(2,"mmenu: %d,%d\n",m.xy.x,m.xy.y);
                switch(menuhit(2, mctl, &mmenu,screen)) {
                case 0: /* red    */
                    cpr.red.x=(double)m.xy.x/WIDTH;
                    cpr.red.y=1.0-(double)m.xy.y/HEIGHT;
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                case 1: /* green  */
                    cpr.grn.x=(double)m.xy.x/WIDTH;
                    cpr.grn.y=1.0-(double)m.xy.y/HEIGHT;
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                case 2: /* blue   */
                    cpr.blu.x=(double)m.xy.x/WIDTH;
                    cpr.blu.y=1.0-(double)m.xy.y/HEIGHT;
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                case 3: /* white  */
                    /* FIXME */
                    tmpxyY = cpr.wht; 
                    cpr.wht.x=(double)m.xy.x/WIDTH;
                    cpr.wht.y=1.0-(double)m.xy.y/HEIGHT;
                    if(updatecolour(&cpr) != 0) {
                        cpr.wht = tmpxyY;
                        updatecolour(&cpr);
                        fprint(2,"warning: invalid white point, reverting\n");
                    }
                    updated = 1;
                    break;
                case 4: /* gammaR */
                    setgamma(&cpr,(double)m.xy.x/(WIDTH/GAMMAUNITS),-1,-1);
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                case 5: /* gammaG */
                    setgamma(&cpr,-1,(double)m.xy.x/(WIDTH/GAMMAUNITS),-1);
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                case 6: /* gammaB */
                    setgamma(&cpr,-1,-1,(double)m.xy.x/(WIDTH/GAMMAUNITS));
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                case 7: /* gamma */
                    setgamma(&cpr,(double)m.xy.x/(WIDTH/GAMMAUNITS),
                                  (double)m.xy.x/(WIDTH/GAMMAUNITS),
                                  (double)m.xy.x/(WIDTH/GAMMAUNITS));
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                }
            }

            if(m.buttons == 4) { /* rmenu */
                if(DEBUG) fprint(2,"rmenu: %d,%d\n",m.xy.x,m.xy.y);
                switch(menuhit(3, mctl, &rmenu,screen)) {
                case 0:     /* clear    */
                    killCList(cl);
                    cl=nil;
                    updated = 1;
                    break;
                case 1:     /* redraw   */
                    updated = 1;
                    break;
                case 2:     /* reset    */
                    initSCProf(&cpr,1.0);
                    updatecolour(&cpr);
                    updated = 1;
                    break;
                case 3:     /* send     */
                    printSCProf(&cpr);
                    printCList(cl);
                    break;
                case 4:     /* exit     */
                    wexits(0);
                case 5:     /* ~~~      */
                    break;
                case 6:     /* colour   */
                    hide ^= (1<<Icolour);
                    rbtn[6] = isset(hide,Icolour) ? "+colour" : "-colour";
                    updated = 1;
                    break;
                case 7:     /* triangle */
                    hide ^= (1<<Itriangle);
                    rbtn[7] = isset(hide,Itriangle) ? "+triangle" : "-triangle";
                    updated = 1;
                    break;
                case 8:     /* dom wlen */
                    hide ^= (1<<Idomwlen);
                    rbtn[8] = isset(hide,Idomwlen) ? "+dom wlen" : "-dom wlen";
                    updated = 1;
                    break;
                case 9:     /* hueLuv   */
                    hide ^= (1<<IhueLuv);
                    rbtn[9] = isset(hide,IhueLuv) ? "+hueLuv" : "-hueLuv";
                    updated = 1;
                    break;
                case 10:    /* hueHSV   */
                    hide ^= (1<<IhueHSV);
                    rbtn[10] = isset(hide,IhueHSV) ? "+hueHSV" : "-hueHSV";
                    updated = 1;
                    break;
                }
            }
        }
        
        if((msg=redraw())!=nil)
            wexits(msg);
    }
    
    wexits(0);
}

char *
cmd(char *b)
{
    char *t=b;
    int kee=0;
    double A,B,C;
    RGBc c;
    CIExyY xyY,tmpxyY;

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
        updatecolour(&cpr);
        updated = 1;
        break;
    case Kgrn:
        if(sscanf(t,"%lf %lf %lf",&(xyY.x),&(xyY.y),&(xyY.Y)) < 3)
            return "incomplete";
        cpr.grn = xyY;
        updatecolour(&cpr);
        updated = 1;
        break;
    case Kblu:
        if(sscanf(t,"%lf %lf %lf",&(xyY.x),&(xyY.y),&(xyY.Y)) < 3)
            return "incomplete";
        cpr.blu = xyY;
        updatecolour(&cpr);
        updated = 1;
        break;
    case Kwht:
        if(sscanf(t,"%lf %lf %lf",&(xyY.x),&(xyY.y),&(xyY.Y)) < 3)
            return "incomplete";
        /* FIXME */
        tmpxyY = cpr.wht; 
        cpr.wht = xyY;
        if(updatecolour(&cpr) != 0) {
            cpr.wht = tmpxyY;
            updatecolour(&cpr);
            fprint(2,"warning: invalid white point, reverting\n");
        }
        updated = 1;
        break;
    case KgammaR:
        if(sscanf(t,"%lf",&A) < 1)
            return "incomplete";
        setgamma(&cpr,A,-1,-1);
        updatecolour(&cpr);
        updated = 1;
        break;
    case KgammaG:
        if(sscanf(t,"%lf",&A) < 1)
            return "incomplete";
        setgamma(&cpr,-1,A,-1);
        updatecolour(&cpr);
        updated = 1;
        break;
    case KgammaB:
        if(sscanf(t,"%lf",&A) < 1)
            return "incomplete";
        setgamma(&cpr,-1,-1,A);
        updatecolour(&cpr);
        updated = 1;
        break;
    case Kgamma:
        if(sscanf(t,"%lf %lf %lf",&A,&B,&C) < 3)
            return "incomplete";
        setgamma(&cpr,A,B,C);
        updatecolour(&cpr);
        updated = 1;
        break;
    case Kredraw:
        updated = 1;
        break;
    case Kreset:
        initSCProf(&cpr,1.0);
        updatecolour(&cpr);
        updated = 1;
        break;
    case Kclear:
        killCList(cl);
        cl=nil;
        updated = 1;
        break;
    case Kexit:
        wexits(0);
    default:
        return "unsupported";
    }
        
    return nil;
}

void
resize(int gw)
{
    if(gw && getwindow(display, Refnone) < 0) {
        fprint(2, "iv: could not reattach to window: %r\n");
        wexits("resized");
    }
    updated=1;
    redraw();
}

char *
redraw()
{
    CList *nd=cl;
    char s[MAX_STR];
    int i;
    /*
    CIExyY xyY,xyYt;
    CIELuv Luv;
    HSVc   hsv;
    double  wl,dv;
    Point pt; FOR DEBUGGING
    */
#ifdef RGB_TRIANGLE_FOR_CIELuv_IS_TO_BE_STUDIED
    double ii,tt;
    CIExyY RG,GB,BR,dRG,dGB,dBR,nx; /* FIXME for triangle */
    double mR,mG,mB;
#endif

    if(!updated) {
        flushimage(display,1);
        return nil;
    }

    updated = 0;
    if(DEBUG) fprint(2,"redraw: blank\n");
    draw(screen, screen->r, display->black, nil, ZP);

    /* gamma bar */
    if(!isset(hide,Igammabar)) {
        if(DEBUG) fprint(2,"redraw: gamma bar\n");
        for(i=0;i<WIDTH;i+=(WIDTH/50)) {
            if(!(i%(WIDTH/GAMMAUNITS))) {
                putpx(Pt(i,20),RGBWhite,Pdiamond);

                if(!isset(hide,Itext)) {
                    snprint(s,MAX_STR,"% 2d",i*GAMMAUNITS/WIDTH);
                    string(screen,Pt(i,21),display->white,ZP,display->defaultfont,s);
                }
            }
            putpx(Pt(i,20),RGBWhite,Pcross);
        }
        putpx(Pt(cpr.gG*WIDTH/GAMMAUNITS,20),RGBGreen,Pdisc);
        putpx(Pt(cpr.gR*WIDTH/GAMMAUNITS,20-6),RGBRed,Pdisc);
        putpx(Pt(cpr.gB*WIDTH/GAMMAUNITS,20+6),RGBBlue,Pdisc);
    }

    /* CIE tongue */
    if(!isset(hide,Itongue)) { /* the line connecting the lower part is not (?) a line on the u*v* plane */
        if(DEBUG) fprint(2,"redraw: cie tongue\n");
        for(i=0;i<NTonguePts-2;i++) {
            line(screen,(*topt)(cietongue[i]),
                        (*topt)(cietongue[i+1]),
                        Enddisc,Enddisc,0,display->white,
                        (*topt)(cietongue[i]));
        }
            line(screen,(*topt)(cietongue[NTonguePts-1]),
                        (*topt)(cietongue[0]),
                        Enddisc,Enddisc,0,display->white,
                        (*topt)(cietongue[NTonguePts-1]));
    }
        

    /* colour triangle */
    if(!isset(hide,Itriangle)) {
        if(DEBUG) fprint(2,"redraw: colour triangle\n");
        /* FIXME */
        if(!isset(hide,Itongue)) { /* don't draw for CIE L*u*v* */
            line(screen,(*topt)(cpr.red),(*topt)(cpr.grn),Enddisc,Enddisc,0,display->white,(*topt)(cpr.red));
            line(screen,(*topt)(cpr.grn),(*topt)(cpr.blu),Enddisc,Enddisc,0,display->white,(*topt)(cpr.red));
            line(screen,(*topt)(cpr.blu),(*topt)(cpr.red),Enddisc,Enddisc,0,display->white,(*topt)(cpr.red));
        }
            
#ifdef RGB_TRIANGLE_FOR_CIELuv_IS_TO_BE_STUDIED
#define ALEF 0.01
        dRG.x=ALEF*(cpr.grn.x-cpr.red.x);
        dRG.y=ALEF*(cpr.grn.y-cpr.red.y);
        dRG.Y=ALEF*(cpr.grn.Y-cpr.red.Y);
        
        dGB.x=ALEF*(cpr.blu.x-cpr.grn.x);
        dGB.y=ALEF*(cpr.blu.y-cpr.grn.y);
        dGB.Y=ALEF*(cpr.blu.Y-cpr.grn.Y);
        
        dBR.x=ALEF*(cpr.red.x-cpr.blu.x);
        dBR.y=ALEF*(cpr.red.y-cpr.blu.y);
        dBR.Y=ALEF*(cpr.red.Y-cpr.blu.Y);
        
        RG=cpr.red; GB=cpr.grn; BR=cpr.blu;

        mR=cpr.red.Y/cpr.red.y;
        mG=cpr.grn.Y/cpr.grn.y;
        mB=cpr.blu.Y/cpr.blu.y;
        /* 
         * see colour combination rules on p. 49 of MacAdam (1985) -- formulae
         * have been changed to have as the parameter the coefficient `ii' for
         * ii*(x1,y1)+(1-ii)*(x2,y2) and `tt' for tt*Y1+(1-tt)*Y2.
         */
        for(ii=0;ii<1.0;ii+=ALEF) {
            tt=mG*(1-ii-ALEF)/(mR-(mR-mG)*(1-ii-ALEF));
            nx.x = RG.x + dRG.x; nx.y = RG.y + dRG.y; nx.Y = tt*cpr.red.Y + (1-tt)*cpr.grn.Y;
            line(screen,(*topt)(RG),(*topt)(nx),Enddisc,Enddisc,0,display->white,(*topt)(cpr.red));
            RG = nx;
            
            tt=mB*(1-ii-ALEF)/(mG-(mG-mB)*(1-ii-ALEF));
            nx.x = GB.x + dGB.x; nx.y = GB.y + dGB.y; nx.Y = tt*cpr.grn.Y + (1-tt)*cpr.blu.Y;
            line(screen,(*topt)(GB),(*topt)(nx),Enddisc,Enddisc,0,display->white,(*topt)(cpr.grn));
            GB = nx;
            
            tt=mR*(1-ii-ALEF)/(mB-(mB-mR)*(1-ii-ALEF));
            nx.x = BR.x + dBR.x; nx.y = BR.y + dBR.y; nx.Y = tt*cpr.blu.Y + (1-tt)*cpr.red.Y;
            line(screen,(*topt)(BR),(*topt)(nx),Enddisc,Enddisc,0,display->white,(*topt)(cpr.blu));
            BR = nx;
        }
#endif

        putpx((*topt)(cpr.red),RGBRed,Pdisc);
        putpx((*topt)(cpr.grn),RGBGreen,Pdisc);
        putpx((*topt)(cpr.blu),RGBBlue,Pdisc);
        putpx((*topt)(cpr.wht),RGBWhite,Pdisc);

        if(!isset(hide,Itext)) {
            string(screen,(*topt)(cpr.red),display->white,ZP,display->defaultfont,"R");
            string(screen,(*topt)(cpr.grn),display->white,ZP,display->defaultfont,"G");
            string(screen,(*topt)(cpr.blu),display->white,ZP,display->defaultfont,"B");
            string(screen,(*topt)(cpr.wht),display->white,ZP,display->defaultfont,"W");
        }
    }
    
    /* other bars */
    if(DEBUG) fprint(2,"redraw: other bars\n");
    if(!isset(hide,Idomwlen)) {
        for(i = 40; i < HEIGHT-20; i+=(HEIGHT-60)/NTonguePts)
            putpx(Pt(WIDTH-60,i),RGBWhite,Pcross);
        /*
        line(screen,Pt(WIDTH-60,40),Pt(WIDTH-60,HEIGHT-20),
             Enddisc,Enddisc,0,display->white,ZP);
        */
        if(!isset(hide,Itext))
            string(screen,Pt(WIDTH-70,HEIGHT-20),display->white,ZP,display->defaultfont,"wl");
    }
    for(i = 40; i < HEIGHT-20; i+=(HEIGHT-60)/36) {
        if(!isset(hide,IhueLuv))
            putpx(Pt(WIDTH-40,i),RGBWhite,Pcross);
        if(!isset(hide,IhueHSV))
            putpx(Pt(WIDTH-20,i),RGBWhite,Pcross);
    }
        /*
        line(screen,Pt(WIDTH-40,40),Pt(WIDTH-40,HEIGHT-20),
             Enddisc,Enddisc,0,display->white,ZP);
        */
        /*
        line(screen,Pt(WIDTH-20,40),Pt(WIDTH-20,HEIGHT-20),
             Enddisc,Enddisc,0,display->white,ZP);
        */
    if(!isset(hide,IhueLuv) && !isset(hide,Itext))
        string(screen,Pt(WIDTH-45,HEIGHT-20),display->white,ZP,display->defaultfont,"h");
    if(!isset(hide,IhueHSV) && !isset(hide,Itext))
        string(screen,Pt(WIDTH-25,HEIGHT-20),display->white,ZP,display->defaultfont,"H");
            
    /* points & etc. */
    if(!isset(hide,Ipoints))
        while(nd != nil) {
            drawcl(nd);
            nd=nd->next;
        }

    flushimage(display, 1);

    return nil;
}

void
drawcl(CList *nd)
{
    CIExyY  xyY,xyYt;
    CIELuv  Luv;
    HSVc    hsv;
    double  wl,dv;

    if(DEBUG) fprint(2,"drawcl\n");
    
    if(nd != nil && !isset(hide,Ipoints)) {
        xyY = RGB2xyY(nd->c, &cpr);
        if(xyY.Y > EPS) putpx((*topt)(xyY), nd->c,Pcross);

        if(!isset(hide,Idomwlen)) {
            xyYt = xyY;
            dv   = (fabs( wl=domwlen(&xyYt,&cpr) ) - cietonguefrqs[NTonguePts-1])/
                   (cietonguefrqs[0]-cietonguefrqs[NTonguePts-1]);
            if(wl<0) {
                putpx(Pt(WIDTH-60,40+dv*(HEIGHT-60)), nd->c,Plwedge);
                if(!isset(hide,Itongue)) putpx((*topt)(xyYt), nd->c,Pcross);
            }
            else {
                putpx(Pt(WIDTH-60,40+dv*(HEIGHT-60)), nd->c,Prwedge);
                if(!isset(hide,Itongue)) putpx((*topt)(xyYt), nd->c,Pcross);
            }
        }

        /* 
         * TODO FIXME this is extremely inefficient, as xyY2Luv is called
         * twice for each point!
         */
        if(!isset(hide,IhueLuv)) {
            Luv = xyY2Luv(xyY,&cpr);
            dv = hueLuv(Luv) / TWOPI;
            putpx(Pt(WIDTH-40,40+dv*(HEIGHT-60)), nd->c,Prwedge);
        }

        if(!isset(hide,IhueHSV)) {
            hsv = RGB2HSV(nd->c);
            dv = hsv.H / 360.0;
            putpx(Pt(WIDTH-20,40+dv*(HEIGHT-60)), nd->c,Prwedge);
        }
    }

    flushimage(display, 1);
}

void
putpx(Point pt,RGBc c,PxShape s)
{
    uchar clr[3]={255,255,255};
    Point shp[4];
    
    if(!isset(hide,Icolour)) {
        clr[0] = c.B;
        clr[1] = c.G;
        clr[2] = c.R;
    }

    if(loadimage(pximg,pximg->r,clr,3)<0) {
        fprint(2, "loadimage failed: %r\n");
        return;
    }

    switch(s) {
    case Pcross:
        if(ptinrect(pt,screen->r))
            draw(screen,Rect(pt.x,pt.y,pt.x+1,pt.y+1),pximg,0,ZP);
        if(ptinrect(Pt(pt.x-1,pt.y),screen->r))
            draw(screen,Rect(pt.x-1,pt.y,pt.x,pt.y+1),pximg,0,ZP);
        if(ptinrect(Pt(pt.x+1,pt.y),screen->r))
            draw(screen,Rect(pt.x+1,pt.y,pt.x+2,pt.y+1),pximg,0,ZP);
        if(ptinrect(Pt(pt.x,pt.y-1),screen->r))
            draw(screen,Rect(pt.x,pt.y-1,pt.x+1,pt.y),pximg,0,ZP);
        if(ptinrect(Pt(pt.x,pt.y+1),screen->r))
            draw(screen,Rect(pt.x,pt.y+1,pt.x+1,pt.y+2),pximg,0,ZP);
        break;
    case Pdisc:
        line(screen,pt,pt,Enddisc,Enddisc,3,pximg,pt);
        break;
    case Plwedge:
        line(screen,addpt(pt,Pt(-2,0)),addpt(pt,Pt(-6,0)),Enddisc,Enddisc,0,pximg,pt);
        break;
    case Prwedge:
        line(screen,addpt(pt,Pt(2,0)),addpt(pt,Pt(6,0)),Enddisc,Enddisc,0,pximg,pt);
        break;
    case Psqr:
        shp[0]=addpt(pt,Pt(-3,-3)); shp[1]=addpt(pt,Pt(-3, 3));
        shp[2]=addpt(pt,Pt( 3, 3)); shp[3]=addpt(pt,Pt( 3,-3));
        fillpoly(screen,shp,4,0,pximg,pt);
        break;
    case Pltri:
        shp[0]=pt; shp[1]=addpt(pt,Pt(6,-3)); shp[2]=addpt(pt,Pt(6,3));
        fillpoly(screen,shp,3,0,pximg,pt);
        break;
    case Prtri:
        shp[0]=pt; shp[1]=addpt(pt,Pt(-6,-3)); shp[2]=addpt(pt,Pt(-6,3));
        fillpoly(screen,shp,3,0,pximg,pt);
        break;
    case Putri:
        shp[0]=pt; shp[1]=addpt(pt,Pt(-3,-6)); shp[2]=addpt(pt,Pt(3,-6));
        fillpoly(screen,shp,3,0,pximg,pt);
        break;
    case Pdtri:
        shp[0]=pt; shp[1]=addpt(pt,Pt(-3,6)); shp[2]=addpt(pt,Pt(3,6));
        fillpoly(screen,shp,3,0,pximg,pt);
        break;
    case Plrtri:
        shp[0]=pt; shp[1]=addpt(pt,Pt(-3,6)); shp[2]=addpt(pt,Pt(3,-6));
        fillpoly(screen,shp,3,0,pximg,pt);
        shp[0]=pt; shp[1]=addpt(pt,Pt(3,-6)); shp[2]=addpt(pt,Pt(-3,6));
        fillpoly(screen,shp,3,0,pximg,pt);
        break;
    case Pdiamond:
        shp[0]=addpt(pt,Pt(-3,0)); shp[1]=addpt(pt,Pt(0,-3));
        shp[2]=addpt(pt,Pt( 3,0)); shp[3]=addpt(pt,Pt(0, 3));
        fillpoly(screen,shp,4,0,pximg,pt);
    }

#ifdef ALTERNATIVE_PUTPX_IMPLEMENTATION
    if(ptinrect(pt,screen->r))
        loadimage(screen,Rect(pt.x,pt.y,pt.x+1,pt.y+1),clr,4);
    if(ptinrect(Pt(pt.x-1,pt.y),screen->r))
        loadimage(screen,Rect(pt.x-1,pt.y,pt.x,pt.y+1),clr,4);
    if(ptinrect(Pt(pt.x+1,pt.y),screen->r))
        loadimage(screen,Rect(pt.x+1,pt.y,pt.x+2,pt.y+1),clr,4);
    if(ptinrect(Pt(pt.x,pt.y-1),screen->r))
        loadimage(screen,Rect(pt.x,pt.y-1,pt.x+1,pt.y),clr,4);
    if(ptinrect(Pt(pt.x,pt.y+1),screen->r))
        loadimage(screen,Rect(pt.x,pt.y+1,pt.x+1,pt.y+2),clr,4);

    if(DEBUG) fprint(2,"putpx: c % 3d % 3d % 3d\n",(val>>24)&0xff,(val>>16)&0xff,(val>>8)&0xff);
#endif

#ifdef BW_PUTPX_IMPLEMENTATION    
    if(ptinrect(pt,screen->r))
        draw(screen,Rect(pt.x,pt.y,pt.x+1,pt.y+1),display->white,0,ZP);
    if(ptinrect(Pt(pt.x-1,pt.y),screen->r))
        draw(screen,Rect(pt.x-1,pt.y,pt.x,pt.y+1),display->white,0,ZP);
    if(ptinrect(Pt(pt.x+1,pt.y),screen->r))
        draw(screen,Rect(pt.x+1,pt.y,pt.x+2,pt.y+1),display->white,0,ZP);
    if(ptinrect(Pt(pt.x,pt.y-1),screen->r))
        draw(screen,Rect(pt.x,pt.y-1,pt.x+1,pt.y),display->white,0,ZP);
    if(ptinrect(Pt(pt.x,pt.y+1),screen->r))
        draw(screen,Rect(pt.x,pt.y+1,pt.x+1,pt.y+2),display->white,0,ZP);
#endif
}

void
putmark(Point pt,RGBc c)
{
    uchar clr[3]={255,255,255};
    
    if(!isset(hide,Icolour)) {
        clr[0] = c.B;
        clr[1] = c.G;
        clr[2] = c.R;
    }

    if(loadimage(pximg,pximg->r,clr,3)<0) {
        fprint(2, "loadimage failed: %r\n");
        return;
    }

    line(screen,pt,pt,Enddisc,Enddisc,3,pximg,pt);
}

Point xyY2pt(CIExyY c)
{
    Point pt;

    pt.x=c.x*(WIDTH-1);
    pt.y=(1.0-c.y)*(HEIGHT-1);

    /*
    if(DEBUG) fprint(2,"xy2pt: % 1.3lf % 1.3lf % 1.3lf -> % 4d % 4d\n",c.x,c.y,c.Y,pt.x,pt.y);
    */

    return pt;
}
    
Point
xyY2Luv2pt(CIExyY c)
{
    Point pt;
    CIELuv Luv = xyY2Luv(c,&cpr);

    /* FIXME current range: -250..250 */
    pt.x=((Luv.u+250.0)/500.0)*(WIDTH-1);
    pt.y=(1-(Luv.v+250.0)/500.0)*(HEIGHT-1);

    if(DEBUG) fprint(2, "xyY2Luv2pt: % 1.3lf,% 1.3lf,% 1.3lf -> % 3.2lf,% 3.2lf,% 3.2lf -> % 4d,% 4d\n",
                        c.x,c.y,c.Y,Luv.L,Luv.u,Luv.v,pt.x,pt.y);

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
    if(msg!=nil)
        fprint(2,"%s: %r\n",msg);

    killCList(cl);
    closeio();
    freeimage(pximg);

    threadexitsall(msg);
}
