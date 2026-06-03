/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            mock_jack.cc
 *
 *  Mock JACK API implementation for testing.
 ****************************************************************************/

#include "mock_jack.h"

static MockJackClient* g_last_client{nullptr};

MockJackClient* mock_jack_last_client()
{
	return g_last_client;
}

extern "C"
{

	jack_client_t* jack_client_open(const char*, jack_options_t,
	    jack_status_t* status, ...)
	{
		auto* client = new MockJackClient{};
		g_last_client = client;
		if(status)
		{
			*status = JackNoError;
		}
		return reinterpret_cast<jack_client_t*>(client);
	}

	int jack_client_close(jack_client_t* client)
	{
		delete reinterpret_cast<MockJackClient*>(client);
		return 0;
	}

	jack_port_t* jack_port_register(jack_client_t* client, const char*,
	    const char*, unsigned long, unsigned long)
	{
		auto* port = new MockJackPort();
		port->client = reinterpret_cast<MockJackClient*>(client);
		return reinterpret_cast<jack_port_t*>(port);
	}

	int jack_port_unregister(jack_client_t*, jack_port_t* port)
	{
		delete reinterpret_cast<MockJackPort*>(port);
		return 0;
	}

	int jack_set_process_callback(jack_client_t* client,
	    JackProcessCallback callback, void* arg)
	{
		auto* c = reinterpret_cast<MockJackClient*>(client);
		c->process_callback = callback;
		c->process_arg = arg;
		return 0;
	}

	int jack_set_latency_callback(jack_client_t* client,
	    JackLatencyCallback callback, void* arg)
	{
		auto* c = reinterpret_cast<MockJackClient*>(client);
		c->latency_callback = callback;
		c->latency_arg = arg;
		return 0;
	}

	int jack_set_freewheel_callback(jack_client_t* client,
	    JackFreewheelCallback callback, void* arg)
	{
		auto* c = reinterpret_cast<MockJackClient*>(client);
		c->freewheel_callback = callback;
		c->freewheel_arg = arg;
		return 0;
	}

	int jack_activate(jack_client_t* client)
	{
		auto* c = reinterpret_cast<MockJackClient*>(client);
		c->active = true;
		return 0;
	}

	jack_nframes_t jack_get_buffer_size(jack_client_t*)
	{
		return 1024;
	}

	jack_nframes_t jack_get_sample_rate(jack_client_t*)
	{
		return 44100;
	}

}
