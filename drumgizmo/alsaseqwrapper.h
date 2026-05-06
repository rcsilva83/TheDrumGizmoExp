/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsaseqwrapper.h
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

#include <alsa/asoundlib.h>

class AlsaSeqWrapper
{
public:
	virtual ~AlsaSeqWrapper() = default;

	virtual int open(snd_seq_t** seq, const char* name, int streams,
	                 int mode) = 0;
	virtual void close(snd_seq_t* seq) = 0;
	virtual int set_client_name(snd_seq_t* seq, const char* name) = 0;
	virtual int create_simple_port(snd_seq_t* seq, const char* name, int caps,
	                               int type) = 0;
	virtual int event_input(snd_seq_t* seq, snd_seq_event_t** ev) = 0;
	virtual void free_event(snd_seq_event_t* ev) = 0;
	virtual const char* strerror(int errnum) = 0;
};

class RealAlsaSeqWrapper : public AlsaSeqWrapper
{
public:
	int open(snd_seq_t** seq, const char* name, int streams,
	         int mode) override
	{
		return snd_seq_open(seq, name, streams, mode);
	}
	void close(snd_seq_t* seq) override
	{
		snd_seq_close(seq);
	}
	int set_client_name(snd_seq_t* seq, const char* name) override
	{
		return snd_seq_set_client_name(seq, name);
	}
	int create_simple_port(snd_seq_t* seq, const char* name, int caps,
	                       int type) override
	{
		return snd_seq_create_simple_port(seq, name, caps, type);
	}
	int event_input(snd_seq_t* seq, snd_seq_event_t** ev) override
	{
		return snd_seq_event_input(seq, ev);
	}
	void free_event(snd_seq_event_t* ev) override
	{
		snd_seq_free_event(ev);
	}
	const char* strerror(int errnum) override
	{
		return snd_strerror(errnum);
	}
};
