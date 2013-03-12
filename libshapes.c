// Adapted from OpenVG library by ajstarks: https://github.com/ajstarks/openvg
//
// libshapes: high-level OpenVG API
// Anthony Starks (ajstarks@gmail.com)
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "EGL/egl.h"
#include "GLES/gl.h"
#include "bcm_host.h"
#include "resources/DejaVuSans.inc"  // font data
#include "eglstate.h"                // data structures for graphics state
#include "fontinfo.h"                // font data structure

static STATE_T _state, *state = &_state;    // global graphics state
static const int MAXFONTPATH = 256;
static const int MAX_PATH_CACHE_SIZE = 256;
static const int MAX_PAINT_CACHE_SIZE = 64;

//
// Font functions
//

// loadfont loads font path data
// derived from http://web.archive.org/web/20070808195131/http://developer.hybrid.fi/font2openvg/renderFont.cpp.txt
Fontinfo loadfont(const int *Points,
          const int *PointIndices,
          const unsigned char *Instructions,
          const int *InstructionIndices, const int *InstructionCounts, const int *adv, const short *cmap, int ng) {

    Fontinfo f;
    int i;

    memset(f.Glyphs, 0, MAXFONTPATH * sizeof(VGPath));
    if (ng > MAXFONTPATH) {
        return f;
    }
    for (i = 0; i < ng; i++) {
        const int *p = &Points[PointIndices[i] * 2];
        const unsigned char *instructions = &Instructions[InstructionIndices[i]];
        int ic = InstructionCounts[i];
        VGPath path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_S_32,
                       1.0f / 65536.0f, 0.0f, 0, 0,
                       VG_PATH_CAPABILITY_ALL);
        f.Glyphs[i] = path;
        if (ic) {
            vgAppendPathData(path, ic, instructions, p);
        }
    }
    f.CharacterMap = cmap;
    f.GlyphAdvances = adv;
    f.Count = ng;
    return f;
}

// unloadfont frees font path data
void unloadfont(VGPath * glyphs, int n) {
    int i;
    for (i = 0; i < n; i++) {
        vgDestroyPath(glyphs[i]);
    }
}

Fontinfo SansTypeface;
VGPath **paths;
VGPaint **paints;
int pathIndex = 0;
int paintIndex = 0;

// newpath creates path data
VGPath newpath() {
    return vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);
}

VGPaint getpaint(int* created) {
    if (paintIndex < MAX_PAINT_CACHE_SIZE) {
        *created = 0;
        VGPaint *paint = paints[paintIndex];
        if (paint == 0) {
          paint = new VGPaint;
          *paint = vgCreatePaint();
          paints[paintIndex] = paint;
        }
        paintIndex++;
        return *paint;
    } else {
        *created = 1;
        return vgCreatePaint();
    }
}


// init sets the system to its initial state
void init(int *w, int *h) {
    bcm_host_init();
    memset(state, 0, sizeof(*state));
    oglinit(state);
    SansTypeface = loadfont(DejaVuSans_glyphPoints,
                DejaVuSans_glyphPointIndices,
                DejaVuSans_glyphInstructions,
                DejaVuSans_glyphInstructionIndices,
                DejaVuSans_glyphInstructionCounts,
                DejaVuSans_glyphAdvances, DejaVuSans_characterMap, DejaVuSans_glyphCount);

    *w = state->screen_width;
    *h = state->screen_height;
    paths = new VGPath*[MAX_PATH_CACHE_SIZE];
    for (int i = 0; i < MAX_PATH_CACHE_SIZE; i++) {
        paths[i] = 0;
    }
    paints = new VGPaint*[MAX_PAINT_CACHE_SIZE];
    for (int i = 0; i < MAX_PAINT_CACHE_SIZE; i++) {
        paints[i] = 0;
    }
}

// finish cleans up
void finish() {
    unloadfont(SansTypeface.Glyphs, SansTypeface.Count);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(state->display, state->surface);
    eglMakeCurrent(state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(state->display, state->surface);
    eglDestroyContext(state->display, state->context);
    eglTerminate(state->display);
    for (int x = 0; x < MAX_PATH_CACHE_SIZE; x++) {
      VGPath* path = paths[x];
      if (path != 0) {
        vgDestroyPath(*path);
        delete path;
      }
    }
    delete paths;

    for (int x = 0; x < MAX_PAINT_CACHE_SIZE; x++) {
      VGPaint* paint = paints[x];
      if (paint != 0) {
        vgDestroyPaint(*paint);
        delete paint;
      }
    }
    delete paints;
}

//
// Style functions
//

// setfill sets the fill color
void setfill(VGfloat color[4]) {
    int created;
    VGPaint fillPaint = getpaint(&created);
    vgSetParameteri(fillPaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv(fillPaint, VG_PAINT_COLOR, 4, color);
    vgSetPaint(fillPaint, VG_FILL_PATH);
    if (created) {
        vgDestroyPaint(fillPaint);
    }
}

// setstroke sets the stroke color
void setstroke(VGfloat color[4]) {
    int created;
    VGPaint strokePaint = getpaint(&created);
    vgSetParameteri(strokePaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetParameterfv(strokePaint, VG_PAINT_COLOR, 4, color);
    vgSetPaint(strokePaint, VG_STROKE_PATH);
    if (created) {
        vgDestroyPaint(strokePaint);
    }
}

// StrokeWidth sets the stroke width
void StrokeWidth(VGfloat width) {
    vgSetf(VG_STROKE_LINE_WIDTH, width);
    vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
    vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
}

//
// Color functions
//
//

// RGBA fills a color vectors from a RGBA quad.
void RGBA(unsigned int r, unsigned int g, unsigned int b, VGfloat a, VGfloat color[4]) {
    if (r > 255) {
        r = 0;
    }
    if (g > 255) {
        g = 0;
    }
    if (b > 255) {
        b = 0;
    }
    if (a < 0.0 || a > 1.0) {
        a = 1.0;
    }
    color[0] = (VGfloat) r / 255.0f;
    color[1] = (VGfloat) g / 255.0f;
    color[2] = (VGfloat) b / 255.0f;
    color[3] = a;
}

// RGB returns a solid color from a RGB triple
void RGB(unsigned int r, unsigned int g, unsigned int b, VGfloat color[4]) {
    RGBA(r, g, b, 1.0f, color);
}

// Stroke sets the stroke color, defined as a RGB triple.
void Stroke(unsigned int r, unsigned int g, unsigned int b, VGfloat a) {
    VGfloat color[4];
    RGBA(r, g, b, a, color);
    setstroke(color);
}

// Fill sets the fillcolor, defined as a RGBA quad.
void Fill(unsigned int r, unsigned int g, unsigned int b, VGfloat a) {
    VGfloat color[4];
    RGBA(r, g, b, a, color);
    setfill(color);
}

// Text renders a string of text at a specified location, size, using the specified font glyphs
// derived from http://web.archive.org/web/20070808195131/http://developer.hybrid.fi/font2openvg/renderFont.cpp.txt
void Text(VGfloat x, VGfloat y, char *s, Fontinfo f, int pointsize) {
    VGfloat size = (VGfloat) pointsize, xx = x, mm[9];
    int i;

    vgGetMatrix(mm);
    for (i = 0; i < (int)strlen(s); i++) {
        unsigned int character = (unsigned int)s[i];
        int glyph = f.CharacterMap[character];
        if (glyph == -1) {
            continue;    //glyph is undefined
        }
        VGfloat mat[9] = {
            size, 0.0f, 0.0f,
            0.0f, size, 0.0f,
            xx, y, 1.0f
        };
        vgLoadMatrix(mm);
        vgMultMatrix(mat);
        vgDrawPath(f.Glyphs[glyph], VG_FILL_PATH | VG_STROKE_PATH);
        xx += size * f.GlyphAdvances[glyph] / 65536.0f;
    }
    vgLoadMatrix(mm);
}

// textwidth returns the width of a text string at the specified font and size.
VGfloat textwidth(char *s, Fontinfo f, VGfloat size) {
    int i;
    VGfloat tw = 0.0;
    for (i = 0; i < (int)strlen(s); i++) {
        unsigned int character = (unsigned int)s[i];
        int glyph = f.CharacterMap[character];
        if (glyph == -1) {
            continue;    //glyph is undefined
        }
        tw += size * f.GlyphAdvances[glyph] / 65536.0f;
    }
    return tw;
}

void SansText(VGfloat x, VGfloat y, char *s, int pointsize) {
    Text(x, y, s, SansTypeface, pointsize);
}

// TextMid draws text, centered on (x,y)
void TextMid(VGfloat x, VGfloat y, char *s, Fontinfo f, int pointsize) {
    VGfloat tw = textwidth(s, f, pointsize);
    Text(x - (tw / 2.0), y, s, f, pointsize);
}

void SansTextMid(VGfloat x, VGfloat y, char *s, int pointsize) {
    TextMid(x, y, s, SansTypeface, pointsize);
}

// TextEnd draws text, with its end aligned to (x,y)
void TextEnd(VGfloat x, VGfloat y, char *s, Fontinfo f, int pointsize) {
    VGfloat tw = textwidth(s, f, pointsize);
    Text(x - tw, y, s, f, pointsize);
}

// Start begins the picture, clearing a rectangular region with a specified color
void Start(int width, int height) {
    VGfloat color[4] = { 255, 255, 255, 1 };
    vgSetfv(VG_CLEAR_COLOR, 4, color);
    vgClear(0, 0, width, height);
    color[0] = 0, color[1] = 0, color[2] = 0;
    setfill(color);
    setstroke(color);
    StrokeWidth(0);
    vgLoadIdentity();
}

// End checks for errors, and renders to the display
void End() {
    assert(vgGetError() == VG_NO_ERROR);
    eglSwapBuffers(state->display, state->surface);
    assert(eglGetError() == EGL_SUCCESS);
    pathIndex = 0;
    paintIndex = 0;
}
