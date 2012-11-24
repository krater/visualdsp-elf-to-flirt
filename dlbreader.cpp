#include "dlbreader.h"

dlbreader::dlbreader(char *file,char *outpath,int verb)
{
    okay=false;
    verbose=verb;

    if((infile = open(file, O_RDWR)) == ERR)
    {
        printf("couldn't open %s\n", file);
        return;
    }

    if(!read_header())
    {
        printf("can't read header");
        return;
    }

    strncpy(path,outpath,PATH_MAX-1);
    okay=true;
}


dlbreader::~dlbreader()
{
    if(infile) close(infile);
}

bool dlbreader::read_header()
{
    char mg[8];
    read(infile,&mg,8);

    if(strcmp(mg,AR_MAGIC))
        return false;

    struct ar_header head;
    read(infile,&head,sizeof(ar_header));

    uint16_t hdrmg;
    hdrmg=(uint16_t)(head.ar_fmag[0]|((uint16_t)(head.ar_fmag[1]))<<8);

    if(hdrmg!=AR_OFFSET_MAGIC)
        return false;

    uint32_t cnt=0;

    read(infile,&cnt,sizeof(cnt));

    if(!cnt)
        return false;

    uint32_t old=0;
    while(cnt--)
    {
        uint32_t offset=0;
        read(infile,&offset,sizeof(offset));
        if(!offset)
            return false;

        if(old==offset)
            continue;

        old=offset;
        offsets.push_back(offset);
    }

    current=offsets.begin();

    return true;
}

size_t dlbreader::write_next_file(char *name,size_t slen)
{
    uint32_t start=*current;
    current++;
    uint32_t end=*current;
    uint32_t len=end-start-sizeof(ar_header);

    if(!okay)
        return 0;

    lseek(infile,start,SEEK_SET);
    struct ar_header head;
    read(infile,&head,sizeof(ar_header));

    char filename[256];
    strncpy(filename,head.ar_name,256);
    for(int i=0;i<255;i++)
    {
        if(filename[i]=='/') 
        {   
            filename[i]=0;
            break;
        }
    }

    filename[255]=0;

    char filepath[256+PATH_MAX];
    strncpy(filepath,path,PATH_MAX);
    strcat(filepath,filename);

    int fd=0;
    if((fd=open(filepath,O_WRONLY|O_CREAT,0666))==ERR)
    {
        printf("couldn't open %s\n", filepath);
        return 0;
    }

    size_t size=0;
    while(len--)
    {
        char c=0;
        if(!read(infile,&c,1))
            break;

        write(fd,&c,1);
        size++;
    }

    close(fd);

    strncpy(name,filename,slen);
    return size;
}

bool dlbreader::remove_file()
{
return true;   
}


