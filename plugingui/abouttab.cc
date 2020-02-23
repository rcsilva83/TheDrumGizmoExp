/* -*- Mode: c++ -*- */
/***************************************************************************
 *            abouttab.cc
 *
 *  Fri Apr 21 18:51:13 CEST 2017
 *  Copyright 2017 André Nusser
 *  andre.nusser@googlemail.com
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "abouttab.h"

#include <version.h>

#include "utf8.h"

namespace GUI
{

//!Simple helper class to avoid direct repeats of text manipulation.
class AboutTextBuilder {
public:
	//!Class constructor. Sets the separator character.
	AboutTextBuilder(char _separator='=')
    :separator_character(_separator)
	{
	}

	//!Adds a new section to the "about" text with the given title and contents.
	//!Returns a reference to the builder so more calls can be chained.
	AboutTextBuilder& add(const std::string& _title, const std::string& _contents) 
	{
		auto repeat=[](std::string& _str, char _fill, size_t _count) 
		{
			_str.insert(_str.size(), _count, _fill);
		};

		repeat(about_text, separator_character, separator_count);
		about_text+="\n";
		repeat(about_text, ' ', space_count);
		about_text+=_title+"\n";
		repeat(about_text, separator_character, separator_count);
		repeat(about_text, '\n', 3);
		about_text+=_contents+"\n";

		return *this;
	}

	//!Returns the built string.
	const std::string& get() const 
	{
		return about_text;
	}

private:
	const size_t separator_count=13; //!< Number of times that the separator character will be added. 
	const size_t space_count=13; //!< Number of times that the space character will be added before the title.
	char separator_character; //!< Separator character. Defaults to '='.
	std::string about_text; //!< Text that will be filled up with calls to "add".
};

AboutTab::AboutTab(Widget* parent)
	: Widget(parent)
{
	text_edit.setText(getAboutText());
	text_edit.setReadOnly(true);
	text_edit.resize(std::max((int)width() - 2*margin,0),
	                 std::max((int)height() - 2*margin, 0));
	text_edit.move(margin, margin);
}

void AboutTab::resize(std::size_t width, std::size_t height)
{
	Widget::resize(width, height);
	text_edit.resize(std::max((int)width - 2*margin, 0),
	                 std::max((int)height - 2*margin, 0));
}

std::string AboutTab::getAboutText()
{
	AboutTextBuilder builder('*');

	//This will casually add an extra newline at the end of License.
	return builder.add("About", about.data())
	    .add("Version", std::string(VERSION))
	    .add("Bugs", bugs.data())
	    .add("Authors", UTF8().toLatin1(authors.data()))
	    .add("License", gpl.data())
	    .get();
}

} // GUI::
