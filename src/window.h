#ifndef __WINDOW_H__
#define __WINDOW_H__

// one of those boxed windows
// the window may update its content, but stacking the windows.... well I don't know if
// that will be possible, try to keep only one window
// visible at a time
//
// the windows are aligned to the blocks of the screen, the window position
// is given as block position and block size
// this makes updating easier

class window_c {

  private:

    unsigned char x, y, w, h;

  public:

    // minimum w and h is 2, but then you don't have space in the middle as everything is taken
    // away by the fame
    window_c(unsigned char x, unsigned char y, unsigned char w, unsigned char h);
    virtual ~window_c(void) {}

    virtual void handleEvent(void);
    virtual void redrawBlock(unsigned char x, unsigned char y);
};

#endif

