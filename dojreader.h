#ifndef DOJREADER_H
#define DOJREADER_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <err.h>
#include <sysexits.h>
#include <libelf/libelf.h>
#include <libelf/gelf.h>

#include "functionblocks.h"
#include "functions.h"

#define ERR -1 

class dojreader
{
public:
    dojreader(char *file,int verb);
    ~dojreader();

    void print_all();
    void print_pattern();

private:
    Elf *elf;
    Elf32_Ehdr *elf_header;

    functionblocks fb;
    symbols symb;
    int verbose;
    bool okay;

    int infile;

    void process_file();

    void find_functionblocks();
    void fill_datablocks();
    void process_relocations();
    void process_sections();
    void fill_symbols();

};


#endif // DOJREADER_H
