#pragma once

#include "pch.h"

#include "Track.h"

class Scrobbler
{
public:
  void stage_track(std::wstring artist, std::wstring title);
  void commit_track();
  void clear_staged_track();
  void set_track_length(int seconds);
  Track* staged_track();
  std::wstring submissions_text();

private:
  Track* _staged_track = nullptr;
  std::chrono::steady_clock::time_point staged_time;
  std::list<Track> submissions;
};

