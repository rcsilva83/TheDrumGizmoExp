/* -*- Mode: c++ -*- */
/***************************************************************************
 *            ossmidi.cc
 *
 *  Sun May 21 10:52:09 CEST 2017
 *  Copyright 2017 Goran Mekić
 *  meka@tilda.center
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
#include "ossmidi.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <sys/soundcard.h>
#include <unistd.h>

static RealOssWrapper real_oss_wrapper;

OSSInputEngine::OSSInputEngine() : OSSInputEngine(real_oss_wrapper)
{
}

OSSInputEngine::OSSInputEngine(OssWrapper& wrapper)
    : oss_wrapper(wrapper), dev{"/dev/midi"}
{
}

OSSInputEngine::~OSSInputEngine() = default;

bool OSSInputEngine::init(const Instruments& instruments)
{
	if(!loadMidiMap(midimap_file, instruments))
	{
		std::cerr << "[OSSInputEngine] Failed to parse midimap '"
		          << midimap_file << '\n';
		return false;
	}
	fd = oss_wrapper.open_device(dev.data(), O_RDONLY | O_NONBLOCK, 0);
	if(fd == -1)
	{
		std::cerr << dev.data() << ' ' << oss_wrapper.strerror_device(errno)
		          << '\n';
		return false;
	}
	return true;
}

void OSSInputEngine::setParm(const std::string& parm, const std::string& value)
{
	if(parm == "dev")
	{
		dev = value;
	}
	else if(parm == "midimap")
	{
		midimap_file = value;
	}
}

bool OSSInputEngine::start()
{
	return true;
}

void OSSInputEngine::stop()
{
}

void OSSInputEngine::pre()
{
	events.clear();
}

void OSSInputEngine::run(size_t pos, size_t len, std::vector<event_t>& events)
{
	(void)pos;
	(void)len;
	ssize_t l;
	unsigned char buf[128];
	l = oss_wrapper.read_device(fd, buf, sizeof(buf));
	if(l != -1)
	{
		processNote(buf, l,
		    0, // No time information available? play as soon as possible
		    events);
	}
	else if(errno != EAGAIN)
	{
		std::cerr << "Error code: " << errno << '\n';
		std::cerr << oss_wrapper.strerror_device(errno) << '\n';
	}
}

void OSSInputEngine::post()
{
}

bool OSSInputEngine::isFreewheeling() const
{
	return false;
}
