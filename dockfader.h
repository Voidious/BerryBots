/*
  Copyright (C) 2013-2015 - Voidious

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef DOCK_FADER_H
#define DOCK_FADER_H

#define HIGH_LOW_FACTOR  0.46
#define VOLUME_EXP       2.5

#include <SFML/Graphics.hpp>
#include "dockitem.h"

class DockFader : public DockItem {
  sf::RectangleShape *faderSlot_;
  sf::RectangleShape *faderCenter_;
  sf::RectangleShape *faderLow_;
  sf::RectangleShape *faderHigh_;
  sf::RectangleShape *faderKnob_;
  sf::Text *shortcutLeft_;
  sf::Text *shortcutRight_;
  int xMin_;
  int xMax_;
  int xZero_;
  int xLow_;
  int xHigh_;
  double volumeBase_;

  public:
    DockFader(int left, int top, int width, int height, const char *hoverText,
              sf::Font *font, int fontSize, int textLeft, int textTop,
              int shortcutFontSize);
    ~DockFader();
    void setKnob(int x);
    int getKnobSetting();
    double getVolume();
    void increaseVolume();
    void decreaseVolume();
    void setTop(int top, int textTop);
    virtual void setHighlighted(bool highlighted);
};

#endif
