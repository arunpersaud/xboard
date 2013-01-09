/*
	zic2xpm.c

	Program to convert pieces from ZIICS format to XPM & XIM format.
	(C version)  By Frank McIngvale <frankm@hiwaay.net>.

	Copyright (C) 1996, 2009, 2010, 2011, 2012, 2013 Free Software Foundation, Inc.

	NOTICE: The piece images distributed with ZIICS are
	    copyrighted works of their original creators.  Images
            converted with zic2xpm may not be redistributed without
	    the permission of the copyright holders.  Do not contact
	    the authors of zic2xpm or of ZIICS itself to request
	    permission.

	NOTICE:  The format of the ZIICS piece file was gleaned from
	    SHOWSETS.PAS, a part of ZIICS.  Thanks to Andy McFarland
	    (Zek on ICC) for making this source available!  ZIICS is a
	    completely separate and copyrighted work of Andy
	    McFarland. 	Use and distribution of ZIICS falls under the
	    ZIICS license, NOT the GNU General Public License.

	NOTICE: The format of the VGA imageblocks was determined
            by experimentation, and without access to any
            of Borland Inc.'s BGI library source code.


	    GNU XBoard is free software: you can redistribute it
	    and/or modify it under the terms of the GNU General Public
	    License as published by the Free Software Foundation,
	    either version 3 of the License, or (at your option) any
	    later version.

	    GNU XBoard is distributed in the hope that it will be
	    useful, but WITHOUT ANY WARRANTY; without even the implied
	    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	    PURPOSE. See the GNU General Public License for more
	    details.

	    You should have received a copy of the GNU General Public
	    License along with this program. If not, see
	    http://www.gnu.org/licenses/.


	** If you find a bug in zic2xpm.c, please report it to me,
  	   Frank McIngvale (frankm@hiwaay.net) so that I may fix it. **
*/

/*
	Usage: zic2xpm file1 [file2 ...]

	We split the ZIICS file(s) into 24 XPM & 24 XIM files with names:

	<piece><type><size>.(xpm|xim).

	Where:
		piece = p, n, b, r, q, k
		type = ll, ld, dl, dd
		size = Piece size.

	Plus 4 files for the light & dark squares.

	This means that you can extract multiple SIZES in one directory
	without name clashes. Extracting two sets of the SAME
	size in a directory will cause the second to overwrite
	the first.
*/

/*
   Technical note: Yes, this file is huge. I made it by cramming
                   `zic2xpm' and `zic2xim' together. This should
                   be less confusing to use, though.
*/

#include "config.h"
#include <stdio.h>
#if STDC_HEADERS
#include <stdlib.h>
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

/*
   XIM file format:

   width	byte
   height	byte

   Data (1 byte per pixel, row major)
*/

/*
   Map colors from ZIICS -> XIM :

   0	0	Dark piece
   2	1	Dark square
   15	2	Light piece
   14	3	Light square
*/

typedef struct {
  int zval;		/* ZIICS value */
  int xval;		/* XIM value */
} z2xim;

/* Associate VGA color with XPM color/sym */

typedef struct {
  int cval;		/* VGA pixel value */
  char xchar;	/* XPM character for this color */
  char *csym;	/* Symbolic name */
  char *cdefault; /* Default color */
} z2xpm;

#define NR_ZIICS_COLORS 4
#define BUFLEN 100

/* SHOWSETS.PAS (from ZIICS) states that images may only
   use color numbers 0, 2, 14, and 15 */

z2xim z2xim_tab[NR_ZIICS_COLORS] = {
  { 0, 0 },
  { 2, 1 },
  { 15, 2 },
  { 14, 3 } };

z2xpm z2xpm_tab[NR_ZIICS_COLORS] = {
  { 15, 'X', "light_piece", "white" },
  { 0, ' ', "dark_piece", "black" },
  { 14, '*', "light_square", "gray" },
  { 2, '.', "dark_square", "green" } };

void
fatal (char *str)
{
  printf("Fatal error: %s\n", str );
  exit(1);
}

z2xim *
lookup_xim_color (int color)
{
  int i;

  for( i=0; i<NR_ZIICS_COLORS; ++i )
	{
	  if ( z2xim_tab[i].zval == color )
		return (z2xim_tab + i);
	}

  fatal("Illegal color in image.");

  /* NOT REACHED */
  return NULL;					/* Make compiler happy */
}

z2xpm *
lookup_xpm_color (int color)
{
  int i;

  for( i=0; i<NR_ZIICS_COLORS; ++i )
	{
	  if ( z2xpm_tab[i].cval == color )
		return (z2xpm_tab + i);
	}

  fatal("Illegal color in image.");

  /* NOT REACHED */
  return NULL;					/* Make compiler happy */
}

char *src_name;

int
up8 (int i)
{
  int r;

  r = i % 8;
  if ( r == 0 )
	return i;

  return i + 8 - r;
}

unsigned int
vga_imagesize (int w, int h)
{
  int w8;
  unsigned int s;

  w8 = up8( w );

  s = 4 + w8/2 * h + 2;

  return s;
}

unsigned char *
decode_byte (unsigned char *dest, unsigned char *b, int w)
{
  int i, j;
  unsigned char byte, bit;

  for( i=7; w > 0; --i, --w )
	{
	  byte = 0;

	  /* 1 bit from each plane */
	  for( j=0; j<4; ++j )
		{
		  bit = b[j];
		  bit &= (1 << i);
		  bit >>= i;
		  bit <<= 3-j;
		  byte |= bit;
		}

	  *(dest++) = byte;
	}

  return dest;
}

/*
   W is width of image in PIXELS.
   SRC is in packed pixel format.
   DEST is filled with 1 BYTE per PIXEL.
*/
unsigned char *
decode_line (unsigned char *dest, unsigned char *src, int w)
{
  unsigned int w8;
  unsigned int bpp;
  unsigned char b[4];
  int i;
  unsigned char *p;

  p = src;
  w8 = up8( w );

  /* 4 planes, bpp BYTES per plane */
  /* Planes are MSB -> LSB */
  bpp = w8 >> 3;

  while( w > 0 )
	{
	  for( i=0; i<4; ++i )
		b[i] = p[i*bpp];

	  if ( w > 8 )
		dest = decode_byte( dest, b, 8 );
	  else
		dest = decode_byte( dest, b, w );

	  w -= 8;
	  ++p;
	}

  return (src + bpp * 4);
}

int
write_xim_header (FILE *fp, int w, int h)
{
  fputc( w, fp );
  fputc( h, fp );

  return 0;
}

int
write_xpm_header (FILE *fp, int w, int h)
{
  int i;
  z2xpm *cv;

  fprintf(fp, "/* XPM */\n");
  fprintf(fp, "/* This file was automatically generated from the file %s\n",
	  src_name );
  fprintf(fp, "using the program ``zic2xpm''.\n");
  fprintf(fp, "\n    %s\n    %s\n    %s\n    %s\n    %s\n    %s */\n",
	  "NOTICE: The piece images distributed with ZIICS are",
	  "    copyrighted works of their original creators.  Images",
          "    converted with zic2xpm may not be redistributed without",
	  "    the permission of the copyright holders.  Do not contact",
	  "    the authors of zic2xpm or of ZIICS itself to request",
	  "    permission.");
  fprintf( fp, "static char * image_name[] = {\n" );
  fprintf( fp, "\"%d %d %d 1\",\n", h, w, NR_ZIICS_COLORS );

  cv = z2xpm_tab;

  for( i=0; i<NR_ZIICS_COLORS; ++i, ++cv )
	{
	  fprintf( fp, "\"%c\tc %s s %s\",\n", cv->xchar,
			  cv->cdefault, cv->csym );
	}

  return 0;
}

void
create_piece_xim (char *outname, FILE *fpin, int W, int H)
{
  FILE *fpout;
  int w, h, i, j, c;
  unsigned char *lump, *p, *line;
  long size;
  z2xim *ent;

  fpout = fopen( outname, "wb" );
  if ( !fpout )
	fatal( "Can't create output file.");

  /* Header is two ints -- Width then Height, x86 format */
  c = fgetc( fpin );
  w = (fgetc(fpin) << 8) | c;

  c = fgetc( fpin );
  h = (fgetc(fpin) << 8) | c;

  ++w; ++h;

  if ( w != W || h != H )
	fatal( "Bad header." );

  size = vga_imagesize( w, h ) - 4;
  lump = (unsigned char*)malloc( size );
  line = (unsigned char*)malloc( w );

  if ( !lump || !line )
	fatal( "Out of memory." );

  fread( lump, 1, size, fpin );

  /* Write XIM header */
  write_xim_header( fpout, w, h );

  p = lump;

  /* Write XIM data */
  for( i=0; i<h; ++i )
	{
	  p = decode_line( line, p, w );

	  for( j=0; j<w; ++j )
		{
		  ent = lookup_xim_color( line[j] );
		  fputc( ent->xval, fpout );
		}
	}

  free( lump );
  free( line );
  fclose( fpout );
}

void
create_piece_xpm (char *outname, FILE *fpin, int W, int H)
{
  FILE *fpout;
  int w, h, i, j, c;
  unsigned char *lump, *p, *line;
  long size;
  z2xpm *cv;

  fpout = fopen( outname, "wb" );
  if ( !fpout )
	fatal( "Can't create output file.");

  /* Header is two ints -- Width then Height, x86 format */
  c = fgetc( fpin );
  w = (fgetc(fpin) << 8) | c;

  c = fgetc( fpin );
  h = (fgetc(fpin) << 8) | c;

  ++w; ++h;

  if ( w != W || h != H )
	fatal( "Bad header." );

  size = vga_imagesize( w, h ) - 4;
  lump = (unsigned char*)malloc( size );
  line = (unsigned char*)malloc( w );

  if ( !lump || !line )
	fatal( "Out of memory." );

  fread( lump, 1, size, fpin );

  /* Write XPM header */
  write_xpm_header( fpout, w, h );

  p = lump;

  /* Write XPM data */
  for( i=0; i<h; ++i )
	{
	  p = decode_line( line, p, w );

	  fprintf( fpout, "\"" );
	  for( j=0; j<w; ++j )
		{
		  cv = lookup_xpm_color( line[j] );
		  fprintf( fpout, "%c", cv->xchar );
		}
	  fprintf( fpout, "\",\n" );
	}

  fprintf( fpout, "};\n" );

  free( lump );
  free( line );
  fclose( fpout );
}

/* The order of the pieces in the ZIICS piece file (from SHOWSETS.PAS) */
char *pieces = "prkqbn";
char *pname[] = { "Pawn", "Rook", "King", "Queen", "Bishop", "Knight" };

/* The suborder - Light/Light, Light/Dark, etc. */
char *prefixes[] = { "ll", "ld", "dl", "dd" };

int
process_file_xim (char *filename)
{
  int w, h, piece, kind, c;
  int nr_pieces = 6;
  int nr_kinds = 4;
  FILE *fp;
  char buf[BUFLEN];

  src_name = filename;

  fp = fopen( filename, "rb" );
  if ( !fp )
	fatal( "Can't open input file." );

  /* Header is two ints -- Width then Height, x86 format */
  c = fgetc( fp );
  w = (fgetc(fp) << 8) | c;

  c = fgetc( fp );
  h = (fgetc(fp) << 8) | c;

  ++w; ++h;

  if ( w != h )
	{
	  printf("ERROR: Can only convert square pieces.\n");
	  printf("       (This set is %dx%d)\n", w, h );
	  exit(1);
	}

  printf("Creating XIM files...\n");
  printf("File: %s, W=%d, H=%d\n", filename, w, h );
  fseek( fp, 0, SEEK_SET );

  /* Write .XIM files */
  for( piece = 0; piece < nr_pieces; ++piece )
	{
	  printf("%s ", pname[piece] );

	  for( kind = 0; kind < nr_kinds; ++kind )
		{
		  printf( "." );
		  /* Form output filename -- <piece><kind><size>.xim */
		  snprintf(buf, BUFLEN, "%c%s%d.xim", pieces[piece], prefixes[kind], w);
		  create_piece_xim( buf, fp, w, h );
		}
	  printf("\n");
	}

  /* Write the light & dark squares */
  snprintf( buf, BUFLEN, "lsq%d.xim", w );
  printf("Light Square" );
  create_piece_xim( buf, fp, w, h );

  snprintf( buf, BUFLEN, "dsq%d.xim", w );
  printf("\nDark Square" );
  create_piece_xim( buf, fp, w, h );
  printf("\n");

  printf("Successfully converted!!\n" );

  fclose( fp );

  return 0;
}

int
process_file_xpm (char *filename)
{
  int w, h, piece, kind, c;
  int nr_pieces = 6;
  int nr_kinds = 4;
  FILE *fp;
  char buf[BUFLEN];

  src_name = filename;

  fp = fopen( filename, "rb" );
  if ( !fp )
	fatal( "Can't open input file." );

  /* Header is two ints -- Width then Height, x86 format */
  c = fgetc( fp );
  w = (fgetc(fp) << 8) | c;

  c = fgetc( fp );
  h = (fgetc(fp) << 8) | c;

  ++w; ++h;

  if ( w != h )
	{
	  printf("ERROR: Can only convert square pieces.\n");
	  printf("       (This set is %dx%d)\n", w, h );
	  exit(1);
	}

  printf("Creating XPM files...\n");
  printf("File: %s, W=%d, H=%d\n", filename, w, h );
  fseek( fp, 0, SEEK_SET );

  /* Write .XPM files */
  for( piece = 0; piece < nr_pieces; ++piece )
	{
	  printf("%s ", pname[piece] );

	  for( kind = 0; kind < nr_kinds; ++kind )
		{
		  printf( "." );
		  /* Form output filename -- <piece><kind><size>.xpm */
		  snprintf(buf, BUFLEN, "%c%s%d.xpm", pieces[piece], prefixes[kind], w);
		  create_piece_xpm( buf, fp, w, h );
		}
	  printf("\n");
	}

  /* Write the light & dark squares */
  snprintf( buf, BUFLEN, "lsq%d.xpm", w );
  printf("Light Square" );
  create_piece_xpm( buf, fp, w, h );

  snprintf( buf, BUFLEN, "dsq%d.xpm", w );
  printf("\nDark Square" );
  create_piece_xpm( buf, fp, w, h );
  printf("\n");

  printf("Successfully converted!!\n" );

  fclose( fp );

  return 0;
}

int
main (int argc, char **argv)
{
  int i;

  if ( argc < 2 )
	{
	  printf("ZIC2XPM 2.01 - by Frank McIngvale (frankm@hiwaay.net)\n");
	  printf("Copyright (C) 1996 Free Software Foundation, Inc.\n\n");
	  printf("Usage: zic2xpm file1 [file2 ...]\n\n");
	  printf("  Splits each file (ZIICS piece files) into 26 XPM & XIM files\n");
	  printf("  suitable for use in XBoard 3.5 or later.\n");
	  printf("\n* ZIICS is a copyrighted work of Andy McFarland (Zek on ICC) *\n");
	  return 1;
	}


  setbuf( stdout, NULL );

  for( i=1; i<argc; ++i )
	{
	  process_file_xpm( argv[i] );
	  process_file_xim( argv[i] );
	}

  fprintf(stderr, "%s\n%s\n%s\n%s\n%s\n%s\n",
	  "NOTICE: The piece images distributed with ZIICS are",
	  "    copyrighted works of their original creators.  Images",
          "    converted with zic2xpm may not be redistributed without",
	  "    the permission of the copyright holders.  Do not contact",
	  "    the authors of zic2xpm or of ZIICS itself to request",
	  "    permission.");
  return 0;
}
