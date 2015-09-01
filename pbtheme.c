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
	fprintf(stderr, "Report bugs to <https://github.com/Lighting/pbtheme/issues>\n");
}

void unpack(char *theme, const char *config)
{
	FILE *ofd = stdout;
	FILE *tfd;
	char buf[32];
	unsigned int *iheader;
	unsigned char *data, *cdata;
	unsigned long len;
	
	//open theme file for reading
	tfd = fopen(theme, "rb");
	if(tfd == NULL)
		terminate("Cannot open theme file %s", theme);
	
	//read beginning of header
	memset(buf, 0, sizeof(buf));
	fread(buf, 1, sizeof(buf), tfd);
	if(strncmp(buf, PBTSIGNATURE, strlen(PBTSIGNATURE)) != 0)
		terminate("%s is not a PocketBook theme file", theme);
	if(buf[15] != PBTVERSION)
		terminate("%s have unsupported PocketBook theme version %d", theme, buf[15]);

	//read config
	iheader = (unsigned int *) buf;
	len = iheader[5];
	data = malloc(len);
	cdata = malloc(iheader[7]);
	memset(data, 0, len);
	memset(cdata, 0, iheader[7]);
	fseek(tfd, iheader[6], SEEK_SET);
	fread(cdata, 1, iheader[7], tfd);
	fclose(tfd);
	
	//unpack config
	if(uncompress(data, &len, cdata, iheader[7]) != Z_OK)
		terminate("decompression error");
	free(cdata);
	
	//open config for writing
	if(strcmp(config, "-") != 0)
		ofd = fopen(config, "wb");
	if(ofd == NULL)
		terminate("Cannot open output file %s", config);
	
	fwrite(data, 1, len, ofd);
	
	fclose(ofd);
	free(data);
}

void pack(char *theme, const char *config)
{
	FILE *ifd = stdin;
	FILE *ofd, *tfd;
	char buf[32], temp[L_tmpnam];
	unsigned char *data, *tdata, *header, *hpos;
	unsigned long len, clen;
	unsigned int *iheader;
	int headersize, delta;
	
	//open theme file for reading
	tfd = fopen(theme, "rb");
	if(tfd == NULL)
		terminate("Cannot open theme file %s", theme);
	
	//read beginning of header
	memset(buf, 0, sizeof(buf));
	fread(buf, 1, sizeof(buf), tfd);
	if(strncmp(buf, PBTSIGNATURE, strlen(PBTSIGNATURE)) != 0)
		terminate("%s is not a PocketBook theme file", theme);
	if(buf[15] != PBTVERSION)
		terminate("%s have unsupported PocketBook theme version %d", theme, buf[15]);
	headersize = *((int *) (buf + 16));
	
	//read full header
	header = malloc(headersize);
	fseek(tfd, 0, SEEK_SET);
	fread(header, 1, headersize, tfd);

	//open config for reading
	if(strcmp(config, "-") != 0)
		ifd = fopen(config, "rb");
	if(ifd == NULL)
		terminate("Cannot open config file %s", config);

	//read config
	data = malloc(MAXSIZE);
	len = fread(data, 1, MAXSIZE, ifd);
	fclose(ifd);
	
	//compress config
	if(len == MAXSIZE)
		terminate("Config %s is too big", config);
	if(len != 0)
		clen = len + (len / 1000) + 12;
	else
		clen = 12;
	tdata = malloc(clen);
	if(compress2(tdata, &clen, data, len, 9) != Z_OK)
		terminate("compression error");
	
	//edit beginning of header for new config
	iheader = (int *) header;
	iheader[5] = len;
	//calc position offset
	delta = clen - iheader[7];
	iheader[7] = clen;
	
	//edit ending header for new config
	hpos = header + 32;
	while(hpos < (header + headersize))
	{
		//shift position to offset
		iheader = ((int *) (hpos + 4));
		iheader[1] = iheader[1] + delta;
		hpos += 12;
		len = strlen(hpos);
		if(len != 0)
			hpos += ((len / 4) + 1) * 4;
	}

	//create temp file
	tmpnam(temp);
	ofd = fopen(temp, "w+b");
	if(ofd == NULL)
		terminate("Cannot open temporary file");

	//write new theme to temp file
	fseek(ofd, 0, SEEK_SET);
	fwrite(header, 1, headersize, ofd);
	fwrite(tdata, 1, clen, ofd);
	free(header);
	free(tdata);
	
	//write theme data to temp file
	fseek(tfd, clen - delta, SEEK_CUR);
	while((len = fread(data, 1, MAXSIZE, tfd)) > 0)
		fwrite(data, 1, len, ofd);
	
	fclose(tfd);
	fclose(ofd);
	free(data);
	
	//replace theme file by temp file
	if(remove(theme) != 0 || rename(temp, theme) != 0)
		terminate("Error while renaming %s to %s", temp, theme);
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		usage(argv);
		terminate("Theme file not found\nFor help, type: %s -h", argv[0]);
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
