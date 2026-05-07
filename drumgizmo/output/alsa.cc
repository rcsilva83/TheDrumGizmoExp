/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsa.cc
 *
 *  Do 21. Jan 16:48:32 CET 2016
 *  Copyright 2016 Christian Glöckner
 *  cgloeckner@freenet.de
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
#include <iostream>

#include "alsa.h"

int const BUFFER_SIZE = 40960;

struct AlsaInitError
{
	int const code;
	const std::string msg;

	AlsaInitError(int op_code, const std::string& msg) : code{op_code}, msg{msg}
	{
	}

	static inline void test(int code, const std::string& msg)
	{
		if(code < 0)
		{
			throw AlsaInitError(code, msg);
		}
	}
};

static RealAlsaPcmWrapper real_pcm_wrapper;

AlsaOutputEngine::AlsaOutputEngine() : AlsaOutputEngine(real_pcm_wrapper)
{
}

AlsaOutputEngine::AlsaOutputEngine(AlsaPcmWrapper& wrapper)
    : pcm_wrapper(wrapper)
    , handle{nullptr}
    , data{}
    , num_channels{0u}
    , dev{"default"}
    , srate{44100}
    , frames{32}
    , periods{3}
{
}

AlsaOutputEngine::~AlsaOutputEngine()
{
	if(handle != nullptr)
	{
		pcm_wrapper.close(handle);
	}
}

bool AlsaOutputEngine::init(const Channels& channels)
{
	// try to initialize alsa
	try
	{
		int value =
		    pcm_wrapper.open(&handle, dev.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
		AlsaInitError::test(value, "snd_pcm_open");
		num_channels = channels.size();
		if(handle == nullptr)
		{
			std::cerr << "[AlsaOutputEngine] Failed to acquire "
			          << "hardware handle\n";
			return false;
		}
		value = pcm_wrapper.configure_hw(
		    handle, num_channels, &srate, &frames, &periods);
		AlsaInitError::test(value, "configure_hw");
	}
	catch(AlsaInitError const& error)
	{
		std::cerr << "[AlsaOutputEngine] " << error.msg
		          << " failed: " << pcm_wrapper.strerror(error.code)
		          << std::endl;
		return false;
	}

	data.clear();
	data.resize(BUFFER_SIZE * num_channels);

	return true;
}

void AlsaOutputEngine::setParm(
    const std::string& parm, const std::string& value)
{
	if(parm == "dev")
	{
		// apply hardware device name
		dev = value;
	}
	else if(parm == "frames")
	{
		// try to apply hardware buffer size
		try
		{
			frames = std::stoi(value);
		}
		catch(...)
		{
			std::cerr << "[AlsaOutputEngine] Invalid buffer size " << value
			          << "\n";
		}
	}
	else if(parm == "periods")
	{
		// try to apply number of periods
		try
		{
			periods = std::stoi(value);
		}
		catch(...)
		{
			std::cerr << "[AlsaOutputEngine] Invalid number of periods "
			          << value << "\n";
		}
	}
	else if(parm == "srate")
	{
		try
		{
			srate = std::stoi(value);
		}
		catch(...)
		{
			std::cerr << "[AlsaOutputEngine] Invalid samplerate " << value
			          << "\n";
		}
	}
	else
	{
		std::cerr << "[AlsaOutputEngine] Unsupported parameter '" << parm
		          << "'\n";
	}
}

bool AlsaOutputEngine::start()
{
	return true;
}

void AlsaOutputEngine::stop()
{
}

void AlsaOutputEngine::pre(size_t nsamples)
{
	(void)nsamples;
}

void AlsaOutputEngine::run(int ch, sample_t* samples, size_t nsamples)
{
	// Write channel data in interleaved buffer
	for(auto i = 0u; i < nsamples; ++i)
	{
		data[i * num_channels + ch] = samples[i];
	}
}

void AlsaOutputEngine::post(size_t nsamples)
{
	// Write the interleaved buffer to the soundcard
	snd_pcm_sframes_t value = pcm_wrapper.writei(handle, data.data(), nsamples);

	if(value == -EPIPE) // under-run
	{
		pcm_wrapper.prepare(handle);
	}
	else if(value == -ESTRPIPE)
	{
		while((value = pcm_wrapper.resume(handle)) == -EAGAIN)
		{
			sleep(1); // wait until the suspend flag is released
		}
		if(value < 0)
		{
			pcm_wrapper.prepare(handle);
		}
	}
}

size_t AlsaOutputEngine::getBufferSize() const
{
	return frames;
}

size_t AlsaOutputEngine::getSamplerate() const
{
	return srate;
}

bool AlsaOutputEngine::isFreewheeling() const
{
	return false;
}
