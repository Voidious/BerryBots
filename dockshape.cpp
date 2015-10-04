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
#include "dockshape.h"

DockShape::DockShape(sf::Shape **shapes, int numShapes, int left, int top,
    int width, int height, const char *hoverText, sf::Font *font, int fontSize,
    int textLeft, int textTop, const char *shortcut, int shortcutFontSize)
        : DockItem(left, top, width, height) {
  drawableShapes_ = shapes;
  drawables_ = (sf::Drawable**) shapes;
  numDrawables_ = numShapes;
  sf::Vector2f newOrigin(left_ + (width_ / 2), top_ + (height_ / 2));
  for (int x = 0; x < numDrawables_; x++) {
    sf::Shape *shape = drawableShapes_[x];
    shape->move(newOrigin);
  }

  hoverText_ = new sf::Text(hoverText, *font, fontSize);
  hoverText_->setPosition(textLeft, textTop);
  hoverText_->setColor(HIGHLIGHTED_COLOR);

  numHighlightedDrawables_ = numDrawables_ + 1;
  highlightedDrawables_ = new sf::Drawable*[numHighlightedDrawables_];
  for (int x = 0; x < numDrawables_; x++) {
    highlightedDrawables_[x] = drawables_[x];
  }
  highlightedDrawables_[numDrawables_] = hoverText_;

  shortcutText_ = new sf::Text(shortcut, *font, shortcutFontSize);
  sf::FloatRect shortcutRect = shortcutText_->getLocalBounds();
  shortcutText_->setPosition(newOrigin.x - (shortcutRect.width / 2),
                             top + height);
  shortcutText_->setColor(SHORTCUT_COLOR);

  numShortcutDrawables_ = numDrawables_ + 1;
  shortcutDrawables_ = new sf::Drawable*[numShortcutDrawables_];
  for (int x = 0; x < numDrawables_; x++) {
    shortcutDrawables_[x] = drawables_[x];
  }
  shortcutDrawables_[numDrawables_] = shortcutText_;
}

DockShape::~DockShape() {
  delete shortcutText_;
}

void DockShape::setTop(int top, int textTop) {
  if (top != top_) {
    int topDelta = top - top_;
    top_ = top;
    sf::Vector2f shift(0, topDelta);
    for (int x = 0; x < numDrawables_; x++) {
      sf::Shape *shape = drawableShapes_[x];
      shape->move(shift);
    }
    sf::FloatRect shortcutRect = shortcutText_->getLocalBounds();
    shortcutText_->setPosition(
        (left_ + (width_ / 2)) - (shortcutRect.width / 2), top_ + height_);
    hoverText_->setPosition(hoverText_->getPosition().x, textTop);
  }
}

void DockShape::setHighlighted(bool highlighted) {
  for (int x = 0; x < numDrawables_; x++) {
    sf::Shape *shape = drawableShapes_[x];
    if (shape->getFillColor() != sf::Color::Black) {
      shape->setFillColor(highlighted ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
    }
    if (shape->getOutlineColor() != sf::Color::Black) {
      shape->setOutlineColor(highlighted ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
    }
  }
  shortcutText_->setColor(highlighted ? HIGHLIGHTED_COLOR : SHORTCUT_COLOR);
}
