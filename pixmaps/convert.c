#include <stdio.h>

// [HGM] This program converts the WinBoard monochrome .bmp piece-symbol bitmap files
//       to .xpm C-source files suitable for inclusion in xboard as buit-in pixmap.
//       It tries all bitmaps in bulk. Run it once in an empty subdirectory 'pixmaps'of the
//       root directory of the source tree, and it will fill it with pixmaps.
//       It overwrites any pixmaps of the same name that already exist. So if you want to
//       keep the pixmaps you already have, move them elsewhere, run convert, and move them back.


int sizeList[] = { 21, 25, 29, 33, 37, 40, 45, 49, 54, 58, 64, 72, 80, 87, 95, 108, 116, 129 };
char *(pieceList[]) = {"p", "n", "b", "r", "q", "f", "e", "as", "c", "w", "m",
		       "o", "h", "a", "dk", "g", "d", "v", "l", "s", "u", "k",
		       "wp", "wl", "wn", "ws", "cv", NULL };
char kindList[] = "sow";

void Load(FILE *f, char data[130][130], char*name)
{
	int i, j, k; char h, w, c;

	if(fscanf(f, "BM%c", &i) != 1) { printf("%s does not have bitmap format\n", name); exit(0); }
	for(i=0; i<15; i++) fgetc(f); fscanf(f, "%c%c%c%c%c", &h, &i, &i, &i, &w);
	for(i=0; i<39; i++) fgetc(f); // no checking is done to see if the format is as we expect!!!

// printf("h=%d, w=%d\n", h, w);
	for(i=0; i<h; i++) {
		for(j=0; j<w; j+=32) {
			c = fgetc(f);
			for(k=0; k<8; k++) {
			    data[i][j+k] = c&0x80; c <<= 1;
			}
			c = fgetc(f);
			for(k=0; k<8; k++) {
			    data[i][j+k+8] = c&0x80; c <<= 1;
			}
			c = fgetc(f);
			for(k=0; k<8; k++) {
			    data[i][j+k+16] = c&0x80; c <<= 1;
			}
			c = fgetc(f);
			for(k=0; k<8; k++) {
			    data[i][j+k+24] = c&0x80; c <<= 1;
			}
		}
	}

	fclose(f);
}

void Paint(char dest[130][130], char src[130][130], int size, char c)
{	// copy monochrome pixmap to destination as if (transparent, c)
	int i, j;

	for(i=0; i<size; i++) for(j=0; j<size; j++) if(!src[i][j]) dest[i][j] = c;
}

FloodFill(char a[130][130], int size, int x, int y)
{
	char old = 'X', new = '.';
	if(a[x][y] != old) return; else {
		a[x][y] = new;
		if(x > 0) FloodFill(a, size, x-1, y);
		if(y > 0) FloodFill(a, size, x, y-1);
		if(x < size-1) FloodFill(a, size, x+1, y);
		if(y < size-1) FloodFill(a, size, x, y+1);
	}

}

void Save(FILE *f, char *name, char data[130][130], int size, char *col, int depth)
{	// write out data in source format for d x d pixmap with specified square color
	int i, j;

	fprintf(f, "/* XPM */\n");
	fprintf(f, "static char *%s[] = {\n", name);
	fprintf(f, "/* columns rows colors chars-per-pixel */\n");
	fprintf(f, "\"%d %d %d 1\",\n", size, size, depth);
	fprintf(f, "\"  c black s dark_piece\",\n");
	fprintf(f, "\". %s\",\n", col);
	if(depth==3) fprintf(f, "\"X c white s light_piece\",\n");
	fprintf(f, "/* pixels */\n");
	for(i=size-1; i>=0; i--) {
		fprintf(f, "\"%s\"%s\n", data[i], i==0 ? "" : ",");
	}
	fprintf(f, "};\n");

	fclose(f);
}


char data[130][130], oData[130][130], sData[130][130], wData[130][130];

main(int argc, char **argv)
{

#define BUFLEN 80

	int i, j, k, d, cnt, p, s, t; char c, h, w, name[BUFLEN], buf[BUFLEN], transparent;
	FILE *f;

    transparent = argc > 1 && !strcmp(argv[1], "-t");

    for(s=0; s<18; s++) for(p=0; pieceList[p] != NULL; p++) {

	// Load the 3 kinds of Windows monochrome bitmaps (outline, solid, white fill)

	snprintf(buf, BUFLEN, "../winboard/bitmaps/%s%d%c.bmp", pieceList[p], sizeList[s], 'o');
	printf("try %s\n", buf);
	f = fopen(buf, "rb");
	if(f == NULL) continue;
	Load(f, oData, buf);

	snprintf(buf, BUFLEN, "../winboard/bitmaps/%s%d%c.bmp", pieceList[p], sizeList[s], 's');
	f = fopen(buf, "rb");
	if(f == NULL) continue;
	Load(f, sData, buf);

	snprintf(buf, BUFLEN, "../winboard/bitmaps/%s%d%c.bmp", pieceList[p], sizeList[s], 'w');
	if(pieceList[p][0]=='w')
	snprintf(buf, BUFLEN, "../winboard/bitmaps/%s%d%c.bmp", "w", sizeList[s], 'w');
	f = fopen(buf, "rb");
	if(f == NULL) continue;
	Load(f, wData, buf);

	printf("%s loaded\n", buf);
	// construct pixmaps as character arrays
	d = sizeList[s];
	for(i=0; i<d; i++) { for(j=0; j<d; j++) data[i][j] = transparent? '.' : 'X'; data[i][d] = 0; } // fill square

	Paint(data, sData, d, ' '); // overay with solid piece bitmap

	if(!transparent) { // background was painted same color as piece details; flood-fill it from corners
	    FloodFill(data, d, 0, 0);
	    FloodFill(data, d, 0, d-1);
	    FloodFill(data, d, d-1, 0);
	    FloodFill(data, d, d-1, d-1);
	}

	snprintf(buf, BUFLEN, "%s%s%d.xpm", pieceList[p], "dd", d);
	snprintf(name, BUFLEN, "%s%s%d", pieceList[p], "dd", d);
	f = fopen(buf, "w");
	Save(f, name, data, d, "c green s dark_square", 3);

	snprintf(buf, BUFLEN, "%s%s%d.xpm", pieceList[p], "dl", d);
	snprintf(name, BUFLEN, "%s%s%d", pieceList[p], "dl", d);
	f = fopen(buf, "w");
	Save(f, name, data, d, "c gray s light_square", 3); // silly duplication; pixmap is te same, but other color

	// now the light piece
	for(i=0; i<d; i++) { for(j=0; j<d; j++) data[i][j] = '.'; data[i][d] = 0; } // fill square

	Paint(data, wData, d, 'X'); // overay with white-filler piece bitmap
	Paint(data, oData, d, ' '); // overay with outline piece bitmaps

	snprintf(buf, BUFLEN, "%s%s%d.xpm", pieceList[p], "ld", d);
	snprintf(name, BUFLEN, "%s%s%d", pieceList[p], "ld", d);
	f = fopen(buf, "w");
	Save(f, name, data, d, "c green s dark_square", 3);

	snprintf(buf, BUFLEN, "%s%s%d.xpm", pieceList[p], "ll", d);
	snprintf(name, BUFLEN, "%s%s%d", pieceList[p], "ll", d);
	f = fopen(buf, "w");
	Save(f, name, data, d, "c gray s light_square", 3);

	printf("%s saved\n", buf);
    }
}

