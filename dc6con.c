/*
Copyright (C) 2000 Ryan Smith

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


modified by Paul Siramy, 19 March 2002
   remove 800*600 limits
   can read up to 1024 frames now
   reformat some of the code for my personal use (djgpp)

modified by Paul Siramy, 26 October 2002
   change the Header's Palette Type from 2 (greyscale) to 1 (color / B/W)

modified by Paul Siramy, 04 August 2011
   source modified to be able to compiled with MS VC++ 2008 Express Edition
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "dc6con.h"

#pragma warning (disable : 4996)

#define MAX_BLOCKCOUNT 1024
#define false          0
#define true           1
#define LittleLong(n)  (n)
#define O_BINARY       0
#define writelong(N, FILE) {long tempy = (N);fwrite(&tempy, 1, 4, (FILE));}

#ifdef WIN32
    #define strcasecmp _stricmp
    #define PATH_DELIMITER '\\'
#else
    #define PATH_DELIMITER '/'
#endif

#define long int // dirty hack to make it work

typedef unsigned char  byte;

char temp[2048]  = "";
int  myargc      = 0;
char ** myargv   = NULL;
long term[3]     = {0, 0, 0};
void * mem       = NULL;
long blockcount  = 0;
long version     = 0; // in case there is another version difference?!
long animated    = 0; // this is a guess
long width       = 0;
long height      = 0;
void * dc6data   = 0;
long dc6datasize = 0;
int  transparent = 0;
int  transcol    = 0;
long ofs_x      [MAX_BLOCKCOUNT];
long ofs_y      [MAX_BLOCKCOUNT];

// format : 256 BRG elements
byte default_palette[768] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x08, 0x18, 0x1C, 0x10, 0x24, 0x2C, 0x18, 0x34, 0x3C, 0x00, 
	0x00, 0x5C, 0x20, 0x40, 0x48, 0x28, 0x48, 0x54, 0x00, 0x00, 0x90, 0x10, 0x48, 0x8C, 0x00, 0x00, 
	0xBC, 0x20, 0x84, 0xD0, 0x6C, 0xC4, 0xF4, 0x50, 0x7C, 0x8C, 0x64, 0x9C, 0xAC, 0x08, 0x0C, 0x0C, 
	0x10, 0x10, 0x14, 0x1C, 0x1C, 0x1C, 0x1C, 0x24, 0x28, 0x2C, 0x2C, 0x2C, 0x30, 0x38, 0x3C, 0x38, 
	0x38, 0x38, 0x48, 0x48, 0x48, 0x48, 0x50, 0x58, 0x34, 0x58, 0x64, 0x58, 0x58, 0x58, 0x3C, 0x64, 
	0x74, 0x64, 0x64, 0x64, 0x70, 0x74, 0x7C, 0x84, 0x84, 0x84, 0x94, 0x98, 0x9C, 0xC4, 0xC4, 0xC4, 
	0xF4, 0xF4, 0xF4, 0x04, 0x04, 0x08, 0x00, 0x04, 0x10, 0x04, 0x08, 0x18, 0x08, 0x10, 0x18, 0x10, 
	0x14, 0x1C, 0x04, 0x0C, 0x24, 0x0C, 0x18, 0x20, 0x14, 0x18, 0x20, 0x08, 0x10, 0x2C, 0x10, 0x1C, 
	0x24, 0x0C, 0x20, 0x28, 0x04, 0x08, 0x38, 0x10, 0x1C, 0x30, 0x14, 0x28, 0x30, 0x0C, 0x14, 0x40, 
	0x10, 0x28, 0x38, 0x04, 0x0C, 0x48, 0x1C, 0x28, 0x38, 0x0C, 0x20, 0x4C, 0x14, 0x2C, 0x44, 0x20, 
	0x2C, 0x40, 0x08, 0x10, 0x58, 0x20, 0x34, 0x48, 0x28, 0x34, 0x44, 0x1C, 0x28, 0x54, 0x14, 0x1C, 
	0x5C, 0x08, 0x24, 0x5C, 0x18, 0x38, 0x54, 0x24, 0x38, 0x54, 0x10, 0x18, 0x6C, 0x10, 0x2C, 0x68, 
	0x24, 0x44, 0x5C, 0x18, 0x24, 0x70, 0x24, 0x3C, 0x68, 0x0C, 0x2C, 0x7C, 0x2C, 0x4C, 0x64, 0x08, 
	0x48, 0x70, 0x18, 0x28, 0x80, 0x28, 0x50, 0x74, 0x24, 0x30, 0x88, 0x40, 0x50, 0x6C, 0x0C, 0x34, 
	0x8C, 0x68, 0x20, 0x70, 0x44, 0x58, 0x78, 0x40, 0x40, 0x8C, 0x30, 0x5C, 0x84, 0x24, 0x34, 0x9C, 
	0x18, 0x28, 0xA4, 0x14, 0x48, 0xA4, 0x40, 0x64, 0x8C, 0x38, 0x70, 0x8C, 0x50, 0x68, 0x8C, 0x34, 
	0x44, 0xB0, 0x20, 0x4C, 0xB4, 0x58, 0x70, 0x98, 0x40, 0x78, 0xA0, 0x1C, 0x60, 0xBC, 0x48, 0x84, 
	0x9C, 0x54, 0x54, 0xC4, 0x20, 0x6C, 0xC8, 0x58, 0x7C, 0xAC, 0x48, 0x88, 0xB0, 0x54, 0x7C, 0xC8, 
	0x20, 0x70, 0xE0, 0x54, 0x9C, 0xB8, 0x00, 0x2C, 0xFC, 0x70, 0x8C, 0xC0, 0x50, 0x98, 0xCC, 0x30, 
	0x84, 0xE4, 0x70, 0x70, 0xE0, 0x78, 0x98, 0xD0, 0x3C, 0x88, 0xF8, 0x38, 0xA0, 0xEC, 0x64, 0xB8, 
	0xD8, 0x84, 0xA4, 0xE0, 0x44, 0xB4, 0xF0, 0x4C, 0xC0, 0xF4, 0x8C, 0xB0, 0xF0, 0x5C, 0xD4, 0xFC, 
	0xB0, 0xB0, 0xFC, 0x10, 0x24, 0x04, 0x18, 0x24, 0x14, 0x18, 0x3C, 0x20, 0x0C, 0x48, 0x18, 0x30, 
	0x44, 0x00, 0x08, 0x64, 0x18, 0x24, 0x5C, 0x24, 0x24, 0x5C, 0x38, 0x44, 0x6C, 0x08, 0x14, 0x7C, 
	0x28, 0x34, 0x74, 0x40, 0x30, 0x78, 0x58, 0x1C, 0x9C, 0x34, 0x38, 0x84, 0x70, 0x34, 0xA0, 0x48, 
	0x4C, 0x90, 0x58, 0x28, 0xBC, 0x44, 0x48, 0x98, 0x84, 0x4C, 0xB8, 0x60, 0x00, 0xFC, 0x18, 0x5C, 
	0xDC, 0x74, 0x7C, 0xD0, 0x8C, 0x88, 0xFC, 0xA0, 0x28, 0x0C, 0x0C, 0x48, 0x18, 0x18, 0x58, 0x00, 
	0x00, 0x44, 0x14, 0x38, 0x68, 0x24, 0x10, 0x64, 0x3C, 0x28, 0x78, 0x28, 0x28, 0x80, 0x10, 0x4C, 
	0x84, 0x4C, 0x38, 0x94, 0x30, 0x30, 0x8C, 0x60, 0x48, 0xA0, 0x5C, 0x38, 0xAC, 0x50, 0x50, 0xAC, 
	0x6C, 0x4C, 0xBC, 0x78, 0x54, 0xD8, 0x60, 0x24, 0xD0, 0x78, 0x64, 0xE0, 0x90, 0x64, 0xDC, 0xA0, 
	0x80, 0xFC, 0x20, 0xA4, 0xF0, 0x84, 0x84, 0xFC, 0xA0, 0xA0, 0xFC, 0xB8, 0x90, 0x90, 0x8C, 0x58, 
	0xA4, 0xA0, 0x68, 0xC4, 0xC0, 0x84, 0xD4, 0xD0, 0x98, 0xFC, 0xCC, 0xA8, 0xF4, 0xF4, 0xCC, 0x80, 
	0xA0, 0xC0, 0xA8, 0xC0, 0xC4, 0x94, 0xC4, 0xE0, 0x74, 0xE8, 0xFC, 0xB0, 0xFC, 0xC4, 0xA4, 0xE4, 
	0xFC, 0xC4, 0xFC, 0xFC, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x0C, 0x0C, 0x0C, 0x10, 0x10, 0x10, 
	0x14, 0x14, 0x14, 0x18, 0x18, 0x18, 0x18, 0x1C, 0x24, 0x20, 0x20, 0x20, 0x24, 0x24, 0x24, 0x28, 
	0x28, 0x28, 0x20, 0x28, 0x30, 0x30, 0x30, 0x30, 0x28, 0x30, 0x38, 0x34, 0x34, 0x34, 0x3C, 0x38, 
	0x34, 0x34, 0x38, 0x44, 0x3C, 0x3C, 0x3C, 0x30, 0x3C, 0x4C, 0x40, 0x40, 0x40, 0x3C, 0x40, 0x44, 
	0x44, 0x44, 0x44, 0x3C, 0x48, 0x50, 0x38, 0x44, 0x58, 0x4C, 0x4C, 0x4C, 0x3C, 0x4C, 0x60, 0x5C, 
	0x58, 0x34, 0x50, 0x50, 0x50, 0x54, 0x54, 0x54, 0x5C, 0x5C, 0x5C, 0x50, 0x5C, 0x68, 0x60, 0x60, 
	0x60, 0x74, 0x70, 0x44, 0x50, 0x64, 0x7C, 0x68, 0x68, 0x68, 0x6C, 0x6C, 0x6C, 0x60, 0x6C, 0x78, 
	0x70, 0x70, 0x70, 0x74, 0x74, 0x74, 0x7C, 0x7C, 0x7C, 0x64, 0x80, 0x94, 0x74, 0x84, 0x90, 0x70, 
	0x88, 0xAC, 0x90, 0x90, 0x90, 0x84, 0x94, 0x9C, 0xB8, 0x94, 0x80, 0xA0, 0xA0, 0xA0, 0x98, 0xAC, 
	0xB0, 0xAC, 0xAC, 0xAC, 0xB8, 0xB8, 0xB8, 0xCC, 0xCC, 0xCC, 0xD8, 0xD8, 0xD8, 0xFC, 0xCC, 0xCC, 
	0xE4, 0xE4, 0xE4, 0x80, 0x00, 0xFC, 0x08, 0x14, 0x10, 0x08, 0x1C, 0x14, 0x08, 0x20, 0x14, 0x0C, 
	0x24, 0x18, 0x0C, 0x2C, 0x20, 0x18, 0x34, 0x20, 0x1C, 0x38, 0x30, 0x44, 0x20, 0x00, 0x68, 0x00, 
	0x18, 0x04, 0x08, 0x08, 0x10, 0x14, 0x14, 0x14, 0x18, 0x18, 0x14, 0x1C, 0x1C, 0x10, 0x20, 0x1C, 
	0x18, 0x20, 0x24, 0x14, 0x28, 0x24, 0x20, 0x28, 0x28, 0x24, 0x28, 0x2C, 0x30, 0x2C, 0x20, 0x24, 
	0x2C, 0x30, 0x28, 0x30, 0x34, 0x2C, 0x34, 0x38, 0x40, 0x3C, 0x30, 0x34, 0x3C, 0x40, 0x38, 0x44, 
	0x48, 0x44, 0x4C, 0x50, 0x4C, 0x58, 0x5C, 0x54, 0x5C, 0x60, 0x58, 0x64, 0x68, 0xFF, 0xFF, 0xFF
};

byte * palette = NULL;

#pragma pack(1)

typedef struct dc6header_s
{
    long version;	       // 06000000
	long unknown01;	     // 01000000
	long unknown02;	     // 00000000
	byte termination[4]; // EEEEEEEE or CDCDCDCD
	long unknown03;	     // 10000000
	long blockcount;	    // 10000000
	// after this, are pointers
	long pointer[MAX_BLOCKCOUNT];
} dc6header;

typedef struct dc6blockheader_s
{
	long unknown01; // 000000000
	long width;
	long height;
	long unknown02;
	long unknown03;
	long unknown04;
	long next_block;
	long length;
	byte data [65000];
} dc6blockheader;

typedef struct
{
 char	          manufacturer;
 char	          version;
 char	          encoding;
 char	          bits_per_pixel;
 unsigned short xmin, ymin, xmax, ymax;
 unsigned short hres, vres;
 unsigned char	 palette [48];
 char	          reserved;
 char	          color_planes;
 unsigned short	bytes_per_line;
 unsigned short	palette_type;
 char	          filler [58];
 unsigned char	 data [65000];			// unbounded
} pcx_t;

#pragma pack()

void * pointer_block [MAX_BLOCKCOUNT]; //800x600 image has 12


byte *allocimage(long width, long height);



void Error (char *error, ...)
{
	va_list argptr;

	printf ("\n************ ERROR ************\n");

	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
	exit(1);
}


int CheckParm (char * check)
{
	int i = 0;


	for (i = 1; i < myargc; i++)
	{
		if (strcasecmp(check, myargv[i]) == 0)
			return i;
	}

	return 0;
}


void StripExtension (char * path)
{
	int length = 0;


	length = strlen(path)-1;
	while (length > 0 && path[length] != '.')
	{
		length--;
		if (path[length] == PATH_DELIMITER)
			return;		// no extension
	}
	if (length)
		path[length] = 0;
}


//long BigLong(long n)
//{
//	byte * a = NULL;
//	byte * b = NULL;
//	long out = 0;


//	a = (void *) &n;
//	b = (void *) &out;
//	b[0] = a[3];
//	b[1] = a[2];
//	b[2] = a[1];
//	b[3] = a[0];
//	return out;
//}


long GetFileInfoDC6()
{
	dc6header * head = NULL;
	long      i      = 0;


    head = (dc6header *)mem;
	version = LittleLong(head->version);
    if (version != 6)
        Error("GetFileInfoDC6: dc6 file is not version 6 - it's %d\n", version);

	for (i=0; i < 4; i++)
		term[i] = head->termination[i];
	blockcount = head->blockcount;
	if ((blockcount > MAX_BLOCKCOUNT) || (blockcount < 1))
		Error("DC6 file has invalid number of blocks\n");

	for (i=0; i < blockcount; i++)
	{
		pointer_block[i] = head; // mem;
		(byte *)pointer_block[i] += head->pointer[i];
	}

	// HACK
	animated = false;
	for (i=0; i < blockcount; i++)
	{
		if (((dc6blockheader *)pointer_block[i])->unknown03 != 0)
		{
			animated = true;
			break;
		}
		else if (((dc6blockheader *)pointer_block[i])->unknown02 != 0)
		{
			animated = true;
			break;
		}
	}

	return true;
}


long GetBlockInfoDC6(void *block)
{
	dc6blockheader *head;
    head = (dc6blockheader *)block;
	width = head->width;
	height = head->height;
	dc6data = head->data;
	dc6datasize =head->length;

	return true;
}


void CompressDC6 (void *indata, FILE *f, long w, long h, long rowbytes)
{
	byte *in, *test;
	long p, line, pixels, pwidth;
	int i;

	line = h-1;
	pixels = 0;
	in = (byte *)indata + rowbytes * line;


	while (line >= 0)
	{
		while (pixels < w)
		{

			pwidth = w-pixels;
			if (pwidth > 0x7F)
				pwidth = 0x7F;
			if (transparent)
			{

				if (*in == transcol)
				{
					p = 0;
					test = in;
					for (i = pixels; i < w; i++)
					{
						if (*test == transcol)
							p++;
						else
							break;
						test++;
					}

					if ((p + pixels) == w)
					{
						pixels += p;
						in += p;

						continue;
					}
					if (p > 0x7F)
						p = 0x7F;
					in += p;
					pixels += p;

					fputc(p + 0x80, f);

					continue;

				}
				else
				{
					for(i = 0; i < pwidth; i++)
					{
						if (in[i] == transcol)
						{
							pwidth = i;
							break;
						}
					}
				}
			}
			fputc(pwidth, f);

			for (i = 0; i < pwidth; i++)
			{
				fputc(*in++, f);
				pixels++;
			}
		}
		pixels = 0;
		line--;
		fputc(0x80, f);
		in = (byte *)indata + rowbytes * line;
	}
	return;
}


void WriteDC6(FILE *handle, void *img)
{
	int c, i, x, y, sizex, sizey;
	long last, end, pointertable;
	long fx,fy;

	fx = width / 256;
	if (width % 256)
	 fx++;

	fy = height / 256;
	if (height % 256)
	 fy++;

	blockcount = fx * fy;
	writelong(6, handle);
	writelong(1, handle);
	writelong(0, handle);
	fputc(0xEE, handle);
	fputc(0xEE, handle);
	fputc(0xEE, handle);
	fputc(0xEE, handle);
	writelong(1, handle);
	writelong(blockcount, handle);
	pointertable = ftell(handle);

	for (i=0; i < blockcount; i++)
		writelong(0, handle);
	printf("Image Size: %i x %i Subdivides: %i\n", width, height, blockcount);
	// write the blocks
	i = 0;
	for (y = 0; y < height; y += 256)
	{
		x = 0;
		while(x < width)
		{

			sizex = width - x > 256 ? 256 : width - x;
			sizey = height - y > 256 ? 256 : height - y;

			// put entry in pointer table
			end = ftell(handle);
			fseek(handle, pointertable + i * 4, SEEK_SET);
			writelong(end, handle);
			fseek(handle, end, SEEK_SET);

			// write block header
			writelong(0, handle);
			writelong(sizex, handle);
			writelong(sizey, handle);
			writelong(0, handle);
			writelong(0, handle);
			writelong(0, handle);
			last = ftell(handle);
			writelong(0, handle);
			writelong(0, handle);

			// write the image data
			CompressDC6((byte *)img + x + (y * width), handle, sizex, sizey, width);
			
			end = ftell(handle);
			fseek(handle, last, SEEK_SET);
			writelong(end + 3, handle);
			writelong(end - last - 8, handle);
			fseek(handle, end, SEEK_SET);
			for (c = 0; c < 3; c++)
				fputc(0xEE, handle);
			i++;
			x += 256;
		}
	}
}


long decompressdc6(void *indata, void *outdata, long insize, long width, long height, long rowbytes)
{
	byte *in, *out, *inend, *outend;
	long c1, line, pixels;

    in = (byte *)indata;
	inend = in + insize;
	line = height-1;
	out = (byte *)outdata + rowbytes * line;
	outend = out + width*height;
	pixels = 0;

	while (in < inend && out >= (byte *)outdata)
	{
		c1 = *in++;
		if (c1 == 0x80)
		{
			if ((in[1] == term[1]) && (in[2] == term[2]) && (in[3] == term[3]))
			{
				return true;
			}
			if (pixels < width)
				transparent = 1;
			line--;
			out = (byte *)outdata + rowbytes * line;
			pixels = 0;
		}
		else
		{

			if (c1 > 0x80)
			{
				while (c1 > 0x80 && pixels < width)
				{
					out++;
					c1--;
					pixels++;
				}
				if (c1 > 0x80)
				{
					printf("span overrun @ %X\n", (byte *)in - (byte *)indata);
				}
				transparent = 1;
			}

			else if (c1 == 0)
			{
				printf("found byte 00\n");
			}
			else
			{
				while (c1 > 0 && pixels < width)
				{
					*out++ = *in++;
					c1--;
					pixels++;
				}

				if (c1 > 0x0)
				printf("span overrun @ %X\n", (byte *)in - (byte *)indata);
			}
		}
	}
	return true;
}


void WritePCXfile (char *filename, byte *data, int w, int h,	int rowbytes, byte *palette) 
{
	int		i, j, length, size;
	FILE *handle;
	pcx_t	*pcx;
	byte		*pack;
	  
 size = w*h*2+1000;
    pcx  = (pcx_t *)malloc(size);
	memset (pcx,0,size);

 pcx->manufacturer = 0x0a;	// PCX id
	pcx->version = 5;			// 256 color
 pcx->encoding = 1;		// RLE type
	pcx->bits_per_pixel = 8;		// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = (short)(w-1);
	pcx->ymax = (short)(h-1);
	pcx->hres = (short)w;
	pcx->vres = (short)h;
	memset (pcx->palette,0,sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = (short)w;
	pcx->palette_type = (short)1;		// not a grey scale
	memset (pcx->filler,0,sizeof(pcx->filler));

// pack the image
	pack = pcx->data;
	
	for (i=0 ; i<h ; i++)
	{
		for (j=0 ; j<w; j++)
		{
			if ( (*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}

		data += rowbytes - w;
	}
			
// write the palette
	*pack++ = 0x0c;	// palette ID byte
	for (i=0 ; i<768 ; i++)
		*pack++ = *palette++;
		
// write output file 
	length = pack - (byte *)pcx;
	handle = fopen(filename,"wb");
	
	fwrite(pcx, 1, length, handle);
	fclose(handle);
} 


byte *DecodePCX (void *in)
{
	pcx_t	*pcx;
	byte	*pix;
	int		x, y;
	byte	*outdata, *out;
	int		runLength;
	byte	databyte;

    pcx = (pcx_t *)in;

	if (   pcx->manufacturer != 0x0a
		   || pcx->version != 5
   		|| pcx->encoding != 1
   		|| pcx->bits_per_pixel != 8
    )
	{
		Error ("Bad pcx file\n");

	}
	outdata = allocimage(pcx->xmax, pcx->ymax);
	out = outdata;
	pix = pcx->data;

	for (y=0 ; y<=pcx->ymax ; y++)
	{
		for (x=0 ; x<=pcx->xmax ; )
		{
			databyte = *pix++;

			if((databyte & 0xC0) == 0xC0)
			{
				runLength = databyte & 0x3F;
				databyte = *pix++;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
			{
				*out = databyte;
				out++;
				x++;
			}
		}
	}
	height = pcx->ymax + 1;
	width = pcx->xmax + 1;
	return outdata;
}


void *loadfile(const char *filename, long *filesize)
{
	FILE          * file = NULL;
	void          * mem  = NULL;
	unsigned long size   = 0;


	if (!(file = fopen(filename, "rb")))
		return NULL;
	fseek(file, 0, SEEK_END);
	if (!(size = ftell(file)))
	{
		fclose(file);
		return NULL;
	}
	if (!(mem = malloc(size + 0x10000))) // temporary, extra 64k for overflow buffering
	{
		fclose(file);
		return NULL;
	}
	fseek(file, 0, SEEK_SET);
	if (fread(mem, 1, size, file) < size)
	{
		fclose(file);
		free(mem);
		return NULL;
	}
	fclose(file);
	*filesize = size;
	return mem;
}


byte *allocimage(long width, long height)
{
	void *pntr;
    pntr = malloc(width*height + 0x10000);
    memset(pntr, transcol, width*height + 0x10000);
    return (byte *)pntr;// temporary, extra 64k for overflow buffering
}


void swappalettebytes()
{
	int i, c;
	for (i = 0;i < 768;i+=3)
	{
		c = palette[i];
		palette[i] = palette[i+2];
		palette[i+2] = c;
	}
}


void dc6ImageData(const char *filename, unsigned char *imageData, int *width_, int *height_)
{
	long filesize;
	long i;

	memset(pointer_block, 0, sizeof(pointer_block));
	transparent = 0;
    palette = default_palette;

	swappalettebytes();
    if (!(mem = loadfile(filename, &filesize)))
        Error("File not found %s\n", filename);

	GetFileInfoDC6();

    long totalwidth = 0, totalheight = 0;
    width = height = 0;
    for (i=0; i < blockcount; i++)
    {
        if (width != 256)
        {
            totalwidth = 0;
        }
        GetBlockInfoDC6(pointer_block[i]);
        ofs_x[i] = totalwidth;

        if (totalwidth == 0)
            totalheight += height;

        ofs_y[i] = totalheight - height;

        totalwidth += width;
    }

    if (!(imageData = allocimage(totalwidth, totalheight)))
        Error("Not enough memory\n");
    for (i=0; i < blockcount; i++)
    {
        GetBlockInfoDC6(pointer_block[i]);
        decompressdc6(dc6data, (byte *)imageData + ofs_x[i] + ofs_y[i] * totalwidth, dc6datasize, width, height, totalwidth);
    }

    *width_ = totalwidth;
    *height_ = totalheight;
}

