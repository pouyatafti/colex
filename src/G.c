#include <u.h>
#include <libc.h>
#include <bio.h>

void
main()
{
    int i;
    double G;
    Biobuf bin;

    Binit(&bin,0,OREAD);

    Bgetd(&bin,&G);
    
    for(i=0;i<256;i++)
        fprint(1,"r8g8b8a8 %d %d 255 255\n",i,(int)G);
    for(i=0;i<256;i++)
        fprint(1,"r8g8b8a8 255 %d %d 255\n",(int)G,i);

    exits(0);
}
