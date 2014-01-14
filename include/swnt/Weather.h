/*

  Weather
  -------
  Fetches an RSS feed from the Yahoo weather services. The current
  weather state is used to influence the visuals of the water.

 */
#ifndef SWNT_WEATHER_H
#define SWNT_WEATHER_H

extern "C" {
#  include <uv.h>
#  include <curl/curl.h>
};

#include <rapidxml.hpp>
#include <vector>
#include <string>
#include <sstream>

#define WEATHER_TASK_NONE 0
#define WEATHER_TASK_FETCH_RSS 1
#define WEATHER_TASK_STOP 2

// -----------------------------------------------------------------------------

struct WeatherInfo;
std::string weather_fetch_url(std::string url);
size_t weather_write_data(void* ptr, size_t size, size_t nmemb, void* str);
std::string weather_download_yahoo_rss();
bool weather_parse_yahoo_rss(std::string& rss, WeatherInfo& result);
void weather_thread(void* user);
bool weather_parse_time(std::string time, int& hour, int& minutes); /* parses a time like: "8:45 am" or "4:28 pm", returns time int 24 hours scale */

// -----------------------------------------------------------------------------

struct WeatherInfo {
  WeatherInfo():temperature(0),humidity(0),visibility(0),pressure(0.0f)
    ,rising(0),chill(0),speed(0.0f),sunset_hour(0),sunset_minute(0),sunrise_hour()
    ,sunrise_minute(0),ttl(0){}

  void print();
  float getSun(float t);      /* gets the sun position based on time. t = 0 means midnight, t = 1 is 23:59 */

  int temperature;            /* condition: in Celcius */
  int humidity;               /* atmosphere: humitidy */
  int visibility;             /* atmosphere: visibility */
  float pressure;             /* atmosphere: pressure, in mb */
  int rising;                 /* atmosphere: rising */
  int chill;                  /* wind: chill */
  int direction;              /* wind: direction, in degrees */
  float speed;                /* wind: speed, in km/h */
  int sunrise_hour;  
  int sunrise_minute; 
  int sunset_hour;
  int sunset_minute; 
  int ttl;           /* number of minutes before we should refresh weather information */
};

// -----------------------------------------------------------------------------

struct WeatherTask {
  WeatherTask():type(WEATHER_TASK_NONE){}
  WeatherTask(int t):type(t){}
  int type;
};

// -----------------------------------------------------------------------------

class Weather {

 public:
  Weather();
  ~Weather();
  bool setup();                         /* initializes the weahter fetcher ... */
  void update();                        /* call this regurlarly, this will check if we need to fetch the rss feed again; the yahoo rss feed contains a TTL field that we use */
  bool hasInfo(WeatherInfo& result);    /* check if we have new weather information, it will set result and return true when we have new info */
  
 private:
  void addTask(WeatherTask* t);     /* add a new task to the thread; signas the thread */
  void fetchYahooRSS();             /* fetches the yahoo weather RSS feed, adds a new task for the thread */

 public:
  uv_thread_t thread;               /* the thread handle */
  uv_mutex_t task_mutex;            /* used to synchorinize thread + signalling the condition var */
  uv_mutex_t mutex;                 /* used for general syncing */
  uv_cond_t task_cv;                /* used to wait/signal the thread so it doesn't suck up cpu when there are no tasks */
  std::vector<WeatherTask*> tasks;  /* used with the threading; when a new task is added we signal the thread */
  WeatherInfo info;                 /* the latest weather info */
  bool has_new_info;                /* is set to true when we have new weather info */
  uint64_t timeout;                 /* when we update the weather data */
  uint64_t ttl;                     /* the last time to life value we got from the rss data; we only update once every ttl */
};

// -----------------------------------------------------------------------------

inline bool Weather::hasInfo(WeatherInfo& result) {
  bool h = has_new_info;
  if(has_new_info) {
    result = info;
    ttl = info.ttl;
    uv_mutex_lock(&mutex);
      has_new_info = false;
    uv_mutex_unlock(&mutex);
  }
  return h;
}

// -----------------------------------------------------------------------------

inline void WeatherInfo::print() {
  printf("weather.temperature: %d C\n", temperature);
  printf("weather.humidity: %d\n", humidity);
  printf("weather.visibility: %d\n", visibility);
  printf("weather.pressure: %f\n", pressure);
  printf("weather.rising: %d\n", rising);
  printf("weather.chill: %d\n", chill);
  printf("weather.direction: %d\n", direction);
  printf("weather.speed: %f\n", speed);
  printf("weather.sunrise: %d:%d\n", sunrise_hour, sunrise_minute);
  printf("weather.sunset: %d:%d\n", sunset_hour, sunset_hour);
  printf("weather.ttl: %d\n", ttl);
  printf("-\n");
}
#endif
