#pragma once

#include <string>

#define INVALID_TRACK_LENGTH -1

class Track
{
public:
  Track(std::wstring artist, std::wstring title);
  std::wstring artist;
  std::wstring title;
  int length; // seconds
};

std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& stream, const Track& track);