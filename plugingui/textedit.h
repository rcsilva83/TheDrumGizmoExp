/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            textedit.h
 *
 *  Tue Oct 21 11:23:58 CEST 2014
 *  Copyright 2014 Jonas Suhr Christensen
 *  jsc@umbraculum.org
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#ifndef __DRUMGIZMO_TEXTEDIT_H__
#define __DRUMGIZMO_TEXTEDIT_H__

#include <string>

#include <list>

#include "widget.h"
#include "font.h"
#include "painter.h"
#include "scrollbar.h"

namespace GUI {

class TextEdit : public Widget {
public:
  TextEdit(Widget *parent);

  bool isFocusable() { return true; }

  std::string text();
  void setText(std::string text);

  void setReadOnly(bool readonly);
  bool readOnly();

  void resize(int width, int height);


  void preprocessText();

  //protected:
//  virtual void keyEvent(KeyEvent *e);
  virtual void repaintEvent(RepaintEvent *e);
//  virtual void buttonEvent(ButtonEvent *e);

protected:
  virtual void textChanged() {}

private:
  Painter::Box box;
  ScrollBar scroll;
  Font font;

  std::string _text;
  size_t pos;

  bool readonly;
  
  std::list< std::string > preprocessedtext;
//  size_t numoflines;

  void (*handler)(void *);
  void *ptr;
};

};

#endif/*__DRUMGIZMO_TEXTEDIT_H__*/
