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
#include "dojreader.h"
#include "dlbreader.h"

#define ERR -1

//Sorry, code is a little uncompleted, will change this in the next decades....

int main(int argc, char *argv[])
{
    int all=0;
    char *filename=NULL, *outname=NULL;
    int c;

    printf("BlackElf Toolkit by Andreas Schuler\ndoj2map -? for help\n");

    while((c=getopt(argc,argv,"l:e:o:a"))!=-1)
    {
        switch(c)
        {
            case 'o':
                outname=optarg;
                break;
            case 'a':
                all=1;
                break;
            case '?':
                if (optopt=='l' || optopt=='o' || optopt=='e')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
                
                return 1;

            default:
                abort ();
        }
    }

    if(optind<argc)
        filename=argv[optind];
/*
    for(int i = optind; i < argc; i++)
        printf ("Non-option argument %s\n", argv[i]);
*/

    if(all)
    {
        //process all doj files in actual directory
        struct dirent *dp;

        DIR *dir=opendir(".");
        while((dp=readdir(dir))!=NULL)
        {
            if(!strcmp(dp->d_name+strlen(dp->d_name)-4,".doj"))
            {
                //printf("%s\t\t\n",dp->d_name);
                dojreader *dr=new dojreader(dp->d_name,0);
                dr->print_pattern();
                delete dr;
            }
        }
        closedir(dir);
        printf("---\n");
    }
    else
    {
        //process doj file
        dojreader *dr=new dojreader(filename,0);
        dr->print_all();
        dr->print_pattern();
        delete dr;
    }

}

