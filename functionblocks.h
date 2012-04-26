#ifndef FUNCTIONBLOCKS_H
#define FUNCTIONBLOCKS_H

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <map>

class functionblock
{
public:
    functionblock(const char *n);

    //chain functions
    void setnext(functionblock *n)  {next=n;}
    void setprev(functionblock *p)  {prev=p;}

    functionblock* getnext()        {return next;}
    functionblock* getprev()        {return prev;}

    //content functions
    void setstart(uint32_t s)       {start=s;svalid=true;}
    void setend(uint32_t e)         {end=e;evalid=true;}

    void setdata(const unsigned char* d,uint32_t len);    //takes the complete buffer and cuts out his function
    void setignorebyte(uint32_t offset) {ignorebytes[offset]=1;}
    bool relocate(uint32_t type,uint32_t offset,uint32_t symval,const char* name);  //take all relocations, process only relocations in own data

    bool isname(const char *n)      {return strcmp(n,name);}
    bool isinrange(uint32_t addr)   {return svalid&&evalid&&(addr>=start)&&(addr<end);}

    const char* getname()           {return name;}
    void printsymbol()              {printf("name:%s %.8x - %.8x\n",name,start,end);}
    void printdata();

    void printpattern();

    void setverbose(int verb);

private:
    char name[256];
    uint32_t start,end;
    bool svalid,evalid;
    unsigned char *buffer;
    int verbose;

    std::map<uint32_t, char> ignorebytes;

    functionblock *prev,*next;
};


class functionblocks
{
public:
    functionblocks()    {blocks=0;}
    ~functionblocks()   {}

    void setblockstart(const char *name,uint32_t start);
    void setblockend(const char *name,uint32_t end);
    void setdata(const unsigned char *data,uint32_t len);
    void relocate(uint32_t type,uint32_t offset,uint32_t symval,const char* name);
    void printsymbols();
    void printpattern();
    int  getsymbolcount();
    void clear()        {blocks=0;}
private:
    functionblock *blocks;

};

#endif // FUNCTIONBLOCKS_H
