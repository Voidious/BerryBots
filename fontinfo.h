// From OpenVG library by ajstarks: https://github.com/ajstarks/openvg

#ifndef FONTINFO_H
#define FONTINFO_H

typedef struct {
	const short *CharacterMap;
	const int *GlyphAdvances;
	int Count;
	VGPath Glyphs[256];
} Fontinfo;

extern Fontinfo SansTypeface, SerifTypeface, MonoTypeface;

#endif
