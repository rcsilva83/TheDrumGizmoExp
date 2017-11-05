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

#include <iostream>

#include <hugin.hpp>
#include <assert.h>

static const char* errorString(OSStatus err)
{
	const char* err_string = "unknown";
	switch(err)
	{
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

static std::vector<AudioDeviceID> getDeviceList()
{
	OSStatus err;
	Boolean is_writable;

	// Get number of devices in device list
	UInt32 size;
	err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices,
	                                   &size, &is_writable);
	if(err != noErr)
	{
		ERR(coreaudio, "Error kAudioHardwarePropertyDevices: %s",
		    errorString(err));
		return {};
	}

	if(size == 0)
	{
		return {};
	}

	std::size_t number_of_devices = size / sizeof(AudioDeviceID);

	// Allocate vector for devices.
	std::vector<AudioDeviceID> devices;
	devices.resize(number_of_devices);

	err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size,
	                               devices.data());
	if(err != noErr)
	{
		ERR(coreaudio, "Error kAudioHardwarePropertyDevices: %s",
		    errorString(err));
		return {};
	}

	return devices;
}

static std::string getDeviceName(AudioDeviceID device_id)
{
	OSStatus err;

	char device_name[256];
	memset(device_name, 0, sizeof(device_name));
	UInt32 size = sizeof(device_name) - 1; // leave space for terminating zero
	err = AudioDeviceGetProperty(device_id, 0, false,
	                             kAudioDevicePropertyDeviceName,
	                             &size, device_name);
	if(err != noErr)
	{
		ERR(coreaudio, "Error kAudioDevicePropertyDeviceName: %s",
		    errorString(err));
		return "";
	}

	return std::string(device_name);
}

static std::string getDeviceUID(AudioDeviceID device_id)
{
	OSStatus err;
	CFStringRef ui_name = nullptr;
	UInt32 size = sizeof(CFStringRef);
	err = AudioDeviceGetProperty(device_id, 0, false,
	                             kAudioDevicePropertyDeviceUID,
	                             &size, &ui_name);
	if(err != noErr)
	{
		ERR(coreaudio, "Error kAudioDevicePropertyDeviceUID: %s",
		    errorString(err));

		if(ui_name != nullptr)
		{
			CFRelease(ui_name);
		}

		return "";
	}

	assert(ui_name != nullptr);

	char internal_name[256];
	memset(internal_name, 0, sizeof(internal_name));
	size = sizeof(internal_name) - 1; // leave space for terminating zero
	CFStringGetCString(ui_name, internal_name, size,
	                   CFStringGetSystemEncoding());

	if(ui_name != nullptr)
	{
		CFRelease(ui_name);
	}

	return std::string(internal_name);
}

CoreAudioOutputEngine::CoreAudioOutputEngine()
{
}

CoreAudioOutputEngine::~CoreAudioOutputEngine()
{
}

bool CoreAudioOutputEngine::init(const Channels& channels)
{
	OSStatus err;
	std::uint32_t size;

	if(uid == "list")
	{
		// Dump device list
		auto device_list = getDeviceList();
		std::cout  << "[CoreAudioOutputEngine] Device list (" <<
			device_list.size() << " devices):\n";
		for(auto device_id : device_list)
		{
			auto device_name = getDeviceName(device_id);
			auto device_uid = getDeviceUID(device_id);
			std::cout  << "[CoreAudioOutputEngine] - Device: '" << device_name <<
				"' (uid: '" << device_uid  << "')\n";
		}

		// Do not proceed
		return false;
	}

	if(uid != "")
	{
		// Get device id from UID
		size = sizeof(AudioValueTranslation);
		CFStringRef in_uid =
			CFStringCreateWithCString(nullptr, uid.data(),
			                          CFStringGetSystemEncoding());
		AudioValueTranslation value =
			{
				&in_uid, sizeof(CFStringRef), &device_id, sizeof(AudioDeviceID)
			};

		err = AudioHardwareGetProperty(kAudioHardwarePropertyDeviceForUID,
		                               &size, &value);
		CFRelease(in_uid);

		if(err != noErr)
		{
			ERR(coreaudio, "Error kAudioHardwarePropertyDeviceForUID: %s",
			    errorString(err));
		}

		DEBUG(coreaudio, "get_device_id_from_uid '%s' %d",
		      uid.data(), device_id);
	}
	else
	{
		// Use default device id
		size = sizeof(AudioDeviceID);
		err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
	                               &size, &device_id);
		if(err != noErr)
		{
			ERR(coreaudio, "Error kAudioHardwarePropertyDefaultOutputDevice: %s",
			    errorString(err));
		}
	}

	auto device_name = getDeviceName(device_id);

	DEBUG(coreaudio, "default device id: %d (%s)",
	      device_id, device_name.data());

	// TODO: Setting buffer size
	//outSize = sizeof(UInt32);
	//err = AudioDeviceSetProperty (driver->device_id, NULL, 0, false, kAudioDevicePropertyBufferFrameSize, outSize, &nframes);
	//if (err != noErr) {
	//	jack_error ("Cannot set buffer size %ld", nframes);
	//	printError (err);
	//	goto error;
	//}

	// https://github.com/jackaudio/jack1/blob/master/drivers/coreaudio/coreaudio_driver.c#L796
	// TODO: Set samplerate
	//// Get sample rate
	//outSize =  sizeof(Float64);
	//err = AudioDeviceGetProperty (driver->device_id, 0, kAudioDeviceSectionGlobal, kAudioDevicePropertyNominalSampleRate, &outSize, &sampleRate);
	//if (err != noErr) {
	//	jack_error ("Cannot get current sample rate");
	//	printError (err);
	//	goto error;
	//}
	//
	// Then; if samplerate doesn't match - set it.
	//err = AudioDeviceSetProperty (driver->device_id, NULL, 0, kAudioDeviceSectionGlobal, kAudioDevicePropertyNominalSampleRate, outSize, &sampleRate);
	//if (err != noErr) {
	//	jack_error ("Cannot set sample rate = %ld", samplerate);
	//	printError (err);
	//	return -1;
	//}

	// Create AU HAL (whatever that is?)
// https://github.com/jackaudio/jack1/blob/master/drivers/coreaudio/coreaudio_driver.c#L825
//	// AUHAL
//#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
//	AudioComponentDescription cd = { kAudioUnitType_Output, kAudioUnitSubType_HALOutput, kAudioUnitManufacturer_Apple, 0, 0 };
//	AudioComponent HALOutput = AudioComponentFindNext (NULL, &cd);
//	err1 = AudioComponentInstanceNew (HALOutput, &driver->au_hal);
//#else
//	ComponentDescription cd = { kAudioUnitType_Output, kAudioUnitSubType_HALOutput, kAudioUnitManufacturer_Apple, 0, 0 };
//	Component HALOutput = FindNextComponent (NULL, &cd);
//	err1 = OpenAComponent (HALOutput, &driver->au_hal);
//#endif
//
//	if (err1 != noErr) {
//#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
//		jack_error ("Error calling AudioComponentInstanceNew");
//#else
//		jack_error ("Error calling OpenAComponent");
//#endif
//		printError (err1);
//		goto error;
//	}


	// Set up channels maps (whaveter that is?!)
	// https://github.com/jackaudio/jack1/blob/master/drivers/coreaudio/coreaudio_driver.c#L901

	return true;
}

void CoreAudioOutputEngine::setParm(const std::string& parm,
                                    const std::string& value)
{
	if(parm == "uid")
	{
		// Use the device pointed to by this UID.
		uid = value;
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
			std::cerr << "[CoreAudioOutputEngine] Invalid buffer size " << value
			          << "\n";
		}
	}
	else if(parm == "srate")
	{
		try
		{
			samplerate = std::stoi(value);
		}
		catch(...)
		{
			std::cerr << "[CoreAudioOutputEngine] Invalid samplerate " << value
			          << "\n";
		}
	}
	else
	{
		std::cerr << "[CoreAudioOutputEngine] Unsupported parameter '" << parm
		          << "'\n";
	}

}

bool CoreAudioOutputEngine::start()
{
	//https://github.com/jackaudio/jack1/blob/master/drivers/coreaudio/coreaudio_driver.c#L864
	// Start I/O
	//enableIO = 1;
	//err1 = AudioUnitSetProperty (driver->au_hal, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableIO, sizeof(enableIO));
	//if (err1 != noErr) {
	//	jack_error ("Error calling AudioUnitSetProperty - kAudioOutputUnitProperty_EnableIO,kAudioUnitScope_Output");
	//	printError (err1);
	//	goto error;
	//}

	// https://github.com/jackaudio/jack1/blob/master/drivers/coreaudio/coreaudio_driver.c#L874
	//// Setup up choosen device, in both input and output cases
	//err1 = AudioUnitSetProperty (driver->au_hal, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &driver->device_id, sizeof(AudioDeviceID));
	//if (err1 != noErr) {
	//	jack_error ("Error calling AudioUnitSetProperty - kAudioOutputUnitProperty_CurrentDevice");
	//	printError (err1);
	//	goto error;
	//}

	return true;
}

void CoreAudioOutputEngine::stop()
{
	//https://github.com/jackaudio/jack1/blob/master/drivers/coreaudio/coreaudio_driver.c#L864
	// Start I/O
	//enableIO = 0;
	//err1 = AudioUnitSetProperty (driver->au_hal, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableIO, sizeof(enableIO));
	//if (err1 != noErr) {
	//	jack_error ("Error calling AudioUnitSetProperty - kAudioOutputUnitProperty_EnableIO,kAudioUnitScope_Output");
	//	printError (err1);
	//	goto error;
	//}{
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
	return samplerate;
}

bool CoreAudioOutputEngine::isFreewheeling() const
{
	return false;
}
