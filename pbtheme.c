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
	fprintf(stderr, "Usage: %s [OPTION] THEME [CONFIG]\n", argv[0]);
	fprintf(stderr, "Pack/unpack CONFIG of PocketBook theme (by default, unpack)\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h\tdisplay this help and exit\n");
	fprintf(stderr, "\t-p\tpack CONFIG to THEME in-place\n");
	fprintf(stderr, "\t-u\tunpack CONFIG from THEME\n\n");
	fprintf(stderr, "With no CONFIG, or when CONFIG is -, read/write standard input/output.\n\n");
	fprintf(stderr, "Report bugs to <https://github.com/Lighting/pbtheme/issues>");
}

void unpack_resource(FILE *fd, const char *name, unsigned long len, int pos, unsigned long clen)
{
	unsigned char *data, *cdata;
	FILE *ofd = stdout;
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
	if(strcmp(name, "-") != 0)
		ofd = fopen(name, "wb");
	if(ofd == NULL)
		terminate("Cannot open output file %s", name);
	fwrite(data, 1, len, ofd);
	fclose(ofd);
	free(cdata);
	free(data);
}

void unpack(char *theme, const char *config)
{
	char buf[32];
	unsigned int *iheader;
	FILE *ifd;
	
	ifd = fopen(theme, "rb");
	if(ifd == NULL)
		terminate("Cannot open theme file %s", theme);
	
	memset(buf, 0, 32);
	fread(buf, 1, 32, ifd);
	if(strncmp(buf, PBTSIGNATURE, strlen(PBTSIGNATURE)) != 0)
		terminate("%s is not a PocketBook theme file", theme);
	if(buf[15] != PBTVERSION)
		terminate("%s have unsupported PocketBook theme version %d", theme, buf[15]);
	
	iheader = (unsigned int *) buf;
	unpack_resource(ifd, config, iheader[5], iheader[6], iheader[7]);
}

void pack(char *theme, const char *config)
{
	FILE *tfd, *ofd, *ifd = stdin;
	char buf[32];
	int headersize, delta;
	unsigned char *data, *data2;
	unsigned long len, clen, pos;
	unsigned int *idata;

	ofd = fopen(theme, "r+b");
	if(ofd == NULL)
		terminate("Cannot open theme file %s", theme);
	
	memset(buf, 0, 32);
	fread(buf, 1, 32, ofd);
	if(strncmp(buf, PBTSIGNATURE, strlen(PBTSIGNATURE)) != 0)
		terminate("%s is not a PocketBook theme file", theme);
	if(buf[15] != PBTVERSION)
		terminate("%s have unsupported PocketBook theme version %d", theme, buf[15]);
	headersize = *((int *) (buf+16));
	
	if(strcmp(config, "-") != 0)
		ifd = fopen(config, "rb");
	if(ifd == NULL)
		terminate("Cannot open config file %s", config);
	data = malloc(MAXSIZE);
	len = fread(data, 1, MAXSIZE, ifd);
	if(len == MAXSIZE)
		terminate("Config %s is too big", config);
	data[len++] = 0;
	clen = len + 16384;
	data2 = malloc(clen);
	compress2(data2, &clen, data, len, 9);
	
	tfd = tmpfile();
	if(tfd == NULL)
		terminate("Cannot open temporary file");
	
	fseek(ofd, 20, SEEK_SET);
	fwrite(len, 1, 4, ofd);
	fseek(ofd, 28, SEEK_SET);
	fread(data, 1, 4, ifd);
	idata = (int *) data;
	delta = clen - idata[0];
	fwrite(clen, 1, 4, ofd);
	
	pos = 32;
	while (pos < headersize)
	{
		fseek(ofd, pos+4, SEEK_SET);
		fread(data, 1, 4, ifd);
		idata[0] = idata[0] + delta;
		fwrite(data, 1, 4, ofd);
		pos += 12;
		pos += ((strlen(hpos) / 4) + 1) * 4;
	}
	

	fclose(tfd);
	fclose(ifd);
	fclose(ofd);
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		usage(argv);
		terminate("Theme file not found\nFor help, type: %s -h", argv[1]);
	}
	
	if(strcmp(argv[1], "-h") == 0)
	    usage(argv);
	else if(argc > 2 && strcmp(argv[1], "-p") == 0)
		pack(argv[2], (argc > 3) ? argv[3] : "-");
	else if(argc > 2 && strcmp(argv[1], "-u") == 0)
		unpack(argv[2], (argc > 3) ? argv[3] : "-");
	else
		unpack(argv[1], (argc > 2) ? argv[2] : "-");

	return 0;
}
