#include <stdio.h>

// [HGM] This program converts a WinBoard monochrome .bmp piece-symbol bitmap file
//       to a .bm C-source file suitable for inclusion in xboard as buit-in bitmap.
//       You will have to convert the bitmaps one by one, and re-direct the output to the desired file!

main(int argc, char **argv)
{
	int i, j, k, d, cnt; char c, *p, data[10000], *name; unsigned char h, w;
	FILE *f;

	if(argc<2) { printf("usage is: convert <bmp filename>\n"); exit(0); }
	f = fopen(argv[1], "rb");
	if(f == NULL) { printf("file %s not found\n", argv[1]); exit(0); }

	if(fscanf(f, "BM%c", &i) != 1) { printf("%s does not have bitmap format\n"); exit(0); }
	for(i=0; i<15; i++) fgetc(f); fscanf(f, "%c%c%c%c%c", &h, &i, &i, &i, &w);
	for(i=0; i<39; i++) fgetc(f);

// printf("h=%d, w=%d\n", h, w);

	p = data;
	for(i=0; i<h; i++) {
		for(j=0; j<w; j+=32) {
			c = fgetc(f);
			for(k=0; k<8; k++) {
				d = (d>>1) | (c&0x80);
				c <<= 1;
			}
			*p++ = d;
			c = fgetc(f);
			for(k=0; k<8; k++) {
				d = (d>>1) | (c&0x80);
				c <<= 1;
			}
			*p++ = d;
			c = fgetc(f);
			for(k=0; k<8; k++) {
				d = (d>>1) | (c&0x80);
				c <<= 1;
			}
			*p++ = d;
			c = fgetc(f);
			for(k=0; k<8; k++) {
				d = (d>>1) | (c&0x80);
				c <<= 1;
			}
			*p++ = d;
		}
	}
	fclose(f);

	name = argv[1];
	for(i=0; argv[1][i]; i++) if(argv[1][i] == '\\') name = argv[1]+i+1;
	for(i=0; name[i]; i++) if(name[i] == '.') name[i] = 0;
	printf("#define %s_width %d\n", name, w);
	printf("#define %s_height %d\n", name, h);
	printf("static unsigned char %s_bits[] = {\n", name);
	cnt = 0;
	for(i=h-1; i>=0; i--) {
		for(j=0; j<w; j+=8) {
			c = ~data[i*((w+31)/8&~3)+j/8];
			if(w-j<8) c &= 255>>(8+j-w);
//			for(k=0; k<8; k++) {
//				printf("%c", c&1 ? 'X' : '.');
//				c >>= 1;
//			}
			if(cnt!=0) printf(",");
			printf("0x%02x", c&255);
			if(++cnt % 15 == 0) { printf("\n"); }
		}
	}
	printf("\n};\n");
}
