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
#include <thread>
#include <chrono>

#include <hugin.hpp>
#include <assert.h>

#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioUnit/AUComponent.h>

struct RAII
{
	std::string id; // value from user arguments
	AudioDeviceID device_id{kAudioDeviceUnknown};
	std::uint32_t frames;
	std::uint32_t samplerate;
	AudioBufferList* input_list;
	ComponentInstance audio_unit;
};

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
	: raii(new struct RAII)
{
	raii->samplerate = 44100;
	raii->frames = 1024;
	DEBUG(coreaudio, "!");
}

CoreAudioOutputEngine::~CoreAudioOutputEngine()
{
	DEBUG(coreaudio, "!");
}

bool CoreAudioOutputEngine::init(const Channels& channels)
{
	OSStatus result = noErr;
	AudioComponent comp;
	AudioComponentDescription desc;
	AudioStreamBasicDescription requestedDesc;
	UInt32 i_param_size, requestedEndian;

	// Locate the default output audio unit
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;

	comp = AudioComponentFindNext(nullptr, &desc);
	if(comp == nullptr)
	{
		ERR(coreaudio, "Failed to start CoreAudio:"
		    " AudioComponentFindNext returned nullptr.");
		return false;
	}

	// Open & initialize the default output audio unit
	result = AudioComponentInstanceNew(comp, &raii->audio_unit);
	if(result)
	{
		ERR(coreaudio,"AudioComponentInstanceNew() error => %d\n", (int)result);
		return false;
	}

	// Set the desired output device if not default
	if(raii->device_id != kAudioObjectUnknown)
	{
		result = AudioUnitSetProperty(raii->audio_unit,
		                              kAudioOutputUnitProperty_CurrentDevice,
		                              kAudioUnitScope_Global,
		                              0,
		                              &raii->device_id,
		                              sizeof(raii->device_id));
		if(result)
		{
			ERR(coreaudio, "AudioComponentSetDevice() error => %d\n", (int)result);
			AudioComponentInstanceDispose(raii->audio_unit);
			return false;
		}
	}

	// Request desired format of the audio unit.  Let HAL do all
	// conversion since it will probably be doing some internal
	// conversion anyway.

	//device->driver_byte_format = format->byte_format;
	requestedDesc.mFormatID = kAudioFormatLinearPCM;
	requestedDesc.mFormatFlags = kAudioFormatFlagIsPacked;
	//switch(format->byte_format){
	//case AO_FMT_BIG:
	//	requestedDesc.mFormatFlags |= kAudioFormatFlagIsBigEndian;
	//	break;
	//case AO_FMT_NATIVE:
	//	if(ao_is_big_endian())
	//		requestedDesc.mFormatFlags |= kAudioFormatFlagIsBigEndian;
	//	break;
	//}
	requestedEndian = requestedDesc.mFormatFlags & kAudioFormatFlagIsBigEndian;
	//if(format->bits > 8)
	//requestedDesc.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
	requestedDesc.mFormatFlags |=kAudioFormatFlagIsFloat;
	requestedDesc.mChannelsPerFrame = channels.size();//device->output_channels;
	requestedDesc.mSampleRate = raii->samplerate;//format->rate;
	requestedDesc.mBitsPerChannel = 32;//format->bits;
	requestedDesc.mFramesPerPacket = 1;
	requestedDesc.mBytesPerFrame =
		requestedDesc.mBitsPerChannel * requestedDesc.mChannelsPerFrame / 8;
	requestedDesc.mBytesPerPacket =
		requestedDesc.mBytesPerFrame * requestedDesc.mFramesPerPacket;

	result = AudioUnitSetProperty(raii->audio_unit,
	                              kAudioUnitProperty_StreamFormat,
	                              kAudioUnitScope_Input,
	                              0,
	                              &requestedDesc,
	                              sizeof(requestedDesc));

	if(result)
	{
		ERR(coreaudio, "AudioUnitSetProperty error => %d\n", (int)result);
		return false;
	}

	// What format did we actually get?
	i_param_size = sizeof(requestedDesc);
	result = AudioUnitGetProperty(raii->audio_unit,
	                              kAudioUnitProperty_StreamFormat,
	                              kAudioUnitScope_Input,
	                              0,
	                              &requestedDesc,
	                              &i_param_size );
	if(result)
	{
		ERR(coreaudio, "Failed to query modified device hardware settings => %d\n",
		    (int)result);
		return false;
	}

	// If any major settings differ, abort
	//if(fabs(requestedDesc.mSampleRate- samplerate) > format->rate*.05)
	//{
	//	 ERR(coreaudio. "Unable to set output sample rate\n");
	//	return false;
	//}
	if(requestedDesc.mChannelsPerFrame != channels.size())
	{
		ERR(coreaudio, "Could not configure %d channel output\n",
		    (int)channels.size());
		return false;
	}

	if(requestedDesc.mBitsPerChannel != 32) // size of float in bits
	{
		ERR(coreaudio, "Could not configure %d bit output\n", 32);
		return false;
	}

	if(requestedDesc.mBitsPerChannel != 32)
	{
		ERR(coreaudio, "Could not configure %d bit output\n", 32);
		return false;
	}

	//if(requestedDesc.mFormatFlags & kAudioFormatFlagIsFloat)
	//{
	//	ERR(coreaudio, "Could not configure integer sample output\n");
	//	return false;
	//}

	if((requestedDesc.mFormatFlags & kAudioFormatFlagsNativeEndian) !=
	   requestedEndian)
	{
		ERR(coreaudio, "Could not configure output endianness\n");
		return false;
	}

	//if(format->bits > 8)
	//{
	//	if(!(requestedDesc.mFormatFlags & kAudioFormatFlagIsSignedInteger))
	//	{
	//		ERR(coreaudio, "Could not configure signed output\n");
	//		return false;
	//	}
	//}
	//else
	//{
	//	if((requestedDesc.mFormatFlags & kAudioFormatFlagIsSignedInteger))
	//	{
	//		ERR(coreaudio, "Could not configure unsigned output\n");
	//		return false;
	//	}
	//}
	if(requestedDesc.mSampleRate != raii->samplerate)
	{
		WARN(coreaudio,
		     "Could not set sample rate to exactly %d; using %g instead.\n",
		     raii->samplerate,(double)requestedDesc.mSampleRate);
	}

	// Set the channel mapping.
	// MacOSX AUHAL is capable of mapping any channel format currently
	// representable in the libao matrix.
	//if(device->output_mask)
	//{
	//	AudioChannelLayout layout;
	//	memset(&layout,0,sizeof(layout));
	//
	//	layout.mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelBitmap;
	//	layout.mChannelBitmap = device->output_mask;
	//
	//	result = AudioUnitSetProperty(raii->audio_unit,
	//	                              kAudioUnitProperty_AudioChannelLayout,
	//	                              kAudioUnitScope_Input, 0, &layout,
	//	                              sizeof(layout));
	//	if(result) {
	//	  ERR(coreaudio, "Failed to set audio channel layout => %d\n",
	//		    (int)result);
	//	}
	//}

	// Set the audio callback
	AURenderCallbackStruct input;
	input.inputProc = (AURenderCallback)render;
	input.inputProcRefCon = this;

	result = AudioUnitSetProperty(raii->audio_unit,
	                              kAudioUnitProperty_SetRenderCallback,
	                              kAudioUnitScope_Input,
	                              0, &input, sizeof(input));
	if(result)
	{
		ERR(coreaudio, "Callback set error => %d\n",(int)result);
		return false;
	}

	result = AudioUnitInitialize(raii->audio_unit);
	if(result)
	{
		ERR(coreaudio, "AudioUnitInitialize() error => %d\n",(int)result);
		return false;
	}

	return true;
}

void CoreAudioOutputEngine::setParm(const std::string& parm,
                                    const std::string& value)
{
	if(parm == "id")
	{
		if(value == "list")
		{
			// Dump device list
			auto device_list = getDeviceList();
			std::cout  << "[CoreAudioOutputEngine] Device list (" <<
				device_list.size() << " devices):\n";
			for(auto device_id : device_list)
			{
				auto device_name = getDeviceName(device_id);
				auto device_uid = getDeviceUID(device_id);
				std::cout  << "[CoreAudioOutputEngine] - id: " << device_id <<
					" device: '" << device_name <<
					"' (uid: '" << device_uid  << "')\n";
			}

			// Do not proceed
			exit(0);
		}

		// Use the device pointed to by this ID.
		raii->id = value;
	}
	else if(parm == "frames")
	{
		// try to apply hardware buffer size
		try
		{
			raii->frames = std::stoi(value);
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
			raii->samplerate = std::stoi(value);
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
	DEBUG(coreaudio, "Starting audio output unit\n");

	int err = AudioOutputUnitStart(raii->audio_unit);
	if(err)
	{
	  ERR(coreaudio, "Failed to start audio output => %d\n",(int)err);
	  return false;
	}

	return true;
}

void CoreAudioOutputEngine::stop()
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	AudioOutputUnitStop(raii->audio_unit);
}

void CoreAudioOutputEngine::pre(size_t nsamples)
{
}

void CoreAudioOutputEngine::run(int ch, sample_t* samples, size_t nsamples)
{
//	std::cout << __PRETTY_FUNCTION__ << std::endl;
	// Write channel data in interleaved buffer
}

void CoreAudioOutputEngine::post(size_t nsamples)
{
	// Write the interleaved buffer to the soundcard
}

size_t CoreAudioOutputEngine::getSamplerate() const
{
	return raii->samplerate;
}

std::size_t CoreAudioOutputEngine::getBufferSize() const
{
	return raii->frames;
}

bool CoreAudioOutputEngine::isFreewheeling() const
{
	return false;
}

OSStatus CoreAudioOutputEngine::render(void *user_data,
                                       AudioUnitRenderActionFlags *action_flags,
                                       const AudioTimeStamp *timestamp,
                                       UInt32 bus_number,
                                       UInt32 number_frames,
                                       AudioBufferList *io_data)
{
	OSStatus err = noErr;
	CoreAudioOutputEngine *engine =
		static_cast<CoreAudioOutputEngine*>(user_data);
	(void)engine;

	// Despite the audio buffer list, playback render can only submit a
	// single buffer.

	if(!io_data)
	{
		ERR(coreaudio, "Unexpected number of buffers (io_data == nullptr)\n");
		return 0;
	}

	if(io_data->mNumberBuffers != 1)
	{
		ERR(coreaudio, "Unexpected number of buffers (%d)\n",
		       (int)io_data->mNumberBuffers);
		return 0;
	}

	float *samples = (float*)io_data->mBuffers[0].mData;
	auto size = io_data->mBuffers[0].mDataByteSize / sizeof(float);

	static double p = 0.0;
	for(auto i = 0u; i < size; ++i)
	{
		samples[i] = 0.4 * sin(p += 0.02);
	}

	return err;
}
