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

#include <string.h>
#include <SFML/Graphics.hpp>
#include "dockitem.h"
#include "rectangle.h"

DockItem::DockItem(int left, int top, int width, int height)
    : Rectangle(left, top, width, height) {
  top_ = bottom_; // SFML draws from top to bottom, unlike BerryBots coordinates
  highlighted_ = false;
  showShortcut_ = false;
  highlightedDrawables_ = 0;
  shortcutDrawables_ = 0;
  hoverText_ = 0;
  shortcutText_ = 0;
  numDrawables_ = numAltDrawables_ = 0;
}

DockItem::~DockItem() {
  if (drawables_ != 0) {
    for (int x = 0; x < numDrawables_; x++) {
      delete drawables_[x];
    }
    delete drawables_;
  }
  if (highlightedDrawables_ != 0) {
    delete highlightedDrawables_;
  }
  if (shortcutDrawables_ != 0) {
    delete shortcutDrawables_;
  }
  if (hoverText_ != 0) {
    delete hoverText_;
  }
  if (shortcutText_ != 0) {
    delete shortcutText_;
  }
}

void DockItem::setHighlights(int mouseX, int mouseY) {
  bool highlighted = contains(mouseX, mouseY);
  if (highlighted != highlighted_) {
    setHighlighted(highlighted);
  }
  highlighted_ = highlighted;
}

void DockItem::showShortcut() {
  if (shortcutText_ != 0) {
    showShortcut_ = true;
  }
}

void DockItem::hideShortcut() {
  showShortcut_ = false;
}

sf::Drawable** DockItem::getDrawables() {
  return (showShortcut_ ? shortcutDrawables_
          : highlighted_ ? highlightedDrawables_ : drawables_);
}

int DockItem::getNumDrawables() {
  return (highlighted_ || showShortcut_) ? numAltDrawables_ : numDrawables_;
}

bool DockItem::contains(int x, int y) {
  return (x >= left_ && x <= left_ + width_ && y >= top_
          && y <= top_  + height_);
}

int DockItem::getTop() {
  return top_;
}
