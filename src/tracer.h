/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            tracer.h
 *
 *  Sun Dec 29 18:04:28 CET 2019
 *  Copyright 2019 Daniel Pastor
 *  marlborometal@gmail.com
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

#include <fstream>
#include <memory>
#include <stdexcept>

//! This file contains the trace tool for developmnent purposes. The trace tool
//! just prints messages to a given file.
namespace tracer 
{

//! This class takes care of the output stream where trace messages will be
//! printed. Its methods are not supposed to be called by anyone else than the
//! "trace" functions.
class tracer_manager 
{
	public:

	//! Returns a reference to the file stream.
	static std::ofstream& get()
	{
		if(nullptr==outfile.get()) 
		{
//TODO: This should not be fixed, but either depend on the OS or be able to
//specified somehow.
			outfile.reset(new std::ofstream("/tmp/drumgizmo.trace", 
								std::ios::app));
		}

		if(nullptr==outfile.get())
		{
			throw std::runtime_error("the tracer file could not be initialized");
		}

		return *(outfile.get());
	}

	private:

	//! Single output file. Will be initialized to nullptr when the program
	//! starts and freed when the program ends.
	static std::unique_ptr<std::ofstream> outfile;	
};

//! Trace function without parameters, used on the last call to the variadic
//! templace "trace".
void trace();

//! Variadic trace function. Obtains a reference to the file stream and prints
//! its arguments there.
template<typename T, typename... Args> 
void trace(T val, Args... args)
{
	tracer_manager::get()<<val;
	trace(args...);
}

}//End of tracer namespace.