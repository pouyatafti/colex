/*
 * colour.h -- part of the colex package
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

#define NLEVELS     256
#define NTonguePts  81

#define max2(a,b)       ( (a)>(b) ? (a) : (b) )
#define max3(a,b,c)     max2(a,max2(b,c)) 
#define min2(a,b)       ( (a)<(b) ? (a) : (b) )
#define min3(a,b,c)     min2(min2(a,b),c) 

#define distR2(x1,x2,y1,y2) sqrt( ((x1)-(x2))*((x1)-(x2)) + ((y1)-(y2))*((y1)-(y2)) )

#ifndef PI
#define PI      3.14159265        
#endif
#ifndef TWOPI
#define TWOPI   (2.0 * PI)  /* FIXME */
#endif

typedef struct CIExyY CIExyY;
struct CIExyY {
    double x;
    double y;
    double Y;
};

typedef struct CIELuv CIELuv;
struct CIELuv {
    double L;
    double u;
    double v;
};

typedef struct RGBc RGBc;
struct RGBc {
    uchar R;
    uchar G;
    uchar B;
};

typedef struct HSVc HSVc;
struct HSVc {
    double H;
    double S;
    uchar  V;
};
    
typedef struct SCProf SCProf;
struct SCProf {
    CIExyY  red;
    CIExyY  grn;
    CIExyY  blu;
    CIExyY  wht;    /* white point */
    double  gR;     /* red   Gamma */
    double  gG;     /* green Gamma */
    double  gB;     /* blue  Gamma */
    double  gcR[NLEVELS]; /* linearized (Gamma-corrected) R values */
    double  gcG[NLEVELS]; /* linearized (Gamma-corrected) G values */
    double  gcB[NLEVELS]; /* linearized (Gamma-corrected) B values */
    double  tangles[NTonguePts]; /* _unwrapped_ angles (in radians) for points from the CIE tongue */
}; /* Simple Colour PROFile */
void initSCProf(SCProf *,double);
void setgamma(SCProf *,double,double,double);

CIExyY RGB2xyY(RGBc,SCProf *);
CIELuv RGB2Luv(RGBc,SCProf *);
CIELuv xyY2Luv(CIExyY,SCProf *);

double domwlen(CIExyY *,SCProf *);
double hueLuv(CIELuv);

HSVc RGB2HSV(RGBc);

void initcolour(SCProf *);
int  updatecolour(SCProf *);

/* from NetPBM (ppmcie.c) */
extern CIExyY cietongue[NTonguePts];

/* TODO initialize this */
extern double cietonguefrqs[NTonguePts];
