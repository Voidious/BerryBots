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

#ifndef LINE2D_H
#define LINE2D_H

#include "point2d.h"

class Line2D {
  double m_, b_, xMin_, xMax_, yMin_, yMax_, x1_, y1_, x2_, y2_, theta_;
  Line2D *inverse_;
  public:
    Line2D(double x1, double y1, double x2, double y2);
    ~Line2D();
    double m();
    double b();
    double x1();
    double y1();
    double x2();
    double y2();
    double xMin();
    double xMax();
    double yMin();
    double yMax();
    double theta();
    void shift(double dx, double dy);
    Line2D *getInverse();
    bool intersects(Line2D *line);
};

#endif
