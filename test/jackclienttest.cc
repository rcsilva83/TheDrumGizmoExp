/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            jackclienttest.cc
 *
 *  Unit tests for JackClient using mock JACK API.
 ****************************************************************************/

#include <doctest/doctest.h>

#include "drumgizmo/jackclient.h"

#include "mock_jack.h"

class TestJackProcess : public JackProcess
{
public:
	int process_count{0};
	int latency_callback_count{0};

	void process(jack_nframes_t) override
	{
		++process_count;
	}

	void jackLatencyCallback(jack_latency_callback_mode_t) override
	{
		++latency_callback_count;
	}
};

TEST_CASE("JackClient")
{
	SUBCASE("constructorCreatesClient")
	{
		JackClient client;
		CHECK_EQ(client.getBufferSize(), 1024u);
		CHECK_EQ(client.getSampleRate(), 44100u);
		CHECK_UNARY(!client.isFreewheeling());
	}

	SUBCASE("addAndRemoveProcess")
	{
		JackClient client;
		TestJackProcess process;

		client.add(process);
		client.remove(process);
	}

	SUBCASE("activateClient")
	{
		JackClient client;
		client.activate();
		client.activate();
	}

	SUBCASE("processWithActiveProcess")
	{
		JackClient client;
		TestJackProcess process;

		client.add(process);

		auto* mock_client = mock_jack_last_client();

		if(mock_client && mock_client->process_callback)
		{
			mock_client->process_callback(1024, mock_client->process_arg);
		}

		CHECK_EQ(process.process_count, 1);
	}

	SUBCASE("processAfterRemove")
	{
		JackClient client;
		TestJackProcess process;

		client.add(process);
		client.remove(process);

		auto* mock_client = mock_jack_last_client();

		if(mock_client && mock_client->process_callback)
		{
			mock_client->process_callback(1024, mock_client->process_arg);
		}

		CHECK_EQ(process.process_count, 0);
	}

	SUBCASE("latencyCallback")
	{
		JackClient client;
		TestJackProcess process;

		client.add(process);

		auto* mock_client = mock_jack_last_client();

		if(mock_client && mock_client->latency_callback)
		{
			mock_client->latency_callback(
			    JackCaptureLatency, mock_client->latency_arg);
		}

		CHECK_EQ(process.latency_callback_count, 1);
	}

	SUBCASE("freewheelCallback")
	{
		JackClient client;

		auto* mock_client = mock_jack_last_client();

		if(mock_client && mock_client->freewheel_callback)
		{
			mock_client->freewheel_callback(1, mock_client->freewheel_arg);
		}

		CHECK_UNARY(client.isFreewheeling());
	}
}

TEST_CASE("JackPort")
{
	SUBCASE("createAndDestroyPort")
	{
		JackClient client;
		JackPort port(client, "test_port", JACK_DEFAULT_AUDIO_TYPE,
		    JackPortIsInput);
	}
}
