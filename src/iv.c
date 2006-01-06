/*
 * iv.c -- draw-based image viewer for X
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
 * $ 2005-12-22 $   initial version
 *
 */

#include <u.h>
#include <tiffio.h> /* linux libtiff */
#include <libc.h>
#include <draw.h>
#include <event.h>

#define BytesPerPixel 	3
#define MAX_STR		256 /* FIXME */

typedef struct {
    Rectangle   r;
    ulong       chan;
    uchar       *data;
} RawImg;
char *igetpx(char *,RawImg *,Point);
RawImg *rdim(char *);
uchar *cremap(ulong,uchar *,ulong);

void usage();
void wexits(char *);

void eresized(int);
char *redraw(RawImg *);

RawImg  *im=nil;
ulong   outchan=RGB24;

void
main(int argc, char **argv)
{
    char *rbtn[] = {"redraw","exit",0};
    Menu menu = {rbtn};
    Mouse m;

    /* FIXME
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
    /*
    if(screen->chan != outchan) {
        fprint(2,"bug: output channel type not supported yet\n");
        wexits("outchannel");
    }
    */
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
            igetpx(s,im,m.xy);
	    if(s!=nil) fprint(1,"%s\n",s); /* FIXME { 
                pm.data=s; pm.ndata=strlen(pm.data);
                plumbsend(pfd,&pm);
            } */
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

    exits(msg);
}

void
eresized(int new)
{
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
    uchar *chdat;

    if((tmp=allocimage(display,img->r,RGB24,0,0)) == 0) {
        fprint(2,"iv: could not allocate image: %r\n");
        return "redraw";
    }

    if((chdat=cremap(outchan,img->data,Dx(img->r)*Dy(img->r))) == nil) {
        fprint(2, "iv: remap failed\n");
        return "remap";
    }

    /* FIXME */
    if(loadimage(tmp, img->r, chdat, BytesPerPixel*Dx(img->r)*Dy(img->r)) < 0) {
        fprint(2, "iv: loadimage failed: %r\n");
        return "loadimage";
    }
    
    draw(screen, screen->clipr, tmp, nil, ZP);
    freeimage(tmp);
    /* free(chdat); */
    
    return nil;
}

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

RawImg *
rdim(char *fn)
{
    RawImg *img;
    TIFF *tif;
    u32int wd,ht;
    u32int *buf,b;
    int i,j;

    if((img=(RawImg *)malloc(sizeof(RawImg)))==nil)
        return nil;

    if((tif=TIFFOpen(fn,"rB"))==0) {
        free(img);
        return nil;
    }
   
    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH , &wd);
    TIFFGetField(tif,TIFFTAG_IMAGELENGTH, &ht);
    
    if((buf=malloc(sizeof(uint32)*wd*ht))==0) {
        free(img);
        TIFFClose(tif);
        return nil;
    }
        
    if(!TIFFReadRGBAImageOriented(tif,wd,ht,buf,ORIENTATION_TOPLEFT,0)) {
        free(buf);
        free(img);
        TIFFClose(tif);
        return nil;
    }
    
    for(i=0;i<wd;i++)
        for(j=0;j<ht;j++) {
            b=buf[j*wd+i];
            buf[j*wd+i] = (TIFFGetR(b)<<24)|(TIFFGetG(b)<<16)|(TIFFGetB( b)<<8)|TIFFGetA(b);
        }

    img->r=Rect(0,0,wd,ht);
    img->chan=RGBA32; /* FIXME */
    img->data=(uchar *)buf;
    
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
