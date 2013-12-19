#include <assert.h>
#include <limits.h>
#include <swnt/Graphics.h>
#include <swnt/Tracking.h>
#include <swnt/Settings.h>

// ---------------------------------------------------------

Tracked::Tracked()
  :age(0)
  ,matched(false)
{
}

// ---------------------------------------------------------

Tracking::Tracking(Settings& settings, Graphics& graphics)
  :settings(settings)
  ,graphics(graphics)
  ,contour_threshold(100)
  ,vao(0)
  ,vbo(0)
  ,allocated_bytes(0)
  ,prev_num_points(0)
{
}

bool Tracking::setup() {

  if(!setupGraphics()) {
    return false;
  }

  return true;
}

bool Tracking::setupGraphics() {
  assert(allocated_bytes == 0);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableVertexAttribArray(0); // pos
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)0);

  return true;
}

void Tracking::track(unsigned char* pixels) {
  assert(pixels);

  findContours(pixels);

  updateVertices();

  clusterPoints();
}

void Tracking::draw(float tx, float ty) {
  
  if(!contour_vertices.size()) {
    return;
  }

  float sx = float(settings.win_w) / settings.image_processing_w;
  float sy = float(settings.win_h) / settings.image_processing_h;

  // Draw contours
  {
    vec3 line_col(0.0f, 1.0f, 0.0f);
    mat4 mm;
    //mm.translate(tx, ty, 0.0f);
    // scale contour to window size
    mm.scale(sx, sy, 1.0f);
 

    glBindVertexArray(vao);
    glUseProgram(graphics.v_prog);
    glUniform3fv(glGetUniformLocation(graphics.v_prog, "u_color"), 1, line_col.ptr());
    glUniformMatrix4fv(glGetUniformLocation(graphics.v_prog, "u_mm"), 1, GL_FALSE, mm.ptr());
    glUniformMatrix4fv(glGetUniformLocation(graphics.v_prog, "u_pm"), 1, GL_FALSE, settings.ortho_matrix.ptr());
    glMultiDrawArrays(GL_LINE_STRIP, &contour_offsets[0], &contour_nvertices[0], contour_nvertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // Draw tracked points 
  {
    vec3 col(0.0f, 1.0f, 0.0f);
    for(size_t i = 0; i < tracked.size(); ++i) {
      Tracked* t = tracked[i];
      graphics.drawCircle(t->position.x * sx, t->position.y * sy, 5, col);
    }
  }
}

bool Tracking::findContours(unsigned char* pixels) { 

  contours.clear();

  int w = settings.image_processing_w;
  int h = settings.image_processing_h;
  cv::Mat input_image(h, w, CV_8UC1, pixels, cv::Mat::AUTO_STEP);
  cv::findContours(input_image, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
  return contours.size();
}

void Tracking::updateVertices() {

  contour_vertices.clear();
  contour_offsets.clear();
  contour_nvertices.clear();
  closest_points.clear();

  if(!contours.size()) {
    return;
  }

  int closest_dist = INT_MAX;
  vec2 closest_point;
  vec2 contour_center(settings.image_processing_w * 0.5, settings.image_processing_h * 0.5);

  // Get all valid contours and find peaks
  {
    for(size_t k = 0; k < contours.size(); ++k) {
      
      if(contours[k].size() < contour_threshold) {
        continue;
      }

      std::vector<cv::Point>& pts = contours[k];
      size_t start_nvertices = contour_vertices.size();
      contour_offsets.push_back(contour_vertices.size());

      for(size_t i = 0; i < pts.size(); ++i) {

        cv::Point& pt = pts[i];
        vec2 v(pt.x, pt.y);
        contour_vertices.push_back(v);

      
        vec2 dir_to_center = contour_center - v;
        int dist_to_center = dot(dir_to_center, dir_to_center);
      
        if(dist_to_center < closest_dist) {
          closest_dist = dist_to_center;
          closest_point = v;
        }
      }

      closest_points.push_back(closest_point);
      closest_dist = INT_MAX;
      contour_nvertices.push_back(contour_vertices.size() - start_nvertices);
    }
  }

  if(!contour_vertices.size()) {
    return;
  }

  // Check if we need to update the vbo
  {

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    size_t nbytes_needed = sizeof(vec2) * contour_vertices.size();
    if(nbytes_needed > allocated_bytes) {
      glBufferData(GL_ARRAY_BUFFER, nbytes_needed, contour_vertices[0].ptr(), GL_STREAM_DRAW);
    }
    else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, nbytes_needed, contour_vertices[0].ptr());
    }

  }
}


void Tracking::clusterPoints() {
  
  // unset all  matched flags.
  for(size_t i = 0; i < tracked.size(); ++i) {
    tracked[i]->matched = false;
  }

  // when a blob went away we need to clear the history else clustering is wrong.
  if(prev_num_points > closest_points.size()) {
    closest_points_history.clear();
  }

  if(closest_points.size()) {
    std::copy(closest_points.begin(), closest_points.end(), std::back_inserter(closest_points_history));
  }

  if(!closest_points_history.size()) {
    return;
  }

  cv::Mat cluster_points(closest_points_history.size(), 2, CV_32F, closest_points_history[0].ptr(), cv::Mat::AUTO_STEP);
  cv::Mat cluster_labels;
  cv::Mat cluster_centers(closest_points.size(), 2, CV_32F);
  cv::TermCriteria cluster_crit = cv::TermCriteria(cv::TermCriteria::MAX_ITER, closest_points_history.size(), 0);
  cv::kmeans(cluster_points, closest_points.size(), cluster_labels, cluster_crit, 0, cv::KMEANS_PP_CENTERS, cluster_centers);

  prev_num_points = closest_points.size();

  // find already tracked points and match the closest ones
  int max_dist = 15 * 15; 

  for(int j = 0; j < cluster_centers.rows; ++j) {

    vec2 cluster_center(cluster_centers.at<float>(j, 0), cluster_centers.at<float>(j, 1));
    Tracked* matched = NULL;
    int min_dist = INT_MAX;

    for(size_t k = 0; k < tracked.size(); ++k) {

      Tracked* tr = tracked[k];
      vec2 dir = tr->position - cluster_center;
      int dist = dot(dir, dir);

      if(dist > max_dist) {
        continue;
      }

      if(dist < min_dist) {
        min_dist = dist;
        matched = tr;
      }
    }

    if(matched) {
      matched->position = cluster_center;
      matched->matched = true;
      matched->age = 0;
    }
    else {
      Tracked* tr = new Tracked();
      tr->position = cluster_center;
      tr->matched = true;
      tracked.push_back(tr);
    }
  }

  // all non-matched will age and die
  std::vector<Tracked*>::iterator it = tracked.begin(); 
  while(it != tracked.end()) {
    Tracked* tr = *it;
    if(!tr->matched) {
      tr->age++;
      if(tr->age > 25) {
        delete tr;
        it = tracked.erase(it);
        continue;
      }
    }
   
    ++it;
  }

  if(closest_points_history.size() > 50) {
    while(closest_points_history.size() > 50) {
      closest_points_history.erase(closest_points_history.begin());
    }
  }
}
