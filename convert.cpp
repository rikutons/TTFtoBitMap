#include <ft2build.h>
#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <vector>
#include <algorithm>
#include "template.h"
#include FT_FREETYPE_H
using namespace std;

FT_Library lib;
FT_Error error;
FT_Face face;
FT_UInt glyphIndex;
std::string fontFilename = "input_font/xxxxxx.ttf";
std::string path = "input_char/ascii.txt";
const int fontSize = HEIGHT;

struct Font {
  Font(int unicode, int width, int bitmap[]) : unicode(unicode), width(width){
    for(int i = 0; i < HEIGHT; i++) {
      this->bitmap[i] = bitmap[i];
    }
  }
  int unicode;
  int width;
  int bitmap[HEIGHT];

  bool operator <(const Font &font) const{
    return unicode < font.unicode;
  };

  bool operator >(const Font &font) const{
    return unicode > font.unicode;
  };
};

bool init()
{
  // init freetype
  error = FT_Init_FreeType(&lib);
  if (error != FT_Err_Ok)
  {
    std::cout << "BitmapFontGenerator > ERROR: FT_Init_FreeType failed, error code: " << error << std::endl;
    return false;
  }

  // load font
  error = FT_New_Face(lib, fontFilename.c_str(), 0, &face);
  if (error == FT_Err_Unknown_File_Format)
  {
    std::cout << "BitmapFontGenerator > ERROR: failed to open file \"" << fontFilename << "\", unknown file format" << std::endl;
    return false;
  }
  else if (error)
  {
    std::cout << "BitmapFontGenerator > ERROR: failed to open file \"" << fontFilename << "\", error code: " << error << std::endl;
    return false;
  }

  // set font size
  error = FT_Set_Pixel_Sizes(face, 0, fontSize);
  if (error)
  {
    std::cout << "BitmapFontGenerator > failed to set font size, error code: " << error << std::endl;
  }

  return true;
}

int main()
{
  if (!init())
    return 1;
  ifstream input(path);

  int imageWidth = (fontSize + 2) * 16;
  int imageHeight = (fontSize + 2) * 8;

  // create an array to save the character widths
  // int *widths = new int[128];

  // draw all characters
  // for (int i = 0; i < 128; ++i)
  char32_t ch;
  int size = 0;
  vector<Font> bitmap_array;
  while (!input.eof())
  {
    char c = input.get();
    // cout << bitset<8>(c) << endl;
    if ((c & 0b11111000) == 0b11111000)
    {
      ch = (c & 0b11);
      size = 5;
      continue;
    }
    else if ((c & 0b11110000) == 0b11110000)
    {
      ch = (c & 0b111);
      size = 4;
      continue;
    }
    else if ((c & 0b11100000) == 0b11100000)
    {
      ch = (c & 0b1111);
      size = 3;
      continue;
    }
    else if ((c & 0b11000000) == 0b11000000)
    {
      ch = (c & 0b11111);
      size = 2;
      continue;
    }
    else if ((c & 0b10000000) == 0b10000000)
    {
      ch <<= 6;
      ch |= (c & 0b111111);
      size--;
      // cout << size << ", ";
      // cout << bitset<16>(ch) << endl;
      if (size != 1)
        continue;
    }
    else
      ch = c;
    // get the glyph index from character code
    glyphIndex = FT_Get_Char_Index(face, ch);

    // load the glyph image into the slot
    error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    if (error)
    {
      std::cout << "BitmapFontGenerator > failed to load glyph, error code: " << error << std::endl;
    }

    // convert to an anti-aliased bitmap
    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (error)
    {
      std::cout << "BitmapFontGenerator > failed to render glyph, error code: " << error << std::endl;
    }

    // find the tile position where we have to draw the character

    // draw the character
    const FT_Bitmap &bitmap = face->glyph->bitmap;
    cout << ch << " " << bitmap.rows << " " << bitmap.width << endl;
    int* out = new int[HEIGHT];
    for (int i = 0; i < HEIGHT; i++)
      out[i] = 0;
    int rows = bitmap.rows;
    if(bitmap.rows > HEIGHT)
      rows = HEIGHT;
    for (int yy = 0; yy < rows; ++yy)
    {
      for (int xx = 0; xx < bitmap.width; ++xx)
      {
        unsigned char r = bitmap.buffer[(yy * (bitmap.width) + xx)];
        if (r > 120)
          out[yy + (HEIGHT - rows)] |= 1 << (bitmap.width - xx - 1);
        // std::cout << bitset<8>((int)r);
        std::cout << ((int)r > 120 ? "\e[47m  \e[0m" : "  ");
      }
      cout << out[yy + (HEIGHT - rows)] << endl;
      }
    bitmap_array.emplace_back(Font{ch, bitmap.width, out});
    // delete[] out;
    ch = 0;
  }

  ofstream file("./output.h");
  file << "// Used font: " << fontFilename << endl;
  file << "unsigned short bitmaps[] = {";
  sort(bitmap_array.begin(), bitmap_array.end());
  int i = 0;
  vector<int> ptr_array;
  for (Font b : bitmap_array)
  {
    ptr_array.emplace_back(i);
    unsigned int p = b.width << 12; // widthは上位4bit固定
    char c = 15 - 4; // 現在何bit目にいるかのカウンタ 最上位ビットのとき15
    for (int i = 0; i < HEIGHT; i++)
    {
      for (int j = b.width - 1; j >= 0; j--)
      {
        p |= (!!(1 << j & b.bitmap[i])) << c;
        if(c > 0)
          c--;
        else {
          file << p << ",";
          // file << bitset<16>(p) << ",";
          p = 0;
          c = 15;
        }
      }
    }
    if(c != 15)
      file << p << ",";
      // file << bitset<16>(p) << ",";
    i += (b.width * HEIGHT + 4 + 15) / 16;
  }
  file << "};" << endl;
  file << "unsigned short unicodes[] = {";
  for (Font b : bitmap_array)
  {
    file << b.unicode;
    file << ",";
  }
  file << "};" << endl;
  file << "unsigned short pointers[] = {";
  for (int p : ptr_array)
  {
    file << p;
    file << ",";
  }
  file << "};" << endl;

  file.close();
}