/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the AUTHORS file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#include "screen.h"

#include "graphicsn.h"
#include "tools.h"

#include "linebreak/linebreak.h"

#include <SDL/SDL_ttf.h>

#include "fribidi.h"

#include <iostream>
#include <vector>
#include <memory>

#include "libintl.h"

#include <assert.h>

screen_c::screen_c(const graphicsN_c & g) :
  animationState(0), blockX(g.blockX()), blockY(g.blockY())
{
  video = SDL_SetVideoMode(g.resolutionX(), g.resolutionY(), 24, 0);
  SDL_WM_SetCaption("Pushover", "Pushover");
}

surface_c::surface_c(uint16_t x, uint16_t y, bool alpha)
{
  video = SDL_CreateRGBSurface(0, x, y, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
  if (alpha)
    SDL_SetAlpha(video, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
  else
    SDL_SetAlpha(video, 0, 0);
}

screen_c::~screen_c(void) { }

surface_c::~surface_c(void)
{
  if (video) SDL_FreeSurface(video);
}

void screen_c::toggleFullscreen(void)
{
  SDL_WM_ToggleFullScreen(video);
}

void bitfield_c::clearDirty(void)
{
  for (unsigned int y = 0; y < 25; y++)
    dynamicDirty[y] = 0;
}

void bitfield_c::markAllDirty(void)
{
  for (unsigned int y = 0; y < 25; y++)
    dynamicDirty[y] = 0xFFFFFFFF;
}

void surface_c::blit(const surface_c & s, int x, int y, int clip) {

  if (s.video && video)
  {
    SDL_Rect dst, src;

    dst.x = x;
    dst.y = y - s.video->h;
    dst.w = s.video->w;
    dst.h = s.video->h;

    src.x = 0;
    src.y = 0;
    src.w = s.video->w;
    src.h = s.video->h;

    if (dst.y < clip)
    {
      int cl = clip-dst.y;

      src.y += cl;
      src.h -= cl;

      dst.y += cl;
      dst.h -= cl;
    }

    SDL_BlitSurface(s.video, &src, video, &dst);
  }
}

void surface_c::blitBlock(const surface_c & s, int x, int y) {

  if (s.video && video)
  {
    SDL_Rect dst;

    dst.x = x;
    dst.y = y;
    dst.w = s.video->w;
    dst.h = s.video->h;

    SDL_BlitSurface(s.video, 0, video, &dst);
  }
}

void surface_c::fillRect(int x, int y, int w, int h, int r, int g, int b, int a) {
  if (video)
  {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_FillRect(video, &dst, SDL_MapRGBA(video->format, r, g, b, a));
  }
}

void surface_c::copy(const surface_c & s, int x, int y, int w, int h, int dx, int dy) {

  if (s.video && video)
  {
    SDL_Rect src, dst;
    src.x = x;
    src.y = y;
    src.w = dst.w = w;
    src.h = dst.h = h;
    dst.x = dx;
    dst.y = dy;

    SDL_BlitSurface(s.video, &src, video, &dst);
  }
}

void surface_c::gradient(int x, int y, int w, int h) {

  // make sure we only work with valid pixels
  if (y+h >= video->h) h = video->h-y;

  for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++) {

      Uint8 r, g, b;

      SDL_GetRGB(*((uint32_t*)(((uint8_t*)video->pixels) + (y+i) * video->pitch + video->format->BytesPerPixel*(x+j))), video->format, &r, &g, &b);

      double val = (2.0-((1.0*x+j)/video->w + (1.0*y+i)/video->h));
      val += (1.0*rand()/RAND_MAX)/20 - 1.0/40;
      if (val < 0) val = 0;
      if (val > 2) val = 2;

      r = (Uint8)(((255.0-r)*val+r)*r/255);
      g = (Uint8)(((255.0-g)*val+g)*g/255);
      b = (Uint8)(((255.0-b)*val+b)*b/255);

      *((uint32_t*)(((uint8_t*)video->pixels) + (y+i) * video->pitch + video->format->BytesPerPixel*(x+j))) = SDL_MapRGB(video->format, r, g, b);
    }
}

void screen_c::flipComplete(void)
{
  SDL_Flip(video);
  animationState = 0;
}

void screen_c::flipDirty(const bitfield_c & dirty)
{
  SDL_Rect rects[10*25];
  int numrects = 0;

  for (int y = 0; y < 25; y++)
  {
    int rowStart = -1;

    for (int x = 0; x < 21; x++)
    {
      if (dirty.isDirty(x, y) && (x < 20))
      {
        if (rowStart == -1)
          rowStart = x;
      }
      else if (rowStart != -1)
      {
        rects[numrects].y = blockY*y/2;
        rects[numrects].x = blockX*rowStart;

        rects[numrects].w = blockX*(x-rowStart);
        rects[numrects].h = blockY/2;
        numrects++;
        rowStart = -1;
      }
    }
  }
  SDL_UpdateRects(video, numrects, rects);
  animationState = 0;
}

static int clip(int v) {
  if (v < 0) return 0;
  if (v > 256) return 256;
  return v;
}

// a list of functions that return value between 0 and 255, depending on x and y
static int f1(int x, int y, int a) { return clip((14-y)*32 - 14*32 + (a*(14*32+256)/64)); }
static int f2(int x, int y, int a) { return clip(x*32 - 19*32 + (a*(19*32+256)/64)); }
static int f3(int x, int y, int a) { return clip(((2*y-14)*(2*y-14)+(2*x-19)*(2*x-19))*256/127 - 1123 + a*((1123+256)/64)); }
static int f4(int x, int y, int a) { return clip((y)*32 - 14*32 + (a*(14*32+256)/64)); }
static int f5(int x, int y, int a) { return clip((19-x)*32 - 19*32 + (a*(19*32+256)/64)); }

static SDL_Rect rects[15*20*255];
static int count;

static void u1(int x, int y, int f0, int f, int blx, int bly) {
  int by = bly*y;
  int bx = blx*x;
  int bw = f*blx/256;
  int bh = bly;

  if (bw > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void u2(int x, int y, int f0, int f, int blx, int bly) {
  int by = bly*y;
  int bx = blx*x;
  int bw = blx;
  int bh = f*bly/256;

  if (bh > 0) {
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void u3(int x, int y, int f0, int f, int blx, int bly) {
  int by = bly*y + bly/2 - bly/2*f/256;
  int bx = blx*x + blx/2 - blx/2*f/256;
  int bw = f*blx/256;
  int bh = f*bly/256;

  if (bh > 0 && bw > 0) {
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void u4(int x, int y, int f0, int f, int blx, int bly) {

  uint8_t rnd = (x+20*y) % 64;

  f0 /= 4;
  f /= 4;

  if (f0 == f) return;

  for (int i = 0; i <= f; i++)
  {
    if (i >= f0)
    {
      int xp = rnd % 8;
      int yp = rnd / 8;

      int by = bly*y+yp*bly/8;
      int bx = blx*x+xp*blx/8;
      int bw = blx/8;
      int bh = bly/8;

      rects[count].x=bx;
      rects[count].y=by;
      rects[count].w=bw;
      rects[count].h=bh;
      count++;
    }

    rnd = (21*rnd+11) % 64;
  }
}

static void u_matrix(int x, int y, int f0, int f, int blx, int bly, uint8_t matrix[64])
{
  f0 /= 4;
  f /= 4;

  for (int i = 0; i < 64; i++)
  {
    if (f0 <= matrix[i] && matrix[i] < f)
    {
      int xp = i % 8;
      int yp = i / 8;

      int by = bly*y+yp*bly/8;
      int bx = blx*x+xp*blx/8;
      int bw = blx/8;
      int bh = bly/8;

      rects[count].x=bx;
      rects[count].y=by;
      rects[count].w=bw;
      rects[count].h=bh;
      count++;
    }
  }
}

static uint8_t matrixes[8][64] =
{
  { 6, 7, 8, 9,10,11,37,38, 5,20,21,22,23,24,12,39, 4,19,34,35,36,25,13,40, 3,18,17,16,15,14,26,41,    // P
    2,31,30,29,28,27,63,42, 1,32,54,55,58,59,62,43, 0,33,53,56,57,60,61,44,52,51,50,49,48,47,46,45 },
  { 0,31,49,60,59,17,16,32, 1,30,50,61,58,18,15,33, 2,29,51,62,57,19,14,34, 3,28,52,63,56,20,13,35,    // U
    4,27,53,54,55,21,12,36, 5,26,25,24,23,22,11,37,47, 6, 7, 8, 9,10,48,38,46,45,44,43,42,41,40,39 },
  {55,13,14,15,16,17,48,47,12,23,22,21,20,19,18,46,24,11,54,53,52,51,50,45,56,25,10, 9, 8,26,49,44,    // S
   57,58,59,50,61, 7,27,43, 0,32,31,30,29,28, 6,42,63, 1, 2, 3, 4, 5,62,41,33,34,35,36,37,38,39,40 },
  { 0,30,53,63,49,24,12,48, 1,29,52,51,50,23,13,47, 2,28,27,26,25,22,14,46, 3, 7, 8, 9,10,11,15,45,    // H
    4,31,56,57,58,21,16,44, 5,32,55,62,59,20,17,43, 6,33,54,61,60,19,18,42,34,35,36,37,38,39,40,41 },
  {51, 0, 1, 2, 3, 4,61,50,19,20,21,22,23,24, 5,49,18,35,52,53,54,25, 6,48,17,34,59,60,55,26, 7,47,    // O
   16,33,58,57,56,27, 8,46,15,32,31,30,29,28, 9,45,63,14,13,12,11,10,62,44,36,37,38,39,40,41,42,43 },
  { 0,23,39,57,47,13,12,38, 1,22,40,56,46,14,11,37, 2,21,41,55,45,15,10,36, 3,20,42,54,44,16, 9,35,    // V
   48, 4,19,43,17, 8,53,34,58,49, 5,18, 7,52,62,33,60,59,50, 6,51,61,63,32,24,25,26,27,28,29,30,31 },
  { 6, 5, 4, 3, 2, 1, 0,51, 7,28,27,26,25,24,23,50, 8,28,51,52,53,55,56,49, 9,19,20,21,22,58,57,48,    // E
   10,30,63,62,61,60,59,47,11,31,32,33,34,35,36,46,12,13,14,15,16,17,18,45,37,38,39,40,41,42,43,44 },
  { 6, 7, 8, 9,10,11,53,52, 5,32,33,34,35,36,12,51, 4,31,57,56,55,37,13,50, 3,18,17,16,15,14,54,49,    // R
    2,28,27,26,25,24,19,48, 1,29,58,61,62,23,20,47, 0,30,59,60,63,22,21,46,38,39,40,41,42,43,44,45 },
};


static void u5(int x, int y, int f0, int f, int blx, int bly)
{
  u_matrix(x, y, f0, f, blx, bly, matrixes[(x+y)%8]);
}

static void u6(int x, int y, int f0, int f, int blx, int bly)
{
  int bx = blx*x;
  int by = bly*y;
  int bw = f*blx/512;
  int bh = bly/2;

  if (bw > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }

  bx = blx*x;
  bh = f*bly/512;
  by = bly*y+bly-bh;
  bw = blx/2;

  if (bh > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }

  bw = f*blx/512;
  bx = blx*x+blx-bw;
  bh = bly/2;
  by = bly*y+bly/2;

  if (bw > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }

  bw = blx/2;
  bx = blx*x+blx/2;
  bh = f*bly/512;
  by = bly*y;

  if (bh > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void u7(int x, int y, int f0, int f, int blx, int bly)
{
  int w = f*blx/512;

  if (w > 0)
  {
    rects[count].x=x*blx;
    rects[count].y=y*bly;
    rects[count].w=blx;
    rects[count].h=w;
    count++;

    rects[count].x=x*blx;
    rects[count].y=y*bly+bly-w;
    rects[count].w=blx;
    rects[count].h=w;
    count++;

    rects[count].x=x*blx;
    rects[count].y=y*bly;
    rects[count].w=w;
    rects[count].h=bly;
    count++;

    rects[count].x=x*blx+blx-w;
    rects[count].y=y*bly;
    rects[count].w=w;
    rects[count].h=bly;
    count++;
  }
}

static void u8(int x, int y, int f0, int f, int blx, int bly)
{
  if ((x+y) % 2)
  {
    return u3(x, y, f0, f, blx, bly);
  }
  else
  {
    return u7(x, y, f0, f, blx, bly);
  }
}

static void u9(int x, int y, int f0, int f, int blx, int bly)
{
  if ((x+y) % 2)
  {
    return u1(x, y, f0, f, blx, bly);
  }
  else
  {
    return u2(x, y, f0, f, blx, bly);
  }
}

static void uA(int x, int y, int f0, int f, int blx, int bly)
{
  int by = bly*y;
  int bw = f*blx/256;
  int bx = blx*x+blx-bw;
  int bh = bly;

  if (bw > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void uB(int x, int y, int f0, int f, int blx, int bly) {
  int bh = f*bly/256;
  int by = bly*y+bly-bh;
  int bx = blx*x;
  int bw = blx;

  if (bh > 0) {
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void uC(int x, int y, int f0, int f, int blx, int bly) {
  switch ((y % 2)*2 + (x%2))
  {
    case 0: return u1(x, y, f0, f, blx, bly);
    case 1: return u2(x, y, f0, f, blx, bly);
    case 2: return uB(x, y, f0, f, blx, bly);
    case 3: return uA(x, y, f0, f, blx, bly);
  }
}

static void uD(int x, int y, int f0, int f, int blx, int bly)
{
  int by = bly*y;
  int bw = f*blx/256;
  int bx = blx*x+blx/2-f*blx/512;
  int bh = bly;

  if (bw > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void uE(int x, int y, int f0, int f, int blx, int bly)
{
  int bx = blx*x;
  int bh = f*bly/256;
  int by = bly*y+bly/2-f*bly/512;
  int bw = blx;

  if (bw > 0){
    rects[count].x=bx;
    rects[count].y=by;
    rects[count].w=bw;
    rects[count].h=bh;
    count++;
  }
}

static void uF(int x, int y, int f0, int f, int blx, int bly)
{
  if ((x+y) % 2)
  {
    return uD(x, y, f0, f, blx, bly);
  }
  else
  {
    return uE(x, y, f0, f, blx, bly);
  }
}

bool screen_c::flipAnimate(void)
{
  assert(video->w == 800 && video->h == 600);

  static int (*f)(int, int, int);
  static void (*u)(int, int, int, int, int, int);

  if (animationState == 0) {
    switch (rand()%5) {
      case 0: f = f1; break;  // up down sweep
      case 1: f = f2; break;  // right to left sewep
      case 2: f = f3; break;  // circular outside in
      case 3: f = f4; break;  // down up
      case 4: f = f5; break;  // left right
      default: assert(0); break;
    }
    switch (rand()%15) {
      case  0: u = u1; break;  // left --> right update
      case  1: u = u2; break;  // up --> down update
      case  2: u = u3; break;  // inside out update
      case  3: u = u4; break;  // diffuse update
      case  4: u = u5; break;  // PUSHOVER
      case  5: u = u6; break;  // windmill
      case  6: u = u7; break;  // outside in
      case  7: u = u8; break;  // checkerboard outside in inside out
      case  8: u = u9; break;  // snakes (checkerboard up down, left right)
      case  9: u = uA; break;  // right --> left
      case 10: u = uB; break;  // down --> up
      case 11: u = uC; break;  // big windmill
      case 12: u = uD; break;  // mittle out left right
      case 13: u = uE; break;  // mittle out up down
      case 14: u = uF; break;  // checkered uD und uE
      default: assert(0); break;
    }
  }

  animationState++;
  count=0;

  for (int y = 0; y < 15; y++)
  {
    for (int x = 0; x < 20; x++)
    {
      int valNew = f(x, y, animationState);
      int valOld = f(x, y, animationState-1);

      if (valNew != valOld)
      {
        u(x, y, valOld, valNew, 40, 40);
      }
    }
  }

  SDL_UpdateRects(video, count, rects);

  if (animationState == 64)
  {
    animationState = 0;
    return true;
  }

  return false;
}

SDL_Surface * surface_c::getIdentical(void) const {

  if (!video)
    return 0;
  else
    return SDL_CreateRGBSurface(0, video->w, video->h, 32, video->format->Rmask, video->format->Gmask, video->format->Bmask, 0);
}


static std::vector<std::vector<TTF_Font *> > fonts;

// list of paths to look for the fonts

const char * fontpaths [] = {
  "/usr/share/fonts/",
  "/usr/share/fonts/truetype/freefont/",
  "" // empty string as end of list
};


static std::string findFont(const std::string & font)
{
  size_t i = 0;

  while (strcmp(fontpaths[i], "") != 0)
  {
    std::string f = findFileInDirectory(fontpaths[i], font);

    if (f != "") return f;

    i++;
  }

  return "";
}


void initText(std::string dir)
{
  //TRANSLATORS: this is a list of fonts that you might need for your translation, please
  //specify a semicolon separated list of fonts to use, FreeSans.ttf should always be
  //the first, making English text legible, but if that font doesn't contains characters
  //you need, you can add them. ATTENTION: do not enter spaces, example for japanese
  //  "FreeSans.ttf;DroidSansJapanese.ttf"
  std::vector<std::string> fontList = splitString(_("FreeSans.ttf"), ";");

  if (TTF_Init() == -1) {
    std::cout << "Can't initialize font engine" << std::endl;
    exit(1);
  }

  fonts.resize(3);

  for (size_t i = 0; i < fontList.size(); i++)
  {
    std::string fname = findFont(fontList[i]);

    if (fname == "")
    {
      std::cout << "Could not find required fontfile for font  " << fontList[i] << std::endl;
    }
    else
    {
      std::cout << "found font " << fname << std::endl;

      TTF_Font * ft;

      ft = TTF_OpenFont(fname.c_str(), 18);
      if (!ft) {
        std::cout << "Can't open Font file: " << fname << std::endl;
        exit(1);
      }

      fonts[0].push_back(ft);

      ft = TTF_OpenFont(fname.c_str(), 30);
      if (!ft) {
        std::cout << "Can't open Font file: " << fname << std::endl;
        exit(1);
      }

      fonts[1].push_back(ft);

      ft = TTF_OpenFont(fname.c_str(), 35);
      if (!ft) {
        std::cout << "Can't open Font file: " << fname << std::endl;
        exit(1);
      }

      fonts[2].push_back(ft);
    }
  }
}

void deinitText(void) {

  for (unsigned int i = 0; i < fonts.size(); i++)
    for (size_t j = 0; j < fonts[i].size(); j++)
      TTF_CloseFont(fonts[i][j]);

  TTF_Quit();
}

bool rightToLeft(void)
{
  //TRANSLATORS: don't translate this string, rather enter the main text direction this will influence how dialogs are
  //layouted, possible values "left to right" "right to left" anything that is not "right to left" will be treated as
  //left to right
  return strcmp(_("left to right"), "right to left") == 0;
}

static unsigned int findFontForChar(int font, Uint16 ch)
{
  unsigned int f = 0;
  while (f < fonts[font].size())
  {
    if (TTF_GlyphIsProvided(fonts[font][f], ch))
    {
      return f;
    }

    f++;
  }

  return 0;
}

static uint16_t calcWidth(Uint16 * string, int font)
{
  if (string[0] == 0) return 0;

  int width = 0;

  uint16_t start = 0;
  uint16_t end = 1;
  int ff = findFontForChar(font, string[start]);

  while (string[end] != 0)
  {
    int nf = findFontForChar(font, string[end]);

    if (nf != ff)
    {
      int www;
      Uint16 charSave = string[end];
      string[end] = 0;

      TTF_SizeUNICODE(fonts[font][ff], string+start, &www, 0);
      width += www;

      string[end] = charSave;
      start = end;
      ff = nf;
    }
    end++;
  }

  int www;

  TTF_SizeUNICODE(fonts[font][ff], string+start, &www, 0);
  width += www;

  return width;
}

static SDL_Surface * outString(Uint16 * string, int font, SDL_Color color)
{
  if (string[0] == 0) return 0;

  std::vector <SDL_Surface * > texts;

  uint16_t start = 0;
  uint16_t end = 1;
  int ff = findFontForChar(font, string[start]);

  while (string[end] != 0)
  {
    int nf = findFontForChar(font, string[end]);

    if (nf != ff)
    {
      Uint16 charSave = string[end];
      string[end] = 0;

      texts.push_back(TTF_RenderUNICODE_Blended(fonts[font][ff], string+start, color));

      string[end] = charSave;
      start = end;
      ff = nf;
    }
    end++;
  }

  texts.push_back(TTF_RenderUNICODE_Blended(fonts[font][ff], string+start, color));

  // shortcut, when only one surface was creates no assembly required
  if (texts.size() == 1)
  {
    return texts[0];
  }

  // now assemble all the texts into one large surface
  //
  // first calculate the required size for that surface

  int w = 0;
  int h = 0;

  for (size_t i = 0; i < texts.size(); i++)
  {
    w += texts[i]->w;
    if (texts[i]->h > h) h = texts[i]->h;
  }

  // create a target surface

  SDL_Surface * res = SDL_CreateRGBSurface(0, w, h, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
  SDL_SetAlpha(res, SDL_SRCALPHA, 0);
  SDL_FillRect(res, 0, SDL_MapRGBA(res->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));

  int xpos = 0;

  for (size_t i = 0; i < texts.size(); i++)
  {
    printf("alpha %x\n", texts[i]->format->Amask);
    SDL_Rect r;
    r.x = xpos;
    r.y = h-texts[i]->h;
    r.w = texts[i]->w;
    r.h = texts[i]->h;
    SDL_SetAlpha(texts[i], 0, 0);
    SDL_BlitSurface(texts[i], 0, res, &r);

    xpos += texts[i]->w;

    SDL_FreeSurface(texts[i]);
  }

  return res;
}


unsigned int renderTextIntern(const fontParams_s * par, const std::string & t, SDL_Surface * video)
{
  // make some safety checks, empty strings are not output
  bool onlySpace = true;
  for (size_t i = 0; i < t.length(); i++)
    if (t[i] != ' ')
    {
      onlySpace = false;
      break;
    }

  if (   (t.length() == 0)
      || (onlySpace)
     )
  {
    return 1;
  }

  // STEP 1 split into paragraphs, paragraphs
  // are separated by \n or \r
  std::vector<std::string> paragraphs = splitString(t.c_str(), "\n\r");;

  int ypos = par->box.y;
  unsigned int height = 0;

  if (par->alignment == ALN_CENTER && video)
    ypos += (par->box.h-getTextHeight(par, t))/2;

  // iterate over all paragraphs
  for (size_t para = 0; para < paragraphs.size(); para++)
  {
    //convert paragraph to utf-16 ... because all other routines can handle only that

    FriBidiCharType base = FRIBIDI_TYPE_ON;
    FriBidiChar * paraString = new FriBidiChar[paragraphs[para].length() + 1];

    size_t ucsLength = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8,
            const_cast<char*>(paragraphs[para].c_str()),
            paragraphs[para].length(), paraString);

    FriBidiCharType bidiTypes[ucsLength];
    FriBidiLevel bidiLevels[ucsLength];

    // analyse direction
    fribidi_get_bidi_types(paraString, ucsLength, bidiTypes);
    fribidi_get_par_embedding_levels(bidiTypes, ucsLength, &base, bidiLevels);

    // find line break points (using unicode algorithm and more)
    char breaks[ucsLength];
    set_linebreaks_utf32((utf32_t*)(paraString), ucsLength, "en", breaks);

    size_t lineStart = 0;

    while (lineStart < ucsLength)
    {
      size_t lineEnd = lineStart+1;

      while (breaks[lineEnd] != LINEBREAK_MUSTBREAK && breaks[lineEnd] != LINEBREAK_ALLOWBREAK && lineEnd < ucsLength)
        lineEnd++;

      while (lineEnd+1 < ucsLength)
      {
        size_t nextLineEnd = lineEnd+1;

        while (breaks[nextLineEnd] != LINEBREAK_MUSTBREAK && breaks[nextLineEnd] != LINEBREAK_ALLOWBREAK && nextLineEnd < ucsLength)
          nextLineEnd++;

        // we should also apply the bidi algorithm here and do shaping as well to
        // properly calculate the width of the line, but for sake of simplicity
        // we leave that out for the moment

        Uint16 outttf[ucsLength+1];

        for (size_t i = 0; i <= nextLineEnd-lineStart; i++)
        {
          outttf[i] = paraString[i+lineStart];
        }

        outttf[nextLineEnd-lineStart+1] = 0;

        int w = calcWidth(outttf, par->font);

        if (w > par->box.w)
        {
          break;
        }

        lineEnd = nextLineEnd;

      }

      // TODO here we should do actual shaping...
      fribidi_reorder_line(FRIBIDI_FLAGS_DEFAULT, // flags...
          bidiTypes,
          lineEnd-lineStart+1,
          lineStart,
          base,
          bidiLevels,
          paraString,
          NULL
          );

      Uint16 outttf[ucsLength+1];

      size_t j = 0;
      for (size_t i = 0; i <= lineEnd-lineStart; i++)
      {
        if (paraString[i+lineStart] < 0x202A || paraString[i+lineStart] > 0x202E)  // exclude bidi characters
        {
          outttf[j] = paraString[i+lineStart];
          j++;
        }
      }

      outttf[j] = 0;

      SDL_Surface * vv = outString(outttf, par->font, par->color);
      SDL_Surface * vb = NULL;

      if (par->shadow)
      {
        SDL_Color bg;
        bg.r = bg.g = bg.b = 0;
        vb = outString(outttf, par->font, bg);
      }

      if (video)
      {
        if (par->alignment == ALN_TEXT) {

          SDL_Rect r = par->box;

          if (rightToLeft())
          {
            r.x = r.x+r.w-vv->w;
          }
          r.w = vv->w;
          r.h = vv->h;
          r.y = ypos;

          if (par->shadow == 1)
          {
            int sa = 1;
            if (par->font == FNT_BIG) sa = 2;

            r.x-=sa; r.y-=sa; SDL_BlitSurface(vb, 0, video, &r);
            r.x+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.y+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.y+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x-=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x-=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.y-=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x+=sa;
          }
          else if (par->shadow == 2)
          {
            int sa = 1;

            r.x+=sa; r.y+=sa; SDL_BlitSurface(vb, 0, video, &r);
            r.x-=sa; r.y-=sa;
          }
          SDL_BlitSurface(vv, 0, video, &r);
        }
        else if (par->alignment == ALN_TEXT_CENTER || par->alignment == ALN_CENTER) {

          SDL_Rect r;

          r.x = par->box.x + (par->box.w - vv->w)/2;
          r.y = ypos;

          r.w = vv->w;
          r.h = vv->h;

          if (par->shadow == 1)
          {
            int sa = 1;
            if (par->font == FNT_BIG) sa = 2;

            r.x-=sa; r.y-=sa; SDL_BlitSurface(vb, 0, video, &r);
            r.x+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.y+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.y+=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x-=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x-=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.y-=sa;          SDL_BlitSurface(vb, 0, video, &r);
            r.x+=sa;
          }
          else if (par->shadow == 2)
          {
            int sa = 1;

            r.x+=sa; r.y+=sa; SDL_BlitSurface(vb, 0, video, &r);
            r.x-=sa; r.y-=sa;
          }
          SDL_BlitSurface(vv, 0, video, &r);
        }
      }

      ypos += vv->h;
      height += vv->h;

      SDL_FreeSurface(vv);
      if (par->shadow) SDL_FreeSurface(vb);

      lineStart = lineEnd+1;

      // when the break reason was a normal space, skip those spaces
      while (paraString[lineStart] == ' ' && lineStart < ucsLength)
        lineStart++;
    }
  }

  return height;
}

unsigned int surface_c::renderText(const fontParams_s * par, const std::string & t)
{
  return renderTextIntern(par, t, video);
}

unsigned int getFontHeight(unsigned int font) {
  if (font < fonts.size())
    return TTF_FontLineSkip(fonts[font][0]);
  else
    return 0;
}

unsigned int getTextWidth(unsigned int font, const std::string & string)
{
  if (string[0] == 0) return 0;

  FriBidiChar * text = new FriBidiChar[string.length() + 1];

  int ucsLength = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8,
      const_cast<char*>(string.c_str()), string.length(), text);

  Uint16 outttf[ucsLength+1];

  for (int i = 0; i <= ucsLength; i++)
  {
    if (text[i] > 0xFFFF) printf("oops unsupported character\n");
    outttf[i] = text[i];
  }

  outttf[ucsLength] = 0;

  int width = calcWidth(outttf, font);

  delete [] text;

  return width;
}

unsigned int getTextHeight(const fontParams_s * par, const std::string & t) {

  return renderTextIntern(par, t, 0);
}


const std::string _(const std::string & x)
{
  return std::string(gettext(x.c_str()));
}
const char * _(const char * x)
{
  return gettext(x);
}

