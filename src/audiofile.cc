/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            audiofile.cc
 *
 *  Tue Jul 22 17:14:11 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
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
 *
 *  Multichannel feature by John Hammen copyright 2014
 */
#include "audiofile.h"

#include <cassert>
#include <sndfile.h>
#include <array>

#include <config.h>

#include <hugin.hpp>

#include "channel.h"

AudioFile::AudioFile(std::string filename, std::size_t filechannel,
                     InstrumentChannel* instrument_channel)
	: filename(std::move(filename))
	, filechannel(filechannel)
	, magic{this}
	, instrument_channel(instrument_channel)
{
}

AudioFile::~AudioFile()
{
	magic = nullptr;
	unload();
}

bool AudioFile::isValid() const
{
	//assert(this == magic);
	return this == magic;
}

void AudioFile::unload()
{
	// Make sure we don't unload the object while loading it...
	const std::lock_guard<std::mutex> guard(mutex);

	load_state.store(LoadState::not_loaded);

	preloadedsize = 0;
	size = 0;
	delete[] data;
	data = nullptr;
}

constexpr std::size_t buffer_size{4096};

void AudioFile::load(const LogFunction& logger, std::size_t sample_limit)
{
	// Make sure we don't unload the object while loading it...
	const std::lock_guard<std::mutex> guard(mutex);

	if(this->data != nullptr) // already loaded
	{
		return;
	}

	load_state.store(LoadState::loading);

	SF_INFO sf_info{};
	SNDFILE *file_handle = sf_open(filename.c_str(), SFM_READ, &sf_info);
	if(file_handle == nullptr)
	{
		ERR(audiofile,"SNDFILE Error (%s): %s\n",
		    filename.c_str(), sf_strerror(file_handle));
		if(logger)
		{
			logger(LogLevel::Warning, "Could not load '" + filename +
			       "': " + sf_strerror(file_handle));
		}
		load_state.store(LoadState::failed);
		return;
	}

	if(sf_info.channels < 1)
	{
		// This should never happen but lets check just in case.
		if(logger)
		{
			logger(LogLevel::Warning, "Could not load '" + filename +
			       "': no audio channels available.");
		}
		load_state.store(LoadState::failed);
		return;
	}

	const std::size_t size = sf_info.frames;
	std::size_t preloadedsize = sf_info.frames;

	if(preloadedsize > sample_limit)
	{
		preloadedsize = sample_limit;
	}

	gsl::owner<sample_t*> data{};
	data = new sample_t[preloadedsize];
	if(sf_info.channels == 1)
	{
		preloadedsize =
			sf_read_float(file_handle, data, static_cast<sf_count_t>(preloadedsize));
	}
	else
	{
		// check filechannel exists
		if(filechannel >= (std::size_t)sf_info.channels)
		{
			if(logger)
			{
				logger(LogLevel::Warning, "Audio file '" + filename +
				       "' does no have " + std::to_string(filechannel + 1) + " channels.");
			}
			filechannel = sf_info.channels - 1;
		}

		std::array<sample_t, buffer_size> buffer{};
		const sf_count_t frame_count{static_cast<sf_count_t>(buffer.size()) /
		                             sf_info.channels};
		std::size_t total_frames_read{};
		std::size_t frames_read{};

		do
		{
			frames_read = static_cast<std::size_t>(
				sf_readf_float(file_handle, buffer.data(), frame_count));
			for(std::size_t i = 0;
	        (i < frames_read) && (total_frames_read < sample_limit);
	        ++i)
	    {
		    data[total_frames_read++] =
			    buffer[i * sf_info.channels + filechannel]; // NOLINT - span
	    }
		}
		while( (frames_read > 0) &&
		       (total_frames_read < preloadedsize) &&
		       (total_frames_read < sample_limit) );

		// set data size to total bytes read
		preloadedsize = total_frames_read;
	}

	sf_close(file_handle);

	this->data = data;
	this->size = size;
	this->preloadedsize = preloadedsize;
	load_state.store(LoadState::loaded);
}

bool AudioFile::isLoaded() const
{
	return load_state.load() == LoadState::loaded;
}

bool AudioFile::isLoading() const
{
	auto state = load_state.load();
	return
		state == LoadState::loading || // in progress
		state == LoadState::not_loaded // not yet started loading
		;
}

main_state_t AudioFile::mainState() const
{
	if(instrument_channel == nullptr)
	{
		DEBUG(audiofile, "no instrument_channel!");
		return main_state_t::unset;
	}

	return instrument_channel->main;
}
