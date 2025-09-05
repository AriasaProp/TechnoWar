#ifndef __STB_INCLUDE_STB_TRUETYPE_H__
#define __STB_INCLUDE_STB_TRUETYPE_H__
#include "common.h"
//     the 'macStyle' header field; i don't know if fonts set this consistently
#define STBTT_MACSTYLE_DONTCARE   0
#define STBTT_MACSTYLE_BOLD       1
#define STBTT_MACSTYLE_ITALIC     2
#define STBTT_MACSTYLE_UNDERSCORE 4
#define STBTT_MACSTYLE_NONE       8 // <= not same as 0, this makes us check the bitfield is 0

#define stbtt_vertex_type short // can't use int16_t because that's not visible in the header file

#define STBTT_POINT_SIZE(x) (-(x))

enum {
  // platformID
  STBTT_PLATFORM_ID_UNICODE = 0,
  STBTT_PLATFORM_ID_MAC = 1,
  STBTT_PLATFORM_ID_ISO = 2,
  STBTT_PLATFORM_ID_MICROSOFT = 3
};
enum {
  // encodingID for STBTT_PLATFORM_ID_UNICODE
  STBTT_UNICODE_EID_UNICODE_1_0 = 0,
  STBTT_UNICODE_EID_UNICODE_1_1 = 1,
  STBTT_UNICODE_EID_ISO_10646 = 2,
  STBTT_UNICODE_EID_UNICODE_2_0_BMP = 3,
  STBTT_UNICODE_EID_UNICODE_2_0_FULL = 4
};
enum {
  // encodingID for STBTT_PLATFORM_ID_MICROSOFT
  STBTT_MS_EID_SYMBOL = 0,
  STBTT_MS_EID_UNICODE_BMP = 1,
  STBTT_MS_EID_SHIFTJIS = 2,
  STBTT_MS_EID_UNICODE_FULL = 10
};
enum {
  // encodingID for STBTT_PLATFORM_ID_MAC; same as Script Manager codes
  STBTT_MAC_EID_ROMAN = 0,
  STBTT_MAC_EID_ARABIC = 4,
  STBTT_MAC_EID_JAPANESE = 1,
  STBTT_MAC_EID_HEBREW = 5,
  STBTT_MAC_EID_CHINESE_TRAD = 2,
  STBTT_MAC_EID_GREEK = 6,
  STBTT_MAC_EID_KOREAN = 3,
  STBTT_MAC_EID_RUSSIAN = 7
};
enum {
  // languageID for STBTT_PLATFORM_ID_MICROSOFT; same as LCID...
  // problematic because there are e.g. 16 english LCIDs and 16 arabic LCIDs
  STBTT_MS_LANG_ENGLISH = 0x0409,
  STBTT_MS_LANG_ITALIAN = 0x0410,
  STBTT_MS_LANG_CHINESE = 0x0804,
  STBTT_MS_LANG_JAPANESE = 0x0411,
  STBTT_MS_LANG_DUTCH = 0x0413,
  STBTT_MS_LANG_KOREAN = 0x0412,
  STBTT_MS_LANG_FRENCH = 0x040c,
  STBTT_MS_LANG_RUSSIAN = 0x0419,
  STBTT_MS_LANG_GERMAN = 0x0407,
  STBTT_MS_LANG_SPANISH = 0x0409,
  STBTT_MS_LANG_HEBREW = 0x040d,
  STBTT_MS_LANG_SWEDISH = 0x041D
};
enum {
  // languageID for STBTT_PLATFORM_ID_MAC
  STBTT_MAC_LANG_ENGLISH = 0,
  STBTT_MAC_LANG_JAPANESE = 11,
  STBTT_MAC_LANG_ARABIC = 12,
  STBTT_MAC_LANG_KOREAN = 23,
  STBTT_MAC_LANG_DUTCH = 4,
  STBTT_MAC_LANG_RUSSIAN = 32,
  STBTT_MAC_LANG_FRENCH = 1,
  STBTT_MAC_LANG_SPANISH = 6,
  STBTT_MAC_LANG_GERMAN = 2,
  STBTT_MAC_LANG_SWEDISH = 5,
  STBTT_MAC_LANG_HEBREW = 10,
  STBTT_MAC_LANG_CHINESE_SIMPLIFIED = 33,
  STBTT_MAC_LANG_ITALIAN = 3,
  STBTT_MAC_LANG_CHINESE_TRAD = 19
};
enum {
  STBTT_vmove = 1,
  STBTT_vline,
  STBTT_vcurve,
  STBTT_vcubic
};

// private structure
typedef struct {
  unsigned char *data;
  int cursor;
  int size;
} stbtt__buf;
// @TODO: don't expose this structure
typedef struct {
  int w, h, stride;
  unsigned char *pixels;
} stbtt__bitmap;

typedef struct {
  stbtt_vertex_type x, y, cx, cy, cx1, cy1;
  unsigned char type, padding;
} stbtt_vertex;
typedef struct {
  unsigned short x0, y0, x1, y1; // coordinates of bbox in bitmap
  float xoff, yoff, xadvance;
} stbtt_bakedchar;
typedef struct {
  float x0, y0, s0, t0; // top-left
  float x1, y1, s1, t1; // bottom-right
} stbtt_aligned_quad;
typedef struct {
  unsigned short x0, y0, x1, y1; // coordinates of bbox in bitmap
  float xoff, yoff, xadvance;
  float xoff2, yoff2;
} stbtt_packedchar;
typedef struct {
  float font_size;
  int first_unicode_codepoint_in_range; // if non-zero, then the chars are continuous, and this is the first codepoint
  int *array_of_unicode_codepoints;     // if non-zero, then this is an array of unicode codepoints
  int num_chars;
  stbtt_packedchar *chardata_for_range; // output
  unsigned char h_oversample, v_oversample; // don't set these, they're used internally
} stbtt_pack_range;
typedef struct stbtt_kerningentry {
  int glyph1; // use stbtt_FindGlyphIndex
  int glyph2;
  int advance;
} stbtt_kerningentry;
typedef struct stbtt_fontinfo {
  void *userdata;
  const unsigned char *data; // pointer to .ttf file
  int fontstart;       // offset of start of font
  int numGlyphs; // number of glyphs, needed for range checking
  int loca,head,glyf,hhea,hmtx,kern,gpos,svg;
  // table locations as offset from start of .ttf
  int index_map;        // a cmap mapping for our chosen character encoding
  int indexToLocFormat; // format needed to map from glyph index to glyph
  stbtt__buf cff;         // cff font data
  stbtt__buf charstrings; // the charstring index
  stbtt__buf gsubrs;      // global charstring subroutines index
  stbtt__buf subrs;       // private charstring subroutines index
  stbtt__buf fontdicts;   // array of font dicts
  stbtt__buf fdselect;    // map from glyph to fontdict
} stbtt_fontinfo;

#ifndef STB_TRUTYPE_IMPLEMENTATION
extern int stbtt_InitFont(stbtt_fontinfo *info, const unsigned char *data, int offset);
// Given an offset into the file that defines a font, this function builds
// the necessary cached info for the rest of the system. You must allocate
// the stbtt_fontinfo yourself, and stbtt_InitFont will fill it out. You don't
// need to do anything special to free it, because the contents are pure
// value data with no additional data structures. Returns 0 on failure.

#endif // STB_TRUTYPE_IMPLEMENTATION

extern int stbtt_BakeFontBitmap(const unsigned char *, int, // font location (use offset=0 for plain .ttf)
                                float,                      // height of font in pixels
                                unsigned char *, int, int,  // bitmap to be filled in
                                int, int,                   // characters to bake
                                stbtt_bakedchar *);         // you allocate this, it's num_chars long
// if return is positive, the first unused row of the bitmap
// if return is negative, returns the negative of the number of characters that fit
// if return is 0, no characters fit and no rows were used
// This uses a very crappy packing.

extern void stbtt_GetBakedQuad(const stbtt_bakedchar *chardata, int pw, int ph, // same data as above
                               int char_index,                                  // character to display
                               float *xpos, float *ypos,                        // pointers to current position in screen pixel space
                               stbtt_aligned_quad *q,                           // output: quad to draw
                               int opengl_fillrule);                            // true if opengl fill rule; false if DX9 or earlier
// Call GetBakedQuad with char_index = 'character - first_char', and it
// creates the quad you need to draw and advances the current position.
//
// The coordinate system used assumes y increases downwards.
//
// Characters will extend both above and below the current position;
// see discussion of "BASELINE" above.
//
// It's inefficient; you might want to c&p it and optimize it.

extern void stbtt_GetScaledFontVMetrics(const unsigned char *fontdata, int index, float size, float *ascent, float *descent, float *lineGap);
// Query the font vertical metrics without having to create a font first.

extern void stbtt_GetPackedQuad(const stbtt_packedchar *chardata, int pw, int ph, // same data as above
                                int char_index,                                   // character to display
                                float *xpos, float *ypos,                         // pointers to current position in screen pixel space
                                stbtt_aligned_quad *q,                            // output: quad to draw
                                int align_to_integer);

extern int stbtt_GetNumberOfFonts(const unsigned char *data);
// This function will determine the number of fonts in a font file.  TrueType
// collection (.ttc) files may contain multiple fonts, while TrueType font
// (.ttf) files only contain one font. The number of fonts can be used for
// indexing with the previous function where the index is between zero and one
// less than the total fonts. If an error occurs, -1 is returned.

extern int stbtt_GetFontOffsetForIndex(const unsigned char *data, int index);
// Each .ttf/.ttc file may have more than one font. Each font has a sequential
// index number starting from 0. Call this function to get the font offset for
// a given index; it returns -1 if the index is out of range. A regular .ttf
// file will only define one font and it always be at offset 0, so it will
// return '0' for index 0, and -1 for all other indices.

// The following structure is defined publicly so you can declare one on
// the stack or as a global or etc, but you should treat it as opaque.

extern int stbtt_FindGlyphIndex(const stbtt_fontinfo *info, int unicode_codepoint);
// If you're going to perform multiple operations on the same character
// and you want a speed-up, call this function with the character you're
// going to process, then use glyph-based functions instead of the
// codepoint-based functions.
// Returns 0 if the character codepoint is not defined in the font.

//////////////////////////////////////////////////////////////////////////////
//
// CHARACTER PROPERTIES
//

extern float stbtt_ScaleForPixelHeight(const stbtt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose "height" is 'pixels' tall.
// Height is measured as the distance from the highest ascender to the lowest
// descender; in other words, it's equivalent to calling stbtt_GetFontVMetrics
// and computing:
//       scale = pixels / (ascent - descent)
// so if you prefer to measure height by the ascent only, use a similar calculation.

extern float stbtt_ScaleForMappingEmToPixels(const stbtt_fontinfo *info, float pixels);
// computes a scale factor to produce a font whose EM size is mapped to
// 'pixels' tall. This is probably what traditional APIs compute, but
// I'm not positive.

extern void stbtt_GetFontVMetrics(const stbtt_fontinfo *info, int *ascent, int *descent, int *lineGap);
// ascent is the coordinate above the baseline the font extends; descent
// is the coordinate below the baseline the font extends (i.e. it is typically negative)
// lineGap is the spacing between one row's descent and the next row's ascent...
// so you should advance the vertical position by "*ascent - *descent + *lineGap"
//   these are expressed in unscaled coordinates, so you must multiply by
//   the scale factor for a given size

extern int stbtt_GetFontVMetricsOS2(const stbtt_fontinfo *info, int *typoAscent, int *typoDescent, int *typoLineGap);
// analogous to GetFontVMetrics, but returns the "typographic" values from the OS/2
// table (specific to MS/Windows TTF files).
//
// Returns 1 on success (table present), 0 on failure.

extern void stbtt_GetFontBoundingBox(const stbtt_fontinfo *info, int *x0, int *y0, int *x1, int *y1);
// the bounding box around all possible characters

extern void stbtt_GetCodepointHMetrics(const stbtt_fontinfo *info, int codepoint, int *advanceWidth, int *leftSideBearing);
// leftSideBearing is the offset from the current horizontal position to the left edge of the character
// advanceWidth is the offset from the current horizontal position to the next horizontal position
//   these are expressed in unscaled coordinates

extern int stbtt_GetCodepointKernAdvance(const stbtt_fontinfo *info, int ch1, int ch2);
// an additional amount to add to the 'advance' value between ch1 and ch2

extern int stbtt_GetCodepointBox(const stbtt_fontinfo *info, int codepoint, int *x0, int *y0, int *x1, int *y1);
// Gets the bounding box of the visible part of the glyph, in unscaled coordinates

extern void stbtt_GetGlyphHMetrics(const stbtt_fontinfo *info, int glyph_index, int *advanceWidth, int *leftSideBearing);
extern int stbtt_GetGlyphKernAdvance(const stbtt_fontinfo *info, int glyph1, int glyph2);
extern int stbtt_GetGlyphBox(const stbtt_fontinfo *info, int glyph_index, int *x0, int *y0, int *x1, int *y1);
// as above, but takes one or more glyph indices for greater efficiency

extern int stbtt_GetKerningTableLength(const stbtt_fontinfo *info);
extern int stbtt_GetKerningTable(const stbtt_fontinfo *info, stbtt_kerningentry *table, int table_length);
// Retrieves a complete list of all of the kerning pairs provided by the font
// stbtt_GetKerningTable never writes more than table_length entries and returns how many entries it did write.
// The table will be sorted by (a.glyph1 == b.glyph1)?(a.glyph2 < b.glyph2):(a.glyph1 < b.glyph1)

//////////////////////////////////////////////////////////////////////////////
//
// GLYPH SHAPES (you probably don't need these, but they have to go before
// the bitmaps for C declaration-order reasons)
//

extern int stbtt_IsGlyphEmpty(const stbtt_fontinfo *info, int glyph_index);
// returns non-zero if nothing is drawn for this glyph

extern int stbtt_GetCodepointShape(const stbtt_fontinfo *info, int unicode_codepoint, stbtt_vertex **vertices);
extern int stbtt_GetGlyphShape(const stbtt_fontinfo *info, int glyph_index, stbtt_vertex **vertices);
// returns # of vertices and fills *vertices with the pointer to them
//   these are expressed in "unscaled" coordinates
//
// The shape is a series of contours. Each one starts with
// a STBTT_moveto, then consists of a series of mixed
// STBTT_lineto and STBTT_curveto segments. A lineto
// draws a line from previous endpoint to its x,y; a curveto
// draws a quadratic bezier from previous endpoint to
// its x,y, using cx,cy as the bezier control point.

extern void stbtt_FreeShape(const stbtt_fontinfo *info, stbtt_vertex *vertices);
// frees the data allocated above

extern const unsigned char *stbtt_FindSVGDoc(const stbtt_fontinfo *info, int gl);
extern int stbtt_GetCodepointSVG(const stbtt_fontinfo *info, int unicode_codepoint, const char **svg);
extern int stbtt_GetGlyphSVG(const stbtt_fontinfo *info, int gl, const char **svg);
// fills svg with the character's SVG data.
// returns data size or 0 if SVG not found.

//////////////////////////////////////////////////////////////////////////////
//
// BITMAP RENDERING
//

extern void stbtt_FreeBitmap(unsigned char *bitmap, void *userdata);
// frees the bitmap allocated below

extern unsigned char *stbtt_GetCodepointBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// allocates a large-enough single-channel 8bpp bitmap and renders the
// specified character/glyph at the specified scale into it, with
// antialiasing. 0 is no coverage (transparent), 255 is fully covered (opaque).
// *width & *height are filled out with the width & height of the bitmap,
// which is stored left-to-right, top-to-bottom.
//
// xoff/yoff are the offset it pixel space from the glyph origin to the top-left of the bitmap

extern unsigned char *stbtt_GetCodepointBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint, int *width, int *height, int *xoff, int *yoff);
// the same as stbtt_GetCodepoitnBitmap, but you can specify a subpixel
// shift for the character

extern void stbtt_MakeCodepointBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int codepoint);
// the same as stbtt_GetCodepointBitmap, but you pass in storage for the bitmap
// in the form of 'output', with row spacing of 'out_stride' bytes. the bitmap
// is clipped to out_w/out_h bytes. Call stbtt_GetCodepointBitmapBox to get the
// width and height and positioning info for it first.

extern void stbtt_MakeCodepointBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int codepoint);
// same as stbtt_MakeCodepointBitmap, but you can specify a subpixel
// shift for the character

extern void stbtt_MakeCodepointBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int codepoint);
// same as stbtt_MakeCodepointBitmapSubpixel, but prefiltering
// is performed (see stbtt_PackSetOversampling)

extern void stbtt_GetCodepointBitmapBox(const stbtt_fontinfo *font, int codepoint, vec2 scale, ivec2 *ip0, ivec2 *ip1);
// get the bbox of the bitmap centered around the glyph origin; so the
// bitmap width is ix1-ix0, height is iy1-iy0, and location to place
// the bitmap top left is (leftSideBearing*scale,iy0).
// (Note that the bitmap uses y-increases-down, but the shape uses
// y-increases-up, so CodepointBitmapBox and CodepointBox are inverted.)

extern void stbtt_GetCodepointBitmapBoxSubpixel(const stbtt_fontinfo *font, int codepoint,vec2 scale, vec2 shift, ivec2 *ip0, ivec2 *ip1);
// same as stbtt_GetCodepointBitmapBox, but you can specify a subpixel
// shift for the character

// the following functions are equivalent to the above functions, but operate
// on glyph indices instead of Unicode codepoints (for efficiency)
extern unsigned char *stbtt_GetGlyphBitmap(const stbtt_fontinfo *info, float scale_x, float scale_y, int glyph, int *width, int *height, int *xoff, int *yoff);
extern unsigned char *stbtt_GetGlyphBitmapSubpixel(const stbtt_fontinfo *info, float scale_x, float scale_y, float shift_x, float shift_y, int glyph, int *width, int *height, int *xoff, int *yoff);
extern void stbtt_MakeGlyphBitmap(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, int glyph);
extern void stbtt_MakeGlyphBitmapSubpixel(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);
extern void stbtt_MakeGlyphBitmapSubpixelPrefilter(const stbtt_fontinfo *info, unsigned char *output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int oversample_x, int oversample_y, float *sub_x, float *sub_y, int glyph);
extern void stbtt_GetGlyphBitmapBox(const stbtt_fontinfo *font, int glyph, vec2 scale, ivec2 *ip0, ivec2 *ip1);
// font data, glyph index, scale, shift, point 0 out, point 1 out
extern void stbtt_GetGlyphBitmapBoxSubpixel(const stbtt_fontinfo *, int, vec2, vec2, ivec2 *, ivec2 *);

// rasterize a shape with quadratic beziers into a bitmap
extern void stbtt_Rasterize(stbtt__bitmap *result,        // 1-channel bitmap to draw into
                            float flatness_in_pixels,     // allowable error of curve in pixels
                            stbtt_vertex *vertices,       // array of vertices defining shape
                            int num_verts,                // number of vertices in above array
                            float scale_x, float scale_y, // scale applied to input vertices
                            float shift_x, float shift_y, // translation applied to input vertices
                            int x_off, int y_off,         // another translation applied to input
                            int invert,                   // if non-zero, vertically flip shape
                            void *userdata);              // context for to STBTT_MALLOC

//////////////////////////////////////////////////////////////////////////////
//
// Signed Distance Function (or Field) rendering

extern void stbtt_FreeSDF(unsigned char *bitmap, void *userdata);
// frees the SDF bitmap allocated below

extern unsigned char *stbtt_GetGlyphSDF(const stbtt_fontinfo *info, float scale, int glyph, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, ivec2 *off);
extern unsigned char *stbtt_GetCodepointSDF(const stbtt_fontinfo *info, float scale, int codepoint, int padding, unsigned char onedge_value, float pixel_dist_scale, int *width, int *height, ivec2 *off);

extern int stbtt_FindMatchingFont(const unsigned char *fontdata, const char *name, int flags);
// returns the offset (not index) of the font that matches, or -1 if none
//   if you use STBTT_MACSTYLE_DONTCARE, use a font name like "Arial Bold".
//   if you use any other flag, use a font name like "Arial"; this checks

extern int stbtt_CompareUTF8toUTF16_bigendian(const char *s1, int len1, const char *s2, int len2);
extern const char *stbtt_GetFontNameString(const stbtt_fontinfo *font, int *length, int platformID, int encodingID, int languageID, int nameID);
#endif // __STB_INCLUDE_STB_TRUETYPE_H__