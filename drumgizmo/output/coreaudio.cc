/* -*- Mode: c++ -*- */
/***************************************************************************
 *            coreaudio.cc
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
#include "coreaudio.h"

#include <hugin.hpp>

static const char* errorString(OSStatus err)
{
	const char* err_string = "unknown";
	switch (err) {
	case kAudioHardwareNoError:
		err_string = "kAudioHardwareNoError";
		break;
	case kAudioHardwareNotRunningError:
		err_string = "kAudioHardwareNotRunningError";
		break;
	case kAudioHardwareUnspecifiedError:
		err_string = "kAudioHardwareUnspecifiedError";
		break;
	case kAudioHardwareUnknownPropertyError:
		err_string = "kAudioHardwareUnknownPropertyError";
		break;
	case kAudioHardwareBadPropertySizeError:
		err_string = "kAudioHardwareBadPropertySizeError";
		break;
	case kAudioHardwareIllegalOperationError:
		err_string = "kAudioHardwareIllegalOperationError";
		break;
	case kAudioHardwareBadDeviceError:
		err_string = "kAudioHardwareBadDeviceError";
		break;
	case kAudioHardwareBadStreamError:
		err_string = "kAudioHardwareBadStreamError";
		break;
	case kAudioDeviceUnsupportedFormatError:
		err_string = "kAudioDeviceUnsupportedFormatError";
		break;
	case kAudioDevicePermissionsError:
		err_string = "kAudioDevicePermissionsError";
		break;
	default:
		break;
	}

	return err_string;
}


CoreAudioOutputEngine::CoreAudioOutputEngine()
{
	OSStatus err;
	std::uint32_t size;

	size = sizeof(AudioDeviceID);
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
	                               &size, &device_id);
	if(err != noErr)
	{
		ERR(coreaudio, "Error kAudioHardwarePropertyDefaultOutputDevice: %s",
		    errorString(err));
	}

	char device_name[256];
	memset(device_name, 0, sizeof(device_name));
	size = sizeof(device_name) - 1; // leave space for terminating zero
	err = AudioDeviceGetProperty(device_id, 0, false,
	                             kAudioDevicePropertyDeviceName,
	                             &size, device_name);
	if(err != noErr)
	{
		ERR(coreaudio, "Error kAudioDevicePropertyDeviceName: %s",
		    errorString(err));
	}

	DEBUG(coreaudio, "default device id: %d (%s)", device_id, device_name);
}

CoreAudioOutputEngine::~CoreAudioOutputEngine()
{
}

bool CoreAudioOutputEngine::init(const Channels& channels)
{
	return true;
}

void CoreAudioOutputEngine::setParm(const std::string& parm,
                                    const std::string& value)
{
}

bool CoreAudioOutputEngine::start()
{
	return true;
}

void CoreAudioOutputEngine::stop()
{
}

void CoreAudioOutputEngine::pre(size_t nsamples)
{
}

void CoreAudioOutputEngine::run(int ch, sample_t* samples, size_t nsamples)
{
	// Write channel data in interleaved buffer
}

void CoreAudioOutputEngine::post(size_t nsamples)
{
	// Write the interleaved buffer to the soundcard
}

size_t CoreAudioOutputEngine::getSamplerate() const
{
	return 0;
}

bool CoreAudioOutputEngine::isFreewheeling() const
{
	return false;
}
