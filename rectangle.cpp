/*
  Copyright (C) 2012 - Voidious

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

#include "line2d.h"
#include "rectangle.h"

Rectangle::Rectangle(int left, int bottom, int width, int height) {
  left_ = left;
  bottom_ = bottom;
  width_ = width;
  height_ = height;
  lines_[0] = new Line2D(left, bottom, left + width, bottom);
  lines_[1] = new Line2D(left + width, bottom, left + width, bottom + height);
  lines_[2] = new Line2D(left + width, bottom + height, left, bottom + height);
  lines_[3] = new Line2D(left, bottom + height, left, bottom);
}

int Rectangle::getLeft() {
  return left_;
}

int Rectangle::getBottom() {
  return bottom_;
}

int Rectangle::getWidth() {
  return width_;
}

int Rectangle::getHeight() {
  return height_;
}

Line2D** Rectangle::getLines() {
  return lines_;
}

Rectangle::~Rectangle() {
  for (int x = 0; x < 4; x++) {
    delete lines_[x];
  }
}
