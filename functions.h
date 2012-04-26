#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <map>

class symbol
{
public:
    symbol() {};
    symbol(uint32_t eoffset,uint32_t etype,uint32_t einfo,const char* ename)
    {
        offset=eoffset;
        type=etype;
        info=einfo;
        strncpy(name,ename,256);
    }

    ~symbol()  {}
    const char* getname() {return name;}
    uint32_t getoffset() {return offset;}

    void print()    {printf("%.8x %u %.8x %s\n",offset,type,info,name);}

private:
    char name[256];
    uint32_t type,info,offset;
};


class symbols
{
public:
    symbols()    {}
    ~symbols()   {}

    void addsymbol(int index,symbol &s)   {sym[index]=s;}

    void printsymbols()
    {
        for(std::map<int,symbol>::iterator ii=sym.begin(); ii!=sym.end(); ++ii)
        {
            int i=(*ii).first;
            symbol s=(*ii).second;

            printf("%u:",i);
            s.print();
        }
    }

    const char* getname(int index)	{return sym[index].getname();}
    uint32_t getoffset(int index)       {return sym[index].getoffset();}
    void clear()                        {sym.clear();}
private:
    std::map<int, symbol> sym;
};



#endif // SYMBOLS_H
