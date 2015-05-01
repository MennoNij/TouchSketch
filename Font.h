#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#define uint unsigned int

#define FONT_CHARS  96
#define FONT_SPACE  32
#define FONT_MARGIN 1

// FTLibraryWrap: wrapper for static freetype library instance
class FTLibraryWrap
{
public:
    FTLibraryWrap();
    ~FTLibraryWrap();
    FT_Library& getLibrary();
private:
    static FT_Library ftLibrary;
};

// OpenGL font generated with freetype2
class Font
{
public:
  Font();
  Font(std::string flnm, int fntsz);
  ~Font();
  void destroy();

  // REGULAR METHODS

  // Compiles the font into a texture for OpenGL
  void compile(std::string& filename, int fontsize);

  // Renders a string
  void renderText(std::string& text, float x, float y);

  // GETTERS/SETTERS

  // Get the font height/size
  int getHeight();

  // Set the font's display color
  void setColor(float rr, float gg, float bb);

  int fontSize, fontHeight;
  int widths[FONT_CHARS];
  uint dListOffset;
  uint texID;
  static FTLibraryWrap library;
  float r, g, b, a;
};