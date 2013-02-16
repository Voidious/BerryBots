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
#include "docktext.h"

DockText::DockText(const char *text, sf::Font *font, int fontSize, int left,
    int top, int width, int height) : DockItem(left, top, width, height) {
  drawableText_ = new sf::Text(text, *font, fontSize);
  drawableText_->setPosition(left_, top_ + (height / 2) - fontSize);
  drawableText_->setColor(DEFAULT_COLOR);
  drawables_ = new sf::Drawable*[1];
  drawables_[0] = drawableText_;
  numDrawables_ = numAltDrawables_ = 1;
  highlightedDrawables_ = new sf::Drawable*[1];
  shortcutDrawables_ = new sf::Drawable*[1];
  shortcutDrawables_[0] = highlightedDrawables_[0] = drawableText_;
}

DockText::~DockText() {

}

void DockText::setHighlighted(bool highlighted) {
  drawableText_->setColor(highlighted ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
}
