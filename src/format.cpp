#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: X day, HH:MM:SS, same format of htop command
string Format::ElapsedTime(long seconds) {
  int d = 0;
  std::string d_unity = "day";
  std::string day_part = "";
  int h = (int)(seconds / 3600);  // only integer to hour
  if (h > 24) {
    d = (int)(h / 24);
    h = (int)(h % 24);
    if (d > 1) {
      d_unity = "days";
    }
    day_part = std::to_string(d) + " " + d_unity + ", ";
  }
  int m = (int)(seconds % 3600 / 60);  // only integer to minute
  int s = (int)(seconds % 3600 % 60);  // only integer to seconds
  std::string hour_part, minutes_part, seconds_part;

  if (h < 10) {
    hour_part = ("0" + std::to_string(h));
  } else {
    hour_part = std::to_string(h);
  }

  if (m < 10) {
    minutes_part = ("0" + std::to_string(m));
  } else {
    minutes_part = std::to_string(m);
  }

  if (s < 10) {
    seconds_part = ("0" + std::to_string(s));
  } else {
    seconds_part = std::to_string(s);
  }

  return day_part + hour_part + ":" + minutes_part + ":" + seconds_part;
}
