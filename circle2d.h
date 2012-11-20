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

#ifndef CIRCLE_H
#define CIRCLE_H

#include "line2d.h"
#include "point2d.h"

class Circle2D {
  double h_, k_, r_;
  Circle2D* inverse_;
  public:
    Circle2D();
    Circle2D(double x, double y, double r);
    ~Circle2D();
    bool intersects(Line2D *line);
    bool intersects(Line2D *line, Point2D **p1, Point2D **p2);
    bool overlaps(Circle2D *circle);
    bool contains(double x, double y);
    double h();
    double k();
    double r();
    Circle2D* getInverse();
    void setPosition(double x, double y);
  private:
    bool intersects(Line2D *line, bool inverted, bool assignPoints,
        Point2D **p1, Point2D **p2);
    void saveToNextPoint(
        Point2D **p1, Point2D **p2, double x, double y, bool inverted);
};

#endif
