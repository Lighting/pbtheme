/*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <zlib.h>

#define MAXSIZE 520000
#define MAXIMGLIST 4096
#define PBTSIGNATURE "PocketBookTheme"
#define PBTVERSION 1

/*
typedef struct ibitmap_s {

	unsigned short width;
	unsigned short height;
	unsigned short depth;
	unsigned short scanline;
	unsigned char data[];

} ibitmap;

struct imgcache {

	int pos;
	int len;
	unsigned char *data;

};
*/
void terminate(const char *fmt, ...) {

	va_list ap;

	fprintf(stderr, "Error: ");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n\n");
	exit(1);

}

void usage(char **argv) {

	fprintf(stderr, "Usage: %s [OPTION] <theme.pbt> [<theme.cfg>]\n",argv[0]);
	fprintf(stderr, "Pack/unpack config of PocketBook theme\n",argv[0]);
	fprintf(stderr, "\t-p\t\tpack\n",argv[0]);
	fprintf(stderr, "\t-u\t\tunpack\n",argv[0]);
	exit(1);

}
/*
char *getname(const char *s) {

	static char tname[64];
	char *p;

	p = strrchr(s, '/');
	if (p == NULL) p = strrchr(s, '\\');
	if (p != NULL) p++;
	if (p == NULL) p = s;
	strncpy(tname, p, 63);
	p = strchr(tname, '.');
	if (p != NULL) *p = 0;
	return tname;

}

void main_t(char *outfile, char *cfgfile, int nfiles, char **infiles) {

	unsigned char *data, *data1, *data2, *header, *hpos;
	unsigned int *iheader;
	int i, j, len, size, headersize, the_same;
	unsigned long clen;
	ibitmap *bm;
	FILE *ifd, *ofd;
	char *fname, *extp;
	struct imgcache *imglist;
	int imgpos;

	data = malloc(MAXSIZE);
	imglist = (struct imgcache *) malloc(MAXIMGLIST * sizeof(struct imgcache));

	ofd = fopen(outfile, "wb");
	if (ofd == NULL) terminate("Cannot open output file %s", outfile);

	ifd = fopen(cfgfile, "rb");
	if (ifd == NULL) terminate("Cannot open configuration file %s", cfgfile);
	len = fread(data, 1, MAXSIZE, ifd);
	if (len == MAXSIZE) terminate("File %s is too big", cfgfile);
	data[len++] = 0;
	clen = len + 16384;
	data2 = malloc(clen);
	compress2 (data2, &clen, data, len, 9);

	headersize = 32;
	for (i=0; i<nfiles; i++) {
		headersize += 12;
		headersize += ((strlen(getname(infiles[i])) / 4) + 1) * 4;
	}

	header = (unsigned char *) malloc(headersize);
	iheader = (int *) header;
	memset(header, 0, headersize);
	strcpy(header, PBTSIGNATURE);
	header[15] = PBTVERSION;
	iheader[4] = headersize;
	iheader[5] = len;
	iheader[6] = headersize;
	iheader[7] = clen;

	fwrite(header, 1, headersize, ofd);
	fwrite(data2, 1, clen, ofd);
	free(data2);

	hpos = header+32;

	imgpos = 0;

	for (i=0; i<nfiles; i++) {

		fname = infiles[i];
		extp = fname + strlen(fname) - 4;
		if (strcasecmp(fname, "thumbs.db") == 0) continue;

		ifd = fopen(fname, "rb");
		if (ifd == NULL) terminate("Cannot open %s", fname);
		memset(data, 0, MAXSIZE);
		len = fread(data, 1, MAXSIZE, ifd);
		if (len == MAXSIZE) terminate("File %s is too big", fname);
		fclose(ifd);

		if (strcasecmp(extp, ".bmp") == 0) {

			bm = bmp2res(fname, data, len, &size);
			data1 = (unsigned char *) bm;
			len = sizeof(ibitmap) + size;

		} else if (strcasecmp(extp, ".ttf") == 0) {

			data1 = data;

		} else {

			data1 = NULL;
			terminate("%s: not a BMP or TTF resource", fname);

		}

		clen = len + 16384;
		data2 = malloc(clen);
		compress2 (data2, &clen, data1, len, 9);


		*((int *) hpos) = len;
		*((int *) (hpos+4)) = ftell(ofd);
		*((int *) (hpos+8)) = clen;

		the_same = 0;
		for (j=0; j<imgpos; j++) {
			if (clen == imglist[j].len && memcmp(data2, imglist[j].data, clen) == 0) {
				*((int *) (hpos+4)) = imglist[j].pos;
				the_same = 1;
				break;
			}
		}
		hpos += 12;
		strcpy(hpos, getname(fname));
		hpos += ((strlen(getname(fname)) / 4) + 1) * 4;

		if (! the_same) {

			if (imgpos < MAXIMGLIST) {
				imglist[imgpos].pos = ftell(ofd);
				imglist[imgpos].len = clen;
				imglist[imgpos].data = malloc(clen);
				memcpy(imglist[imgpos].data, data2, clen);
				imgpos++;
			}
			if (fwrite(data2, 1, clen, ofd) != clen) {
				terminate("Error writing output file");
			}

		}

		free(data2);

	}

	fseek(ofd, 0, SEEK_SET);
	fwrite(header, 1, headersize, ofd);
	fclose(ofd);

}
*/
void unpack_resource(FILE *fd, char *name, unsigned long len, int pos, unsigned long clen) {

	unsigned char *data, *cdata;
	FILE *ofd;
	int r;

	cdata = malloc(clen);
	data = malloc(len+16);
	memset(cdata, 0, clen);
	memset(data, 0, len+16);
	fseek(fd, pos, SEEK_SET);
	fread(cdata, 1, clen, fd);
	r = uncompress (data, &len, cdata, clen);
	if (r != Z_OK) {
		terminate("decompression error");
	}
	ofd = fopen(name, "wb");
	if (ofd == NULL) terminate("Cannot open output file %s", name);
	fwrite(data, 1, len, ofd);
	fclose(ofd);
	free(cdata);
	free(data);

}

void main_u(char *themefile, char *resource) {

	char buf[32];
	unsigned char *header, *hpos;
	unsigned int *iheader;
	int pos, headersize;
	unsigned long len, clen;
	FILE *tfd;

	tfd = fopen(themefile, "rb");
	if (tfd == NULL) terminate("Cannot open theme file %s", themefile);

	memset(buf, 0, 32);
	fread(buf, 1, 32, tfd);
	if (strncmp(buf, PBTSIGNATURE, strlen(PBTSIGNATURE)) != 0) terminate("%s is not a PocketBook theme file");
	headersize = *((int *) (buf+16));
	header = malloc(headersize);
	iheader = (int *) header;
	fseek(tfd, 0, SEEK_SET);
	fread(header, 1, headersize, tfd);

	hpos = header+32;

	if (strcmp(resource, "-") == 0) {
		unpack_resource(tfd, "theme.cfg", iheader[5], iheader[6], iheader[7]);
		return;
	}

	terminate("resource %s is not found in %s", resource, themefile);

}
/*
void main_l(const char *themefile) {

	char buf[32];
	unsigned char *header, *hpos;
	unsigned int *iheader;
	int headersize;
	unsigned long len, clen;
	FILE *tfd;

	tfd = fopen(themefile, "rb");
	if (tfd == NULL) terminate("Cannot open theme file %s", themefile);

	memset(buf, 0, 32);
	fread(buf, 1, 32, tfd);
	if (strncmp(buf, PBTSIGNATURE, strlen(PBTSIGNATURE)) != 0) terminate("%s is not a PocketBook theme file");
	headersize = *((int *) (buf+16));
	header = malloc(headersize);
	iheader = (int *) header;
	fseek(tfd, 0, SEEK_SET);
	fread(header, 1, headersize, tfd);

	hpos = header+32;

	printf("resource                 size     compressed\n");
	printf("--------------------------------------------\n");
	printf("<theme.cfg>          %8i      %8i\n", iheader[5], iheader[7]);

	while (hpos < header+headersize) {

		len = *((int *) hpos);
		clen = *((int *) (hpos+8));
		printf("%-20s %8i      %8i\n", hpos+12, (int)len, (int)clen);

		hpos += 12;
		hpos += ((strlen(hpos) / 4) + 1) * 4;

	}
	printf("\n");

}
*/
int main(int argc, char **argv) {


	if (argc < 3){ usage(argv);
	}

	if (strcmp(argv[1], "-c") == 0) {
//		if (argc < 4) usage(argv);
//		main_c(argv[2], argc-3, &argv[3]);
//	} else if (strcmp(argv[1], "-t") == 0) {
//		if (argc < 4) usage(argv);
//		main_t(argv[2], argv[3], argc-4, &argv[4]);
	} else if (strcmp(argv[1], "-u") == 0) {
		if (argc < 4) usage(argv);
		main_u(argv[2], argv[3]);
//	} else if (strcmp(argv[1], "-l") == 0) {
//		main_l(argv[2]);
	} else {
		usage(argv);
	}

	return 0;


}
