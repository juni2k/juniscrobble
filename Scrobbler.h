#pragma once

#include <string>

#include "Track.h"
#include <chrono>
#include <list>

class Scrobbler
{
public:
  void stage_track(std::wstring artist, std::wstring title);
  void commit_track();
  void clear_staged_track();

private:
  Track* staged_track = nullptr;
  std::chrono::steady_clock::time_point staged_time;
  std::list<Track*> submissions;
};

