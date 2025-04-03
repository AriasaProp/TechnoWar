#ifndef STB_LOCAL_INCLUDED_
#define STB_LOCAL_INCLUDED_

#define PNG_SIGNATURE "\x89PNG\r\n\x1A\n"

#define MAX_DIMENSIONS (1 << 24)


enum STBI_FLAGS {
	// for image formats that explicitly notate that they have premultiplied alpha,
	// we just return the colors as stored in the file. set this flag to force
	// unpremultiplication. results are undefined if the unpremultiply overflow.
	UNPREMULTIPLY_ON_LOAD = 1,
	// indicate whether we should process iphone images back to canonical format,
	// or just pass them through "as-is"
	CONVERT_IPHONE_PNG_TO_RGB = 2,
	// flip the image vertically, so the first pixel is the bottom left
	FLIP_VERTICALLY = 4,
	
	// 
	WRITE_TGA_WITH_RLE = 8,
	
	ALL_FLAGS = -1,
};

extern int stbi_enabled_flags;

extern void stbi_enable_flags(int);
extern void stbi_disable_flags(int);

#endif // STB_LOCAL_INCLUDED_