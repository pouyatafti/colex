/*
 * colour.c -- part of the colex package
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

#define DEBUG   1
#define EPS     1e-6

#include "colour.h"

/* from NetPBM (ppmcie.c) */
CIExyY cietongue[NTonguePts] = {
    { 0.1741, 0.0050, 1.0 },               /* 380 nm */
    { 0.1740, 0.0050, 1.0 },
    { 0.1738, 0.0049, 1.0 },
    { 0.1736, 0.0049, 1.0 },
    { 0.1733, 0.0048, 1.0 },
    { 0.1730, 0.0048, 1.0 },
    { 0.1726, 0.0048, 1.0 },
    { 0.1721, 0.0048, 1.0 },
    { 0.1714, 0.0051, 1.0 },
    { 0.1703, 0.0058, 1.0 },
    { 0.1689, 0.0069, 1.0 },
    { 0.1669, 0.0086, 1.0 },
    { 0.1644, 0.0109, 1.0 },
    { 0.1611, 0.0138, 1.0 },
    { 0.1566, 0.0177, 1.0 },
    { 0.1510, 0.0227, 1.0 },
    { 0.1440, 0.0297, 1.0 },
    { 0.1355, 0.0399, 1.0 },
    { 0.1241, 0.0578, 1.0 },
    { 0.1096, 0.0868, 1.0 },
    { 0.0913, 0.1327, 1.0 },
    { 0.0687, 0.2007, 1.0 },
    { 0.0454, 0.2950, 1.0 },
    { 0.0235, 0.4127, 1.0 },
    { 0.0082, 0.5384, 1.0 },
    { 0.0039, 0.6548, 1.0 },
    { 0.0139, 0.7502, 1.0 },
    { 0.0389, 0.8120, 1.0 },
    { 0.0743, 0.8338, 1.0 },
    { 0.1142, 0.8262, 1.0 },
    { 0.1547, 0.8059, 1.0 },
    { 0.1929, 0.7816, 1.0 },
    { 0.2296, 0.7543, 1.0 },
    { 0.2658, 0.7243, 1.0 },
    { 0.3016, 0.6923, 1.0 },
    { 0.3373, 0.6589, 1.0 },
    { 0.3731, 0.6245, 1.0 },
    { 0.4087, 0.5896, 1.0 },
    { 0.4441, 0.5547, 1.0 },
    { 0.4788, 0.5202, 1.0 },
    { 0.5125, 0.4866, 1.0 },
    { 0.5448, 0.4544, 1.0 },
    { 0.5752, 0.4242, 1.0 },
    { 0.6029, 0.3965, 1.0 },
    { 0.6270, 0.3725, 1.0 },
    { 0.6482, 0.3514, 1.0 },
    { 0.6658, 0.3340, 1.0 },
    { 0.6801, 0.3197, 1.0 },
    { 0.6915, 0.3083, 1.0 },
    { 0.7006, 0.2993, 1.0 },
    { 0.7079, 0.2920, 1.0 },
    { 0.7140, 0.2859, 1.0 },
    { 0.7190, 0.2809, 1.0 },
    { 0.7230, 0.2770, 1.0 },
    { 0.7260, 0.2740, 1.0 },
    { 0.7283, 0.2717, 1.0 },
    { 0.7300, 0.2700, 1.0 },
    { 0.7311, 0.2689, 1.0 },
    { 0.7320, 0.2680, 1.0 },
    { 0.7327, 0.2673, 1.0 },
    { 0.7334, 0.2666, 1.0 },
    { 0.7340, 0.2660, 1.0 },
    { 0.7344, 0.2656, 1.0 },
    { 0.7346, 0.2654, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 },
    { 0.7347, 0.2653, 1.0 }                /* 780 nm */
};

/* TODO initialize this */
double cietonguefrqs[NTonguePts];

/* TODO MOVE to some better place */
double
adjustA(double a)
{
    return a < 0     ? adjustA(a + TWOPI) : (
           a > TWOPI ? adjustA(a - TWOPI) :
                       a);  
}

/* TODO MOVE to some better place */
double *
unwrapA(double *a,int n)
{
    do {
        while(a[n]-a[n-1] > PI)
            a[n-1] += TWOPI;
        while(a[n-1]-a[n] > PI)
            a[n-1] -= TWOPI;
    } while(--n>0);

    return a;
}

void
initSCProf(SCProf *cp,double g)
{
    /* 
     * CIE default values -- from Poynton's RGB to XYZ conversion matrix
     * 
     * FIXME I am currently confused as to whether Y values should all be 1.0
     * -- since we are looking at a constant luminosity plane ... or are we?
     */
    cp->red.x = 0.6400; cp->red.y = 0.3300; cp->red.Y = 0.212671;
    cp->grn.x = 0.3000; cp->grn.y = 0.6000; cp->grn.Y = 0.715160; 
    cp->blu.x = 0.1500; cp->blu.y = 0.0600; cp->blu.Y = 0.072169;
    cp->wht.x = 0.3127; cp->wht.y = 0.3291; cp->wht.Y = 1.0;

    setgamma(cp,g,g,g);
}

/* this implementation is blatantly WRONG */
void
setgamma(SCProf *cp,double gR,double gG,double gB)
{
    int i;
    
    if(gR>0) cp->gR=gR;
    if(gG>0) cp->gG=gG;
    if(gB>0) cp->gB=gB;

    if(DEBUG) fprint(2,"setgamma: % 2.2lf % 2.2lf % 2.2lf\n",gR,gG,gB);

    if(DEBUG) fprint(2,"setgamma: red: ");
    for(i=0;i<NLEVELS;i++) {
        if(gR>0) cp->gcR[i]=pow((double)i/(NLEVELS-1),gR);
        if(DEBUG) fprint(2,"% 2.2lf",cp->gcR[i]);
        if(gG>0) cp->gcG[i]=pow((double)i/(NLEVELS-1),gG);
        if(gB>0) cp->gcB[i]=pow((double)i/(NLEVELS-1),gB);
    }
    if(DEBUG) fprint(2,"\n");
}

CIExyY
RGB2xyY(RGBc c,SCProf *cp)
{
    CIExyY xyY = {0,0,0};
    double X,Y,Z;

    X   =   (cp->gcR)[c.R] * ((cp->red).x/(cp->red).y)*(cp->red).Y +
            (cp->gcG)[c.G] * ((cp->grn).x/(cp->grn).y)*(cp->grn).Y +
            (cp->gcB)[c.B] * ((cp->blu).x/(cp->blu).y)*(cp->blu).Y;

    Y   =   (cp->gcR)[c.R] * (cp->red).Y +
            (cp->gcG)[c.G] * (cp->grn).Y + 
            (cp->gcB)[c.B] * (cp->blu).Y;

    Z   =   (cp->gcR)[c.R] * ((1-(cp->red).x-(cp->red).y)/(cp->red).y)*(cp->red).Y +
            (cp->gcG)[c.G] * ((1-(cp->grn).x-(cp->grn).y)/(cp->grn).y)*(cp->grn).Y +
            (cp->gcB)[c.B] * ((1-(cp->blu).x-(cp->blu).y)/(cp->blu).y)*(cp->blu).Y;

    if(X+Y+Z>EPS) {
        xyY.x = X/(X+Y+Z);
        xyY.y = Y/(X+Y+Z);
        xyY.Y = Y;
    }

    if(DEBUG) fprint(2,"RGB2xyY: % 3d % 3d % 3d -> % 1.3lf % 1.3lf % 1.3lf\n",c.R,c.G,c.B,xyY.x,xyY.y,xyY.Y);
    
    return xyY;
}

CIELuv
RGB2Luv(RGBc c,SCProf *cp)
{
    CIELuv Luv = {0,0,0};
    double X,Y,Z;
    double Xn,Yn,Zn;

    X   =   (cp->gcR)[c.R] * ((cp->red).x/(cp->red).y)*(cp->red).Y +
            (cp->gcG)[c.G] * ((cp->grn).x/(cp->grn).y)*(cp->grn).Y +
            (cp->gcB)[c.B] * ((cp->blu).x/(cp->blu).y)*(cp->blu).Y;

    Y   =   (cp->gcR)[c.R] * (cp->red).Y +
            (cp->gcG)[c.G] * (cp->grn).Y + 
            (cp->gcB)[c.B] * (cp->blu).Y;

    Z   =   (cp->gcR)[c.R] * ((1-(cp->red).x-(cp->red).y)/(cp->red).y)*(cp->red).Y +
            (cp->gcG)[c.G] * ((1-(cp->grn).x-(cp->grn).y)/(cp->grn).y)*(cp->grn).Y +
            (cp->gcB)[c.B] * ((1-(cp->blu).x-(cp->blu).y)/(cp->blu).y)*(cp->blu).Y;

    /* FIXME X_N,Y_N,Z_N should go to the profile... probably it's better to
     * use XYZ rather than xyY for inner representation of colours */
    Xn  =   (cp->wht.x)/(cp->wht.y)*(cp->wht.Y);
    Yn  =   cp->wht.Y;
    Zn  =   (1.0-cp->wht.x-cp->wht.y)/(cp->wht.y)*(cp->wht.Y);

    /* 
     * formula from:
     *      D.L. MacAdam: Color Measurement, 2nd ed., p. 220.
     *      Springer-Verlag, 1985.
     */
    if(X+15*Y+3*Z > EPS) {
        Luv.L   =   116*pow(Y/Yn,1.0/3.0)-16;
        Luv.u   =   13*Luv.L*( 4*X/(X+15*Y+3*Z) - 4*Xn/(Xn+15*Yn+3*Zn) );
        Luv.v   =   13*Luv.L*( 9*Y/(X+15*Y+3*Z) - 9*Yn/(Xn+15*Yn+3*Zn) );
    }

    return Luv;
}

CIELuv
xyY2Luv(CIExyY c,SCProf *cp)
{
    CIELuv Luv = {0,0,0};
    double X,Y,Z;
    double Xn,Yn,Zn;


    if(c.y < EPS) return Luv;

    X   =   (c.x)/(c.y)*(c.Y);
    Y   =   c.Y;
    Z   =   (1.0-c.x-c.y)/(c.y)*(c.Y);
    
    /* FIXME X_N,Y_N,Z_N should go to the profile... probably it's better to
     * use XYZ rather than xyY for inner representation of colours */
    Xn  =   (cp->wht.x)/(cp->wht.y)*(cp->wht.Y);
    Yn  =   cp->wht.Y;
    Zn  =   (1.0-cp->wht.x-cp->wht.y)/(cp->wht.y)*(cp->wht.Y);

    /* 
     * formula from:
     *      D.L. MacAdam: Color Measurement, 2nd ed., p. 220.
     *      Springer-Verlag, 1985.
     */
    if(X+15*Y+3*Z > EPS) {
        Luv.L   =   116*pow(Y/Yn,1.0/3.0)-16;
        Luv.u   =   13*Luv.L*( 4*X/(X+15*Y+3*Z) - 4*Xn/(Xn+15*Yn+3*Zn) );
        Luv.v   =   13*Luv.L*( 9*Y/(X+15*Y+3*Z) - 9*Yn/(Xn+15*Yn+3*Zn) );
    }

    return Luv;
}

/* 
 * returns negative values for complementary freqs.
 * returns 0 in the case of error
 * also returns the pt of contact in xyY.
 */
/* FIXME FIXME FIXME PDT 2006-01-06 0135-0500 */
double
domwlen(CIExyY *xyY,SCProf *cp)
{
    double phi;
    double ret = 1;
    double a1,b1,c1,
           a2,b2,c2; 
    double D,xt,yt,wght;
    int i, j = 0;
 
    /* 
     * phi is the angle between the colour and the first tongue point
     * its range is tangles[NTonguePts-1]..TWOPI+tangles[NTonguePts-1] as this
     * allows for comparison with tangles
     */
    phi = adjustA(atan2(xyY->y-cp->wht.y,xyY->x-cp->wht.x) - cp->tangles[NTonguePts-1]);
    phi += (phi<0) ? TWOPI : 0;
    phi += cp->tangles[NTonguePts-1];

    i=NTonguePts-1;
    while( phi > cp->tangles[i] )
        if(--i < 0) { /* complementary wave length */
            i=NTonguePts-1;
            ret = -1;
            phi=adjustA(phi+PI-cp->tangles[NTonguePts-1]);
            phi += (phi<0) ? TWOPI : 0;
            phi += cp->tangles[NTonguePts-1];
            assert(j++ < 1); /* FIXME for debugging */
        }

    /* FIXME approixmate the projection on the CIE tongue */
    a1 = xyY->y - cp->wht.y;     /* parameters of the line */
    b1 = cp->wht.x - xyY->x;     /* connecting xyY to the  */
    c1 = cp->wht.x * xyY->y -    /* white point            */ 
         xyY->x * cp->wht.y;

    a2 = cietongue[i+1].y - cietongue[i].y;         /* parameters of the line */
    b2 = cietongue[i].x - cietongue[i+1].x;         /* connecting the two pts */
    c2 = cietongue[i].x * cietongue[i+1].y -        /* on the tongue          */ 
         cietongue[i+1].x * cietongue[i].y;

    /* FIXME quick hack for when the points on the tongue are too close and
     * lead to numerical issues */
    /*
    if(fabs(a2) < 1e-2 && fabs(b2) < 1e-2) {
        xt = (cietongue[i].x + cietongue[i+1].x) / 2;
        yt = (cietongue[i].y + cietongue[i+1].y) / 2;
        wght = 0.5;
    }
    else {
    */
    /* Cramer's rule */
    if(fabs(D = a1*b2-b1*a2) < EPS) {
        fprint(2,"domwlen: warning: fabs(D) < EPS\n");
        return 0;
    }
    xt = (c1*b2-b1*c2) / D; /* intersection */
    yt = (a1*c2-c1*a2) / D; /*    point     */

    wght = distR2(xt,cietongue[i].x,
                  yt,cietongue[i].y) /
           distR2(cietongue[i+1].x,cietongue[i].x,
                  cietongue[i+1].y,cietongue[i].y);

    ret *= wght * cietonguefrqs[i+1] + (1.0-wght) * cietonguefrqs[i];
    xyY->x = xt; xyY->y = yt; /* TODO xyY->Y */
    
    if(DEBUG)
        fprint(2,"domwlen: % 3.2lf (w:% 1.2lf % 2.3lf ..% 2.3lf ..% 2.3lf) % 3.2lf[% 2d] ..% 3.2lf[% 2d]\n",
                  ret,wght,cp->tangles[i],phi,cp->tangles[i+1],cietonguefrqs[i+1],i+1,cietonguefrqs[i],i);
        
    return ret;
}

double
hueLuv(CIELuv Luv)
{
    double h = atan2(Luv.v,Luv.u);
    return (h<0) ? h+TWOPI : h;
}

/* 
 * Ref: Foley et. al, Computer Graphics: Principles and Practice, 2nd ed.
 * Addison Wesley, 1993.
 */
HSVc
RGB2HSV(RGBc c)
{
    HSVc hsv;

    int max = max3(c.R,c.G,c.B),
        min = min3(c.R,c.G,c.B),
        del = max-min;

    hsv.V = max3(c.R,c.G,c.B);
    
    hsv.S = (max != 0) ? (double)(max-min)/max : 0;

    if(del == 0) { hsv.H = -1; return hsv; }

    hsv.H = (max == c.R) ? 60.0*(double)(c.G-c.B)/del + 0   : (
            (max == c.G) ? 60.0*(double)(c.B-c.R)/del + 120 :
                           60.0*(double)(c.R-c.G)/del + 240);

    if(hsv.H<0) hsv.H += 360;

    return hsv;
}

void
initcolour(SCProf *cp)
{
    int i;

    for(i=0;i<NTonguePts;i++)
        cietonguefrqs[i]=380+5*i;
    
    initSCProf(cp,1.0);
    updatecolour(cp);
}

/*
 * FIXME 
 * always call this after changing the colour profile.  it updates
 * tangles.
 */
int
updatecolour(SCProf *cp)
{
    int i;
    double chk=0;
   
    for(i=0;i<NTonguePts;i++) {
        cp->tangles[i] = atan2( (cietongue+i)->y - cp->wht.y,
                                (cietongue+i)->x - cp->wht.x);
        /* to check that the white point is inside the CIE tongue */
        chk+= (i==0) ? 0 : fabs(adjustA(cp->tangles[i] - cp->tangles[i-1]));
    }
    /*
    chk += cp->tangles[0] - cp->tangles[NTonguePts];
    if(fabs(chk - TWOPI) > EPS) return -1;
    */
    if(chk < PI+EPS) return -1;

    unwrapA(cp->tangles,NTonguePts);
    if(DEBUG) {
        fprint(2,"tangles:");
        for(i=0;i<NTonguePts;i++)
            fprint(2," % 2.3lf",cp->tangles[i]);
        fprint(2,"\n");
    }
    
    return 0;
}
