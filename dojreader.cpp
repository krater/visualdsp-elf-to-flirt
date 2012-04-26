#include "dojreader.h"


dojreader::dojreader(char *file,int verb)
{
    struct stat elf_stats;	// fstat struct

    elf_header=0;
    okay=false;
    verbose=verb;

    if((infile = open(file, O_RDWR)) == ERR)
    {
            printf("couldn't open %s\n", file);
            return;
    }

    if((fstat(infile, &elf_stats)))
    {
            printf("couldn't fstat %s\n", file);
            return;
    }

    if((elf_header = (Elf32_Ehdr *) malloc((size_t)(elf_stats.st_size))) == NULL)
    {
            printf("couldn't malloc\n");
            return;
    }

    if((read(infile, elf_header, (size_t)(elf_stats.st_size))) < elf_stats.st_size)
    {
            printf("couldn't read %s\n", file);
            return;
    }

    okay=true;

    /* Check libelf version first */
    if(elf_version(EV_CURRENT) == EV_NONE)
            printf("WARNING Elf Library is out of date!\n");

    Elf *start;

    start = elf_begin(infile, ELF_C_READ, NULL);	// Initialize 'elf' pointer to our file descriptor

    if(elf_kind(start)==ELF_K_ELF)
    {
        elf=start;
        process_file();
        return;	
    }

    if(elf_kind(start)==ELF_K_AR)
    {
        while((elf=elf_begin(infile,ELF_C_READ,start))!=NULL)
	{
            Elf_Arhdr *arh;

	    if((arh=elf_getarhdr(elf))==NULL)
               errx(EX_SOFTWARE,"elf_getarhdr() failed:%s.",elf_errmsg(-1));

            printf("%20s %d\n",arh->ar_name,(int)(arh->ar_size));

            if(elf_kind(start)==ELF_K_ELF) 
	        process_file();

            elf_next(elf);
            elf_end(elf);
	}

	return; 
    }

    printf("Filetype unknown or wrong\n");

}

dojreader::~dojreader()
{
    if(elf_header) free(elf_header);
    if(infile) close(infile);
}

void dojreader::process_file()
{
    find_functionblocks();
    fill_datablocks();
    fill_symbols();
    process_relocations();

    if(verbose)
    {
        //printf("Found %u symbols\n",fb.getsymbolcount());
        fb.printsymbols();
    }
		   
}

void dojreader::find_functionblocks()
{
    int i=0;
    Elf_Data *edata=0;
    GElf_Sym symbol;
    GElf_Shdr shdr;
    Elf_Scn *secdescr=0;

    while((secdescr = elf_nextscn(elf,secdescr))!=NULL)
    {
        gelf_getshdr(secdescr, &shdr);
        if(shdr.sh_type == SHT_SYMTAB)
        {
            edata = elf_getdata(secdescr, edata);
            for(i = 0;i < (int)(shdr.sh_size/shdr.sh_entsize); i++)
            {
                gelf_getsym(edata, (int)i, &symbol);

                if(!strncmp(elf_strptr(elf, shdr.sh_link, symbol.st_name),".LN.",4))
                {
                    //found end
                    char name[256];
                    strncpy(name,elf_strptr(elf, shdr.sh_link, symbol.st_name)+4,256);
                    name[strlen(name)-4]=0;

                    fb.setblockend(name,((uint32_t)symbol.st_value));
                }
                else if(!strncmp(elf_strptr(elf, shdr.sh_link, symbol.st_name),".LN",3))
                {
                    //found start
                    char name[256];
                    strncpy(name,elf_strptr(elf, shdr.sh_link, symbol.st_name)+3,256);

                    fb.setblockstart(name,((uint32_t)symbol.st_value));
                }
            }
        }
    }
}


void dojreader::fill_datablocks()
{
    Elf_Scn *scn=0;
    Elf_Data *edata=0;
    GElf_Shdr shdr;
    scn=0;

    while((scn = elf_nextscn(elf, scn)) != NULL)
    {
        gelf_getshdr(scn, &shdr);
        if(shdr.sh_type == SHT_PROGBITS)
        {
            if(!strcmp("program",elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name)))
            {
                edata = elf_getdata(scn, 0);
                fb.setdata(((const unsigned char*)(edata->d_buf)),edata->d_size);
            }
        }
    }
}

void dojreader::process_relocations()
{
    Elf_Scn *scn=0;
    Elf_Data *edata=0;
    GElf_Rela rela;
    GElf_Shdr shdr;
    scn=0;

    unsigned int symbol_count;
    unsigned int i;


    while((scn = elf_nextscn(elf, scn)) != NULL)
    {
        gelf_getshdr(scn, &shdr);

        if((shdr.sh_type == SHT_RELA) &&
            (!strcmp(".rela.program",elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name))))
        {
            edata=0;
            edata = elf_getdata(scn, edata);
            symbol_count = (unsigned int)(shdr.sh_size / shdr.sh_entsize);

            // process all relocations
            for(i = 0; i < symbol_count; i++)
            {
                if(gelf_getrela(edata,(int)i,&rela))
                {
                    int symnum=(int)GELF_R_SYM(rela.r_info);
                    fb.relocate(((uint32_t)GELF_R_TYPE(rela.r_info)),(uint32_t)rela.r_offset,symb.getoffset(symnum),symb.getname(symnum));
                }
                else
                    printf("%6d: err\n",i);
            }
        }
    }
}


void dojreader::process_sections()
{
    Elf_Scn *scn=0;
    Elf_Data *edata=0;
    GElf_Rela rela;
    GElf_Shdr shdr;

    scn=0;

    unsigned int symbol_count;
    unsigned int i;

    while((scn = elf_nextscn(elf, scn)) != NULL)
    {
        gelf_getshdr(scn, &shdr);
        if((shdr.sh_type == SHT_RELA) &&
            (!strcmp(".rela.program",elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name))))
        {

            edata=0;
            edata = elf_getdata(scn, edata);
            symbol_count = (unsigned int)(shdr.sh_size / shdr.sh_entsize);
            printf("%x xxx\n",(int)((uint64_t)edata));
            // loop through to grab all symbols
            for(i = 0; i < symbol_count; i++)
            {
                if(gelf_getrela(edata,(int)i,&rela))
                printf("%6d: %08x %08x %08x\n",
                i,
                ((int32_t)rela.r_offset),
                ((int32_t)GELF_R_TYPE(rela.r_info)),
                ((int32_t)GELF_R_SYM(rela.r_info)));
                else
                printf("%6d: err\n",i);
            }
        }
        else if(shdr.sh_type == SHT_PROGBITS)
        {
            edata = elf_getdata(scn, 0);
            fb.setdata((const unsigned char*)edata,edata->d_size);
        }
    }
}

void dojreader::fill_symbols()
{
    Elf_Scn *scn=0;
    Elf_Data *edata=0;
    GElf_Sym sym;
    GElf_Shdr shdr;

    unsigned int symbol_count;
    unsigned int i;

    while((scn = elf_nextscn(elf, scn)) != NULL)
    {
        gelf_getshdr(scn, &shdr);

        if((shdr.sh_type == SHT_SYMTAB) &&
            (!strcmp(".symtab",elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name))))
        {
            edata = elf_getdata(scn, edata);
            symbol_count = (unsigned int)(shdr.sh_size / shdr.sh_entsize);

            for(i = 0; i < symbol_count; i++)
            {
                // libelf grabs the symbol data using gelf_getsym()
                gelf_getsym(edata, (int)i, &sym);

                symbol *s=new symbol(
                        ((int32_t)sym.st_value),
                        ((int32_t)sym.st_shndx),
                        sym.st_info,
                        (const char*)elf_strptr(elf, shdr.sh_link, sym.st_name));

                symb.addsymbol(i,*s);
            }
        }
    }
}


void dojreader::print_pattern()
{
    fb.printpattern();
}

void dojreader::print_all()
{
    Elf_Scn *scn=0;
    Elf_Data *edata=0;
    GElf_Sym sym;
    GElf_Rela rela;
    GElf_Shdr shdr;

    if(!okay)
    {
        printf("file not correct loaded !\n");
        return;
    }

    while((scn = elf_nextscn(elf, scn)) != 0)
    {
        // point shdr at this section header entry
        gelf_getshdr(scn, &shdr);

        // print the section header type
        printf("Type: ");

        switch(shdr.sh_type)
        {
            case SHT_NULL: printf( "SHT_NULL\t");               break;
            case SHT_PROGBITS: printf( "SHT_PROGBITS");       break;
            case SHT_SYMTAB: printf( "SHT_SYMTAB");           break;
            case SHT_STRTAB: printf( "SHT_STRTAB");           break;
            case SHT_RELA: printf( "SHT_RELA\t");               break;
            case SHT_HASH: printf( "SHT_HASH\t");               break;
            case SHT_DYNAMIC: printf( "SHT_DYNAMIC");         break;
            case SHT_NOTE: printf( "SHT_NOTE\t");               break;
            case SHT_NOBITS: printf( "SHT_NOBITS");           break;
            case SHT_REL: printf( "SHT_REL\t");                 break;
            case SHT_SHLIB: printf( "SHT_SHLIB");             break;
            case SHT_DYNSYM: printf( "SHT_DYNSYM");           break;
            case SHT_INIT_ARRAY: printf( "SHT_INIT_ARRAY");   break;
            case SHT_FINI_ARRAY: printf( "SHT_FINI_ARRAY");   break;
            case SHT_PREINIT_ARRAY: printf( "SHT_PREINIT_ARRAY"); break;
            case SHT_GROUP: printf( "SHT_GROUP");             break;
            case SHT_SYMTAB_SHNDX: printf( "SHT_SYMTAB_SHNDX"); break;
            case SHT_NUM: printf( "SHT_NUM\t");                 break;
            case SHT_LOOS: printf( "SHT_LOOS\t");               break;
            case SHT_GNU_verdef: printf( "SHT_GNU_verdef");   break;
            case SHT_GNU_verneed: printf( "SHT_VERNEED");     break;
            case SHT_GNU_versym: printf( "SHT_GNU_versym");   break;
            default: printf( "(none) ");                      break;
        }

        // print the section header flags
        printf("\t(");
        if(shdr.sh_flags & SHF_WRITE) { printf("W"); }
        if(shdr.sh_flags & SHF_ALLOC) { printf("A"); }
        if(shdr.sh_flags & SHF_EXECINSTR) { printf("X"); }
        if(shdr.sh_flags & SHF_STRINGS) { printf("S"); }
        printf(")\t");

        // the shdr name is in a string table, libelf uses elf_strptr() to find it
        // using the e_shstrndx value from the elf_header
        printf("%s\n", elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name));
    }

    // Iterate through section headers again this time well stop when we find symbols
    scn=0;

    unsigned int symbol_count;
    unsigned int i;

    while((scn = elf_nextscn(elf, scn)) != NULL)
    {
        gelf_getshdr(scn, &shdr);

        // When we find a section header marked SHT_SYMTAB stop and get symbols
        if((shdr.sh_type == SHT_SYMTAB) &&
            (!strcmp(".symtab",elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name))))
        {
            printf("%s\n",elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name));
            // edata points to our symbol table
            edata = elf_getdata(scn, edata);

            // how many symbols are there? this number comes from the size of
            // the section divided by the entry size
            symbol_count = (unsigned int)(shdr.sh_size / shdr.sh_entsize);

            // loop through to grab all symbols
            for(i = 0; i < symbol_count; i++)
            {
                // libelf grabs the symbol data using gelf_getsym()
                gelf_getsym(edata, (int)i, &sym);

                // print out the value and size
                printf("%6d: %08x %d ", i, ((int32_t)sym.st_value), ((int32_t)sym.st_shndx));

                // type of symbol binding
                switch(ELF32_ST_BIND(sym.st_info))
                {
                    case STB_LOCAL: printf("LOCAL"); break;
                    case STB_GLOBAL: printf("GLOBAL"); break;
                    case STB_WEAK: printf("WEAK"); break;
                    case STB_NUM: printf("NUM"); break;
                    case STB_LOOS: printf("LOOS"); break;
                    case STB_HIOS: printf("HIOS"); break;
                    case STB_LOPROC: printf("LOPROC"); break;
                    case STB_HIPROC: printf("HIPROC"); break;
                    default: printf("UNKNOWN"); break;
                }

                printf("\t");

                // type of symbol
                switch(ELF32_ST_TYPE(sym.st_info))
                {
                    case STT_NOTYPE: printf("NOTYPE"); break;
                    case STT_OBJECT: printf("OBJECT"); break;
                    case STT_FUNC:  printf("FUNC"); break;
                    case STT_SECTION: printf("SECTION"); break;
                    case STT_FILE: printf("FILE"); break;
                    case STT_COMMON: printf("COMMON"); break;
                    case STT_TLS: printf("TLS"); break;
                    case STT_NUM: printf("NUM"); break;
                    case STT_LOOS: printf("LOOS"); break;
                    case STT_HIOS: printf("HIOS"); break;
                    case STT_LOPROC: printf("LOPROC"); break;
                    case STT_HIPROC: printf("HIPROC"); break;
                    default: printf("UNKNOWN"); break;
                }

                printf("\t");

                // the name of the symbol is somewhere in a string table
                // we know which one using the shdr.sh_link member
                // libelf grabs the string using elf_strptr()
                printf("%s\n", elf_strptr(elf, shdr.sh_link, sym.st_name));

                /*symbol *s=new symbol(
                        ((int32_t)sym.st_value),
                        ((int32_t)sym.st_shndx),
                        sym.st_info,
                        (const char*)elf_strptr(elf, shdr.sh_link, sym.st_name));

                symb.addsymbol(i,*s);*/
            }

        }

        else if((shdr.sh_type == SHT_RELA) &&
            (!strcmp(".rela.program",elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name))))
        {

            edata=0;
            edata = elf_getdata(scn, edata);
            symbol_count = (unsigned int)(shdr.sh_size / shdr.sh_entsize);
            printf("%x xxx\n",(int)((uint64_t)edata));
            // loop through to grab all symbols
            for(i = 0; i < symbol_count; i++)
            {
                if(gelf_getrela(edata,(int)i,&rela))
                    printf("%6d: %08x %08x %08x\n",
                    i,
                    ((int32_t)rela.r_offset),
                    ((int32_t)GELF_R_TYPE(rela.r_info)),
                    ((int32_t)GELF_R_SYM(rela.r_info)));
                else
                    printf("%6d: err\n",i);
            }
        }
        else if(shdr.sh_type == SHT_PROGBITS)
        {
            edata = elf_getdata(scn, 0);
            for(i=0;i<(edata->d_size);i++)
            {
                if(!(i%16))
                    printf("\n");

                printf(" %02x",((unsigned char*)(edata->d_buf))[i]);
            }

            printf("\n");
        }
    }
}
