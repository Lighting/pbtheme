/*
 pbtheme - pack/unpack PocketBook theme config file
 Developed by Lit
 Based on source code by Dmitry Zakharov https://github.com/yuryfdr/xpbres.git
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <zlib.h>

#define MAXSIZE 520000
#define PBTSIGNATURE "PocketBookTheme"
#define PBTVERSION 1

static const char *default_config "theme.cfg"

void terminate(const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "Error: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n\n");
	exit(1);
}

void usage(char **argv)
{
	fprintf(stderr, "Usage: %s [OPTION] <theme.pbt> [<theme.cfg>]\n",argv[0]);
	fprintf(stderr, "Pack/unpack config of PocketBook theme\n");
	fprintf(stderr, "\t-p\tpack <theme.cfg> to <theme.pbt>\n");
	fprintf(stderr, "\t-u\tunpack <theme.cfg> from <theme.pbt>\n");
	exit(1);
}

void unpack_resource(FILE *fd, const char *name, unsigned long len, int pos, unsigned long clen)
{
	unsigned char *data, *cdata;
	FILE *ofd;
	int r;
	
	cdata = malloc(clen);
	data = malloc(len+16);
	memset(cdata, 0, clen);
	memset(data, 0, len+16);
	fseek(fd, pos, SEEK_SET);
	fread(cdata, 1, clen, fd);
	r = uncompress(data, &len, cdata, clen);
	if(r != Z_OK)
		terminate("decompression error");
	ofd = fopen(name, "wb");
	if(ofd == NULL)
		terminate("Cannot open output file %s", name);
	fwrite(data, 1, len, ofd);
	fclose(ofd);
	free(cdata);
	free(data);
}

void pack(char *theme, const char *config)
{
	terminate("This function not supported yet");
}

void unpack(char *theme, const char *config)
{
	char buf[32];
	unsigned char *header;
	unsigned int *iheader;
	int headersize;
	FILE *tfd;
	
	tfd = fopen(theme, "rb");
	if(tfd == NULL)
		terminate("Cannot open theme file %s", theme);
	
	memset(buf, 0, 32);
	fread(buf, 1, 32, tfd);
	if(strncmp(buf, PBTSIGNATURE, strlen(PBTSIGNATURE)) != 0)
		terminate("%s is not a PocketBook theme file", theme);
	if(buf[15] != PBTVERSION)
		terminate("%s have unsupported PocketBook theme version %d", theme, buf[15]);
	
	headersize = *((int *) (buf+16));
	if(headersize > MAXSIZE)
		terminate("%s have too big header of PocketBook theme", theme);
	
	header = malloc(headersize);
	iheader = (unsigned int *) header;
	fseek(tfd, 0, SEEK_SET);
	fread(header, 1, headersize, tfd);
	unpack_resource(tfd, config, iheader[5], iheader[6], iheader[7]);
}

int main(int argc, char **argv)
{
	if(argc < 3)
		usage(argv);
	
	if(strcmp(argv[1], "-p") == 0)
		pack(argv[2], (argc < 4) ? default_config : argv[3]);
	else if(strcmp(argv[1], "-u") == 0)
		unpack(argv[2], (argc < 4) ? default_config : argv[3]);
	else
		usage(argv);
}
