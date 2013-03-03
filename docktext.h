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

#ifndef DOCK_TEXT_H
#define DOCK_TEXT_H

#include <SFML/Graphics.hpp>
#include "rectangle.h"

#define DISABLED_COLOR  sf::Color(150, 0, 0, 255)
#define ERRORED_COLOR   sf::Color(255, 0, 0, 255)

class DockText : public DockItem {
  sf::Text *drawableText_;
  bool disabled_;
  bool errored_;
  bool hidden_;
  int fontSize_;

  public:
    DockText(const char *text, sf::Font *font, int fontSize, int left, int top,
             int width, int height);
    ~DockText();
    void setDisabled(bool disabled);
    void setErrored(bool errored);
    bool hidden();
    void setHidden(bool hidden);
    void setTop(int top);
    virtual void setHighlighted(bool highlighted);
};

#endif
