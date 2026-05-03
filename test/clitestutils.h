/* -*- Mode: c++ -*- */
/***************************************************************************
 *            clitestutils.h
 *
 *  Sun May  3 2026
 *  Copyright 2026 DrumGizmo team
 *
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
#pragma once

#include "scopedfile.h"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#ifndef _WIN32
#include <sys/wait.h>
#endif

struct CommandResult
{
	int exit_code;
	std::string output;
};

static std::string shellEscape(const std::string& arg)
{
#ifndef _WIN32
	std::string escaped = "'";
	for(const auto ch : arg)
	{
		if(ch == '\'')
		{
			escaped += "'\\''";
		}
		else
		{
			escaped += ch;
		}
	}
	escaped += "'";
	return escaped;
#else
	std::string escaped = "\"";
	for(const auto ch : arg)
	{
		if(ch == '"')
		{
			escaped += "\\\"";
		}
		else
		{
			escaped += ch;
		}
	}
	escaped += "\"";
	return escaped;
#endif
}

static CommandResult runCommand(const std::string& binary,
                                const std::vector<std::string>& args)
{
	std::string command = shellEscape(binary);
	for(const auto& arg : args)
	{
		command += " ";
		command += shellEscape(arg);
	}

	ScopedFile output_file("");
	command += " >";
	command += shellEscape(output_file.filename());
	command += " 2>&1";

	auto status = std::system(command.c_str());
	int exit_code = status;
#ifndef _WIN32
	if(WIFEXITED(status))
	{
		exit_code = WEXITSTATUS(status);
	}
#endif

	std::ifstream stream(output_file.filename());
	std::string output((std::istreambuf_iterator<char>(stream)),
	                   std::istreambuf_iterator<char>());

	return {exit_code, output};
}
