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

#ifndef RUNNER_FORM_H
#define RUNNER_FORM_H

#include <wx/wx.h>

#define SELECT_HEIGHT       4
#define TEXT_HEIGHT         1
#define OK_CANCEL_HEIGHT    2
#define MAX_COLUMN_HEIGHT  10
#define CHECKBOX_HEIGHT     1
#define DROPDOWN_HEIGHT     1

class RunnerFormElement;
class RunnerFormListener;

class RunnerForm : public wxFrame {
  RunnerFormElement **formElements_;
  int numElements_;
  wxPanel *mainPanel_;
  wxButton *cancelButton_;
  wxButton *okButton_;
  wxBoxSizer *mainSizer_;
  wxBoxSizer *borderSizer_;
  wxEventFilter *eventFilter_;
  bool done_;
  bool ok_;
  char *message_;
  RunnerFormListener *listener_;

  public:
    RunnerForm(const char *runnerName, RunnerFormElement **formElements,
        int numElements, char **stageNames, int numStages, char **shipNames,
        int numShips, const char *message, RunnerFormListener *listener);
    ~RunnerForm();
    void onCancel(wxCommandEvent &event);
    void cancel();
    void onOk(wxCommandEvent &event);
    void ok();
    void onClose(wxCommandEvent &event);
    void onFormChange(wxUpdateUIEvent &event);
    void onUpdateUi(wxUpdateUIEvent &event);
    bool isDone();
    bool isOk();
    void setMnemonicLabels(bool modifierDown);

  private:
    wxControl* addFormElement(int &colHeight, int &numCols,
        wxBoxSizer *topSizer, wxBoxSizer *&colSizer, const char *name, int type,
        char **stageNames, int numStages, char **shipNames, int numShips,
        char **options, int numOptions);
    wxControl* addSelectElement(const char *name, int type,
        char **stageNames, int numStages, char **shipNames, int numShips,
        wxSizer *colSizer);
    wxControl* addTextElement(const char *name, wxSizer *colSizer);
    wxControl* addCheckboxElement(const char *name, wxSizer *colSizer);
    wxControl* addDropdownElement(const char *name, char **options,
                                  int numOptions, wxSizer *colSizer);
    void addOkCancelElement(wxSizer *colSizer);
    wxStaticText* getNameLabel(const char *name);
    int getHeight(int type);
    void setFormValues(wxControl *control, RunnerFormElement *element);
};

class RunnerFormElement {
  char *name_;
  int type_;
  char** stringValues_;
  int numStringValues_;
  int maxStringValues_;
  int intValue_;
  bool booleanValue_;
  char** options_;
  int numOptions_;
  wxControl *control_;

  public:
    RunnerFormElement(const char *name, int type, int maxStringValues,
                      char **options);
    ~RunnerFormElement();
    const char *getName();
    int getType();
    void addStringValue(const char *value);
    char** getStringValues();
    int getNumStringValues();
    void setIntegerValue(int value);
    int getIntegerValue();
    void setBooleanValue(bool value);
    int getBooleanValue();
    char** getOptions();
    int getNumOptions();
    void clearValues();
    void setControl(wxControl *control);
    wxControl* getControl();
};

class RunnerFormListener {
  public:
    virtual void onUpdateUi() = 0;
    virtual ~RunnerFormListener() {};
};

class RunnerFormEventFilter : public wxEventFilter {
  RunnerForm *runnerForm_;
  
  public:
    RunnerFormEventFilter(RunnerForm *runnerForm);
    ~RunnerFormEventFilter();
    virtual int FilterEvent(wxEvent& event);
};

#endif
