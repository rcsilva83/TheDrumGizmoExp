/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            atomic.cc
 *
 *  Wed Mar 23 09:17:12 CET 2016
 *  Copyright 2016 Christian Gl�ckner
 *  cgloeckner@freenet.de
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

#include <atomic.h>

struct AtomicTestFixture
{
	template <typename T> bool isUsingStandardImpl()
	{
		return std::is_base_of<std::atomic<T>, Atomic<T>>::value;
	}

	template <typename T> bool isLockFree()
	{
		Atomic<T> a{T{}};
		return a.is_lock_free();
	}
};

TEST_CASE_FIXTURE(AtomicTestFixture, "AtomicTest")
{
	SUBCASE("podAtomicsUseStandardImpl")
	{
		CHECK(isUsingStandardImpl<bool>());
		CHECK(isUsingStandardImpl<unsigned short int>());
		CHECK(isUsingStandardImpl<short int>());
		CHECK(isUsingStandardImpl<unsigned int>());
		CHECK(isUsingStandardImpl<int>());
		CHECK(isUsingStandardImpl<unsigned long int>());
		CHECK(isUsingStandardImpl<long int>());
		CHECK(isUsingStandardImpl<unsigned long long int>());
		CHECK(isUsingStandardImpl<long long int>());
		CHECK(isUsingStandardImpl<float>());
		CHECK(isUsingStandardImpl<double>());
		CHECK(isUsingStandardImpl<long double>());
	}

	SUBCASE("nonPodAtomicsUseOwnImpl")
	{
		CHECK(!isUsingStandardImpl<std::string>());
	}

	SUBCASE("podAtomicDefaultInitializationSupportsStateTransitions")
	{
		Atomic<int> i;
		i.store(5);
		CHECK_EQ(i.load(), 5);

		CHECK_EQ(i.exchange(9), 5);
		CHECK_EQ(i.load(), 9);
	}

	SUBCASE("nonPodAtomicCanBeDefaultInitialized")
	{
		Atomic<std::string> s;
		CHECK_EQ(s.load(), std::string{});
	}

	SUBCASE("podAtomicCanBeValueInitialized")
	{
		Atomic<int> i{5};
		CHECK_EQ(i.load(), 5);
	}

	SUBCASE("nonPodAtomicCanBeValueInitialized")
	{
		Atomic<std::string> s{"hello world"};
		CHECK_EQ(s.load(), std::string{"hello world"});
	}

	SUBCASE("podAtomicCanBeValueAssigned")
	{
		Atomic<int> i;
		i = 5;
		CHECK_EQ(i.load(), 5);
	}

	SUBCASE("nonPodAtomicCanBeValueAssigned")
	{
		Atomic<std::string> s;
		s = "hello world";
		CHECK_EQ(s.load(), std::string{"hello world"});
	}

	SUBCASE("podAtomicsAreLockFree")
	{
		CHECK(isLockFree<bool>());
		CHECK(isLockFree<unsigned short int>());
		CHECK(isLockFree<short int>());
		CHECK(isLockFree<unsigned int>());
		CHECK(isLockFree<int>());
		CHECK(isLockFree<unsigned long int>());
		CHECK(isLockFree<long int>());
		CHECK(isLockFree<float>());
		CHECK(isLockFree<std::size_t>());

		// NOTE: Not lock free on small systems
		// CHECK(isLockFree<unsigned long long int>());
		// CHECK(isLockFree<long long int>());
		// CHECK(isLockFree<double>());
		// CHECK(isLockFree<long double>());
	}
}
