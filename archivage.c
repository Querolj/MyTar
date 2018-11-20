#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <pwd.h>
#include <time.h>
#include <math.h>

#include "archivage.h"



char *chemin(char *n)
{
  char *buf;
  char *chem ;
  buf=malloc(1024);
  chem=getcwd(buf,1024);
  strcat(chem,"/");
  strcat(chem,n);
  return  chem;
}

int convertOctalToDecimal(int octalNumber)
{
    int decimalNumber = 0, i = 0;

    while(octalNumber != 0)
    {
        decimalNumber += (octalNumber%10) * pow(8,i);
        ++i;
        octalNumber/=10;
    }

    i = 1;

    return decimalNumber;
}

int convertDecimalToOctal(int decimalNumber)
{
	int octalNumber = 0, i = 1;

	while (decimalNumber != 0)
	{
		octalNumber += (decimalNumber % 8) * i;
		decimalNumber /= 8;
		i *= 10;
	}

	return octalNumber;
}

int archivageArbo(int niv,char *rep,char *arbo,int *fn, char arch[], int symb)
{
	archivage(arch, rep ,1,symb,0);
	*fn++;
	DIR *d;int i = *fn;

	struct dirent *de;
	struct stat st;
	char Chem[200];
	if((d=opendir(rep))==NULL)
	{

		perror("ouverture du répertoire impossible");
		exit(1);
	}
	if(niv==0) 
	{
		sprintf(Chem,"%s",rep);
	}
	else 
	{
		sprintf(Chem,"%s/%s",arbo,rep);
	}
	if(chdir(rep)!=0)
		fprintf(stderr,"err :  %s\n",rep) ;
	else
	{
		printf("%s (chemin arbo)\n",Chem);
	}
	while(de=readdir(d)) 
	{
		int st2=stat(de->d_name,&st);
		if(S_ISDIR(st.st_mode)) 
		{
			if(strcmp(de->d_name,".")!=0 && strcmp(de->d_name,"..")!=0)
			{
				i++; 
				fn = &i;
				i+= archivageArbo(niv+1,de->d_name,Chem,fn, arch,symb);
			}
		}
		else
		{
			printf("%s/%s (chemin arbo)\n",Chem,de->d_name); 
			archivage(arch,de->d_name,1 ,symb,0);
			i++; 
			fn = &i; 
		}
	}
	chdir("..");
	return i;
}

void archivage(char arch[], char filename[], int ajout, int symb, int seul)
{
	struct posix_header posix_header;
	memset( &posix_header, 0, sizeof( struct posix_header ) );
	struct stat st;
	int archive, dc;
	// Si ajout = 1, alors on ajoute un fichier/dossier à une archive (lorsqu'on travaille sur une arborescence)
	if(ajout == 1)
	{
		if((archive=open(arch,O_WRONLY|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO))<0)
		{
			printf("(fonction archive, mode ajout) Archive impossible à ouvrir : %s\n", arch);
			exit(1);
		}
	}
	else if((archive=open(arch,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO))<0)
	{
		printf("Archive impossible à ouvrir : %s\n", arch);
		exit(1);
	}

	if((stat(filename,&st) != 0)|((dc = open(filename,O_RDONLY))<0))
	{
		fprintf(stderr,"impossible d archiver %s ",filename);
		exit(1);
	}
	//A partir d'ici, on construit le header de filename (qui peut etre un dossier ou un fichier)
	if(S_ISREG(st.st_mode))
	{
		posix_header.typeflag = '0';
	}
	else if(S_ISLNK(st.st_mode))
	{
		posix_header.typeflag = '2';
	}else if(S_ISDIR(st.st_mode))
	{
		posix_header.typeflag = '5';
	}
	else if(S_ISFIFO(st.st_mode))
	{
		posix_header.typeflag = '7';
	}
	
	char empty32[32];
	int i;
	for(i = 0;i<32;i++)
	{
		empty32[i] = '\0';
	}
	empty32[31] = '\0';
	//Le nom correspond au chemin du fichier par rapport à l'arborescence
	snprintf( posix_header.name, 100, "%s", chemin(filename));
	posix_header.name[99] = '\0';
	//%s signifie qu'on print un char* dans name, %06o signifie que c'est un int convertit en octal
	//donc st.st_mode est en décimal et posix_header.mode est en octal après la ligne suivante
	snprintf( posix_header.mode, 8, "%06o ", st.st_mode);
	posix_header.mode[7]='\0';
	snprintf( posix_header.uid, 8, "%06o ", st.st_uid );
	posix_header.uid[7]='\0';
	snprintf( posix_header.gid, 8, "%06o ", st.st_gid );
	posix_header.gid[7]='\0';
	snprintf( posix_header.size, 12, "%06o ", st.st_size);
	posix_header.size[11]='\0';
	snprintf( posix_header.mtime, 12, "%06o ", st.st_mtime);
	posix_header.mtime[11]='\0';
	if(posix_header.typeflag == '2')
	{
		snprintf( posix_header.linkname, 100, "%s", chemin(filename));
		posix_header.linkname[99]= '\0';
	}
	char* ustar = "ustar\0";
	snprintf( posix_header.magic, 6, "%s", ustar);
	snprintf( posix_header.version, 2, "%s", "00" );
	snprintf(posix_header.uname,32,"%s",empty32);
	snprintf(posix_header.gname,32,"%s",empty32);
	printf("size : %d, posix :  %s\n, version : %s, mtime %s \n",st.st_size, posix_header.size, posix_header.version, posix_header.mtime);
	char empty183[183];
	for(i = 0;i<183;i++)
	{
		empty183[i] = '\0';
	}
	snprintf(posix_header.prefix,183,"%s",empty183);
	//on réserve de la place pour le header pour ensuite faire le calcul puis on le met dans 
	//la structure et on convertit en octal
	memset( posix_header.chksum, ' ', 8);
	posix_header.chksum[6]='\0';
	posix_header.chksum[7]=' ';

	size_t checksum = 0;
	const unsigned char* bytes = &posix_header;
	for( i = 0; i < sizeof(struct posix_header); i++ ){
		checksum += bytes[i];
	}
	snprintf( posix_header.chksum, 8, "%06o ", convertDecimalToOctal(checksum ));
	//On a finit de remplir la structure donc on l'écrit dans l'archive
	write(archive, &posix_header, sizeof(posix_header));
//------------------------------------------------------------------------------------------
	//Si filename n'est pas un fichier, alors on doit copier son contenu
	if(posix_header.typeflag != '5')
	{
		//printf("------------pas un dir : %s\n",filename );
		//Ecriture dans le fichier
		char* buff;
		int n=0;
		int s; //Taille copier
		//Ici le principe est de c/c le contenu du fichier dans un buffer de max 1024 bit mais si le contenu est
		// inférieur à 1024 alors le buffer l'est aussi
		if((st.st_size ) >= 1024)
		{
			buff = malloc(sizeof(*buff) * 1024);
			s = 1024;
		}
		else if((st.st_size - n)<1024)
		{
			buff = malloc(sizeof(*buff) * st.st_size);
			s = st.st_size;
		}

		while(read(dc, buff, sizeof(*buff)*s)!=0)
		{
			//Le while ne fait qu'un tour si la taille du fichier est <1024, sinon on refait un tour 
			//, cad on réajuste le buffer et on refait un read puis un write
			if((st.st_size - n)<1024 && (st.st_size - n ) > 0)
			{
				write(archive, buff, sizeof(*buff)*s);
				//printf("contenu du buf : %s\n", buff);
				break;
			}
			else if ((st.st_size - n) >= 1024)
			{
				write(archive, buff, sizeof(*buff)*s);
				n += 1024;
			}
			else
			{
				break;
			}		    
			//printf("contenu du buff : %s\n", buff);
			if((st.st_size - n)<1024 && (st.st_size - n ) > 0)
			{
				buff = malloc(sizeof(*buff) * (st.st_size - n));
				s = st.st_size - n;
			}
		}
		//Ici on est censé avoir copier tout le contenu du fichier
		//printf("%d\n",(st.st_size % 512) );
		//La consigne nous dit qu'il faut remplir le contenu jusqu'à avoir un multiple de 512 :
		if(st.st_size % 512 != 0)
		{
			struct stat sta;
			int reste = 512-(st.st_size%512);
			unsigned char* fin_contenu = malloc(reste);
			write(archive, fin_contenu, reste);
			printf("RESTE : %d\n",reste );

		}
		if(seul==1)
		{
			unsigned char* empty1024[1024];
			for(i = 0;i<1024;i++)
			{
				empty1024[i] = '\0';
			}
			write(archive, empty1024, 1024);
		}
	}
}

void desarchivage(char arch[]){
	struct posix_header posix;
	int archive,fichier,s,n=0,d;
	off_t arch_size;
	char* buff;
	off_t t = 0;
	struct stat st;

	//On ouvre l'archive à désarchiver
	if((archive = open(arch, O_RDONLY))<0)
    {
      printf("archive impossible à lire");
      exit(1);
    }
    if(stat(arch,&st) != 0)
	{
		fprintf(stderr,"impossible de voir les stats de l'archive %s ",arch);
		exit(1);
	}
	arch_size = st.st_size;
    printf("desarchivage\n");
    int count = 0;
    //L'archive commence forcément par le header du premier fichier archiver, donc on applique un read avec
    //notre structure en guise de buffer
    while ( ( d = read(archive,&posix,sizeof(posix))) > 0)
    {
    	if(!(t< (arch_size - 1024)))
    	{
    		printf("fin du désarchivage (on atteind les 1024 derniers bits)\n");
    		break;
    	}
    	n = 0;
    	//Ici on chope les modes est la size parce qu'on en aura besoin après, et on doit les convertir en int et en
    	//décimal pour les utiliser (cad comme il était initialement)
    	int mode = convertOctalToDecimal(atoi(posix.mode));
    	int size = convertOctalToDecimal(atoi(posix.size));
    	printf("%d : posix :  %s, version : %s, size %d\nmode %d\n",count,posix.size, posix.version, posix.mtime, size, mode);
    	count += 1;
    	//Si c'est un dossier alors mkdir de name avec son mode
    	if(S_ISDIR(mode)!=0)
    	{
    		if(mkdir(posix.name,mode) == -1 )
    		{
    			fprintf(stderr,"création du dossier %s impossible\n", posix.name);
    			//exit(1);
    		}
    		else
    		{
    			t += sizeof(posix);//(4096+512);
    			lseek(archive,t , SEEK_SET);
    		}
    	}
    	//Si c'est un lien symbolique alors on le recréé, cette partie n'est pas terminé, il faudrait remplacer
    	//un des deux arguments par le chemin du lien original qu'on est censé écrire lors de l'archivage (et c'est pas 
    	//fait d'ailleurs...). C'est pour ca que char linkname[100] est là (dans la structure posix)
    	else if(S_ISLNK(mode))
    	{
    		printf("SYMLINK ::\n");
    		symlink(posix.name, posix.name);
    	}
    	//Donc c'est un fichier 
    	else
    	{
    		// On recréé le fichier avec son mode
    		if((fichier = open(posix.name, O_WRONLY|O_CREAT,mode)) < 0)
    		{
    			fprintf(stderr, "création du fichier %s échoué\n", posix.name);
    		}
    		else
    		{
    			//Idem que pour l'archivage ici, on recopie tous le contenu mais dans le fichier
    			//qu'on vient de créer cette fois
    			if(size >= 1024)
    			{
    				buff = malloc(sizeof(*buff) * 1024);
    				s = 1024;
    			}
    			else if(size <1024 && size > 0)
    			{
    				buff = malloc(sizeof(*buff) * size);
    				s = size;
    			}

    			while(read(archive, buff, sizeof(*buff)*s)!=0)
    			{
    				if((size - n)<1024 && (size - n ) > 0)
    				{
    					write(fichier, buff, sizeof(*buff)*s);
    					break;
    				}
    				else if ((size - n) >= 1024)
    				{
    					write(fichier, buff, sizeof(*buff)*s);
    					n += 1024;
    				}
    				else
    				{
    					break;
    				}		    
    				if((size - n)<1024 && (size - n) > 0)
    				{
    					buff = malloc(sizeof(*buff) * (size - n));
    					s = size - n;
    				}

    			}

    			close(fichier);
    			//On avance dans l'archive pour arriver à un multiple de 512, puis on passe au header suivant
    			t +=  sizeof(posix)+size;
    			if(size % 512 != 0)
    			{
    				int reste = 512-(size%512);
    				t += reste;
    			}

    			printf("T : %d\n",t );
    			lseek(archive, t , SEEK_SET);
    		}
    	}

    }
    free(buff);
    close(archive);
}

void inserer(char arch[], char filename[])
{
	struct posix_header posix_header;
	int archive,dc;
	off_t arch_size;
	struct stat archst;
	struct stat filest;

	
	if((archive = open(arch, O_RDWR|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO))<0)
    {
      printf("archive impossible à lire/écrire");
      exit(1);
    }
    if(stat(arch,&archst) != 0)
	{
		fprintf(stderr,"impossible de voir les stats de l'archive %s ",arch);
		exit(1);
	}
	arch_size = archst.st_size;
	lseek(archive,arch_size-1024,SEEK_SET);

	if((stat(filename,&filest) != 0)|((dc = open(filename,O_RDONLY))<0))
	{
		fprintf(stderr,"impossible d archiver %s ",filename);
		exit(1);
	}
	//A partir d'ici, on construit le header de filename (qui peut etre un dossier ou un fichier)
	if(S_ISREG(filest.st_mode))
	{
		posix_header.typeflag = '0';
	}
	else if(S_ISLNK(filest.st_mode))
	{
		posix_header.typeflag = '2';
	}else if(S_ISDIR(filest.st_mode))
	{
		posix_header.typeflag = '5';
	}
	else if(S_ISFIFO(filest.st_mode))
	{
		posix_header.typeflag = '7';
	}
	
	char empty32[32];
	int i;
	for(i = 0;i<32;i++)
	{
		empty32[i] = '\0';
	}
	empty32[31] = '\0';
	//Le nom correspond au chemin du fichier par rapport à l'arborescence
	snprintf( posix_header.name, 100, "%s", chemin(filename));
	posix_header.name[99] = '\0';
	//%s signifie qu'on print un char* dans name, %06o signifie que c'est un int convertit en octal
	//donc st.st_mode est en décimal et posix_header.mode est en octal après la ligne suivante
	snprintf( posix_header.mode, 8, "%06o ", filest.st_mode);
	posix_header.mode[7]='\0';
	snprintf( posix_header.uid, 8, "%06o ", filest.st_uid );
	posix_header.uid[7]='\0';
	snprintf( posix_header.gid, 8, "%06o ", filest.st_gid );
	posix_header.gid[7]='\0';
	snprintf( posix_header.size, 12, "%06o ", filest.st_size);
	posix_header.size[11]='\0';
	snprintf( posix_header.mtime, 12, "%06o ", filest.st_mtime);
	posix_header.mtime[11]='\0';
	if(posix_header.typeflag == '2')
	{
		snprintf( posix_header.linkname, 100, "%s", chemin(filename));
		posix_header.linkname[99]= '\0';
	}
	char* ustar = "ustar\0";
	snprintf( posix_header.magic, 6, "%s", ustar);
	snprintf( posix_header.version, 2, "%s", "00" );
	snprintf(posix_header.uname,32,"%s",empty32);
	snprintf(posix_header.gname,32,"%s",empty32);
	printf("size : %d, posix :  %s\n, version : %s, mtime %s \n",filest.st_size, posix_header.size, posix_header.version, posix_header.mtime);
	char empty183[183];
	for(i = 0;i<183;i++)
	{
		empty183[i] = '\0';
	}
	snprintf(posix_header.prefix,183,"%s",empty183);
	//on réserve de la place pour le header pour ensuite faire le calcul puis on le met dans 
	//la structure et on convertit en octal
	memset( posix_header.chksum, ' ', 8);
	posix_header.chksum[6]='\0';
	posix_header.chksum[7]=' ';

	size_t checksum = 0;
	const unsigned char* bytes = &posix_header;
	for( i = 0; i < sizeof(struct posix_header); i++ ){
		checksum += bytes[i];
	}
	snprintf( posix_header.chksum, 8, "%06o ", convertDecimalToOctal(checksum ));
	//On a finit de remplir la structure donc on l'écrit dans l'archive
	write(archive, &posix_header, sizeof(posix_header));
//------------------------------------------------------------------------------------------
	//Si filename n'est pas un fichier, alors on doit copier son contenu
	if(posix_header.typeflag != '5')
	{
		//printf("------------pas un dir : %s\n",filename );
		//Ecriture dans le fichier
		char* buff;
		int n=0;
		int s; //Taille copier
		//Ici le principe est de c/c le contenu du fichier dans un buffer de max 1024 bit mais si le contenu est
		// inférieur à 1024 alors le buffer l'est aussi
		if((filest.st_size ) >= 1024)
		{
			buff = malloc(sizeof(*buff) * 1024);
			s = 1024;
		}
		else if((filest.st_size - n)<1024)
		{
			buff = malloc(sizeof(*buff) * filest.st_size);
			s = filest.st_size;
		}

		while(read(dc, buff, sizeof(*buff)*s)!=0)
		{
			//Le while ne fait qu'un tour si la taille du fichier est <1024, sinon on refait un tour 
			//, cad on réajuste le buffer et on refait un read puis un write
			if((filest.st_size - n)<1024 && (filest.st_size - n ) > 0)
			{
				write(archive, buff, sizeof(*buff)*s);
				//printf("contenu du buf : %s\n", buff);
				break;
			}
			else if ((filest.st_size - n) >= 1024)
			{
				write(archive, buff, sizeof(*buff)*s);
				n += 1024;
			}
			else
			{
				break;
			}		    
			//printf("contenu du buff : %s\n", buff);
			if((filest.st_size - n)<1024 && (filest.st_size - n ) > 0)
			{
				buff = malloc(sizeof(*buff) * (filest.st_size - n));
				s = filest.st_size - n;
			}
		}
		//Ici on est censé avoir copier tout le contenu du fichier
		//printf("%d\n",(st.st_size % 512) );
		//La consigne nous dit qu'il faut remplir le contenu jusqu'à avoir un multiple de 512 :
		if(filest.st_size % 512 != 0)
		{
			int reste = 512-(filest.st_size%512);
			unsigned char* fin_contenu = malloc(reste);
			write(archive, fin_contenu, reste);
			printf("RESTE : %d\n",reste );
		}
	printf("pass\n");
	}
}

void ls(char arch[])
{
	struct passwd pass;
	struct posix_header posix;
	int archive;
	off_t t = 0, arch_size;
	char* md;
	struct tm *tm;
	char buff[200];
	struct stat st;

	if(stat(arch,&st) != 0)
	{
		fprintf(stderr,"impossible de voir les stats de l'archive %s ",arch);
		exit(1);
	}

	if((archive = open(arch, O_RDONLY))<0)
	{
		fprintf(stderr,"archive impossible à lire");
		exit(1);
	}
	arch_size = st.st_size;
	int k = 0;
	while ( (read(archive,&posix,sizeof(posix))) >0)
	{
		if(!(t< (arch_size - 1024)))
    	{
    		break;
    	}
		int size = convertOctalToDecimal(atoi(posix.size));
		int mtime = convertOctalToDecimal(atoi(posix.mtime));
		int lmode = convertOctalToDecimal(atoi(posix.mode));
		k++;
		if(S_ISDIR(lmode) != 0)
		{
			t += sizeof(posix);
		}
		else
		{
			t +=  sizeof(posix)+size;    
		}
		md = mode(lmode);
      //getpwam(et.path);

      /* convert time_t to broken-down time representation */
		tm = localtime(&mtime);
      /* format time days.month.year hour:minute:seconds */
      //memset(buf, '\0', strlen(buf));
		printf("%s %d ",md,  size);
		strftime(buff, sizeof(buff), "%x", tm);
		printf("%s %s\n", buff, basename(posix.name));
		md = malloc(sizeof(char)*10);

		int reste = 0;
		if(size % 512 != 0)
		{
			reste = 512 - (size%512);
		}
		t += reste;
		lseek(archive,t , SEEK_SET); 

	} 
	close(archive);

}
int main(int argc, char *argv[]){
	struct stat st;
	int i;
	// argv[2] = nom de l'archive
	// argv[3] = nom du fichier/dossier à archiver
	// argv[1] = option
	if(strcmp(argv[1],"-c")==0)
	{
		if(stat(argv[3], &st)!=0)
		{
			printf("erreur arg2 \n");
			exit(1);
		}

			if(S_ISDIR(st.st_mode)==0)
			{
				printf("Un seul fichier\n");
				archivage(argv[2],argv[3],0,0,1);
			}
			else
			{
	    		int archive, fn;
	    		if((archive=open(argv[2],O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO))<0)
	    		{
	    			printf(" (main) Archive impossible à ouvrir : %s\n", argv[2]);
	    			exit(1);
	    		}
	    		
	    		printf("la cible a archiver est un dossier : \n");
	    		fn = archivageArbo(0,argv[3],"", &fn, chemin(argv[2]),0);
	    		unsigned char* empty1024[1024];
	    		for(i = 0;i<1024;i++)
				{
					empty1024[i] = '\0';
				}
				write(archive, empty1024, 1024);
				close(archive);
	    	}
	}
	else if(strcmp(argv[1],"-x")==0)
	{
		desarchivage(argv[2]);
	}
	else if(strcmp(argv[1], "-t") == 0)
	{
		ls(argv[2]);
	}
	else if(strcmp(argv[1], "-r") == 0)
	{
		inserer(argv[2],argv[3]);
	}
	else
	{
		printf("pas assez d'argument\n");
	}

	return 0;

}
