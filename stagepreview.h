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

#include <string.h>
#include <wx/wx.h>
#include "menubarmaker.h"
#include "bbengine.h"
#include "gfxmanager.h"

#define MAX_PREVIEW_WIDTH   650
#define MAX_PREVIEW_HEIGHT  500

class StagePreviewListener;

class StagePreview : public wxFrame {
  wxPanel *mainPanel_;
  wxSizer *infoSizer_;
  wxSizer *descSizer_;
  GfxManager *previewGfxManager_;
  wxStaticBitmap *visualPreview_;
  MenuBarMaker *menuBarMaker_;
  bool menusInitialized_;
  FileManager *fileManager_;
  char *stagesBaseDir_;
  char *stageName_;
  std::string lastPreviewUrl_;
  StagePreviewListener *listener_;
  wxEventFilter *eventFilter_;

  public:
    StagePreview(const char *stagesBaseDir, MenuBarMaker *menuBarMaker);
    ~StagePreview();
    void onActivate(wxActivateEvent &event);
    void onClose(wxCommandEvent &event);
    void onUp();
    void onDown();
    void setListener(StagePreviewListener *listener);
    void showPreview(const char *stageName, int x, int y);
  private:
    void addInfo(wxSizer *sizer, const char *name, const char *value);
    void addInfo(wxSizer *sizer, const char *name, int i);
    char *savePreviewImage(BerryBotsEngine *engine);
};

class StagePreviewListener {
  public:
    virtual void onClose() = 0;
    virtual void onUp() = 0;
    virtual void onDown() = 0;
    virtual ~StagePreviewListener() {};
};

class PreviewEventFilter : public wxEventFilter {
  StagePreview *stagePreview_;
  
  public:
    PreviewEventFilter(StagePreview *stagePreview_);
    virtual int FilterEvent(wxEvent& event);
};

#endif
