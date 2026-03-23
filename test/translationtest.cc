/* -*- Mode: c++ -*- */
/***************************************************************************
 *            translationtest.cc
 *
 *  Sun Sep  8 14:51:08 CEST 2019
 *  Copyright 2019 Bent Bisballe Nyeng
 *  deva@aasimon.org
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
#include <doctest/doctest.h>

#include <locale>

#include <translation.h>
#include <dggui/uitranslation.h>
#include <stdlib.h>

TEST_CASE("TranslationTest")
{
SUBCASE("testFromFile")
{
Translation t;
char buf[100000];
FILE* fp = fopen(MO_SRC, "r");
CHECK(fp != nullptr);
auto sz = fread(buf, 1, sizeof(buf), fp);
fclose(fp);
CHECK(t.load(buf, sz));

// Look up translation from .mo file
CHECK_EQ(std::string("Trommes\xc3\xb8t"),
          std::string(_("Drumkit")));

// No translation, return key
CHECK_EQ(std::string("No translation"),
          std::string(_("No translation")));
}

SUBCASE("testFromLocale")
{
#ifdef _WIN32
_putenv_s("LANG", "da_DK.UTF-8");
#else
setenv("LANG", "da_DK.UTF-8", 1);
#endif
dggui::UITranslation t;

// Look up translation from .mo file
CHECK_EQ(std::string("Trommes\xc3\xb8t"),
          std::string(_("Drumkit")));

// No translation, return key
CHECK_EQ(std::string("No translation"),
          std::string(_("No translation")));
}
}
