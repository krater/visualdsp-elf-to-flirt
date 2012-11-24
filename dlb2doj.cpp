#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

#include "functionblocks.h"
#include "functions.h"
#include "dlbreader.h"

#define ERR -1

#define MAX_PATH 512

int main(int argc, char *argv[])
{
    char *filename, outdir[MAX_PATH]="";
    int c;

    printf("VisualDSP Elf Reverse Engineering Toolkit by Andreas Schuler (andreas at schulerdev.de)\n");

    while((c=getopt(argc,argv,"o:"))!=-1)
    {
        switch(c)
        {
            case 'o':
        printf("sdsdd\n");
                strncpy(outdir,optarg,MAX_PATH-2);
                if(outdir[strlen(outdir)-1]!='/')
                    strcat(outdir,"/");

                break;
            case '?':
/*                if ( optopt=='o')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
  */              
                printf("Exit with errors...\n");
                return 1;

            default:
                printf("Usage: dlb2doj -o outputdir filename\n");
                abort();
        }
    }

printf("a\n");
    if(optind<argc)
        filename=argv[optind];
    else
    {
        printf("Usage: dlb2doj -o outputdir filename\n");
        abort();
    }
    printf("b\n");
    if(strlen(outdir))
        mkdir(outdir,0777);

printf("c\n");
    //extract dlb lib 
    dlbreader *dlb=new dlbreader(filename,outdir,1);

printf("c\n");
    char name[256];
    while(dlb->write_next_file(name,256))
    {
        printf("Extracting %s%s\n",outdir,name);
    }

}

