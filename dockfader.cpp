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
#include <math.h>
#include <SFML/Graphics.hpp>
#include "bbutil.h"
#include "dockitem.h"
#include "dockfader.h"

DockFader::DockFader(int left, int top, int width, int height,
    const char *hoverText, sf::Font *font, int fontSize, int textLeft,
    int textTop, int shortcutFontSize)
        : DockItem(left, top, width, height) {
  faderSlot_ = new sf::RectangleShape(sf::Vector2f(width, 2));
  faderCenter_ = new sf::RectangleShape(sf::Vector2f(2, height / 2));
  faderCenter_->setFillColor(sf::Color(125, 125, 125));
  faderCenter_->setOutlineThickness(0);
  faderLow_ = new sf::RectangleShape(sf::Vector2f(2, (height / 2) - 6));
  faderLow_->setFillColor(sf::Color(125, 125, 125));
  faderLow_->setOutlineThickness(0);
  faderHigh_ = new sf::RectangleShape(sf::Vector2f(2, (height / 2) - 6));
  faderHigh_->setFillColor(sf::Color(125, 125, 125));
  faderHigh_->setOutlineThickness(0);
  faderKnob_ = new sf::RectangleShape(sf::Vector2f(4, height / 2));

  drawables_ = new sf::Drawable*[5];
  drawables_[0] = faderCenter_;
  drawables_[1] = faderSlot_;
  drawables_[2] = faderLow_;
  drawables_[3] = faderHigh_;
  drawables_[4] = faderKnob_;
  numDrawables_ = 5;

  hoverText_ = new sf::Text(hoverText, *font, fontSize);
  hoverText_->setPosition(textLeft, textTop);
  hoverText_->setColor(HIGHLIGHTED_COLOR);

  numHighlightedDrawables_ = numDrawables_ + 1;
  highlightedDrawables_ = new sf::Drawable*[numHighlightedDrawables_];
  for (int x = 0; x < numDrawables_; x++) {
    highlightedDrawables_[x] = drawables_[x];
  }
  highlightedDrawables_[numDrawables_] = hoverText_;

  xMin_ = left_;
  xMax_ = left_ + width_ - 4;
  xZero_ = (xMin_ + xMax_) / 2;
  int highLowOffset = (xZero_ - xMin_) * HIGH_LOW_FACTOR;
  xLow_ = xZero_ - highLowOffset;
  xHigh_ = xZero_ + highLowOffset;
  faderKnob_->setPosition(xZero_, top_ + (height_ / 4));
  volumeBase_ = pow(
      2, 1.0 / pow(((double) highLowOffset) / (xZero_ - xMin_), VOLUME_EXP));

  shortcutLeft_ = new sf::Text("[", *font, shortcutFontSize);
  shortcutLeft_->setColor(SHORTCUT_COLOR);
  shortcutRight_ = new sf::Text("]", *font, shortcutFontSize);
  shortcutRight_->setColor(SHORTCUT_COLOR);

  numShortcutDrawables_ = numDrawables_ + 2;
  shortcutDrawables_ = new sf::Drawable*[numShortcutDrawables_];
  for (int x = 0; x < numDrawables_; x++) {
    shortcutDrawables_[x] = drawables_[x];
  }
  shortcutDrawables_[numDrawables_] = shortcutLeft_;
  shortcutDrawables_[numDrawables_ + 1] = shortcutRight_;

  setTop(top_, textTop);
}

DockFader::~DockFader() {
  delete shortcutLeft_;
  delete shortcutRight_;
}

void DockFader::setKnob(int x) {
  int xLeft = x - 2;
  int xNew = (abs(xLeft - xZero_) < 4) ? xZero_
      : (abs(xLeft - xLow_) < 3) ? xLow_
      : (abs(xLeft - xHigh_) < 3) ? xHigh_
      : limit(xMin_, xLeft, xMax_);
  faderKnob_->setPosition(xNew, top_ + (height_ / 4));
}

int DockFader::getKnobSetting() {
  return round(faderKnob_->getPosition().x + 2);
}

double DockFader::getVolume() {
  int xKnob = faderKnob_->getPosition().x;
  if (xKnob == xMin_) {
    return 0;
  } else {
    double linearVolume = ((double) (xKnob - xZero_)) / (xMax_ - xZero_);
    double volume = pow(
        volumeBase_, pow(abs(linearVolume), VOLUME_EXP) * signum(linearVolume));
    return volume;
  }
}

void DockFader::increaseVolume() {
  setKnob(getKnobSetting() + 6);
}

void DockFader::decreaseVolume() {
  setKnob(getKnobSetting() - 6);
}

void DockFader::setTop(int top, int textTop) {
  if (top != top_) {
    top_ = top;
    faderSlot_->setPosition(left_, top_ + (height_ / 2) - 1);
    faderCenter_->setPosition(xZero_ + 1, top_ + (height_ / 4));
    faderLow_->setPosition(xLow_ + 1, top_ + (height_ / 4) + 3);
    faderHigh_->setPosition(xHigh_ + 1, top_ + (height_ / 4) + 3);
    faderKnob_->setPosition(faderKnob_->getPosition().x, top_ + (height_ / 4));
    sf::FloatRect leftRect = shortcutLeft_->getLocalBounds();
    int shortcutTop =
        faderSlot_->getPosition().y + 1 - (leftRect.height * 5 / 6);
    shortcutLeft_->setPosition(xMin_ - leftRect.width - 2, shortcutTop);
    shortcutRight_->setPosition(xMax_ + 4 + 2, shortcutTop);
  }
  hoverText_->setPosition(hoverText_->getPosition().x, textTop);
}

void DockFader::setHighlighted(bool highlighted) {
  for (int x = 0; x < numDrawables_; x++) {
    sf::RectangleShape *shape = (sf::RectangleShape*) drawables_[x];
    if (shape != faderCenter_ && shape != faderLow_ && shape != faderHigh_) {
      shape->setFillColor(highlighted ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
      shape->setOutlineColor(highlighted ? HIGHLIGHTED_COLOR : DEFAULT_COLOR);
    }
  }
}
