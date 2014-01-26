/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------
*/

#ifndef ROXLU_TIDES_H
#define ROXLU_TIDES_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <algorithm>

#define TIDES_PARSE_STATE_NONE 0
#define TIDES_PARSE_STATE_HEADER0 1
#define TIDES_PARSE_STATE_HEADER1 2


int tides_to_int(std::string& str); 
float tides_to_foat(std::string& str);
void tides_parse_date(std::string& str, int& year, int& month, int& day);

// -------------------------------------------------------------

struct TidesEvent {
  TidesEvent();
  void clear();

  char name;
  float height;
  int time_hour;
  int time_minutes;
};

// -------------------------------------------------------------

struct TidesHeaderLine0 {
  TidesHeaderLine0();
  void clear();
  void print();

  int port_number;
  std::string port_name;
  float latitude;
  float longitude;
  int date_year;
  int date_month;
  int date_day;
  int time_interval_height;
  int time_zone;
};

// -------------------------------------------------------------

struct TidesHeaderLine1 {
  TidesHeaderLine1();
  void clear();

  int time_hour;
  int time_minutes;

  TidesEvent event0;
  TidesEvent event1;

};

// -------------------------------------------------------------

struct TidesEntry {
  void clear();
  float getInterpolatedHeight(float t);  /* this will return the interpolated height for the given percentage. 0 = begin of data, 1 = end of data. The first entry is assumed to be 00:00 (midnight) */

  TidesHeaderLine0 header0;
  TidesHeaderLine1 header1;
  std::vector<int> interval_data;
  uint32_t min_value;
  uint32_t max_value;
};

// -------------------------------------------------------------

class Tides {

 public:
  Tides();
  bool setup(std::string filepath);                                /* loads the tides information from a text file */
  void print();                                                    /* prints some debugging info */
  bool getEntry(int year, int month, int day, TidesEntry& result); /* get a specific entry for the given day */

 private:
  bool parse(std::string& data);
  std::vector<TidesEntry> entries;
};

#endif
