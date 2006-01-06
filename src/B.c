#include <u.h>
#include <libc.h>
#include <bio.h>

void
main()
{
    int i;
    double B;
    Biobuf bin;

    Binit(&bin,0,OREAD);

    Bgetd(&bin,&B);
    
    for(i=0;i<256;i++)
        fprint(1,"r8g8b8a8 255 %d %d 255\n",i,(int)B);
    for(i=0;i<256;i++)
        fprint(1,"r8g8b8a8 %d 255 %d 255\n",i,(int)B);

    exits(0);
}
