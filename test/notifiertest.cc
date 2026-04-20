/* -*- Mode: c++ -*- */
/***************************************************************************
 *            notifiertest.cc
 *
 *  Sat Jul 15 09:56:51 CEST 2017
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
#include <doctest/doctest.h>

#include <string>
#include <vector>

#include <notifier.h>

class Probe : public Listener
{
public:
	// cppcheck-suppress noExplicitConstructor
	Probe(std::vector<Probe*>& triggers) : triggers(triggers)
	{
	}

	void slot()
	{
		triggers.push_back(this);
	}

	std::vector<Probe*>& triggers;
};

class IntProbe : public Listener
{
public:
	void slot(int value)
	{
		received.push_back(value);
	}

	std::vector<int> received;
};

class TwoArgProbe : public Listener
{
public:
	void slot(int a, float b)
	{
		ints.push_back(a);
		floats.push_back(b);
	}

	std::vector<int> ints;
	std::vector<float> floats;
};

class StringProbe : public Listener
{
public:
	void slot(const std::string& str)
	{
		received.push_back(str);
	}

	std::vector<std::string> received;
};

class ThreeArgProbe : public Listener
{
public:
	void slot(int a, double b, const std::string& c)
	{
		ints.push_back(a);
		doubles.push_back(b);
		strings.push_back(c);
	}

	std::vector<int> ints;
	std::vector<double> doubles;
	std::vector<std::string> strings;
};

// Note: Non-Listener objects cannot be connected to Notifiers because
// the internal storage requires the object pointer to be a Listener*.
// This is by design - all receivers must inherit from Listener.

TEST_CASE("NotifierTest")
{
	SUBCASE("testTest")
	{
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe foo1(triggers);
		Probe foo2(triggers);

		{ // Order as initialisation
			notifier.connect(&foo1, &Probe::slot);
			notifier.connect(&foo2, &Probe::slot);
			notifier();
			std::vector<Probe*> ref;
			ref.push_back(&foo1);
			ref.push_back(&foo2);
			CHECK_EQ(ref.size(), triggers.size());
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[0], triggers[0]);
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[1], triggers[1]);
			notifier.disconnect(&foo1);
			notifier.disconnect(&foo2);
			triggers.clear();
		}

		{ // Reverse order
			notifier.connect(&foo2, &Probe::slot);
			notifier.connect(&foo1, &Probe::slot);
			notifier();
			std::vector<Probe*> ref;
			ref.push_back(&foo2);
			ref.push_back(&foo1);
			CHECK_EQ(ref.size(), triggers.size());
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[0], triggers[0]);
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[1], triggers[1]);
			notifier.disconnect(&foo1);
			notifier.disconnect(&foo2);
			triggers.clear();
		}
	}

	SUBCASE("notifier_with_int_argument")
	{
		// Notifier<int> forwards the integer argument to the connected slot.
		Notifier<int> notifier;
		IntProbe probe;
		notifier.connect(&probe, &IntProbe::slot);

		notifier(42);
		notifier(7);

		CHECK_EQ(2u, probe.received.size());
		// cppcheck-suppress containerOutOfBounds
		CHECK_EQ(42, probe.received[0]);
		// cppcheck-suppress containerOutOfBounds
		CHECK_EQ(7, probe.received[1]);
	}

	SUBCASE("notifier_with_two_arguments")
	{
		// Notifier<int, float> forwards both arguments to the connected slot.
		Notifier<int, float> notifier;
		TwoArgProbe probe;
		notifier.connect(&probe, &TwoArgProbe::slot);

		notifier(1, 2.5f);
		notifier(3, 4.0f);

		CHECK_EQ(2u, probe.ints.size());
		// cppcheck-suppress containerOutOfBounds
		CHECK_EQ(1, probe.ints[0]);
		// cppcheck-suppress containerOutOfBounds
		CHECK_EQ(3, probe.ints[1]);
		CHECK_EQ(2u, probe.floats.size());
		// cppcheck-suppress containerOutOfBounds
		CHECK_EQ(2.5f, probe.floats[0]);
		// cppcheck-suppress containerOutOfBounds
		CHECK_EQ(4.0f, probe.floats[1]);
	}

	SUBCASE("auto_disconnect_on_listener_destruction")
	{
		// When a Listener is destroyed its slots must be automatically removed
		// from all connected Notifiers, so firing the notifier afterwards does
		// not invoke the deleted object.
		Notifier<> notifier;
		std::vector<Probe*> triggers;

		{
			Probe shortLived(triggers);
			notifier.connect(&shortLived, &Probe::slot);
			notifier(); // shortLived is alive, so it must be called once
			CHECK_EQ(1u, triggers.size());
			triggers.clear();
		} // shortLived is destroyed here; its dtor disconnects it

		notifier(); // no listeners remain, triggers must stay empty
		CHECK_EQ(0u, triggers.size());
	}

	SUBCASE("disconnect_stops_notifications")
	{
		// After an explicit disconnect(), the slot must no longer be called.
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe foo(triggers);

		notifier.connect(&foo, &Probe::slot);
		notifier();
		CHECK_EQ(1u, triggers.size());
		triggers.clear();

		notifier.disconnect(&foo);
		notifier();
		CHECK_EQ(0u, triggers.size());
	}

	SUBCASE("disconnect_later_slot_traverses_earlier_slot")
	{
		// When two slots are connected and the second one is disconnected,
		// the iterator must traverse the first slot (false branch of the
		// it->first == object comparison) before finding the second.
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe foo1(triggers);
		Probe foo2(triggers);

		notifier.connect(&foo1, &Probe::slot);
		notifier.connect(&foo2, &Probe::slot);

		// Disconnect the second slot; this causes the iterator to check foo1
		// first (false branch) and then foo2 (true branch).
		notifier.disconnect(&foo2);

		notifier();
		// Only foo1 must still receive the notification.
		REQUIRE_EQ(1u, triggers.size());
		CHECK_EQ(&foo1, triggers[0]);
	}

	SUBCASE("disconnect_nonexistent_listener_is_safe")
	{
		// Calling disconnect() with a listener that was never connected must
		// be a safe no-op.
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe connected(triggers);
		Probe unconnected(triggers);

		notifier.connect(&connected, &Probe::slot);
		notifier.disconnect(&unconnected); // must not crash or remove anything

		notifier();
		REQUIRE_EQ(1u, triggers.size());
		CHECK_EQ(&connected, triggers[0]);
	}

	SUBCASE("notifier_dtor_unregisters_from_listeners")
	{
		// When a Notifier is destroyed before its Listeners, its dtor must
		// unregister itself from those Listeners so they do not try to
		// disconnect from a dangling pointer later.
		std::vector<Probe*> triggers;
		Probe listener(triggers);

		{
			Notifier<> notifier;
			notifier.connect(&listener, &Probe::slot);
			notifier(); // fires once while alive
		} // notifier destroyed here while listener still alive

		// The listener is still alive and must not crash or reference a
		// dangling notifier when it is later destroyed.
		CHECK_EQ(1u, triggers.size());
	}

	SUBCASE("connect_macro")
	{
		// The CONNECT macro is designed for notifiers that are class members.
		// It expands (SRC)->SIG.connect(TAR, SLO) to connect a member notifier.
		class WidgetWithNotifier
		{
		public:
			Notifier<> signal;
		};

		WidgetWithNotifier widget;
		std::vector<Probe*> triggers;
		Probe probe(triggers);

		CONNECT(&widget, signal, &probe, &Probe::slot);
		widget.signal();

		REQUIRE_EQ(1u, triggers.size());
		CHECK_EQ(&probe, triggers[0]);
	}

	SUBCASE("empty_notifier_no_crash")
	{
		// Calling an empty Notifier (with no slots) must not crash.
		Notifier<> notifier;
		// Should not crash even with no connections
		notifier();
		CHECK_UNARY(true); // Test reaches here = no crash
	}

	SUBCASE("notifier_with_string_argument")
	{
		// Notifier<std::string> forwards string arguments correctly.
		Notifier<std::string> notifier;
		StringProbe probe;
		notifier.connect(&probe, &StringProbe::slot);

		notifier("hello");
		notifier("world");

		CHECK_EQ(2u, probe.received.size());
		CHECK_EQ("hello", probe.received[0]);
		CHECK_EQ("world", probe.received[1]);
	}

	SUBCASE("notifier_with_three_arguments")
	{
		// Notifier with three different argument types.
		Notifier<int, double, std::string> notifier;
		ThreeArgProbe probe;
		notifier.connect(&probe, &ThreeArgProbe::slot);

		notifier(1, 2.5, "first");
		notifier(3, 4.0, "second");

		CHECK_EQ(2u, probe.ints.size());
		CHECK_EQ(1, probe.ints[0]);
		CHECK_EQ(3, probe.ints[1]);
		CHECK_EQ(2.5, probe.doubles[0]);
		CHECK_EQ(4.0, probe.doubles[1]);
		CHECK_EQ("first", probe.strings[0]);
		CHECK_EQ("second", probe.strings[1]);
	}

	SUBCASE("notifierbase_default_disconnect")
	{
		// The base NotifierBase::disconnect should be callable (it is a
		// no-op for the base class).
		NotifierBase base;
		std::vector<Probe*> triggers;
		Probe listener(triggers);
		base.disconnect(&listener); // should not crash
		CHECK_UNARY(true); // Test reaches here = no crash
	}

	SUBCASE("listener_unregister_notifier_directly")
	{
		// Test direct call to Listener::unregisterNotifier.
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe probe(triggers);

		notifier.connect(&probe, &Probe::slot);
		notifier();
		CHECK_EQ(1u, triggers.size());
		triggers.clear();

		// Manually unregister - this simulates what Notifier dtor does
		probe.unregisterNotifier(&notifier);

		// After unregister, the notifier should no longer call the probe
		notifier.disconnect(&probe);
		notifier();
		CHECK_EQ(0u, triggers.size());
	}

	SUBCASE("multiple_notifiers_same_listener")
	{
		// A single Listener can be connected to multiple Notifiers.
		Notifier<> notifier1;
		Notifier<> notifier2;
		std::vector<Probe*> triggers;
		Probe probe(triggers);

		notifier1.connect(&probe, &Probe::slot);
		notifier2.connect(&probe, &Probe::slot);

		notifier1();
		CHECK_EQ(1u, triggers.size());

		notifier2();
		CHECK_EQ(2u, triggers.size());

		// Disconnect from only one notifier
		notifier1.disconnect(&probe);
		notifier1();
		CHECK_EQ(2u, triggers.size()); // no change

		notifier2();
		CHECK_EQ(3u, triggers.size());
	}

	SUBCASE("disconnect_first_of_multiple")
	{
		// Disconnect the first slot when multiple slots exist.
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe foo1(triggers);
		Probe foo2(triggers);
		Probe foo3(triggers);

		notifier.connect(&foo1, &Probe::slot);
		notifier.connect(&foo2, &Probe::slot);
		notifier.connect(&foo3, &Probe::slot);

		notifier.disconnect(&foo1); // disconnect first

		notifier();
		REQUIRE_EQ(2u, triggers.size());
		CHECK_EQ(&foo2, triggers[0]);
		CHECK_EQ(&foo3, triggers[1]);
	}

	SUBCASE("disconnect_middle_of_multiple")
	{
		// Disconnect a middle slot when multiple slots exist.
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe foo1(triggers);
		Probe foo2(triggers);
		Probe foo3(triggers);

		notifier.connect(&foo1, &Probe::slot);
		notifier.connect(&foo2, &Probe::slot);
		notifier.connect(&foo3, &Probe::slot);

		notifier.disconnect(&foo2); // disconnect middle

		notifier();
		REQUIRE_EQ(2u, triggers.size());
		CHECK_EQ(&foo1, triggers[0]);
		CHECK_EQ(&foo3, triggers[1]);
	}
}
