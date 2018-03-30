#ifndef defs_H
#define defs_H

enum attribute_type{
  ONOFF = 0,
  LIGHTNESS,
  TEMPERATURE,
  COLOR,
  MODE,
  TIMER_ON,
  TIMER_OFF,
  ONOFFLINE
};

enum onoff_value{
  OFF = 0,
  ON
};

enum mode_value{
  LIGHTNING = 0,
  READING,
  MEAL,
  MOVIE,
  PARTY,
  NIGHTLAMP
};

enum online_value{
  OFFLINE = 0,
  ONLINE
};


#endif
