/*
Multifont GFX library is adapted from Adafruit_GFX library by Paul Kourany
v1.0.0, May 2014 Initial Release
v1.0.1, June 2014 Font Compilation update
v1.0.2, Aug 2015 Added charWidth(char) function to return char width in pixels

Please read README.pdf for details
*/

//  fonts.h

#ifndef _fonts_h
#define _fonts_h

#include "Particle.h"

//Font selection for compiling - comment out or uncomment definitions as required
//NOTE: GLCDFONT is default font and always included
//#define TIMESNEWROMAN8
//#define CENTURYGOTHIC8
//#define ARIAL8
//#define COMICSANSMS8
//#define TESTFONT
#define ARIAL12
#define ARIAL12N
#define ARIAL8
#define ARIAL8N
#define ARIAL36
// Font selection descriptors - Add an entry for each new font and number sequentially
#define ARIAL_12	0
#define ARIAL_12_N	1
#define ARIAL_8		2
#define ARIAL_8_N	3
#define GLCDFONT	4
#define ARIAL_36	5


#define FONT_START 0
#define FONT_END 1

struct FontDescriptor
{
	uint8_t	width;		// width in bits
	uint8_t	height; 	// char height in bits
	uint16_t offset;	// offset of char into char array
};

// Font references - add pair of references for each new font
#ifdef ARIAL12
extern const unsigned char Arial_12ptBitmaps[];
extern const FontDescriptor Arial_12ptDescriptors[];
#endif

#ifdef ARIAL12N
extern const unsigned char Arial_12pt_NBitmaps[];
extern const FontDescriptor Arial_12pt_NDescriptors[];
#endif

#ifdef ARIAL8N
extern const unsigned char Arial_8pt_NBitmaps[];
extern const FontDescriptor Arial_8pt_NDescriptors[];
#endif


#ifdef ARIAL8
extern const unsigned char arial_8ptBitmaps[];
extern const FontDescriptor arial_8ptDescriptors[];
#endif

#ifdef COMICSANSMS8
extern const unsigned char comicSansMS_8ptBitmaps[];
extern const FontDescriptor comicSansMS_8ptDescriptors[];
#endif

extern const unsigned char glcdfontBitmaps[];
extern const FontDescriptor glcdfontDescriptors[];

#ifdef ARIAL36
extern const unsigned char Arial_36ptsBitmaps[];
extern const FontDescriptor Arial_36ptsDescriptors[];
#endif





#endif
