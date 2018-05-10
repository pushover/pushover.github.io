/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the AUTHORS file, which is included
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

#ifndef __GRAPHICS_N_H__
#define __GRAPHICS_N_H__

#include "graphics.h"
#include "screen.h"
#include "ant.h"

#include <string>
#include <vector>

class pngLoader_c;
class levelData_c;
class ant_c;

typedef enum {
  CURS_FG,
  CURS_BG
} cursorMode_e;

/* implementation for graphics class using the original graphics */
class graphicsN_c : public graphics_c {

  public:

    graphicsN_c(const std::string & path);
    ~graphicsN_c(void);

    virtual unsigned int resolutionX(void) const { return 800; }
    virtual unsigned int resolutionY(void) const { return 600; }

    /* to get the block size of one block */
    virtual unsigned int blockX(void) const { return 40; }
    virtual unsigned int blockY(void) const { return 48; }

    void markAllDirty(void) { dirty.markAllDirty(); }
    void markAllDirtyBg(void) { dirtybg.markAllDirty(); dirty.markAllDirty(); }
    const bitfield_c & getDirty(void) const { return dirty; }
    void clearDirty(void) { dirty.clearDirty(); }

    void setPaintData(const levelData_c * l, const ant_c * a, surface_c * t);

    void drawLevel(void);

    surface_c * getBoxBlock(unsigned int num) { return boxBlocks[num]; }
    // get domino animation image
    const surface_c * getHelpDominoImage(unsigned int domino);

    void setEditorMode(bool on);
    void setCursorMode(cursorMode_e mode);
    void setShowGrid(bool on);
    bool getShowGrid(void) const { return grid; }
    size_t getCursorX(void) const { return cursorX; }
    size_t getCursorY(void) const { return cursorY; }
    size_t getCursorW(void) const { return cursorW; }
    size_t getCursorH(void) const { return cursorH; }
    void setCursor(size_t x, size_t y, size_t w, size_t h);

    // set a status text, this text automatically vanishes after a while
    void setStatus(const std::string & txt);

    // set Overlay to be displayed over the level
    void setOverlay(const surface_c * o);

    void setForegroundVisibility(bool on);
    bool getForegroundVisibility(void) const { return foregroundVisible; }

    void setBgDrawMode(uint8_t mode);
    void setBgDrawLayer(uint8_t layer);

    const std::vector<surface_c *> getBgTiles(void) const { return bgTiles[curTheme]; }

    void setEditPlaneLayer(uint8_t layer);
    void setEditPlaneTile(uint8_t x, uint8_t y, uint16_t tile);
    void clearEditPlane(void);

    void setShowBgNumbers(bool on);
    bool getShowBgNumbers(void) const { return showBgNumbers; }

    // the separates patterns alway work like this:
    // first index contains the number of columns, the followind entries in the vector
    // contain the different tiles to use
    const std::vector<std::vector<uint16_t> > & getBgTilePatterns(void) const { return bgTilePatterns[curTheme]; }

  private:

    bool editorMode;
    cursorMode_e cursorMode;
    bool grid;
    size_t cursorX, cursorY, cursorW, cursorH;
    std::string statusText;
    uint16_t statusTime;
    const surface_c * overlay;
    bool foregroundVisible;
    uint8_t bgDrawMode, bgDrawLayer;
    uint8_t editPlaneLayer;
    std::vector<std::vector<uint16_t> > editPlane;
    bool showBgNumbers;

    // add an image to a specific ant animation, the new image is added at the end
    // you must also provide an y-offset used when animating to displace the image
    // if free is false it is assumed that the surface is used elsewhere and not freed
    // on deletion of object
    void addAnt(unsigned int anim, unsigned int img, signed char yOffset, surface_c * v, bool free = true);
    void getAnimation(AntAnimationState anim, pngLoader_c * png);

    // get index into the fg tiles for the tile at position x, y
    // values bigger than the size of fgTiles means leave empty
    void getPlatformImage(size_t x, size_t y, uint16_t out[4]);

    void drawDomino(uint16_t x, uint16_t y);

    std::string dataPath;

    int Min, Sec;   // number of minutes and seconds shown in display

    /* this surface contains the background. It is only updated when necessary
     * the content it used to restore stuff behind the sprites
     */
    surface_c * background;
    surface_c * target;

    // contains the box with the tutorial text, when
    // calculated, otherwise the 0 ointer
    surface_c * tutorial;
    uint16_t tutorial_x, tutorial_y, tutorial_w, tutorial_h; // contains the position where to put tutorial box

    void calcTutorial(void);

    const ant_c * ant;
    const levelData_c * level;
    levelData_c l2;  // level copy for dirty Block detection
    int16_t antX, antY;
    unsigned int antAnim, antImage;

    bitfield_c dirty, dirtybg;

    std::vector<std::string> themeNames;

    std::vector<std::vector<surface_c *> > bgTiles;
    std::vector<std::vector<surface_c *> > fgTiles;

    // for all themes a vector of arrays with Tile Patterns to use for the level editor
    std::vector<std::vector<std::vector<uint16_t> > > bgTilePatterns;

    std::vector<std::vector<surface_c *> > dominoes;

    std::vector<surface_c *> boxBlocks;

    typedef struct {
      surface_c * v;
      int ofs;
      bool free;
    } antSprite;

    std::vector<std::vector<antSprite> > antImages;

    unsigned int curTheme;

    void setTheme(const std::string & name);


    /* draw the changed stuff into the target surface */
    void drawDominoes(void);
    void drawLadders(bool before);
    void drawAnt(void);

    void markDirty(int x, int y) { dirty.markDirty(x, y); }
    void markDirtyBg(int x, int y) { dirtybg.markDirty(x, y); dirty.markDirty(x, y); }
    bool isDirty(int x, int y) const { return dirty.isDirty(x, y); }


    void findDirtyBlocks(void);

};

#endif

