#include "functionblocks.h"

static const uint16_t crc_table[256] =
{
    0x0000,0x1189,0x2312,0x329B,0x4624,0x57AD,0x6536,0x74BF,
    0x8C48,0x9DC1,0xAF5A,0xBED3,0xCA6C,0xDBE5,0xE97E,0xF8F7,
    0x1081,0x0108,0x3393,0x221A,0x56A5,0x472C,0x75B7,0x643E,
    0x9CC9,0x8D40,0xBFDB,0xAE52,0xDAED,0xCB64,0xF9FF,0xE876,
    0x2102,0x308B,0x0210,0x1399,0x6726,0x76AF,0x4434,0x55BD,
    0xAD4A,0xBCC3,0x8E58,0x9FD1,0xEB6E,0xFAE7,0xC87C,0xD9F5,
    0x3183,0x200A,0x1291,0x0318,0x77A7,0x662E,0x54B5,0x453C,
    0xBDCB,0xAC42,0x9ED9,0x8F50,0xFBEF,0xEA66,0xD8FD,0xC974,
    0x4204,0x538D,0x6116,0x709F,0x0420,0x15A9,0x2732,0x36BB,
    0xCE4C,0xDFC5,0xED5E,0xFCD7,0x8868,0x99E1,0xAB7A,0xBAF3,
    0x5285,0x430C,0x7197,0x601E,0x14A1,0x0528,0x37B3,0x263A,
    0xDECD,0xCF44,0xFDDF,0xEC56,0x98E9,0x8960,0xBBFB,0xAA72,
    0x6306,0x728F,0x4014,0x519D,0x2522,0x34AB,0x0630,0x17B9,
    0xEF4E,0xFEC7,0xCC5C,0xDDD5,0xA96A,0xB8E3,0x8A78,0x9BF1,
    0x7387,0x620E,0x5095,0x411C,0x35A3,0x242A,0x16B1,0x0738,
    0xFFCF,0xEE46,0xDCDD,0xCD54,0xB9EB,0xA862,0x9AF9,0x8B70,
    0x8408,0x9581,0xA71A,0xB693,0xC22C,0xD3A5,0xE13E,0xF0B7,
    0x0840,0x19C9,0x2B52,0x3ADB,0x4E64,0x5FED,0x6D76,0x7CFF,
    0x9489,0x8500,0xB79B,0xA612,0xD2AD,0xC324,0xF1BF,0xE036,
    0x18C1,0x0948,0x3BD3,0x2A5A,0x5EE5,0x4F6C,0x7DF7,0x6C7E,
    0xA50A,0xB483,0x8618,0x9791,0xE32E,0xF2A7,0xC03C,0xD1B5,
    0x2942,0x38CB,0x0A50,0x1BD9,0x6F66,0x7EEF,0x4C74,0x5DFD,
    0xB58B,0xA402,0x9699,0x8710,0xF3AF,0xE226,0xD0BD,0xC134,
    0x39C3,0x284A,0x1AD1,0x0B58,0x7FE7,0x6E6E,0x5CF5,0x4D7C,
    0xC60C,0xD785,0xE51E,0xF497,0x8028,0x91A1,0xA33A,0xB2B3,
    0x4A44,0x5BCD,0x6956,0x78DF,0x0C60,0x1DE9,0x2F72,0x3EFB,
    0xD68D,0xC704,0xF59F,0xE416,0x90A9,0x8120,0xB3BB,0xA232,
    0x5AC5,0x4B4C,0x79D7,0x685E,0x1CE1,0x0D68,0x3FF3,0x2E7A,
    0xE70E,0xF687,0xC41C,0xD595,0xA12A,0xB0A3,0x8238,0x93B1,
    0x6B46,0x7ACF,0x4854,0x59DD,0x2D62,0x3CEB,0x0E70,0x1FF9,
    0xF78F,0xE606,0xD49D,0xC514,0xB1AB,0xA022,0x92B9,0x8330,
    0x7BC7,0x6A4E,0x58D5,0x495C,0x3DE3,0x2C6A,0x1EF1,0x0F78
};

unsigned short crc16(unsigned char *buffer,unsigned long len)
{
    if(buffer&&len)
    {
        uint16_t dat;
        uint16_t crc = 0xFFFF;

        do
        {
            dat=*buffer++;
            crc=(uint16_t)((crc>>8)^crc_table[((crc^dat)&0xff)]);

        }
        while(--len);

        dat=crc=(uint16_t)(~crc);
        crc=(uint16_t)((crc<<8)|((dat>>8)&0xff));

        return crc;
    }

    return 0;
}


//functionblock
functionblock::functionblock(const char *n)
{
    strncpy(name,n,256);
    prev=0;
    next=0;
    svalid=false;
    evalid=false;
    verbose=0;
}
 

void functionblock::setdata(const unsigned char *d,uint32_t len)
{
    if(!svalid) return;
    if(!evalid) return;
    if(end>len) return;
    if(end<start) {evalid=false;svalid=false;return;}

    buffer=(unsigned char*)malloc(end-start);
    if(!buffer) return;

    memcpy(buffer,d+start,end-start);

}

bool functionblock::relocate(uint32_t r_type,uint32_t r_offset,uint32_t r_symval,const char* r_name)
{
    if(!svalid) return false;
    if(!evalid) return false;
    if(end<start) {evalid=false;svalid=false;return false;}
    if(!buffer) return false;

    if((r_offset>=start)&&(r_offset<=end))
    {
        if(verbose)
            printf("%s: reloc type:%u,offs:%.8x,symval:%.8x -> %s ",name,r_type,r_offset,r_symval,r_name);
        
        switch(r_type)
        {
            case 1:  //lsetup pcrel5m2 +ignorebytes 
            {
                if(verbose) printf("lsetup\n");
                int r=(r_symval-r_offset)/2;
                buffer[r_offset-start+1]=(unsigned char)((buffer[r_offset-start+1]&0xf0)|(r&0x0f));
                setignorebyte(r_offset+2);
                setignorebyte(r_offset+3);
                break;
            }
            case 3:  //pcrel11m2
            {
                if(verbose) printf("patching\n");
                int r=(r_symval-r_offset)/2;
                buffer[r_offset-start]=(unsigned char)r;
                buffer[r_offset-start+1]=(unsigned char)((buffer[r_offset-start+1]&0xfc)|(unsigned char)((r&0x3ff)>>8));
                break;
            }

            case 4:  //pcrel13m2
            {
                if(verbose) printf("patching\n");
                int r=(r_symval-r_offset)/2;
                buffer[r_offset-start]=(unsigned char)r;
                buffer[r_offset-start+1]=(unsigned char)((buffer[r_offset-start+1]&0xf0)|(unsigned char)((r&0xfff)>>8));
                break;
            }
            case 6:  //2 byte data (low/high) relocation
            case 7:
                if(verbose) printf("ignorebytes uimm16\n");
                setignorebyte(r_offset+2);
                setignorebyte(r_offset+3);
                break;
            case 14:  //call relocation
                if(verbose) printf("ignorebytes\n");
                setignorebyte(r_offset);
                setignorebyte(r_offset+2);
                setignorebyte(r_offset+3);
                break;
            case 224: //lsetup2 when symval>0
    /*            if(r_symval)
                {
                    printf("lsetup2\n");
                }
                else
                    printf("ignore reloc\n");
                break;*/
            case 210: //??
            case 225: //??
            case 226: //??
            case 227: //???
            case 19:
                if(verbose) printf("ignore reloc\n");
                break;
            default:
            {
                 if(verbose) printf("unknown\n");
                 exit(-1);
            }
        }
        return true;
    }

    return false;
}

void functionblock::printdata()
{
    if(buffer)
    { 
        for(uint32_t i=0;i<(end-start);i++)
        {
            if(ignorebytes[i])
                printf("..");
            else
                printf("%.2x",buffer[i]);
        }
        printf("\n");
    }
}

void functionblock::printpattern()
{
    uint32_t len=end-start;
    if(buffer)
    {
   
        //write bytes
        uint32_t flen=(len>32)?32:len;
        for(uint32_t i=0;i<flen;i++)
        {
            if(ignorebytes[i])
                printf("..");
            else
                printf("%.2x",buffer[i]);
        }
        
        for(uint32_t i=0;i<(32-flen);i++)
            printf("..");


        //write lengths and checksum
        uint32_t a=0;
        while(!ignorebytes[a+32]&&((a+32)<len)&&(a<255))
            a++;
     
        printf(" %.2x %.4x %.4x ",a,crc16(buffer+32,a),len);

        //write name
        printf(":0000 %s ",name); 

        //write public names

        //write tail bytes
        for(uint32_t i=a+32;i<len;i++)
        {
            if(ignorebytes[i])
                printf("..");
            else
                printf("%.2x",buffer[i]);
        }

        printf("\n");
    }
}


//functionblocks
void functionblocks::setblockstart(const char *name,uint32_t start)
{
    functionblock *lf=0,*f=blocks;
    while(f)
    {
        if(!f->isname(name))
        {
            f->setstart(start);
            break;
        }

	lf=f;
        f=f->getnext();
    }

    if(!f)
    {
        functionblock *nf=new functionblock(name);
        nf->setstart(start);
	
	if(lf)  lf->setnext(nf);
	else    blocks=nf;
    }
}

void functionblocks::setblockend(const char *name,uint32_t end)
{
    functionblock *lf=0,*f=blocks;
    while(f)
    {
        if(!f->isname(name))
        {
            f->setend(end);
            break;
        }

	lf=f;
        f=f->getnext();
    }

    if(!f)
    {
        functionblock *nf=new functionblock(name);
        nf->setend(end);

        if(lf)	lf->setnext(nf);
	else	blocks=nf;
    }
}


void functionblocks::setdata(const unsigned char *d,uint32_t len)
{
    functionblock *f=blocks;
    while(f)
    {
        f->setdata(d,len);
        f=f->getnext();
    }

}

void functionblocks::relocate(uint32_t type,uint32_t offset,uint32_t symval,const char* name)
{
    functionblock *f=blocks;
    while(f)
    {
        if(f->relocate(type,offset,symval,name))
	    break;

        f=f->getnext();
    }
}

void functionblocks::printsymbols()
{
    functionblock *f=blocks;
    while(f)
    {
        f->printsymbol();
	f->printdata();
        f=f->getnext();
    }
}


void functionblocks::printpattern()
{
    functionblock *f=blocks;
    while(f)
    {
        f->printpattern();
        f=f->getnext();
    }
}
