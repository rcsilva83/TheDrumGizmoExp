/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            mock_jack.h
 *
 *  Mock JACK API for testing jackclient.cc without a running JACK server.
 ****************************************************************************/

#pragma once

#include <jack/jack.h>

#ifdef __cplusplus
extern "C"
{
#endif

	struct MockJackClient
	{
		JackProcessCallback process_callback;
		void* process_arg;
		JackLatencyCallback latency_callback;
		void* latency_arg;
		JackFreewheelCallback freewheel_callback;
		void* freewheel_arg;
		bool active;
	};

	struct MockJackPort
	{
		MockJackClient* client;
	};

#ifdef __cplusplus
}
#endif

struct MockJackClient* mock_jack_last_client();
