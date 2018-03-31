/* -*- Mode: c++ -*- */
/***************************************************************************
 *            coreaudio.h
 *
 *  Sat Oct 21 18:27:52 CEST 2017
 *  Copyright 2017 Bent Bisballe Nyeng
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
#pragma once

#include <audiotypes.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioUnit/AUComponent.h>

#include "audiooutputengine.h"

class CoreAudioOutputEngine
	: public AudioOutputEngine
{
public:
	CoreAudioOutputEngine();
	~CoreAudioOutputEngine();

	// based on AudioOutputEngine
	bool init(const Channels& chan) override;
	void setParm(const std::string& parm, const std::string& value) override;
	bool start() override;
	void stop() override;
	void pre(size_t nsamples) override;
	void run(int ch, sample_t* samples, size_t nsamples) override;
	void post(size_t nsamples) override;
	size_t getSamplerate() const override;
	bool isFreewheeling() const override;

private:
	AudioDeviceID device_id{kAudioDeviceUnknown};
	std::string id;
	std::uint32_t frames{1024u};
	std::uint32_t samplerate{44100u};
	AudioUnit au_hal;
	AudioBufferList* input_list;

	// libao:
static OSStatus audioCallback (void *inRefCon, 
                                               AudioUnitRenderActionFlags *inActionFlags,
                                               const AudioTimeStamp *inTimeStamp, 
                                               UInt32 inBusNumber, 
                                               UInt32 inNumberFrames,
                                                      AudioBufferList *ioData);

	AudioDeviceID                outputDevice;
  ComponentInstance            outputAudioUnit;
  int                          output_p;

  /* Keep track of whether the output stream has actually been
     started/stopped */
  Boolean                      started;
  Boolean                      isStopping;

  /* Our internal queue of samples waiting to be consumed by
     CoreAudio */
  void                        *buffer;
  unsigned int                 bufferByteCount;
  unsigned int                 firstValidByteOffset;
  unsigned int                 validByteCount;

  unsigned int                 buffer_time;
};
