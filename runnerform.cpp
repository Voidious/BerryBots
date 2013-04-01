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

#include <string.h>
#include "runnerform.h"

RunnerFormElement::RunnerFormElement(const char *name, int type,
                                     int maxStringValues) {
  name_ = new char[strlen(name) + 1];
  strcpy(name_, name);
  type_ = type;
  numStringValues_ = 0;
  maxStringValues_ = maxStringValues;
  stringValues_ = new char*[maxStringValues_];
  defaultStringValues_ = new char*[maxStringValues_];
  intValue_ = 0;
  defaultIntValue_ = 0;
}

RunnerFormElement::~RunnerFormElement() {
  delete name_;
  for (int x = 0; x < numStringValues_; x++) {
    delete stringValues_[x];
  }
  delete stringValues_;
  for (int x = 0; x < numDefaultStringValues_; x++) {
    delete defaultStringValues_[x];
  }
  delete defaultStringValues_;
}

const char* RunnerFormElement::getName() {
  return name_;
}

int RunnerFormElement::getType() {
  return type_;
}

void RunnerFormElement::addStringValue(const char *value) {
  if (numStringValues_ < maxStringValues_) {
    char *newValue = new char[strlen(value) + 1];
    strcpy(newValue, value);
    stringValues_[numStringValues_++] = newValue;
  }
}

char** RunnerFormElement::getStringValues() {
  return stringValues_;
}

int RunnerFormElement::getNumStringValues() {
  return numStringValues_;
}

void RunnerFormElement::addDefaultStringValue(const char *value) {
  if (numDefaultStringValues_ < maxStringValues_) {
    char *newValue = new char[strlen(value) + 1];
    strcpy(newValue, value);
    defaultStringValues_[numDefaultStringValues_++] = newValue;
  }
}

char** RunnerFormElement::getDefaultStringValues() {
  return defaultStringValues_;
}

int RunnerFormElement::getNumDefaultStringValues() {
  return numDefaultStringValues_;
}

void RunnerFormElement::setIntegerValue(int value) {
  intValue_ = value;
}

int RunnerFormElement::getIntegerValue() {
  return intValue_;
}

void RunnerFormElement::setDefaultIntegerValue(int value) {
  defaultIntValue_ = value;
}

int RunnerFormElement::getDefaultIntegerValue() {
  return defaultIntValue_;
}

void RunnerFormElement::clearDefaults() {
  for (int x = 0; x < numDefaultStringValues_; x++) {
    delete defaultStringValues_[x];
  }
  numDefaultStringValues_ = 0;
  defaultIntValue_ = 0;
}
