#ifndef DLBREADER_H
#define DLBREADER_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libelf/libelf.h>
#include <libelf/gelf.h>
#include <list>
#include <limits.h>

#include "functionblocks.h"
#include "functions.h"
#include "dojreader.h"

#define ERR -1 
#define AR_MAGIC         "!<elfa>\012"
#define AR_OFFSET_MAGIC  0x0a60

struct ar_header
{
    char ar_name[16];               // Name of this member
    char ar_date[12];               // File mtime
    char ar_uid[6];                 // Owner uid; printed as decimal
    char ar_gid[6];                 // Owner gid; printed as decimal
    char ar_mode[8];                // File mode, printed as octal
    char ar_size[10];               // File size, printed as decimal
    char ar_fmag[2];                // Should contain ARFMAG
};




class dlbreader
{
public:
    dlbreader(char *file,char *outpath,int verb);
    ~dlbreader();

    void print_filenames();
    void print_all_infos();
    void print_patterns();

    size_t write_next_file(char *name,size_t slen);
    bool remove_file();

private:
    bool read_header();

    int verbose;
    int infile;
    bool okay;
    char path[PATH_MAX];
    std::list<int32_t> offsets;
    std::list<int32_t>::iterator current;
};


#endif // DLBREADER_H
