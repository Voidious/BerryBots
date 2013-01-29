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
#include <string.h>
#include <algorithm>
#include "bbutil.h"

double limit(double p, double q, double r) {
  return std::min(r, std::max(p, q));
}

int signum(double x) {
  if (x < 0) {
    return -1;
  } else if (x > 0) {
    return 1;
  } else {
    return 0;
  }
}

double square(double x) {
  return x * x;
}

double abs(double x) {
  if (x < 0) {
    return -x;
  }
  return x;
}

double normalRelativeAngle(double x) {
  while (x > M_PI) {
    x -= (2 * M_PI);
  }
  while (x < -M_PI) {
    x += (2 * M_PI);
  }
  return x;
}

double normalAbsoluteAngle(double x) {
  while (x > 2 * M_PI) {
    x -= (2 * M_PI);
  }
  while (x < 0) {
    x += (2 * M_PI);
  }
  return x;
}

double toDegrees(double x) {
  return x * 180 / M_PI;
}

char** parseFlag(int argc, char *argv[], const char *flag, int numValues) {
  for (int x = 0; x < argc - numValues; x++) {
    char *arg = argv[x];
    if (strlen(arg) > 1 && arg[0] == '-' && strcmp(&(arg[1]), flag) == 0) {
      char **values = new char*[numValues];
      for (int y = 0; y < numValues; y++) {
        values[y] = argv[x + y + 1];
      }
      return values;
    }
  }
  return 0;
}

bool flagExists(int argc, char *argv[], const char *flag) {
  for (int x = 0; x < argc; x++) {
    char *arg = argv[x];
    if (strlen(arg) > 1 && arg[0] == '-' && strcmp(&(arg[1]), flag) == 0) {
      return true;
    }
  }
  return false;
}
