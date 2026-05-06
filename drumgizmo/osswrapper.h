/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            osswrapper.h
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

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

class OssWrapper
{
public:
	virtual ~OssWrapper() = default;

	virtual int open_device(const char* pathname, int flags, mode_t mode) = 0;
	virtual int close_device(int fd) = 0;
	virtual ssize_t read_device(int fd, void* buf, size_t count) = 0;
	virtual ssize_t write_device(int fd, const void* buf, size_t count) = 0;
	virtual int ioctl_device(int fd, unsigned long request, void* arg) = 0;
	virtual const char* strerror_device(int errnum) = 0;
};

class RealOssWrapper : public OssWrapper
{
public:
	int open_device(const char* pathname, int flags, mode_t mode) override
	{
		return ::open(pathname, flags, mode);
	}
	int close_device(int fd) override
	{
		return ::close(fd);
	}
	ssize_t read_device(int fd, void* buf, size_t count) override
	{
		return ::read(fd, buf, count);
	}
	ssize_t write_device(int fd, const void* buf, size_t count) override
	{
		return ::write(fd, buf, count);
	}
	int ioctl_device(int fd, unsigned long request, void* arg) override
	{
		return ::ioctl(fd, request, arg);
	}
	const char* strerror_device(int errnum) override
	{
		return ::strerror(errnum);
	}
};
