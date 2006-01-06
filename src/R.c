#include <u.h>
#include <libc.h>
#include <bio.h>

void
main()
{
    int i;
    double R;
    Biobuf bin;

    Binit(&bin,0,OREAD);

    Bgetd(&bin,&R);
    
    for(i=0;i<256;i++)
        fprint(1,"r8g8b8a8 %d %d 255 255\n",(int)R,i);
    for(i=0;i<256;i++)
        fprint(1,"r8g8b8a8 %d 255 %d 255\n",(int)R,i);

    exits(0);
}
