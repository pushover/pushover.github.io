#include "screen.h"

#include "graphics.h"

#include <iostream>

screen_c::screen_c(const graphics_c & g) : gr(g), animationState(0), fullscreen(false)
{
}

screen_c::~screen_c(void)
{
}

void screen_c::init(void)
{
  SDL_Init(SDL_INIT_VIDEO);

  video = SDL_SetVideoMode(gr.resolutionX(), gr.resolutionY(), 24, fullscreen?SDL_FULLSCREEN:0);
}

void screen_c::toggleFullscreen(void)
{
  fullscreen = !fullscreen;
  video = SDL_SetVideoMode(gr.resolutionX(), gr.resolutionY(), 24, fullscreen?SDL_FULLSCREEN:0);
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

pixelSurface_c::pixelSurface_c(const surface_c & pre) {

  const SDL_Surface * vid = pre.getVideo();

  if (!vid)
    throw std::exception();

  video = SDL_CreateRGBSurface(0, vid->w, vid->h, 32,
        vid->format->Rmask, vid->format->Gmask, vid->format->Bmask, 0);
}

pixelSurface_c::~pixelSurface_c(void) {
  delete video;
}

