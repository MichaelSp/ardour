/*
 * Copyright (C) 2016-2017 Robin Gareus <robin@gareus.org>
 *
 * Stubbed for the CMake/mac migration: this tree does not currently carry
 * a usable libarchive dependency on this machine.
 */

#include "pbd/file_archive.h"

#include <cstdlib>

using namespace PBD;

FileArchive::FileArchive (const std::string& url, Progress* p)
	: _req (url, p)
	, _progress (p)
	, _current_entry (0)
	, _archive (0)
{
}

FileArchive::~FileArchive () = default;

std::string
FileArchive::fetch (const std::string&, const std::string&) const
{
	return std::string ();
}

int
FileArchive::inflate (const std::string&)
{
	return -1;
}

std::vector<std::string>
FileArchive::contents ()
{
	return std::vector<std::string> ();
}

std::string
FileArchive::next_file_name ()
{
	return std::string ();
}

int
FileArchive::extract_current_file (const std::string&)
{
	return -1;
}

std::vector<std::string>
FileArchive::contents_file ()
{
	return std::vector<std::string> ();
}

std::vector<std::string>
FileArchive::contents_url ()
{
	return std::vector<std::string> ();
}

int
FileArchive::extract_file ()
{
	return -1;
}

int
FileArchive::extract_url ()
{
	return -1;
}

std::vector<std::string>
FileArchive::get_contents (struct archive*)
{
	return std::vector<std::string> ();
}

int
FileArchive::do_extract (struct archive*)
{
	return -1;
}

int
FileArchive::create (const std::string&, CompressionLevel)
{
	return -1;
}

int
FileArchive::create (const std::map<std::string, std::string>&, CompressionLevel)
{
	return -1;
}

struct archive*
FileArchive::setup_file_archive ()
{
	return 0;
}
