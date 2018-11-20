/* tar Header Block, from POSIX 1003.1-1990.  */

/* POSIX header.  */

struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char prefix[183];             /* 329 */
                                /* 512 */
};

int archivageArbo(int niv,char *rep,char *arbo,int *fn, char arch[], int symb);
void archivage(char arch[], char filename[], int ajout, int symb, int seul);
void desarchivage(char arch[]);
char *chemin(char *n);
void inserer(char arch[], char filename[]);
void ls(char arch[]);

#define TMAGIC   "ustar"        /* ustar and a null */
#define TMAGLEN  6
#define TVERSION "00"           /* 00 and no null */
#define TVERSLEN 2

/* Values used in typeflag field.  */
#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

#define XHDTYPE  'x'            /* Extended header referring to the
                                   next file in the archive */
#define XGLTYPE  'g'            /* Global extended header */

/* Bits used in the mode field, values in octal.  */
#define TSUID    04000          /* set UID on execution */
#define TSGID    02000          /* set GID on execution */
#define TSVTX    01000          /* reserved */
                                /* file permissions */
#define TUREAD   00400          /* read by owner */
#define TUWRITE  00200          /* write by owner */
#define TUEXEC   00100          /* execute/search by owner */
#define TGREAD   00040          /* read by group */
#define TGWRITE  00020          /* write by group */
#define TGEXEC   00010          /* execute/search by group */
#define TOREAD   00004          /* read by other */
#define TOWRITE  00002          /* write by other */
#define TOEXEC   00001          /* execute/search by other */

char* mode(mode_t md)
{
  char* droit = malloc(sizeof(char)*10);
  /* if(S_ISDIR(md))
    {
      strcat(droit,"d");
    }
  else if(S_ISLNK(md))
    {
      strcat(droit,"l");
    }
  else
  {*/
      strcat(droit,"-"); 
      //}
  if(S_IRUSR & md)
    {
      strcat(droit,"r");
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_IWUSR & md)
    {
      strcat(droit,"w");
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_ISUID & md)
    {
      strcat(droit,"s"); //setuid
    }
  else if(S_IXUSR & md)
    {
      strcat(droit,"x"); 
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_IRGRP & md)
    {
      strcat(droit,"r");
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_IWGRP & md)
    {
      strcat(droit,"w");
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_ISGID & md)
    {
      strcat(droit,"s"); //setgid
    }
  else if(S_IXGRP & md)
    {
      strcat(droit,"x");
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_IROTH & md)
    {
      strcat(droit,"r");
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_IWOTH & md)
    {
      strcat(droit,"w");
    }
  else
    {
      strcat(droit,"-");
    }
  if(S_ISVTX & md)
    {
      strcat(droit,"t"); //sticky
    }
  else if(S_IXOTH & md)
    {
      strcat(droit,"x");
    }
  else
    {
      strcat(droit,"-");
    }
   return droit;
}
