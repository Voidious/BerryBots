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

#include <string.h>
#include <SFML/Graphics.hpp>
#include "dockitem.h"
#include "rectangle.h"

DockItem::DockItem(const char *text, sf::Font *font, int fontSize, int left,
    int top, int width, int height) : Rectangle(left, top, width, height) {
  top_ = bottom_; // SFML draws from top to bottom, unlike BerryBots coordinates
  highlighted_ = false;
  drawableText_ = new sf::Text(text, *font, fontSize);
  drawableText_->setPosition(10, top_ + (height / 2) - fontSize);
  drawableText_->setColor(DEFAULT_COLOR);
  drawables_ = new sf::Drawable*[1];
  drawables_[0] = drawableText_;
  numDrawables_ = 1;
}

DockItem::DockItem(sf::Shape **shapes, int numShapes, int left, int top,
    int width, int height) : Rectangle(left, top, width, height) {
  top_ = bottom_; // SFML draws from top to bottom, unlike BerryBots coordinates
  highlighted_ = false;
  drawableText_ = 0;
  drawableShapes_ = shapes;
  drawables_ = (sf::Drawable**) shapes;
  numDrawables_ = numShapes;
  sf::Vector2f newOrigin(left + (width / 2), top + (height / 2));
  for (int x = 0; x < numDrawables_; x++) {
    sf::Shape *shape = drawableShapes_[x];
    shape->move(newOrigin);
  }
}

DockItem::~DockItem() {
  for (int x = 0; x < numDrawables_; x++) {
    delete drawables_[x];
  }
  delete drawables_;
}

void DockItem::setHighlights(int mouseX, int mouseY) {
  bool highlight = contains(mouseX, mouseY);
  if (highlight != highlighted_) {
    if (drawableText_ != 0) {
      drawableText_->setColor(highlight ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
    } else if (drawables_ != 0) {
      for (int x = 0; x < numDrawables_; x++) {
        sf::Shape *shape = drawableShapes_[x];
        shape->setFillColor(highlight ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
      }
    }
    highlighted_ = highlight;
  }
}

sf::Drawable** DockItem::getDrawables() {
  return drawables_;
}

int DockItem::getNumDrawables() {
  return numDrawables_;
}

bool DockItem::contains(int x, int y) {
  return (x >= left_ && x <= left_ + width_ && y >= top_
          && y <= top_  + height_);
}
