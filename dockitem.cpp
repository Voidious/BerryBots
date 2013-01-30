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
                   int top, int width, int height)
    : Rectangle(left, top, width, height) {
  top_ = bottom_; // SFML draws from top to bottom, unlike BerryBots coordinates
  highlighted_ = false;
  sfmlText_ = new sf::Text(text, *font, fontSize);
  sfmlText_->setPosition(10, top_ + (height / 2) - fontSize);
  sfmlText_->setColor(TEXT_COLOR);
}

DockItem::~DockItem() {
  delete sfmlText_;
}

void DockItem::setHighlights(int mouseX, int mouseY) {
  bool highlight = contains(mouseX, mouseY);
  if (highlight != highlighted_) {
    sfmlText_->setColor(highlight ? HIGHLIGHTED_COLOR : TEXT_COLOR);
    highlighted_ = highlight;
  }
}

sf::Text* DockItem::getSfmlText() {
  return sfmlText_;
}

bool DockItem::contains(int x, int y) {
  return (x >= left_ && x <= left_ + width_ && y >= top_
          && y <= top_  + height_);
}
