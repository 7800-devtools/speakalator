#include <stdio.h>
#include <stdlib.h>

#define GLADEFILE "speakalator.glade"
#define HEADERFILE "gladeui.h"
#define MAXGLADESIZE 1500000

int main()
{
	FILE *gladein, *headerout;
	unsigned char buffer[MAXGLADESIZE];
	long t, gladesize;

	gladein=fopen(GLADEFILE,"rb");
	if(gladein==NULL)
	{
		fprintf(stderr,"ERR: couldn't open %s for reading. Exiting.\n",GLADEFILE);
		exit(1);
	}
	headerout=fopen(HEADERFILE,"w");
	if(headerout==NULL)
	{
		fprintf(stderr,"ERR: couldn't open %s for writing. Exiting.\n",GLADEFILE);
		exit(1);
	}
	
	gladesize=fread(buffer, 1, MAXGLADESIZE,gladein);
	fclose(gladein);

	fprintf(headerout," unsigned char gladeui[] = {\n");
	for(t=0;t<gladesize;t++)
	{
		fprintf(headerout," 0x%02x,",buffer[t]);
		if((t%12)==11)
			fprintf(headerout,"\n");
	}
	fprintf(headerout," 0 };\n");
	fclose(headerout);
}
