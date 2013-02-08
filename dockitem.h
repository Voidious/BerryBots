/*
  Copyright (C) 2013 - Voidious

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

#ifndef DOCK_ITEM_H
#define DOCK_ITEM_H

#include <SFML/Graphics.hpp>
#include "rectangle.h"

#define DEFAULT_COLOR      sf::Color(255, 255, 255, 255)
#define HIGHLIGHTED_COLOR  sf::Color(0, 255, 0, 255)

class DockItem : public Rectangle {
  int top_;
  bool highlighted_;
  sf::Text *drawableText_;
  sf::Shape **drawableShapes_;
  sf::Drawable **drawables_;
  sf::Drawable **highlightedDrawables_;
  sf::Text *hoverText_;
  int numDrawables_;
  int numHighlightedDrawables_;

  public:
    DockItem(const char *text, sf::Font *font, int fontSize, int left, int top,
             int width, int height);
    DockItem(sf::Shape **shapes, int numShapes, int left, int top, int width,
             int height, const char *hoverText, sf::Font *font, int fontSize,
             int textLeft, int textTop);
    ~DockItem();
    void setHighlights(int mouseX, int mouseY);
    sf::Drawable** getDrawables();
    int getNumDrawables();
    bool contains(int x, int y);
};

#endif
