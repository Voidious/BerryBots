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
#include <math.h>
#include <SFML/Graphics.hpp>
#include "bbutil.h"
#include "dockitem.h"
#include "dockfader.h"

DockFader::DockFader(int left, int top, int width, int height,
    const char *hoverText, sf::Font *font, int fontSize, int textLeft,
    int textTop) : DockItem(left, top, width, height) {
  faderSlot_ = new sf::RectangleShape(sf::Vector2f(width, 2));
  faderSlot_->setPosition(left_, top_ + (height / 2) - 1);
  faderCenter_ = new sf::RectangleShape(sf::Vector2f(2, (height / 2)));
  faderCenter_->setPosition(left_ + (width / 2) - 1, top_ + (height / 4));
  faderCenter_->setFillColor(sf::Color(125, 125, 125));
  faderCenter_->setOutlineThickness(0);
  faderKnob_ = new sf::RectangleShape(sf::Vector2f(4, (height / 2)));
  faderKnob_->setPosition(left_ + (width / 2) - 2, top_ + (height / 4));

  drawables_ = new sf::Drawable*[3];
  drawables_[0] = faderCenter_;
  drawables_[1] = faderSlot_;
  drawables_[2] = faderKnob_;
  numDrawables_ = 3;

  hoverText_ = new sf::Text(hoverText, *font, fontSize);
  hoverText_->setPosition(textLeft, textTop);
  hoverText_->setColor(HIGHLIGHTED_COLOR);

  highlightedDrawables_ = new sf::Drawable*[numDrawables_ + 1];
  for (int x = 0; x < numDrawables_; x++) {
    highlightedDrawables_[x] = drawables_[x];
  }
  highlightedDrawables_[numDrawables_] = hoverText_;
  numAltDrawables_ = numDrawables_ + 1;

  xMin_ = left_;
  xMax_ = left_ + width_ - 2;
  xZero_ = faderKnob_->getPosition().x;
}

DockFader::~DockFader() {

}

void DockFader::setKnob(int x) {
  int xNew = (abs(x - 2 - xZero_) < 6) ? xZero_ : limit(xMin_, x - 2, xMax_);
  faderKnob_->setPosition(xNew, top_ + (height_ / 4));
}

double DockFader::getVolume() {
  int xKnob = faderKnob_->getPosition().x;
  if (xKnob == xMin_) {
    return 0;
  } else {
    double linearVolume =
        MAX_VOLUME_EXP * (((double) (xKnob - xZero_)) / (xMax_ - xZero_));
    double volume = pow(VOLUME_BASE, linearVolume);
    return volume;
  }
}

void DockFader::setHighlighted(bool highlighted) {
  for (int x = 0; x < numDrawables_; x++) {
    sf::RectangleShape *shape = (sf::RectangleShape*) drawables_[x];
    if (shape != faderCenter_) {
      shape->setFillColor(highlighted ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
      shape->setOutlineColor(highlighted ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
    }
  }
}
