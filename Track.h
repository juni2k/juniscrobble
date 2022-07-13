#pragma once

#include <string>

class Track
{
public:
  Track(std::wstring artist, std::wstring title);
  std::wstring artist;
  std::wstring title;
};

std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& stream, const Track& track);