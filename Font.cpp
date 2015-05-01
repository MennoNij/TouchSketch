#include "Font.h"

#include <windows.h>
#include <gl\gl.h>

// declare static members
FT_Library FTLibraryWrap::ftLibrary;
FTLibraryWrap Font::library;

Font::Font()
{
  fontSize    = 0;
  fontHeight  = 0;
  dListOffset = 0;
	r = g = b = 0.0f;
}

Font::Font(std::string flnm, int fntsz)
{
	fontSize = fntsz;
  compile(flnm, fntsz);
	r = g = b = 0.0f;
}

Font::~Font()
{
}

void Font::destroy()
{

}

// REGULAR METHODS

void Font::compile(std::string& flnm, int fntsz)
{
  // Open font
  FT_Face fontFace;

  //TODO: implement error handler
  if(FT_New_Face(library.getLibrary(), flnm.c_str(), 0, &fontFace)) //load font
    return;

  //stop if the font isn't scalable (as errors are likely to happen)
  if(!(fontFace->face_flags & FT_FACE_FLAG_SCALABLE) || !(fontFace->face_flags & FT_FACE_FLAG_HORIZONTAL))
    return;

  FT_Set_Pixel_Sizes(fontFace, fontSize, 0); //set height

  int imageWidth  = 256;
  int imageHeight = 256;
  int lineRest    = imageWidth - FONT_MARGIN;
  int numLines    = 1;
  int  maxDesc     = 0;
  int  maxAsc      = 0;
  int  charIndx;

  //calculate glyph dimensions, maximal font extent
  for(int i=0; i<FONT_CHARS; i++)
  {
    //get the char index, 32 is the map offset
    charIndx = FT_Get_Char_Index(fontFace, i+FONT_SPACE);

    // have freetype render the glyph with this index
    FT_Load_Glyph(fontFace, charIndx, FT_LOAD_RENDER);

    // get the horizontal extent of this glyph
    widths[i] = (fontFace->glyph->metrics.horiAdvance >> 6) + FONT_MARGIN;
    if (widths[i] > lineRest)
    { //start on a new line
      lineRest = imageWidth - FONT_MARGIN;
      numLines++;
    }
    lineRest -= widths[i];

		maxDesc = max(fontFace->glyph->bitmap.rows - fontFace->glyph->bitmap_top, maxDesc);
		maxAsc  = max(fontFace->glyph->bitmap_top, maxAsc);
  }

  //calculate maximal extent of the font
  fontHeight = maxAsc + maxDesc;
  int minHeight = (maxAsc + maxDesc + FONT_MARGIN) * numLines + FONT_MARGIN;

  //textures need to be powers of two in size
  while(imageHeight < minHeight)
    imageHeight <<= 1;

  //generate the texture itself
  unsigned char* fontRaster = new unsigned char[imageHeight * imageWidth];
  std::memset(fontRaster, 0, imageHeight * imageWidth);
  int u = FONT_MARGIN; int v = FONT_MARGIN + maxAsc;
  float x0, y0, x1, y1;

  dListOffset = glGenLists(FONT_CHARS); //OpenGL specific

  //'render' the glyphs into the texture
  for (int i=0; i<FONT_CHARS; i++)
  {
    //get the glyph
    uint charIndx = FT_Get_Char_Index(fontFace, i+FONT_SPACE);
    FT_Load_Glyph(fontFace, charIndx, FT_LOAD_DEFAULT);
    FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);

    if (widths[i] > imageWidth - u) //glyph exceeds current line
    {
      u = FONT_MARGIN;
      v += fontHeight + FONT_MARGIN;
    }
    //texture cordinates
    x0 = ((float)u) / imageWidth;
    x1 = ((float)(u + widths[i])) / imageWidth;
    y0 = ((float)(v - maxAsc)) / imageHeight;
    y1 = y0 + ((float)fontHeight) / imageHeight;

    // OpenGL specific: generate display list of this glyph
    glNewList(dListOffset + i, GL_COMPILE);
     glBegin(GL_QUADS);
      glTexCoord2f(x0,y0);  glVertex2i(0, fontHeight);
      glTexCoord2f(x1,y0);  glVertex2i(widths[i], fontHeight);
      glTexCoord2f(x1,y1);  glVertex2i(widths[i], 0);
      glTexCoord2f(x0,y1);  glVertex2i(0, 0);
     glEnd();
     glTranslatef(widths[i], 0, 0);  // translate forward
    glEndList();

    //now copy the rendered glyph into the bitmap
    for(int row=0; row<fontFace->glyph->bitmap.rows; row++)
    {
      for(int pixel=0; pixel<fontFace->glyph->bitmap.width; pixel++)
      {
        // set pixel at position to intensity (0-255) at the position
        fontRaster[(u + fontFace->glyph->bitmap_left + pixel) + (v - fontFace->glyph->bitmap_top + row) * imageWidth] =
        fontFace->glyph->bitmap.buffer[pixel + row * fontFace->glyph->bitmap.pitch];
      }
    }
    u += widths[i];
  }//end for

  //OpenGL specific: generate an OpenGL texture from the bitmap
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, imageWidth, imageHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, fontRaster);

  delete[] fontRaster;
  FT_Done_Face(fontFace);

}

void Font::renderText(std::string& text, float x, float y)
{
	glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture(GL_TEXTURE_2D, texID);

		glColor4f(r,g,a,1.0);

	/*glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3d(0,  256, 0);
    glTexCoord2f(1, 0); glVertex3d(256, 256, 0);
    glTexCoord2f(1, 1); glVertex3d(256, 0, 0);
    glTexCoord2f(0, 1); glVertex3d(0,  0, 0);
  glEnd();
	*/

		glTranslatef(x,y,0.0f);
		glListBase(dListOffset-32);

		glCallLists(text.length(), GL_UNSIGNED_BYTE, text.c_str());
	
		glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}


// GETTERS/SETTERS

int Font::getHeight()
{
	return fontSize;
}

void  Font::setColor(float rr, float gg, float bb)
{
	r = rr;
	g = gg;
	b = bb;
}

//FT_Library wrapper
FTLibraryWrap::FTLibraryWrap()
{
	FT_Init_FreeType(&ftLibrary);
}

FTLibraryWrap::~FTLibraryWrap()
{
	FT_Done_FreeType(ftLibrary);
}

FT_Library& FTLibraryWrap::getLibrary()
{
	return ftLibrary;
}
