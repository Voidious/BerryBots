// Adapted from OpenVG library by ajstarks: https://github.com/ajstarks/openvg

#ifndef SHAPES_H
#define SHAPES_H

#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"

extern void Text(VGfloat, VGfloat, char *, Fontinfo, int);
extern void SansText(VGfloat, VGfloat, char *, int);
extern void TextMid(VGfloat, VGfloat, char *, Fontinfo, int);
extern void SansTextMid(VGfloat, VGfloat, char *, int);
extern void TextEnd(VGfloat, VGfloat, char *, Fontinfo, int);
extern void Start(int, int);
extern void End();
extern void init(int *, int *);
extern void finish();
extern void setfill(VGfloat[4]);
extern void setstroke(VGfloat[4]);
extern void StrokeWidth(VGfloat);
extern void Stroke(unsigned int, unsigned int, unsigned int, VGfloat);
extern void Fill(unsigned int, unsigned int, unsigned int, VGfloat);
extern void RGBA(unsigned int, unsigned int, unsigned int, VGfloat, VGfloat[4]);
extern void RGB(unsigned int, unsigned int, unsigned int, VGfloat[4]);
extern Fontinfo loadfont(const int *, const int *, const unsigned char *, const int *, const int *, const int *, const short *, int);
extern void unloadfont(VGPath *, int);
extern VGfloat textwidth(char *, Fontinfo, VGfloat);
extern void makeimage(VGfloat, VGfloat, int, int, void *, VGImageFormat);
extern VGPath newpath();

#endif
