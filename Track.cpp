#include "pch.h"
#include "Track.h"

#include <ostream>

/* How can a language be this fucking ugly... */
Track::Track(std::wstring artist, std::wstring title) :
  artist(artist),
  title(title),
  length(INVALID_TRACK_LENGTH)
  {}

std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& stream, const Track& track) {
  return stream
    << "[" << track.artist<< "] - [" << track.title << "]";
}