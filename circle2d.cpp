/*
  Copyright (C) 2012-2013 - Voidious

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
#include "bbutil.h"
#include "circle2d.h"

Circle2D::Circle2D() {
  h_ = k_ = r_ = 0;
  inverse_ = 0;
}

Circle2D::Circle2D(double x, double y, double r) {
  h_ = x;
  k_ = y;
  r_ = r;
  inverse_ = 0;
}

bool Circle2D::intersects(Line2D *line) {
  return intersects(line, false, false, (Point2D**) 0, (Point2D**) 0);
}

bool Circle2D::intersects(Line2D *line, Point2D **p1, Point2D **p2) {
  return intersects(line, false, true, p1, p2);
}

bool Circle2D::intersects(Line2D *line, bool inverted, bool assignPoints,
    Point2D **p1, Point2D **p2) {
  if (h_ - line->xMax() > r_ || line->xMin() - h_ > r_ || k_ - line->yMax() > r_
      || line->yMin() - k_ > r_) {
    return false;
  }
  
  double linem = line->m();
  double lineb = line->b();

  if (linem == DBL_MAX) {
    Line2D *invLine = line->getInverse();
    Circle2D *invCircle = this->getInverse();
    return invCircle->intersects(invLine, !inverted, assignPoints, p1, p2);
  } else {
    bool intersects = false;

    if (contains(line->x1(), line->y1())) {
      if (assignPoints) {
        saveToNextPoint(p1, p2, line->x1(), line->y1(), inverted);
        intersects = true;
      } else {
        return true;
      }
    }
    if (contains(line->x2(), line->y2())) {
      if (assignPoints) {
        saveToNextPoint(p1, p2, line->x2(), line->y2(), inverted);
        intersects = true;
        if (*p2 != 0) {
          return true;
        }
      } else {
        return true;
      }
    }

    double a = square(linem) + 1;
    double b = 2 * ((lineb * linem) - (k_ * linem) - h_);
    double c = square(h_) + square(k_) + square(lineb) - (2 * lineb * k_)
        - square(r_);

    double discrim = (b * b) - (4 * a * c);
    if (discrim < 0) {
      return false;
    }
      
    double sqrtDiscrim = sqrt(discrim);
    double x1 = (-b + sqrtDiscrim) / (2 * a);
    double y1 = (linem * x1) + lineb;

    if (x1 >= line->xMin() && x1 <= line->xMax()) {
      if (assignPoints) {
        saveToNextPoint(p1, p2, x1, y1, inverted);
        intersects = true;
        if (*p2 != 0) {
          return true;
        }
      } else {
        return true;
      }
    }
      
    if (sqrtDiscrim > 0) {
      double x2 = (-b - sqrtDiscrim) / (2 * a);
      double y2 = (linem * x2) + lineb;
      if (x2 >= line->xMin() && x2 <= line->xMax()) {
        if (assignPoints) {
          saveToNextPoint(p1, p2, x2, y2, inverted);
          intersects = true;
        } else {
          return true;
        }
      }
    }
    return intersects;
  }
}

bool Circle2D::overlaps(Circle2D *circle) {
  double xDiff = circle->h() - h_;
  double yDiff = circle->k() - k_;
  double rSum = circle->r() + r_;
  return (((xDiff * xDiff) + (yDiff * yDiff)) <= (rSum * rSum));
}

bool Circle2D::contains(double x, double y) {
  double xDiff = x - h_;
  double yDiff = y - k_;
  return (((xDiff * xDiff) + (yDiff * yDiff)) <= (r_ * r_));
}

double Circle2D::h() {
  return h_;
}

double Circle2D::k() {
  return k_;
}

double Circle2D::r() {
  return r_;
}

Circle2D* Circle2D::getInverse() {
  if (inverse_ == 0) {
    inverse_ = new Circle2D(k_, h_, r_);
  }
  return inverse_;
}

void Circle2D::setPosition(double x, double y) {
  h_ = x;
  k_ = y;
  if (inverse_ != 0) {
    delete inverse_;
    inverse_ = 0;
  }
}

void Circle2D::saveToNextPoint(
    Point2D **p1, Point2D **p2, double x, double y, bool inverted) {
  Point2D *newPoint = (inverted ? new Point2D(y, x) : new Point2D(x, y));
  if (*p1 == 0) {
    *p1 = newPoint;
  } else if (*p2 == 0) {
    *p2 = newPoint;
  } else {
    delete newPoint;
  }
}

Circle2D::~Circle2D() {
  if (inverse_ != 0) {
    delete inverse_;
  }
}
