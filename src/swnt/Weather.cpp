#include <swnt/Weather.h>
#include <tinylib.h>

#define XML_ERR_CHECK(n, err) if(!n) { printf("%s", err); return false; } 

using namespace rapidxml;

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
//   std::string url = "http://weather.yahooapis.com/forecastrss?w=10242&u=c"; // w=10242 is Aberaron
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

  printf("weather thread.\n");
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
              w->has_new_info = true;
            }
            uv_mutex_unlock(&w->mutex);
          }
        }
        delete task;
        task = NULL;
      }
      else if(task->type == WEATHER_TASK_STOP) {
        printf("Weather thread asked to stop.\n");
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

  printf("Weather thread stopped.\n");
}

// -----------------------------------------------------------------------------

Weather::Weather() 
  :has_new_info(false)
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
}

