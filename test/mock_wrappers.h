/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            mock_wrappers.h
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

#include <config.h>

#ifdef HAVE_INPUT_ALSAMIDI
#include "../drumgizmo/alsaseqwrapper.h"

#include <alsa/asoundlib.h>

class MockAlsaSeqWrapper : public AlsaSeqWrapper
{
public:
	int open_return{0};
	int set_client_name_return{0};
	int create_simple_port_return{0};
	int event_input_return{-1};
	snd_seq_event_t event_data{};

	int open(snd_seq_t** seq, const char*, int, int) override
	{
		*seq = reinterpret_cast<snd_seq_t*>(0x1);
		return open_return;
	}
	void close(snd_seq_t*) override
	{
	}
	int set_client_name(snd_seq_t*, const char*) override
	{
		return set_client_name_return;
	}
	int create_simple_port(snd_seq_t*, const char*, int, int) override
	{
		return create_simple_port_return;
	}
	int event_input(snd_seq_t*, snd_seq_event_t** ev) override
	{
		if(event_input_return >= 0)
		{
			*ev = &event_data;
		}
		else
		{
			*ev = nullptr;
		}
		return event_input_return;
	}
	void free_event(snd_seq_event_t*) override
	{
	}
	const char* strerror(int) override
	{
		return "mock error";
	}
};
#endif // HAVE_INPUT_ALSAMIDI

#ifdef HAVE_OUTPUT_ALSA
#include "../drumgizmo/alsapcmwrapper.h"

class MockAlsaPcmWrapper : public AlsaPcmWrapper
{
public:
	int open_return{0};
	int configure_hw_return{0};
	snd_pcm_sframes_t writei_return{0};
	int prepare_return{0};
	int resume_return{0};

	int open(snd_pcm_t** handle, const char*, int, int) override
	{
		*handle = reinterpret_cast<snd_pcm_t*>(0x1);
		return open_return;
	}
	void close(snd_pcm_t*) override
	{
	}
	int configure_hw(snd_pcm_t*, unsigned int, unsigned int*,
	    snd_pcm_uframes_t*, unsigned int*) override
	{
		return configure_hw_return;
	}
	snd_pcm_sframes_t writei(
	    snd_pcm_t*, const void*, snd_pcm_uframes_t) override
	{
		return writei_return;
	}
	int prepare(snd_pcm_t*) override
	{
		return prepare_return;
	}
	int resume(snd_pcm_t*) override
	{
		return resume_return;
	}
	const char* strerror(int) override
	{
		return "mock error";
	}
};
#endif // HAVE_OUTPUT_ALSA

#ifdef HAVE_INPUT_OSS
#include "../drumgizmo/osswrapper.h"

class MockOssWrapper : public OssWrapper
{
public:
	int open_return{-1};
	int close_return{0};
	ssize_t read_return{-1};
	ssize_t write_return{0};
	int ioctl_return{-1};

	int open_device(const char*, int, mode_t) override
	{
		return open_return;
	}
	int close_device(int) override
	{
		return close_return;
	}
	ssize_t read_device(int, void*, size_t) override
	{
		return read_return;
	}
	ssize_t write_device(int, const void*, size_t) override
	{
		return write_return;
	}
	int ioctl_device(int, unsigned long, void*) override
	{
		return ioctl_return;
	}
	const char* strerror_device(int) override
	{
		return "mock error";
	}
};
#endif // HAVE_INPUT_OSS
