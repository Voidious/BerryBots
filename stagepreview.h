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

#ifndef STAGE_PREVIEW_H
#define STAGE_PREVIEW_H

#include <wx/wx.h>
#include "menubarmaker.h"
#include "bbengine.h"

#define MAX_PREVIEW_WIDTH   500
#define MAX_PREVIEW_HEIGHT  400

class StagePreview : public wxFrame {
  wxPanel *mainPanel_;
  wxBoxSizer *mainSizer_;
  wxBoxSizer *borderSizer_;
  MenuBarMaker *menuBarMaker_;
  bool menusInitialized_;
  FileManager *fileManager_;

  public:
    StagePreview(const char *stagesBaseDir, const char *stageName,
                 int x, int y, MenuBarMaker *menuBarMaker);
    ~StagePreview();
    void onActivate(wxActivateEvent &event);
  private:
    std::string savePreviewReplay(BerryBotsEngine *engine,
        const char *stagesBaseDir, const char *stageName)
        throw (EngineException *);
};

#endif
