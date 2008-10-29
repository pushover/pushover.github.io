#include "screen.h"

#include "graphics.h"

#include <SDL/SDL_ttf.h>

#include <iostream>
#include <vector>

screen_c::screen_c(const graphics_c & g) :
  gr(g), animationState(0), fullscreen(false)
{
  video = SDL_SetVideoMode(gr.resolutionX(), gr.resolutionY(), 24, 0);
}

screen_c::~screen_c(void) { }

surface_c::~surface_c(void)
{
  if (video) SDL_FreeSurface(video);

}

void screen_c::toggleFullscreen(void)
{
  fullscreen = !fullscreen;
//  video = SDL_SetVideoMode(gr.resolutionX(), gr.resolutionY(), 24, fullscreen?SDL_FULLSCREEN:0);
}

void surface_c::clearDirty(void)
{
  for (unsigned int y = 0; y < 13; y++)
    dynamicDirty[y] = 0;
}

void surface_c::markAllDirty(void)
{
  for (unsigned int y = 0; y < 13; y++)
    dynamicDirty[y] = 0xFFFFFFFF;
}

void surface_c::blit(SDL_Surface * s, int x, int y) {

  if (s && video)
  {
    SDL_Rect dst;

    dst.x = x;
    dst.y = y - s->h;
    dst.w = s->w;
    dst.h = s->h;

    SDL_BlitSurface(s, 0, video, &dst);
  }
}

void surface_c::fillRect(int x, int y, int w, int h, int r, int g, int b) {
  if (video)
  {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_FillRect(video, &dst, SDL_MapRGB(video->format, r, g, b));
  }
}

void surface_c::blitBlock(SDL_Surface * s, int x, int y) {

  if (s && video)
  {
    SDL_Rect dst;

    dst.x = x;
    dst.y = y;
    dst.w = s->w;
    dst.h = s->h;

    SDL_BlitSurface(s, 0, video, &dst);
  }
}

void surface_c::copy(surface_c & s, int x, int y, int w, int h) {

  if (s.video && video)
  {
    SDL_Rect src, dst;
    src.x = dst.x = x;
    src.y = dst.y = y;
    src.w = dst.w = w;
    src.h = dst.h = h;
    SDL_BlitSurface(s.video, &src, video, &dst);
  }
}

void surface_c::gradient(int x, int y, int w, int h) {

  // make sure we only work with valid pixels and
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
}

void screen_c::flipDirty(void)
{
  SDL_Rect rects[10*13];
  int numrects = 0;

  for (int y = 0; y < 13; y++)
  {
    int rowStart = -1;

    for (int x = 0; x < 21; x++)
    {
      if (isDirty(x, y) && (x < 20))
      {
        if (rowStart == -1)
          rowStart = x;
      }
      else if (rowStart != -1)
      {
        rects[numrects].y = gr.blockY()*y;
        rects[numrects].x = gr.blockX()*rowStart;

        if (y == 12)
          rects[numrects].h = gr.blockY()/2;
        else
          rects[numrects].h = gr.blockY();

        rects[numrects].w = gr.blockX()*(x-rowStart);
        numrects++;
        rowStart = -1;
      }
    }
  }
  SDL_UpdateRects(video, numrects, rects);
}

static int clip(int v) {
  if (v < 0) return 0;
  if (v > 256) return 256;
  return v;
}

// a list of functions that return value between 0 and 255, depending on x and y
static int f1(int x, int y, int a) { return clip(y*256/3 - 1024 + a*((1024+256)/32)); }
static int f2(int x, int y, int a) { return clip(y*x*256/3/20 - 1024 + a*((1024+256)/32)); }
static int f3(int x, int y, int a) { return clip(((2*y-12)*(2*y-12)+(2*x-19)*(2*x-19))*256/110 - 1024 + a*((1024+256)/32)); }

static void u1(SDL_Surface * video, int x, int y, int f, int blx, int bly) {
  int by = bly*y;
  int bx = blx*x;
  int bw = f*blx/256;
  int bh = (y == 12) ? bly/2 : bly;

  if (bw > 0)
    SDL_UpdateRect(video, bx, by, bw, bh);
}

static void u2(SDL_Surface * video, int x, int y, int f, int blx, int bly) {
  int by = bly*y;
  int bx = blx*x;
  int bw = blx;
  int bh = (y == 12) ? bly/2 : bly;

  bh = bh*f/256;

  if (bh > 0)
    SDL_UpdateRect(video, bx, by, bw, bh);
}

static void u3(SDL_Surface * video, int x, int y, int f, int blx, int bly) {
  int by = bly*y + bly/2 - bly/2*f/256;
  int bx = blx*x + blx/2 - blx/2*f/256;
  int bw = blx;
  int bh = (y == 12) ? bly/2 : bly;

  bh = bh*f/256;
  bw = bw*f/256;

  if (bh > 0)
    SDL_UpdateRect(video, bx, by, bw, bh);
}

bool screen_c::flipAnimate(void)
{
  animationState++;

  for (int y = 0; y < 13; y++)
  {
    for (int x = 0; x < 20; x++)
    {
      if (isDirty(x, y))
      {
        int valNew = f3(x, y, animationState);
        int valOld = f3(x, y, animationState-1);

        if (valNew != valOld)
        {
          u3(video, x, y, valNew, gr.blockX(), gr.blockY());
        }
      }
    }
  }

  if (animationState == 32)
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


pixelSurface_c::pixelSurface_c(const surface_c & pre) : surface_c(pre.getIdentical()) { }

static std::vector<TTF_Font * > fonts;


void initText(std::string dir) {

  std::string fname = dir+"/data/FreeSans.ttf";

  if (TTF_Init() == -1) {
    std::cout << "Can not initialize font engine\n";
    exit(1);
  }

  TTF_Font * ft;

  ft = TTF_OpenFont(fname.c_str(), 20);
  if (!ft) {
    std::cout << "Can not open Font file: " << fname << std::endl;
    exit(1);
  }

  fonts.push_back(ft);

  ft = TTF_OpenFont(fname.c_str(), 30);
  if (!ft) {
    std::cout << "Can not open Font file " << fname << std::endl;
    exit(1);
  }

  fonts.push_back(ft);

  ft = TTF_OpenFont(fname.c_str(), 35);
  if (!ft) {
    std::cout << "Can not open Font file " << fname << std::endl;
    exit(1);
  }

  fonts.push_back(ft);
}

void deinitText(void) {

  for (unsigned int i = 0; i < fonts.size(); i++)
    TTF_CloseFont(fonts[i]);

  TTF_Quit();
}

static std::vector<std::string> split(const std::string & text, char splitter)
{
  std::string current;
  std::vector<std::string> res;

  for (unsigned int i = 0; i < text.length(); i++)
  {
    if (text[i] == splitter)
    {
      res.push_back(current);
      current = "";
    }
    else
    {
      current += text[i];
    }
  }

  if (current.length()) res.push_back(current);

  return res;
}

unsigned int surface_c::renderText(const fontParams_s * par, const std::string & t) {

  std::vector<std::string> words = split(t, ' ');

  int ypos = par->box.y;

  if (par->alignment == ALN_CENTER)
  {
    unsigned int word = 0;
    int height = 0;

    while (word < words.size()) {

      std::string curLine = words[word];
      word++;

      while (word < words.size())
      {
        int w;
        TTF_SizeUTF8(fonts[par->font], (curLine+words[word]).c_str(), &w, 0);

        if (w > par->box.w) break;

        curLine = curLine + " " + words[word];
        word++;
      }

      int h, w;
      TTF_SizeUTF8(fonts[par->font], curLine.c_str(), &w, &h);

      height += h;
    }

    ypos += (par->box.h-height)/2;
  }

  unsigned int word = 0;
  unsigned int lines = 0;

  while (word < words.size()) {

    std::string curLine = words[word];
    word++;
    lines++;

    while (word < words.size())
    {
      int w;
      TTF_SizeUTF8(fonts[par->font], (curLine+words[word]).c_str(), &w, 0);

      if (w > par->box.w) break;

      curLine = curLine + " " + words[word];
      word++;
    }

    SDL_Surface * vv = TTF_RenderUTF8_Blended(fonts[par->font], curLine.c_str(), par->color);
    SDL_Surface * vb = NULL;

    if (par->shadow)
    {
      SDL_Color bg;
      bg.r = bg.g = bg.b = 0;
      vb = TTF_RenderUTF8_Blended(fonts[par->font], curLine.c_str(), bg);
    }

    if (par->alignment == ALN_TEXT) {

      SDL_Rect r = par->box;
      r.w = vv->w;
      r.h = vv->h;
      r.y = ypos;

      if (par->shadow)
      {
        int sa = 1;
        if (par->font == FNT_BIG) sa = 2;

        r.x-=sa; r.y-=sa; SDL_BlitSurface(vb, 0, video, &r);
        r.x+=2*sa;        SDL_BlitSurface(vb, 0, video, &r);
        r.y+=2*sa;        SDL_BlitSurface(vb, 0, video, &r);
        r.x-=2*sa;        SDL_BlitSurface(vb, 0, video, &r);
        r.y-=sa; r.x+=sa;
      }
      SDL_BlitSurface(vv, 0, video, &r);
    }
    else if (par->alignment == ALN_TEXT_CENTER || par->alignment == ALN_CENTER) {

      SDL_Rect r;

      r.x = par->box.x + (par->box.w - vv->w)/2;
      r.y = ypos;

      r.w = vv->w;
      r.h = vv->h;

      if (par->shadow)
      {
        int sa = 1;
        if (par->font == FNT_BIG) sa = 2;

        r.x-=sa; r.y-=sa; SDL_BlitSurface(vb, 0, video, &r);
        r.x+=2*sa;        SDL_BlitSurface(vb, 0, video, &r);
        r.y+=2*sa;        SDL_BlitSurface(vb, 0, video, &r);
        r.x-=2*sa;        SDL_BlitSurface(vb, 0, video, &r);
        r.y-=sa; r.x+=sa;
      }
      SDL_BlitSurface(vv, 0, video, &r);
    }

    ypos += vv->h;

    SDL_FreeSurface(vv);
    if (par->shadow) SDL_FreeSurface(vb);
  }

  return lines;
}

unsigned int getFontHeight(unsigned int font) {
  if (font < fonts.size())
    return TTF_FontLineSkip(fonts[font]);
  else
    return 0;
}


