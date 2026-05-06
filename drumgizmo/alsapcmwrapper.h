/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsapcmwrapper.h
 *
 *  Wed May  6 10:00:00 CEST 2026
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

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

class AlsaPcmWrapper
{
public:
	virtual ~AlsaPcmWrapper() = default;

	virtual int open(snd_pcm_t** handle, const char* name, int stream,
	                 int mode) = 0;
	virtual void close(snd_pcm_t* handle) = 0;
	virtual int configure_hw(snd_pcm_t* handle, unsigned int channels,
	                         unsigned int* srate, snd_pcm_uframes_t* frames,
	                         unsigned int* periods) = 0;
	virtual snd_pcm_sframes_t writei(snd_pcm_t* handle, const void* buffer,
	                                 snd_pcm_uframes_t size) = 0;
	virtual int prepare(snd_pcm_t* handle) = 0;
	virtual int resume(snd_pcm_t* handle) = 0;
	virtual const char* strerror(int errnum) = 0;
};

class RealAlsaPcmWrapper : public AlsaPcmWrapper
{
public:
	int open(snd_pcm_t** handle, const char* name, int stream,
	         int mode) override
	{
		return snd_pcm_open(handle, name, (snd_pcm_stream_t)stream, mode);
	}
	void close(snd_pcm_t* handle) override
	{
		snd_pcm_close(handle);
	}
	int configure_hw(snd_pcm_t* handle, unsigned int channels,
	                 unsigned int* srate, snd_pcm_uframes_t* frames,
	                 unsigned int* periods) override
	{
		snd_pcm_hw_params_t* params;
		int value;
		snd_pcm_hw_params_alloca(&params);
		value = snd_pcm_hw_params_any(handle, params);
		if(value < 0)
		{
			return value;
		}
		value = snd_pcm_hw_params_set_access(handle, params,
		    SND_PCM_ACCESS_RW_INTERLEAVED);
		if(value < 0)
		{
			return value;
		}
		value = snd_pcm_hw_params_set_format(handle, params,
		    SND_PCM_FORMAT_FLOAT);
		if(value < 0)
		{
			return value;
		}
		value = snd_pcm_hw_params_set_channels(handle, params, channels);
		if(value < 0)
		{
			return value;
		}
		value = snd_pcm_hw_params_set_rate_near(handle, params, srate, 0);
		if(value < 0)
		{
			return value;
		}
		value = snd_pcm_hw_params_set_period_size_near(
		    handle, params, frames, 0);
		if(value < 0)
		{
			return value;
		}
		value = snd_pcm_hw_params_set_periods_near(
		    handle, params, periods, 0);
		if(value < 0)
		{
			return value;
		}
		value = snd_pcm_hw_params(handle, params);
		return value;
	}
	snd_pcm_sframes_t writei(snd_pcm_t* handle, const void* buffer,
	                         snd_pcm_uframes_t size) override
	{
		return snd_pcm_writei(handle, buffer, size);
	}
	int prepare(snd_pcm_t* handle) override
	{
		return snd_pcm_prepare(handle);
	}
	int resume(snd_pcm_t* handle) override
	{
		return snd_pcm_resume(handle);
	}
	const char* strerror(int errnum) override
	{
		return snd_strerror(errnum);
	}
};
