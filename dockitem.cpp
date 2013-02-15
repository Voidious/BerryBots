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
  showShortcut_ = false;
  drawableText_ = new sf::Text(text, *font, fontSize);
  drawableText_->setPosition(10, top_ + (height / 2) - fontSize);
  drawableText_->setColor(DEFAULT_COLOR);
  drawables_ = new sf::Drawable*[1];
  drawables_[0] = drawableText_;
  numDrawables_ = numAltDrawables_ = 1;
  highlightedDrawables_ = new sf::Drawable*[1];
  shortcutDrawables_ = new sf::Drawable*[1];
  shortcutDrawables_[0] = highlightedDrawables_[0] = drawableText_;
  hoverText_ = 0;
  shortcutText_ = 0;
}

DockItem::DockItem(sf::Shape **shapes, int numShapes, int left, int top,
    int width, int height, const char *hoverText, sf::Font *font, int fontSize,
    int textLeft, int textTop, const char *shortcut, int shortcutFontSize)
        : Rectangle(left, top, width, height) {
  top_ = bottom_; // SFML draws from top to bottom, unlike BerryBots coordinates
  highlighted_ = false;
  showShortcut_ = false;
  drawableText_ = 0;
  drawableShapes_ = shapes;
  drawables_ = (sf::Drawable**) shapes;
  numDrawables_ = numShapes;
  numAltDrawables_ = numDrawables_ + 1;
  sf::Vector2f newOrigin(left + (width / 2), top + (height / 2));
  for (int x = 0; x < numDrawables_; x++) {
    sf::Shape *shape = drawableShapes_[x];
    shape->move(newOrigin);
  }

  hoverText_ = new sf::Text(hoverText, *font, fontSize);
  hoverText_->setPosition(textLeft, textTop);
  hoverText_->setColor(HIGHLIGHTED_COLOR);
  shortcutText_ = new sf::Text(shortcut, *font, shortcutFontSize);
  sf::FloatRect shortcutRect = shortcutText_->getLocalBounds();
  shortcutText_->setPosition(newOrigin.x - (shortcutRect.width / 2),
                             top + height);
  shortcutText_->setColor(SHORTCUT_COLOR);
    
  highlightedDrawables_ = new sf::Drawable*[numDrawables_ + 1];
  for (int x = 0; x < numDrawables_; x++) {
    highlightedDrawables_[x] = drawables_[x];
  }
  highlightedDrawables_[numDrawables_] = hoverText_;

  shortcutDrawables_ = new sf::Drawable*[numDrawables_ + 1];
  for (int x = 0; x < numDrawables_; x++) {
    shortcutDrawables_[x] = drawables_[x];
  }
  shortcutDrawables_[numDrawables_] = shortcutText_;
}

DockItem::~DockItem() {
  for (int x = 0; x < numDrawables_; x++) {
    delete drawables_[x];
  }
  delete drawables_;
  delete highlightedDrawables_;
  if (hoverText_ != 0) {
    delete hoverText_;
  }
  delete shortcutDrawables_;
  if (shortcutText_ != 0) {
    delete shortcutText_;
  }
}

void DockItem::setHighlights(int mouseX, int mouseY) {
  bool highlight = contains(mouseX, mouseY);
  if (highlight != highlighted_) {
    if (drawableText_ != 0) {
      drawableText_->setColor(highlight ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
    } else if (drawables_ != 0) {
      for (int x = 0; x < numDrawables_; x++) {
        sf::Shape *shape = drawableShapes_[x];
        if (shape->getFillColor() != sf::Color::Black) {
          shape->setFillColor(highlight ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
        }
        if (shape->getOutlineColor() != sf::Color::Black) {
          shape->setOutlineColor(highlight ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
        }
      }
      shortcutText_->setColor(highlight ? HIGHLIGHTED_COLOR : SHORTCUT_COLOR);
    }
    highlighted_ = highlight;
  }
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
