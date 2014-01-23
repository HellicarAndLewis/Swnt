#include <swnt/Weather.h>
#include <tinylib.h>


#define XML_ERR_CHECK(n, err) if(!n) { printf("%s", err); return false; } 

using namespace rapidxml;

// -----------------------------------------------------------------------------

// safely parsing a time string like: "8:25 pm", "8:30 am", and converting it to 24 time
bool weather_parse_time(std::string t, int& hour, int& minute) {

  hour = 0;
  minute = 0;

  if(!t.size()) {
    return false;
  }

  std::string shour;
  std::string smin;
  std::string sampm;
  int state = 0;

  for(size_t i = 0; i < t.size(); ++i) {
    if(t[i] == ':') {
      state = 1;
      continue;
    }
    else if(state == 1 && t[i] == ' ') {
      state = 2;
      continue;
    }

    if(state == 0) {
      shour.push_back(t[i]);
    }
    else if(state == 1) {
      smin.push_back(t[i]);
    }
    else if(state == 2) {
      sampm.push_back(t[i]);
    }
  }

  if(!shour.size() || !smin.size() || !sampm.size()) {
    printf("Warning: could not parse weather time: %s\n", t.c_str());
    return false;
  }

  if(sampm.size() != 2) {
    printf("Warning: the weather am/pm flag is invalid.\n");
    return false;
  }

  std::stringstream ss_hour(shour);
  std::stringstream ss_min(smin);
  ss_hour >> hour;
  ss_min >> minute;
  if(sampm == "pm") {
    hour += 12;
  }

  return true;
}
// -----------------------------------------------------------------------------

bool weather_parse_yahoo_rss(std::string& rss, WeatherInfo& result) {

  xml_document<> doc;
  try {
    doc.parse<0>((char*)rss.c_str());
    xml_node<>* rss = doc.first_node("rss");             XML_ERR_CHECK(rss,     "Error: cannot find the rss node.\n");
    xml_node<>* channel = rss->first_node("channel");    XML_ERR_CHECK(channel, "Error: cannot find the channel node.\n");
    xml_node<>* item = channel->first_node("item");      XML_ERR_CHECK(item,    "Error: cannot find the item node.\n");
    
    // CONDITION
    {
      xml_node<>* condition = item->first_node("yweather:condition");  XML_ERR_CHECK(condition, "Error: cannot find the yweather:condition tag in the rss. Maybe invalid rss or yahoo changed their feed.\n");
      xml_attribute<>* temp_att = condition->first_attribute("temp");  XML_ERR_CHECK(temp_att,  "Error: cannot find the temperature attirbute in the weather rss.\n");
      result.temperature = rx_to_int(temp_att->value());
    }

    // ATMOSPHERE
    {
      xml_node<>* atmos = channel->first_node("yweather:atmosphere");     XML_ERR_CHECK(atmos,      "Error: cannot find the atmosphere node.\n");
      xml_attribute<>* humid_att = atmos->first_attribute("humidity");    XML_ERR_CHECK(humid_att,  "Error: cannot find the humitidy in the weather rss.\n");
      xml_attribute<>* vis_att = atmos->first_attribute("visibility");    XML_ERR_CHECK(vis_att,    "Error: cannot find the visibility attribute in the weather rss.\n");
      xml_attribute<>* press_att = atmos->first_attribute("pressure");    XML_ERR_CHECK(press_att,  "Error: cannot find the pressure attribute in the weather rss.\n");
      xml_attribute<>* rising_att = atmos->first_attribute("rising");     XML_ERR_CHECK(rising_att, "Error: cannot find the rising attirbute in the weather rss.\n");
      result.humidity = rx_to_int(humid_att->value());
      result.visibility = rx_to_int(vis_att->value());
      result.pressure = rx_to_float(press_att->value());
      result.rising = rx_to_int(rising_att->value());
    }

    // WIND
    {
      xml_node<>* wind = channel->first_node("yweather:wind");        XML_ERR_CHECK(wind,       "Error: cannot find the wind node in the weather rss.\n");
      xml_attribute<>* chill_att = wind->first_attribute("chill");    XML_ERR_CHECK(chill_att,  "Error: cannot find the chill attribute in the wind node.\n");
      xml_attribute<>* dir_att = wind->first_attribute("direction");  XML_ERR_CHECK(dir_att,    "Error: cannot find the directon attribute in the wind node.\n");
      xml_attribute<>* speed_att = wind->first_attribute("speed");    XML_ERR_CHECK(speed_att,  "Error: cannot find the speed attribute in the wind node.\n");
      result.chill = rx_to_int(chill_att->value());
      result.direction = rx_to_int(dir_att->value());
      result.speed = rx_to_float(speed_att->value());
    }

    // ASTRONOMY
    {
      xml_node<>* astr = channel->first_node("yweather:astronomy");  XML_ERR_CHECK(astr, "Error: cannot find the astronomy part.\n");
      xml_attribute<>* sunrise = astr->first_attribute("sunrise");   XML_ERR_CHECK(sunrise, "Error: cannot get the sunrise attribute.\n");
      xml_attribute<>* sunset = astr->first_attribute("sunset");     XML_ERR_CHECK(sunset, "Error: cannot get the sunset attribute.\n");
      weather_parse_time(sunrise->value(), result.sunrise_hour, result.sunrise_minute);
      weather_parse_time(sunset->value(), result.sunset_hour, result.sunset_minute);
    }    

    // TTL
    {
      xml_node<>* ttl = channel->first_node("ttl"); XML_ERR_CHECK(ttl, "Error: cannot find the TTL entry in the weather data.\n");
      result.ttl = rx_to_int(ttl->value());
    }
  }
  catch(parse_error& er) {
    printf("Error: cannot parse yahoo weather rss  %s\n", er.what());
  }
  catch(...) {
    printf("Error: cannot open/parse the yahoo rss.\n");
    return false;
  }
  return true;
}

size_t weather_write_data(void* ptr, size_t size, size_t nmemb, void* str) {
  std::string* s = static_cast<std::string*>(str);
  std::copy((char*)ptr, (char*)ptr + (size * nmemb), std::back_inserter(*s));
  return size * nmemb;
}

// "http://weather.yahooapis.com/forecastrss?w=10242&u=c"; // w=10242 is Aberaron
std::string weather_fetch_url(std::string url) {
  std::string result;

  CURL* curl = NULL;

  CURLcode res;
  curl = curl_easy_init();
  if(!curl) {
    printf("Error: cannot initialize CURL.\n");
    return result;
  }

  res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  if(res != CURLE_OK) {
    printf("Cannot set curl url.\n");
    goto curl_error;
  }

  res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  if(res != CURLE_OK) {
    printf("Cannot set curl follow location flag.\n");
    goto curl_error;
  }

  res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, weather_write_data);
  if(res != CURLE_OK) {
    printf("Cannot set the weather write function.\n");
    goto curl_error;
  }
  
  res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
  if(res != CURLE_OK) {
    printf("Cannot set the curl write data.\n");
    goto curl_error;
  }

  res = curl_easy_perform(curl);
  if(res != CURLE_OK) {
    printf("Cannot perform curl.\n");
    goto curl_error;
  }

  return result;
  
 curl_error: 
    curl_easy_cleanup(curl);
    curl = NULL;
    return result;

}

std::string weather_download_yahoo_rss() {
  std::string result;
  std::string url = "http://weather.yahooapis.com/forecastrss?w=10242&u=c"; // w=10242 is Aberaron
  result = weather_fetch_url(url);
  return result;
}

void weather_thread(void* user) {
  Weather* w = static_cast<Weather*>(user);
  std::vector<WeatherTask*> work;
  bool must_stop = false;

  while(true) {
    
    uv_mutex_lock(&w->task_mutex);
    while(w->tasks.size() == 0) {
      uv_cond_wait(&w->task_cv, &w->task_mutex);
    }
    std::copy(w->tasks.begin(), w->tasks.end(), std::back_inserter(work));
    w->tasks.clear();
    uv_mutex_unlock(&w->task_mutex);
    
    for(std::vector<WeatherTask*>::iterator it = work.begin(); it != work.end(); ++it) {
      WeatherTask* task = *it;
      if(task->type == WEATHER_TASK_FETCH_RSS) {
        WeatherInfo weather_info;
        std::string weather_rss = weather_download_yahoo_rss();
        if(weather_rss.size()) {
          if(weather_parse_yahoo_rss(weather_rss, weather_info)) {
            uv_mutex_lock(&w->mutex);
            {
              w->info = weather_info;
              weather_info.print();
              w->has_new_info = true;
            }
            uv_mutex_unlock(&w->mutex);
          }
        }
        delete task;
        task = NULL;
      }
      else if(task->type == WEATHER_TASK_STOP) {
        must_stop = true;
        delete task;
        task = NULL;
        break;
      }
    }

    work.clear();
    
    if(must_stop) {
      break;
    }

  }
}

// -----------------------------------------------------------------------------

Weather::Weather() 
  :has_new_info(false)
  ,timeout(0)
  ,ttl(5)
{
  uv_mutex_init(&mutex);
  uv_mutex_init(&task_mutex);
  uv_cond_init(&task_cv);
}

Weather::~Weather() {
  // stop + join the thread
  WeatherTask* t = new WeatherTask(WEATHER_TASK_STOP);
  addTask(t);
  uv_thread_join(&thread);

  // cleanup
  uv_mutex_destroy(&task_mutex);
  uv_cond_destroy(&task_cv);
  uv_mutex_destroy(&mutex);

  for(std::vector<WeatherTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
    delete *it;
  }
  tasks.clear();

  has_new_info = false;
}

bool Weather::setup() {
  uv_thread_create(&thread, weather_thread, this);
  fetchYahooRSS();
  return true;
}

void Weather::fetchYahooRSS() {
  WeatherTask* t = new WeatherTask(WEATHER_TASK_FETCH_RSS);
  addTask(t);
}

void Weather::addTask(WeatherTask* t) {
  uv_mutex_lock(&task_mutex);
  tasks.push_back(t);
  uv_cond_signal(&task_cv);
  uv_mutex_unlock(&task_mutex);
}

void Weather::update() {
  uint64_t now = uv_hrtime();
  if(now > timeout) {
    fetchYahooRSS();
    uint64_t minutes = ttl;
    timeout = now + minutes * (1000000ull * 1000ull * 60ull);
  }
}

// -----------------------------------------------------------------------------
float WeatherInfo::getSun(float t) {
  t = CLAMP(t, 0.0f, 1.0f);
  float day_hours = sunset_hour - sunrise_hour;
  float in_hour = t * 24;

  // night
  if(in_hour < sunrise_hour || in_hour > sunset_hour) {
    return 0.0f;
  }

  // day (@todo maybe add minutes too)
  float k_hour = (in_hour - sunrise_hour) / day_hours;
  float sunpos = sin(k_hour * PI);
  return sunpos;
}
