#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <gtk/gtk.h>

#include "phrasealator.h"

void LoadDictionary(void);
void UnloadDictionary(void);
char *LookUpWord(char *UserWord);
char *sgets(char *destbuffer,int maxlen,char *sourcebuffer);

char *dictionarywords[100000];
char *word2phoneme[100000];

#define DICTIONARYFILEMAX 150000

void LoadDictionary(void)
{
        FILE *in;
        char dictionaryline[1000];
        char *p1, *p2;
        char *equalschar;
	int index,t,s;

	char dictionaryfilestring[DICTIONARYFILEMAX];

	memset(dictionaryfilestring,0,DICTIONARYFILEMAX);

	for(t=0;t<10000;t++)
	{
		dictionarywords[t]=NULL;
		word2phoneme[t]=NULL;
	}
	
	index=0;
        in=fopen("PhraseALator.Dic","r");
        if(in==NULL)
        {
                fprintf(stderr,"An external Phrasealtor.Dic wasn't found. Using the internal dictionary.\n");
		strcpy(dictionaryfilestring,(const char *)phrasealator);
	}
	else
	{
                fprintf(stderr,"An external copy of Phrasealtor.Dic was found. Using this dictionary instead of the built-in copy.\n");
		if(fread(dictionaryfilestring,1,DICTIONARYFILEMAX-1,in)<1)
		{
                	fprintf(stderr,"couldn't load dictionary file.\n");
			exit(1);
		}
        }
        while(sgets(dictionaryline,1000,dictionaryfilestring)!=NULL)
        {
                if(dictionaryline[0]=='[')
                        continue;

                if((dictionaryline[0]=='f')&&(dictionaryline[1]=='x'))
                        continue;

                //remove spaces, newlines, carriage returns
                p1=dictionaryline; p2=dictionaryline;
                while (*p1!=0)
                {
                        if((*p1==' ')||(*p1=='\n')||(*p1=='\r'))
                                p1++;
                        else
                                *p2++=*p1++;
                }
                *p2=0;
                equalschar=strstr(dictionaryline,"=");
                if(equalschar==NULL)
                        continue;
                *equalschar=0;
		
		dictionarywords[index]=calloc(strlen(dictionaryline)+1,1);
		word2phoneme[index]=calloc(strlen(equalschar+1)+1,1);
		if((dictionarywords[index]==NULL)||(word2phoneme[index]==NULL))
		{
			fprintf(stdout,"** ERR: couldn't allocate memory\n");
			break;
		}
                strcpy(dictionarywords[index],dictionaryline);
                strcpy(word2phoneme[index],equalschar+1);
		index++;
        }
        if(in!=NULL)
		fclose(in);
	for(t=0;t<10000;t++)
	{
		if(dictionarywords[t]==NULL)
			break;
		for(s=0;s<strlen(dictionarywords[t]);s++)
			dictionarywords[t][s]=tolower(dictionarywords[t][s]);
		for(s=0;s<strlen(word2phoneme[t]);s++)
			word2phoneme[t][s]=toupper(word2phoneme[t][s]);
	}
}

// sgets - a string based variant of fgets...
char *sgets(char *destbuffer,int maxlen,char *sourcebuffer)
{
	static char *remembersource=NULL;
	static char *stringstart=NULL;
	int s,t;

	//iniitialize if the sgets() was called with a new string source compared to last time...
	if(remembersource!=sourcebuffer)
	{
		stringstart=sourcebuffer;
		remembersource=sourcebuffer;
	}

	if((stringstart==NULL)||(stringstart[0]==0))
	{
		return(NULL);
	}

	memset(destbuffer,0,maxlen);

	for(t=0;t<maxlen;t++)
	{
		destbuffer[t]=stringstart[t];
		if(stringstart[t]==0)
		{
			stringstart=stringstart+t;
			return(destbuffer);
		}
		if((stringstart[t]=='\r')||(stringstart[t]=='\n'))
		{
			for(s=t;s<maxlen;s++)
			{
				if((stringstart[s]=='\r')||(stringstart[s]=='\n'))
					destbuffer[s]=0;
				else
				{
					s--;
					break;
				}
			}
			destbuffer[s]=0;
			stringstart=stringstart+s+1;
			return(destbuffer);
			
		}
	}
	t=t-1;
	destbuffer[t]=0;
	stringstart=stringstart+t;
	return(destbuffer);
}

void UnloadDictionary(void)
{
	int t;
	for(t=0;t<10000;t++)
	{
		if(dictionarywords[t]!=NULL)
		{
			free(dictionarywords[t]);
			dictionarywords[t]=NULL;
		}
		if(word2phoneme[t]!=NULL)
		{
			free(word2phoneme[t]);
			word2phoneme[t]=NULL;
		}
	}
}

char *LookUpWord(char *UserWord)
{
	int t;
	for(t=0;t<10000;t++)
	{
		if(dictionarywords[t]==NULL)
			break;
		if(strcmp(UserWord,dictionarywords[t])==0)
			return(word2phoneme[t]);
	}
	return(NULL);
}
