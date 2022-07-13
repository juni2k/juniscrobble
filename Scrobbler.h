#pragma once

#include <string>

#include "Track.h"
#include <list>

class Scrobbler
{
public:
  void stage_track(std::wstring artist, std::wstring title);
  void commit_track();

private:
  Track* staged_track = nullptr;
  std::list<Track*> submissions;
  void clear_staged_track();
};

