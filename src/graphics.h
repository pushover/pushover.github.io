/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <SDL.h>
#include <string>
#include <vector>
#include <map>

/* this class contains all the information for all graphics */
class graphics_c {

  public:

    graphics_c(void);
    virtual ~graphics_c(void);

    virtual void loadGraphics(void) = 0;

    /* load a new theme, and activate it, maybe that very theme is already
     * loaded and just a pointer is replaced
     */
    void setTheme(const std::string & name);

    /* to get the resolution that should be used */
    virtual unsigned int resolutionX(void) const = 0;
    virtual unsigned int resolutionY(void) const = 0;

    /* to get the block size of one block */
    virtual unsigned int blockX(void) const = 0;
    virtual unsigned int blockY(void) const = 0;
    virtual unsigned int halveBlockDisplace(void) const = 0;  // return and noffset to actually place the objects
    virtual unsigned int antDisplace(void) const = 0;  // return and noffset to actually place the objects

    SDL_Surface * getBgTile(unsigned int num) {
      if (num < bgTiles[curTheme].size())
        return bgTiles[curTheme][num];
      else
        throw std::exception();
    }
    SDL_Surface * getFgTile(unsigned int num) {
      if (num < fgTiles[curTheme].size())
        return fgTiles[curTheme][num];
      else
        throw std::exception();
    }


    SDL_Surface * getDomino(unsigned int domino, unsigned int image) { return dominos[domino][image]; }
    SDL_Surface * getAnt(unsigned int animation, unsigned int step) { return ant[animation][step].v; }
    int getAntOffset(unsigned int animation, unsigned int step) { return step<ant[animation].size()?ant[animation][step].ofs:0; }
    unsigned int getAntImages(unsigned int animation) { return ant[animation].size(); }
    SDL_Surface * getCarriedDomino(unsigned int domino, unsigned int image) { return carriedDominos[domino][image]; }

    SDL_Surface * getBoxBlock(unsigned int num) { return boxBlocks[num]; }

    // these are offsets that are used together with carried domino to displace
    // that domino
    virtual signed int getCarryOffsetX(unsigned int animation, unsigned int image) const = 0;
    virtual signed int getCarryOffsetY(unsigned int animation, unsigned int image) const = 0;

    virtual signed int getMoveOffsetX(unsigned int animation, unsigned int image) const = 0;
    virtual signed int getMoveOffsetY(unsigned int animation, unsigned int image) const = 0;
    virtual signed int getMoveImage(unsigned int animation, unsigned int image) const = 0;

    virtual void loadTheme(const std::string & name) = 0;

    static const unsigned char numDominoTypes;
    static const unsigned char numDominos[23];

    static const unsigned char numAntAnimations;

    // the position of the time in the level
    virtual int timeXPos(void) const = 0;
    virtual int timeYPos(void) const = 0;

    virtual int getDominoYStart(void) const = 0;
    virtual int convertDominoX(int x) const = 0;
    virtual int convertDominoY(int y) const = 0;
    virtual int splitterY(void) const = 0;

  protected:

    /* some functions for the loaders to store the loaded images */
    void addBgTile(SDL_Surface * v);
    void addFgTile(SDL_Surface * v);
    void addBgTile(unsigned int idx, SDL_Surface * v);
    void addFgTile(unsigned int idx, SDL_Surface * v);

    // sets a specific domino for a specific domino type and animation state
    void setDomino(unsigned int type, unsigned int num, SDL_Surface * v);
    void setCarriedDomino(unsigned int type, unsigned int num, SDL_Surface * v);

    // add an image to a specific ant animation, the new image is added at the end
    // you must also provide an y-offset used when animating to displace the image
    // if free is false it is assumed that the surface is used elsewhere and not freed
    // on deletion of object
    void addAnt(unsigned int anim, unsigned int img, signed char yOffset, SDL_Surface * v, bool free = true);

    // add a box block, there need to be 9 of those blocks, 4 corners, 4 edges and one center
    // block for the filling, add the blocks as if you had painted a 3x3 box, upper left corner
    // upper edge, upper right corner, left edge, ...., lower right corner
    void addBoxBlock(SDL_Surface * v);

  private:

    std::vector<std::string> themeNames;

    std::vector<std::vector<SDL_Surface *> > bgTiles;
    std::vector<std::vector<SDL_Surface *> > fgTiles;

    std::vector<std::vector<SDL_Surface *> > dominos;
    std::vector<std::vector<SDL_Surface *> > carriedDominos;

    std::vector<SDL_Surface *> boxBlocks;

    typedef struct {
      SDL_Surface * v;
      int ofs;
      bool free;
    } antSprite;

    std::vector<std::vector<antSprite> > ant;

    unsigned int curTheme;
};

#endif

