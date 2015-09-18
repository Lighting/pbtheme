/*
 pbtheme - Replace/extract PocketBook theme config file
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

const char *pbtheme_header = "pbtheme v1.2 (" __DATE__ " " __TIME__ ")";

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
	fprintf(stderr, "%s\n", pbtheme_header);
	fprintf(stderr, "Usage: %s [option] THEME [CONFIG] [NEW_THEME]\n", argv[0]);
	fprintf(stderr, "Replace/extract CONFIG of PocketBook theme (by default, extract)\n\n");
	fprintf(stderr, "Mandatory arguments to long options are mandatory for short options too.\n");
	fprintf(stderr, "  -h,  --help      display this help and exit\n");
	fprintf(stderr, "  -r,  --replace   replace CONFIG in THEME into NEW_THEME\n");
	fprintf(stderr, "  -e,  --extract   extract CONFIG from THEME\n\n");
	fprintf(stderr, "With no CONFIG, or when CONFIG is -, read/write standard input/output.\n");
	fprintf(stderr, "With no NEW_THEME, or when NEW_THEME is -, write to standard output.\n\n");
	fprintf(stderr, "Report bugs to <https://github.com/Lighting/pbtheme/issues>\n");
}

void extract(const char *theme, const char *config)
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

void replace(const char *theme, const char *config, const char *new_theme)
{
	FILE *ifd = stdin;
	FILE *ofd = stdout;
	FILE *tfd;
	char buf[32];
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
	iheader = (unsigned int *) header;
	iheader[5] = len;
	//calc position offset
	delta = clen - iheader[7];
	iheader[7] = clen;
	
	//edit ending header for new config
	hpos = header + 32; //28 + 4
	while(hpos < (header + headersize))
	{
		//shift position to offset
		iheader = (unsigned int *) hpos;
		iheader[1] = iheader[1] + delta;
		hpos += 12;
		len = strlen((char *) hpos);
		if(len != 0)
			hpos += ((len / 4) + 1) * 4;
	}

	//open output for writing
	if(strcmp(new_theme, "-") != 0)
		ofd = fopen(new_theme, "wb");
	if(ofd == NULL)
		terminate("Cannot open output file %s", new_theme);
	
	//write new theme to temp file
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
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		usage(argv);
		terminate("Theme file not found\nFor help, type: %s -h", argv[0]);
	}
	
	if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
	    usage(argv);
	else if(argc > 2 && (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--replace") == 0))
		replace(argv[2], (argc > 3) ? argv[3] : "-", (argc > 4) ? argv[4] : "-");
	else if(argc > 2 && (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--extract") == 0))
		extract(argv[2], (argc > 3) ? argv[3] : "-");
	else
		extract(argv[1], (argc > 2) ? argv[2] : "-");

	return 0;
}
