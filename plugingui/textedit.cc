/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            lineedit.cc
 *
 *  Tue Oct 21 11:25:26 CEST 2014
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
#include "textedit.h"

#include <stdio.h>

#include "window.h"

#include <assert.h>

#include <list>

#include <hugin.hpp>

#define BORDER 10

static void scrolled(void *ptr)
{
  GUI::TextEdit *l = (GUI::TextEdit *)ptr;
  l->repaintEvent(NULL);
}

GUI::TextEdit::TextEdit(Widget *parent)
  : GUI::Widget(parent), scroll(this)
{
  pos = 0;
  setReadOnly(true);

  scroll.move(width()-5,1);
  scroll.resize(20, 100);
  scroll.registerValueChangeHandler(scrolled, this);

  box.topLeft     = new Image(":widget_tl.png");
  box.top         = new Image(":widget_t.png");
  box.topRight    = new Image(":widget_tr.png");
  box.left        = new Image(":widget_l.png");
  box.right       = new Image(":widget_r.png");
  box.bottomLeft  = new Image(":widget_bl.png");
  box.bottom      = new Image(":widget_b.png");
  box.bottomRight = new Image(":widget_br.png");
  box.center      = new Image(":widget_c.png");

  handler = NULL;
}

void GUI::TextEdit::resize(int height, int width)
{
  Widget::resize(height, width);
  scroll.resize(scroll.width(), height-10);
  scroll.move(width-30,7);
}

void GUI::TextEdit::setReadOnly(bool ro)
{
  readonly = ro;
}

bool GUI::TextEdit::readOnly()
{
  return readonly;
}

void GUI::TextEdit::setText(std::string text)
{
  _text = text;
  preprocessText();
  int ran = height() / font.textHeight();
  DEBUG(textedit, "Setting range and max of scrollbar"
                  " to '%d' and '%d'\n", ran, preprocessedtext.size());
  scroll.setRange(ran);
  scroll.setMaximum(preprocessedtext.size());
  repaintEvent(NULL);
  textChanged();
}

std::string GUI::TextEdit::text()
{
  return _text;
}

//void GUI::TextEdit::buttonEvent(ButtonEvent *e)
//{
//  if(readOnly()) return;
//}

//void GUI::TextEdit::keyEvent(GUI::KeyEvent *e)
//{
//  if(readOnly()) return;
//}

void GUI::TextEdit::preprocessText()
{
  preprocessedtext.clear();
  std::string text = _text;

  { // Handle tan characters
    for(size_t i = 0; i < text.length(); i++) {
      char ch = text.at(i);
      if(ch == '\t') {
        text.erase(i, 1);
        text.insert(i, 4, ' ');
      }
    }
  }

  std::list<std::string> lines;
  { // Handle new line characters
   size_t pos = 0;
   do {
     pos = text.find("\n");
     lines.push_back(text.substr(0, pos));
     text = text.substr(pos+1);
   }
   while(pos != std::string::npos);
  }

  { // Wrap long lines
    std::list<std::string>::iterator it;
    for(it = lines.begin(); it != lines.end(); it++) {
      std::string line = *it;

      for(size_t i = 0; i < line.length(); i++) {
        size_t linewidth = font.textWidth(line.substr(0, i));
        if(linewidth >= width() - BORDER - 4 + 3 - 10 - scroll.width()) {
          preprocessedtext.push_back(line.substr(0, i));
          line = line.substr(i);
          i = 0;
        }
      }
      preprocessedtext.push_back(line);
    }
  }
}

void GUI::TextEdit::repaintEvent(GUI::RepaintEvent *e)
{
  Painter p(this);

  p.clear();

  int w = width();
  int h = height();
  if(w == 0 || h == 0) return;
  p.drawBox(0, 0, &box, w, h);

  p.setColour(GUI::Colour(183.0/255.0, 219.0/255.0 , 255.0/255.0, 1));

  int skip = scroll.value();

  int ypos = font.textHeight() + 5 + 1 + 1 + 1;
  std::list<std::string>::iterator it;
  it = preprocessedtext.begin();

  int c = 0;
  for( ; c < skip; c++) {
    it++;
  }

  c = 0;
  for( ; it != preprocessedtext.end(); it++) {
    if(c * font.textHeight() >= height() - 8 - font.textHeight()) break;
    std::string line = *it;
    p.drawText(BORDER - 4 + 3, ypos, font, line);
    ypos += font.textHeight();
    c++;
  }
}

#ifdef TEST_TEXTEDIT
//Additional dependency files
//deps:
//Required cflags (autoconf vars may be used)
//cflags:
//Required link options (autoconf vars may be used)
//libs:
#include "test.h"

TEST_BEGIN;

// TODO: Put some testcode here (see test.h for usable macros).

TEST_END;

#endif/*TEST_TEXTEDIT*/
