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

#define SELECT_HEIGHT      4
#define TEXT_HEIGHT        1
#define OK_CANCEL_HEIGHT   2
#define MAX_COLUMN_HEIGHT  10

class RunnerFormElement {
  char *name_;
  int type_;
  char** stringValues_;
  int numStringValues_;
  int maxStringValues_;
  int intValue_;
  wxControl *control_;

  public:
    RunnerFormElement(const char *name, int type, int maxStringValues);
    ~RunnerFormElement();
    const char *getName();
    int getType();
    void addStringValue(const char *value);
    char** getStringValues();
    int getNumStringValues();
    void setIntegerValue(int value);
    int getIntegerValue();
    void clearValues();
    void setControl(wxControl *control);
    wxControl* getControl();
};

class RunnerForm : public wxFrame {
  RunnerFormElement **formElements_;
  int numElements_;
  wxPanel *mainPanel_;
  wxButton *cancelButton_;
  wxButton *okButton_;
  wxBoxSizer *mainSizer_;
  wxBoxSizer *borderSizer_;
  bool done_;
  bool ok_;
  char *message_;

  public:
    RunnerForm(const char *runnerName, RunnerFormElement **formElements,
        int numElements, char **stageNames, int numStages, char **shipNames,
        int numShips, const char *message);
    ~RunnerForm();
    void onCancel(wxCommandEvent &event);
    void onOk(wxCommandEvent &event);
    void onClose(wxCommandEvent &event);
    void onFormChange(wxUpdateUIEvent &event);
    bool isDone();
    bool isOk();
  private:
    wxControl* addFormElement(int &colHeight, int &numCols,
        wxBoxSizer *topSizer, wxBoxSizer *&colSizer, const char *name, int type,
        char **stageNames, int numStages, char **shipNames, int numShips);
    void setFormValues(wxControl *control, RunnerFormElement *element);
};

#endif
