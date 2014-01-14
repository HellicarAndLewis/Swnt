#include <string>
#include <swnt/Tides.h>

// -------------------------------------------------------------
int tides_to_int(std::string& str) {
  int r = 0;
  std::stringstream ss(str);
  ss >> r;
  return r;
}

float tides_to_float(std::string& str) {
  float r = 0.0;
  std::stringstream ss(str);
  ss >> r;
  return r;
}

void tides_parse_date(std::string& str, int& year, int& month, int& day) {

  if(str.size() != 8) {
    printf("Warning: tides date is invalid: %s\n", str.c_str());
    return;
  }

  std::string s_year = str.substr(0, 4);
  std::string s_month = str.substr(4, 2);
  std::string s_day = str.substr(6, 2);

  year = tides_to_int(s_year);
  month = tides_to_int(s_month);
  day = tides_to_int(s_day);
}

// -------------------------------------------------------------

TidesEvent::TidesEvent() {
  clear();
}

void TidesEvent::clear() {
  name = '0';
  height = 0.0f;
  time_hour = -1;
  time_minutes = -1;
}

// -------------------------------------------------------------

TidesHeaderLine0::TidesHeaderLine0() {
  clear();
}

void TidesHeaderLine0::clear() {
  port_number = -1;
  latitude = -1.0f;
  longitude = -1.0f;
  date_year = -1;
  date_month = -1;
  time_interval_height = -1;
  time_zone = 0;
}

void TidesHeaderLine0::print() {
  printf("line0.port_number: %d\n", port_number);
  printf("line0.port_name: %s\n", port_name.c_str());
  printf("line0.longitude: %f\n", longitude);
  printf("line0.latitude: %f\n", latitude);
  printf("line0.date_year: %d\n", date_year);
  printf("line0.date_month: %d\n", date_month);
  printf("line0.date_day: %d\n", date_day);
  printf("line0.time_interval_height: %d\n", time_interval_height);
  printf("line0.time_zone: %d\n", time_zone);
}

// -------------------------------------------------------------

TidesHeaderLine1::TidesHeaderLine1() {
  clear();
}

void TidesHeaderLine1::clear() {
  time_hour = -1;
  time_minutes = -1;
}

// -------------------------------------------------------------

void TidesEntry::clear() {
  header0.clear();
  header1.clear();
  interval_data.clear();
  min_value = 99999;
  max_value = 0;
}

float TidesEntry::getInterpolatedHeight(float t) {
  if(interval_data.size() <= 2) {
    return interval_data[0];
  }

  size_t dx0 = 0;
  size_t dx1 = 0;

  if(t <= 0.0001) {
    t = 0.0;
    dx0 = 0;
    dx1 = 1;
  }
  else if(t > 1.0) {
    t = 1.0;
    dx0 = interval_data.size() - 2;
    dx1 = interval_data.size() - 1;
  }
  else {
    dx0 = t * (interval_data.size()-1);
    dx1 = dx0 + 1;
  }

  float h0 = interval_data[dx0];
  float h1 = interval_data[dx1];
  float mapped = t / (1.0 / interval_data.size());
  mapped = mapped - int(mapped);

  //printf("dx0: %ld, dx1: %ld, total: %ld, %d, %d\n", dx0, dx1, interval_data.size(), interval_data[dx0], interval_data[dx1]);
  return h0 + t * (h1 - h0);
}

// -------------------------------------------------------------

Tides::Tides() {
}

bool Tides::setup(std::string filepath) {

  std::ifstream ifs(filepath.c_str(), std::ios::in);

  if(!ifs.is_open()) {
    printf("Error: cannot open the given Tides file.\n");
    return false;
  }

  int state = TIDES_PARSE_STATE_NONE;
  std::string line;

  TidesEntry entry;

  while(std::getline(ifs, line)) {

    if(!line.size()) {
      continue;
    }

    if(line.at(0) == '"') {
      state = TIDES_PARSE_STATE_NONE;

      if(entry.interval_data.size()) {
        entries.push_back(entry);
      }
    }

    switch(state) {

      // PARSE THE FIRST HEADER LINE
      case TIDES_PARSE_STATE_NONE: {

        if(line.at(0) == '"') {
          state = TIDES_PARSE_STATE_HEADER0; 
          entry.clear();
          std::stringstream ss(line);
          std::string part; 
          int part_num = 0;

          while(std::getline(ss, part, ',')) {
            switch(part_num) {
              case 0: {
                std::replace(part.begin(), part.end(),'"',' ');
                entry.header0.port_number = tides_to_int(part);
                break;
              }
              case 1: { 
                entry.header0.port_name = part;    
                break;   
              }
              case 2: { 
                entry.header0.latitude = tides_to_float(part); 
                break; 
              } 
              case 3: {
                entry.header0.longitude = tides_to_float(part); 
                break;
              }
              case 4: { 
                tides_parse_date(part, 
                                 entry.header0.date_year, 
                                 entry.header0.date_month, 
                                 entry.header0.date_day); 
                break; 
              } 
              case 5: { 
                entry.header0.time_interval_height = tides_to_int(part); 
                break; 
              } 
              case 6: {
                if(part.size()) {
                  std::string sign = part.substr(0,1);
                  std::string s_timezone = part.substr(1);
                  entry.header0.time_zone = tides_to_int(s_timezone);
                  if(sign == "-") {
                    entry.header0.time_zone *= -1;
                  }
                }
                else {
                  printf("Error: the timezone field is invalid: %s\n", part.c_str());
                }
                break;
              }

            }
            part_num++;
          }
          break;
        }
        else {
          printf("Error: no header line found int Tides data, line starts with: %c\n", line.at(0));
        }
        break;
      } // end: TIDES_PARSE_STATE_NONE



      // PARSE THE SECOND HEADER LINE
      case TIDES_PARSE_STATE_HEADER0: {
        state = TIDES_PARSE_STATE_HEADER1; 
        break;
      }


      // PARSE THE DATA ENTRIES
      case TIDES_PARSE_STATE_HEADER1: {
        int curr_h = tides_to_int(line);

        if(curr_h > entry.max_value) {
          entry.max_value = curr_h;
        }

        if(curr_h < entry.min_value) {
          entry.min_value = curr_h;
        }

        entry.interval_data.push_back(curr_h);
        break;
      };


      default: {
        // printf("Error: unknown data in tides file. Cannot parse.\n");
        break;
      }
    }
  }
  return true;
}

void Tides::print() {
  for(std::vector<TidesEntry>::iterator it = entries.begin(); it != entries.end(); ++it) {
    TidesEntry& e = *it;
    printf("%d - %d - %d, intervals: %ld\n", e.header0.date_year, e.header0.date_month, e.header0.date_day, e.interval_data.size());
  }
}


bool Tides::getEntry(int year, int month, int day, TidesEntry& result) {
  for(std::vector<TidesEntry>::iterator it = entries.begin(); it != entries.end(); ++it) {
    TidesEntry e = *it;
    if(e.header0.date_year == year && e.header0.date_month == month && e.header0.date_day == day) {
      result = e;
      return true;
    }
  }
  return false;
}
