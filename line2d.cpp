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

#include <math.h>
#include <float.h>
#include <algorithm>
#include "bbutil.h"
#include "line2d.h"

Line2D::Line2D(double x1, double y1, double x2, double y2) {
  if (x1 == x2) {
    m_ = DBL_MAX;
    b_ = DBL_MIN;
    xMin_ = xMax_ = x1;
  } else {
    m_ = (y2 - y1) / (x2 - x1);
    b_ = y1 - (m_ * x1);
    xMin_ = std::min(x1, x2);
    xMax_ = std::max(x1, x2);
  }
  yMin_ = std::min(y1, y2);
  yMax_ = std::max(y1, y2);

  x1_ = x1;
  y1_ = y1;
  x2_ = x2;
  y2_ = y2;
  theta_ = DBL_MIN;
  inverse_ = 0;
}

double Line2D::m() {
  return m_;
}

double Line2D::b() {
  return b_;
}

double Line2D::x1() {
  return x1_;
}

double Line2D::y1() {
  return y1_;
}

double Line2D::x2() {
  return x2_;
}

double Line2D::y2() {
  return y2_;
}

double Line2D::xMin() {
  return xMin_;
}

double Line2D::xMax() {
  return xMax_;
}

double Line2D::yMin() {
  return yMin_;
}

double Line2D::yMax() {
  return yMax_;
}

double Line2D::theta() {
  if (theta_ == DBL_MIN) {
    theta_ = atan2(y2_ - y1_, x2_ - x1_);
  }
  return theta_;
}

void Line2D::shift(double dx, double dy) {
  x1_ += dx;
  x2_ += dx;
  xMin_ += dx;
  xMax_ += dx;
  y1_ += dy;
  y2_ += dy;
  yMin_ += dy;
  yMax_ += dy;
  if (inverse_ != 0) {
    inverse_->shift(dy, dx);
  }
}

Line2D* Line2D::getInverse() {
  if (inverse_ == 0) {
    inverse_ = new Line2D(this->y1(), this->x1(), this->y2(), this->x2());
  }
  return inverse_;
}

bool Line2D::intersects(Line2D *line) {
  double linem = line->m();
  if (m_ == linem) {
    return false;
  } else if (m_ == DBL_MAX || linem == DBL_MAX) {
    if (m_ == DBL_MAX && linem == 0) {
      return (xMin_ >= line->xMin() && xMax_ <= line->xMax()
              && yMin_ <= line->yMin() && yMax_ >= line->yMax());
    } else {
      return getInverse()->intersects(line->getInverse());
    }
  }
  
  double x = (line->b() - b_) / (m_ - linem);
  return (x >= xMin_ && x <= xMax_ && x >= line->xMin() && x <= line->xMax());
}

Line2D::~Line2D() {
  if (inverse_ != 0) {
    delete inverse_;
  }
}
