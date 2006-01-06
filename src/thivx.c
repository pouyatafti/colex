/*
 * thivx.ck -- part of the colex package
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
 * $ 2006-01-05 $
 *
 */

#include <u.h>
#include <tiffio.h> /* linux libtiff */
#include <libc.h>
#include <draw.h>
#include <event.h>

#include "colour.h"

#define MAX_STR 256 /* FIXME */
#ifndef PI
#define PI      3.14159265        
#endif

#define isset(rg,flg)   ( (rg) & (1 << (flg)) )

typedef struct {
    Rectangle   r;
    uchar       *data;
} RawImg;
char *igetpx(char *,RawImg *,Point);
RawImg *rdim(char *);
#ifdef CREMAP_IS_NOT_NEEDE
uchar *cremap(ulong,uchar *,ulong);
#endif

typedef enum {
    Mred,
    Mgreen,
    Mblue
} ChanMask;
uchar maskc;

Image *duplim(RawImg *);
Image *isoYim(RawImg *);
Image *imsoLim(RawImg *);
Image *wyim(RawImg *);
Image *elim(RawImg *);
Image *hueHSVim(RawImg *);
Image *domwlenim(RawImg *);
Image *hueLuvim(RawImg *);

void usage();
void wexits(char *);

void eresized(int);
char *redraw(RawImg *);

RawImg  *im=nil;
ulong   outchan=RGB24;

int redrawreq = 0; /* this is set when toimg changes */
Image *(*toimg)(RawImg *)=&duplim; /* function ptr for conversion to viewable */

char title[MAX_STR] = "RGB";

void
main(int argc, char **argv)
{
    char *mbtn[] = {"RGB","RGB iso Y","RGB iso L*","Y","L*","hue HSV","hue L*u*v*","dom wave length",0};
    char *rbtn[] = {"redraw","exit",0};
    Menu mmenu = {mbtn};
    Menu rmenu = {rbtn};
    Mouse m;

    /* FIXME plumb
    int pfd = 0;
    Plumbmsg pm={"iv","colex","text",nil,0,nil};
     */

    char *fn=nil,*msg=nil;

    /* process cmd-line args */
    ARGBEGIN {
    case 'h':
        usage();
    case 'i':
        (fn = ARGF()) ? fn : nil;
        break;
    default:
        usage();
    } ARGEND

    /* read image file */
    if(fn == nil)
        usage();
    if((im=rdim(fn)) == nil) {
        fprint(2,"iv: could not read image\n");
        wexits("file");
    }
 
    /* FIXME open plumb port
    if((pfd = plumbopen("send",OREAD)) < 0) {
        fprint(2,"iv: plumbopen failed: %r\n");
        wexits("plumb");
    }
     */
        
    /* init interface */
    if(initdraw(0,0,"iv")==0) {
        fprint(2,"iv: initdraw failed: %r\n");
        wexits("initdraw");
    }
    if(screen->chan != outchan) {
        fprint(2,"bug: current output channel type not supported yet\n");
        wexits("outchannel");
    }

    einit(Emouse);
    drawresizewindow(im->r); /* this should be replaced by a write to /dev/wctl for Plan 9 rio. */
    eresized(0);
    /*
    if((msg=redraw(im)) != nil)
        wexits(msg);
    */

    /* main loop */
    for(;;){
        m = emouse();
        if(m.buttons == 1) {
            char *s[MAX_STR];
            if(ptinrect(m.xy,Rect( 0,0,20,20))) /* R btn */
                maskc ^= (1<<Mred);
            else
            if(ptinrect(m.xy,Rect(20,0,40,20))) /* G btn */
                maskc ^= (1<<Mgreen);
            else
            if(ptinrect(m.xy,Rect(40,0,60,20))) /* B btn */
                maskc ^= (1<<Mblue);
            else { 
                igetpx(s,im,m.xy);
                if(s!=nil) fprint(1,"%s\n",s); 
            }
            /* FIXME plumb
            { 
                pm.data=s; pm.ndata=strlen(pm.data);
                plumbsend(pfd,&pm);
            } */
        }
        if(m.buttons == 2)
            switch(emenuhit(2, &m, &menu)) {
                redrawreq = 1;
                if(DEBUG) fprint(2,"redrawreq set\n");
            case 0:     /* RGB */
                snprint(title,MAX_STR,"%c%c%c",
                                    "R-"[isset(ChanMask,Mred)],
                                    "G-"[isset(ChanMask,Mgreen)],
                                    "B-"[isset(ChanMask,Mblue)]);
                toimg = &duplim;
                break;
            case 1:
                snprint(title,MAX_STR,"RGB iso Y");
                toimg = &isoYim;
                break;
            case 2:
                snprint(title,MAX_STR,"RGB iso L*");
                toimg = &isoLim;
                break;
            case 3:
                snprint(title,MAX_STR,"Y channel");
                toimg = &wyim;
                break;
            case 4:
                snprint(title,MAX_STR,"L* channel");
                toimg = &elim;
                break;
            case 5:
                snprint(title,MAX_STR,"hue HSV");
                toimg = &hueHSVim;
                break;
            case 6:
                snprint(title,MAX_STR,"hue L*u*v*");
                toimg = &hueLuvim;
                break;
            case 7:
                snprint(title,MAX_STR,"dom wlen");
                toimg = &domwlenim;
                break;
            }

        if(m.buttons == 4)
            switch(emenuhit(3, &m, &menu)) {
            case 0:
                if((msg=redraw(im)) != nil)
                    wexits(msg);
                break;
            case 1:
                wexits(0);
            }

        if((msg=redraw())!=nil) wexits(msg);
    }
    
    wexits(0);
}

void
usage()
{
    fprint(1,"usage: iv -i file\n");
    wexits("usage");
}

void
wexits(char *msg)
{
    if(im!=nil) {
        free(im->data);
        free(im);
    }

    if(msg!=nil) fprint(2,"exits: %s: %r\n",msg);

    exits(msg);
}

void
eresized(int new)
{
    redrawreq = 1;

    if(new && getwindow(display, Refnone) < 0) {
        fprint(2, "iv: could not reattach to window: %r\n");
        wexits("resized");
    }
    draw(screen, screen->r, display->white, nil, ZP);
    if(im != nil)
        redraw(im);
    flushimage(display, 1);
}

char *
redraw(RawImg *img)
{
    Image *tmp;
    uchar clr[3];
    
    /*
    if((tmp=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        return "redraw";
    }

    if((chdat=cremap(outchan,img->data,Dx(img->r)*Dy(img->r))) == nil) {
        fprint(2, "iv: remap failed\n");
        return "remap";
    }

    if(loadimage(tmp, img->r, chdat, BytesPerPixel*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        return "loadimage";
    }
    */


    if(redrawreq) {
        if((tmp=(*toimg)(img)) == nil)
            return "toimg";
        draw(screen, screen->clipr, tmp, nil, ZP);
        freeimage(tmp);
        redrawreq = 0;
    }


    /* TODO set-up replicating pximag, as in thcv.c */
    /* R button */
    clr[0]=0;
    clr[1]=0;
    clr[2]=255^(isset(maskc,Mred)<<7);
    if(loadimage(pximg,pximg->r,clr,3)<0) {
        fprint(2, "loadimage failed: %r\n");
        return "redraw";
    }
    loadimage(screen,Rect(0,0,20,20),clr,3);
    /* G button */
    clr[0]=0;
    clr[1]=255^(isset(maskc,Mgreen)<<7);
    clr[2]=0;
    if(loadimage(pximg,pximg->r,clr,3)<0) {
        fprint(2, "loadimage failed: %r\n");
        return "redraw";
    }
    loadimage(screen,Rect(20,0,40,20),clr,3);
    /* B button */
    clr[0]=255^(isset(maskc,Mblue)<<7);
    clr[1]=0;
    clr[2]=0;
    if(loadimage(pximg,pximg->r,clr,3)<0) {
        fprint(2, "loadimage failed: %r\n");
        return "redraw";
    }
    loadimage(screen,Rect(40,0,60,20),clr,3);

    /* FIXME */
    string(screen,Pt(Dx(screen->r)-40,10),display->black,ZP,display->defaultfont,title);

    flushimage(display,1);
    
    return nil;
}

#ifdef CREMAP_IS_NOT_NEEDED
uchar *
cremap(ulong chan,uchar *data,ulong ndata)
{
    uchar *chdat;
    int i,j=0;

    if(chan!=RGB24) {
        fprint(2,"bug: only r8g8b8 displays are currently supported\n");
        return nil;
    }

    if((chdat=malloc(3*ndata))==nil)
        return nil;

    for(i=0;i<4*ndata;i+=4) {
        chdat[j++]  = data[i+1];
        chdat[j++]  = data[i+2];
        chdat[j++]  = data[i+3];
    }

    fprint(2,"cremap done\n");
    return chdat;
    
    /* FIXME */
    /*
    if(chan != im->chan) {
        fprint(2,"iv: channel conversion not yet implemented\n");
        return nil;
    }
    */
    
    return data;
}
#endif

RawImg *
rdim(char *fn)
{
    RawImg *img;
    TIFF *tif;
    u32int wd,ht;
    uchar *buf,*ret;
    int i,j;

    if((img=(RawImg *)malloc(sizeof(RawImg)))==nil)
        return nil;

    if((tif=TIFFOpen(fn,"rB"))==0) {
        free(img);
        return nil;
    }
   
    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH , &wd);
    TIFFGetField(tif,TIFFTAG_IMAGELENGTH, &ht);
    
    if((buf=malloc(4*wd*ht))==0) {
        free(img);
        TIFFClose(tif);
        return nil;
    }
    
    if((ret=malloc(3*wd*ht))==0) {
        free(buf);
        free(img);
        TIFFClose(tif);
        return nil;
    }
        
    if(!TIFFReadRGBAImageOriented(tif,wd,ht,(u32int *)buf,ORIENTATION_TOPLEFT,0)) {
        free(buf);
        free(ret);
        free(img);
        TIFFClose(tif);
        return nil;
    }
    
    j=0;
    for(i=0;i<4*wd*ht;i+=4) {
        ret[j++]=buf[i+1];
        ret[j++]=buf[i+2];
        ret[j++]=buf[i+3];
    }

    img->r=Rect(0,0,wd,ht);
    img->data=ret;
    
    TIFFClose(tif);
    return img;
}

/* 
  * TODO
  * support different channel formats
  */
char *
igetpx(char *s, RawImg *img, Point pt)
{
    char ch[MAX_STR];
    u32int *buf = img->data;
    u32int px = buf[pt.x+pt.y*Dx(img->r)];
	
    snprint(s,MAX_STR,"%s %d %d %d %d",chantostr(ch,img->chan),(px>>24)&0xFF,(px>>16)&0xFF,(px>>8)&0xFF,px&0xFF);

    return s;
}

Image *
duplim(RawImg *)
{
    Image *ret;
    uchar *buf;
    int i;

    if((ret=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        wexits("idupl");
    }

    if((buf=malloc(3*Dx(img->r)*Dy(img->r))) == 0) {
        fprint(2,"iv: could not allocate buffer: %r\n");
        wexits("idupl");
    }
    
    for(i=0;i<3*Dx(img->r)*Dy(img->r);i+=3) {
        buf[i  ] = isset(ChanMask,Mblue ) ? 0x00 : (img->data)[i  ];
        buf[i+1] = isset(ChanMask,Mgreen) ? 0x00 : (img->data)[i+1];
        buf[i+2] = isset(ChanMask,Mred  ) ? 0x00 : (img->data)[i+2];
    }

    if(loadimage(tmp, img->r, buf, BytesPerPixel*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        wexits("idupl");
    }

    free(buf);
    return ret;
}

Image *
wyim(RawImg *)
{
    Image *ret;
    uchar *buf;
    RGBc     c;
    CIExyY xyY;
    int i;

    if((ret=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        wexits("wy");
    }

    if((buf=malloc(3*Dx(img->r)*Dy(img->r))) == 0) {
        fprint(2,"iv: could not allocate buffer: %r\n");
        wexits("wy");
    }
    
    for(i=0;i<3*Dx(img->r)*Dy(img->r);i+=3) {
        c.R = (img->data)[i+2]; c.G = (img->data)[i+1]; c.B = (img->data)[i];
        xyY=RGB2xyY(c,&cpr);
        /* 
         * FIXME FIXME FIXME
         * this programme should be re-written in the multi-threaded mode.  a
         * named pipe will then be used to communicate colour profile info
         * from thcv
         */
        buf[i] = buf[i+1] = buf[i+2] = xyY.Y*256; 
    }

    if(loadimage(tmp, img->r, buf, 3*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        wexits("wy");
    }

    free(buf);
    return ret;
}

/*
 * FIXME
 * PDT 2006-01-03 CONTINUE FROM HERE -- BUT ALSO THE WHOLE PROGRAMME SHOULD BE
 * RE-WRITTEN AS MULTI-THREADED; SUPPORT FOR COLOUR PROFILES SHOULD ALSO BE
 * ADDED.
 */
Image *
elim(RawImg *)
{
    Image *ret;
    uchar *buf;
    RGBc     c;
    CIELuv Luv;
    int i;

    if((ret=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        wexits("el");
    }

    if((buf=malloc(3*Dx(img->r)*Dy(img->r))) == 0) {
        fprint(2,"iv: could not allocate buffer: %r\n");
        wexits("el");
    }
    
    for(i=0;i<3*Dx(img->r)*Dy(img->r);i+=3) {
        c.R = (img->data)[i+2]; c.G = (img->data)[i+1]; c.B = (img->data)[i];
        Luv=RGB2Luv(c,&cpr);
        buf[i] = buf[i+1] = buf[i+2] = 256*(Luv.L/100); 
        if(DEBUG) if(Luv.L>100 || Luv.L<0) fprint(2,"warning: L=% 3.2d\n",Luv.L);
    }

    if(loadimage(tmp, img->r, buf, 3*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        wexits("el");
    }

    free(buf);
    return ret;
}

Image *
domwlenim(RawImg *)
{
    Image *ret;
    uchar *buf;
    RGBc     c;
    CIExyY xyY;
    double tangle[NTonguePts];
    int i;

    if((ret=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        wexits("domwlenim");
    }

    if((buf=malloc(3*Dx(img->r)*Dy(img->r))) == 0) {
        fprint(2,"iv: could not allocate buffer: %r\n");
        wexits("domwlenim");
    }
    
    for(i=0;i<tangle
    for(i=0;i<3*Dx(img->r)*Dy(img->r);i+=3) {
        c.R = (img->data)[i+2]; c.G = (img->data)[i+1]; c.B = (img->data)[i];
        xyY=RGB2xyY(c,&cpr);

        /* 
         * FIXME FIXME FIXME PDT 2006-01-03 0030-0500 
         * I'm writing domwlen from colour.c.  see also yesterday's notes.
         */

        
        buf[i] = buf[i+1] = buf[i+2] = xyY.Y*256; 
    }

    if(loadimage(tmp, img->r, buf, 3*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        wexits("domwlenim");
    }

    free(buf);
    return ret;
}

Image *
hueLuvim(RawImg *img)
{
    Image *ret;
    uchar *buf;
    RGBc     c;
    CIELuv Luv;
    double   h;
    int i;

    if((ret=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        wexits("hueLuv");
    }

    if((buf=malloc(3*Dx(img->r)*Dy(img->r))) == 0) {
        fprint(2,"iv: could not allocate buffer: %r\n");
        wexits("hueLuv");
    }
    
    for(i=0;i<3*Dx(img->r)*Dy(img->r);i+=3) {
        c.R = (img->data)[i+2]; c.G = (img->data)[i+1]; c.B = (img->data)[i];
        Luv=RGB2Luv(c,&cpr);
        h=hueLuv(Luv); 
        buf[i] = buf[i+1] = buf[i+2] = 256*h; 
    }

    if(loadimage(tmp, img->r, buf, 3*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        wexits("hueLuv");
    }

    free(buf);
    return ret;
}

Image *
hueHSVim(RawImg *img)
{
    Image *ret;
    uchar *buf;
    RGBc     c;
    HSVc   hsv;
    int i;

    if((ret=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        return "hueHSV";
    }

    if((buf=malloc(3*Dx(img->r)*Dy(img->r))) == 0) {
        fprint(2,"iv: could not allocate buffer: %r\n");
        return "hueHSV";
    }
    
    for(i=0;i<3*Dx(img->r)*Dy(img->r);i+=3) {
        c.R = (img->data)[i];
        c.G = (img->data)[i+1];
        c.B = (img->data)[i+2];
        
        hsv = RGB2HSV(c);
        
        if(hsv.H < 0) {
            buf[i]=0; buf[i+1]=0; buf[i+2]=255;
            continue;
        }

        buf[i]=buf[i+1]=buf[i+2]= (h/360.0)*256;
    }

    if(loadimage(tmp, img->r, chdat, BytesPerPixel*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        return "hueHSV";
    }

    free(buf);
}

Image *
isoYim(RawImg *img)
{
    fprint(2,"bug: isoY not yet implemented\n");
    return idupl(img);
}

Image *
isoLim(RawImg *img)
{
    fprint(2,"bug: isoL not yet implemented\n");
    return idupl(img);
}
