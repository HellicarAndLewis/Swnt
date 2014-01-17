#include <assert.h>
#include <limits.h>
#include <swnt/Graphics.h>
#include <swnt/Tracking.h>
#include <swnt/Settings.h>

// ---------------------------------------------------------

Tracked::Tracked()
  :age(0)
  ,matched(false)
  ,id(0)
  ,lost(0)
{
}

// ---------------------------------------------------------

Tracking::Tracking(Settings& settings, Graphics& graphics)
  :settings(settings)
  ,graphics(graphics)
  ,draw_contours(false)
  ,draw_triangulated_blobs(true)
  ,draw_tracking_points(false)
  ,contour_threshold(100)
  ,vao(0)
  ,vbo(0)
  ,allocated_bytes(0)
  ,prev_num_points(0)
  ,tan_count(0)
  ,tan_offset(0)
  ,num_tracked(0)
  ,blob_vertices_vbo(0)
  ,blob_vao(0)
  ,blob_vertices_allocated(0)
  ,blob_frag(0)
  ,blob_vert(0)
  ,blob_prog(0)
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

  // program to render the triangulated blobs
  const char* blob_atts[] = { "a_pos" } ;
  blob_vert = rx_create_shader(GL_VERTEX_SHADER, BLOB_VS);
  blob_frag = rx_create_shader(GL_FRAGMENT_SHADER, BLOB_FS);
  blob_prog = rx_create_program_with_attribs(blob_vert, blob_frag, 1, blob_atts);
  glUseProgram(blob_prog);
  glUniformMatrix4fv(glGetUniformLocation(blob_prog, "u_pm"), 1, GL_FALSE, settings.ortho_matrix.ptr());

  glGenVertexArrays(1, &blob_vao);
  glBindVertexArray(blob_vao);
  glGenBuffers(1, &blob_vertices_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, blob_vertices_vbo);
  glEnableVertexAttribArray(0); // pos
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(BlobVertex), (GLvoid*)0);
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
  if(draw_contours) {
    vec3 line_col(0.0f, 1.0f, 0.0f);
    mat4 mm;

    // scale contour to window size
    mm.scale(sx, sy, 1.0f);
 
    // draw contour
    glBindVertexArray(vao);
    glUseProgram(graphics.v_prog);
    glUniform3fv(glGetUniformLocation(graphics.v_prog, "u_color"), 1, line_col.ptr());
    glUniformMatrix4fv(glGetUniformLocation(graphics.v_prog, "u_mm"), 1, GL_FALSE, mm.ptr());
    glUniformMatrix4fv(glGetUniformLocation(graphics.v_prog, "u_pm"), 1, GL_FALSE, settings.ortho_matrix.ptr());
    glMultiDrawArrays(GL_LINE_STRIP, &contour_offsets[0], &contour_nvertices[0], contour_nvertices.size());

    // draw tangents
    line_col.set(1.0, 0.0, 0.0);
    glUniform3fv(glGetUniformLocation(graphics.v_prog, "u_color"), 1, line_col.ptr());
    glDrawArrays(GL_LINES, tan_offset, tan_count);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // Draw tracked points 
  if(draw_tracking_points && tracked.size()) {
    vec3 col(0.0f, 1.0f, 0.0f);
    for(size_t i = 0; i < tracked.size(); ++i) {
      Tracked* t = tracked[i];
      graphics.drawCircle(t->position.x * sx, t->position.y * sy, 5, col);
    }
  }

  // Draw triangulated
  if(draw_triangulated_blobs && blob_counts.size()) {
    mat4 mm;
    mm.scale(sx, sy, 1.0f);

    glUseProgram(blob_prog);
    glUniformMatrix4fv(glGetUniformLocation(blob_prog, "u_mm"), 1, GL_FALSE, mm.ptr());
    glBindVertexArray(blob_vao);
    glMultiDrawArrays(GL_TRIANGLES, &blob_offsets.front(), &blob_counts.front(), blob_counts.size());
  }
}

bool Tracking::findContours(unsigned char* pixels) { 

  contours.clear();

  int w = settings.image_processing_w;
  int h = settings.image_processing_h;
  cv::Mat input_image(h, w, CV_8UC1, pixels, cv::Mat::AUTO_STEP);
  //cv::findContours(input_image, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
  cv::findContours(input_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  return contours.size();
}

void Tracking::updateVertices() {

  contour_vertices.clear();
  contour_offsets.clear();
  contour_nvertices.clear();
  closest_points.clear();
  closest_dirs.clear();
  closest_centers.clear();
  blob_offsets.clear();
  blob_counts.clear();
  blob_vertices.clear();

  if(!contours.size()) {
    return;
  }

  int closest_dist = INT_MAX;
  vec2 closest_point;
  vec2 closest_dir; 
  vec2 contour_center(settings.image_processing_w * 0.5, settings.image_processing_h * 0.5);
  float radius = 220;

  // Get all valid contours and find peaks
  {
    for(size_t k = 0; k < contours.size(); ++k) {
      
      if(contours[k].size() < contour_threshold) {
        continue;
      }

      std::vector<cv::Point>& pts = contours[k];
      size_t start_nvertices = contour_vertices.size();
      contour_offsets.push_back(contour_vertices.size());

      int last_point = 0;
      tri.clear();

      for(size_t i = 0; i < pts.size(); ++i) {

        cv::Point& pt = pts[i];
        vec2 v(pt.x, pt.y);

        contour_vertices.push_back(v);
    
        vec2 dir_to_center = contour_center - v;
        int dist_to_center = dot(dir_to_center, dir_to_center);

        if(dist_to_center < closest_dist) {
          closest_dir = dir_to_center;
          closest_dist = dist_to_center;
          closest_point = v;
        }

        // The Triangle library is a little bit awkward as it "may" crash when you 
        // use the "p" option (necessary to create non convect triangulated shapes). The 
        // library may crash in these situations when the angle between points is too small,
        // because of rounding issue, etc..  I found that adding every 5th element seems
        // to work well.
        if((i - last_point) >= 5) {
          tri.add(pt.x, pt.y);
          last_point = int(i);
        }
      }

      #if 1
      if(tri.size() > 3) {

        struct triangulateio out;
        tri.triangulate("zpQ", out);

        if(out.numberoftriangles) {
          blob_offsets.push_back(blob_vertices.size());
          for(int i = 0; i  < out.numberoftriangles; ++i) {
            int a = out.trianglelist[i * 3 + 0];
            int b = out.trianglelist[i * 3 + 1];
            int c = out.trianglelist[i * 3 + 2];
            vec2 pa(out.pointlist[(a * 2) + 0], out.pointlist[(a * 2) + 1]);
            vec2 pb(out.pointlist[(b * 2) + 0], out.pointlist[(b * 2) + 1]);
            vec2 pc(out.pointlist[(c * 2) + 0], out.pointlist[(c * 2) + 1]);
            blob_vertices.push_back(BlobVertex(pa));
            blob_vertices.push_back(BlobVertex(pb));
            blob_vertices.push_back(BlobVertex(pc));
          }
          blob_counts.push_back(blob_vertices.size() - blob_offsets.back());
        }

        tri.free(out);
      }
      #endif

      // adjust the closest point from the center to the contain, so it's between the `radius` and the found point. this will make sure the water drops are nicely on the hands
      vec2 closest_dir_norm = normalized(closest_dir);
      vec2 to_radius = closest_dir_norm * radius;
      vec2 to_radius_d = settings.radius - length(to_radius - closest_dir);
      float dist_to_radius = length(to_radius_d);
      closest_point = contour_center - (closest_dir_norm * dist_to_radius * 0.5); 

      closest_dirs.push_back(closest_dir);
      closest_points.push_back(closest_point);
      closest_dist = INT_MAX;
      contour_nvertices.push_back(contour_vertices.size() - start_nvertices);
    }
  }
  
  {
    tan_offset = contour_vertices.size();

    // calculate the tangents which are used to spawn particles 
    std::vector<vec2> tan_vertices;
    int step_size = 30;
    for(int i = 0; i < int(contour_vertices.size())-(step_size+1); i += step_size) {
      if( ( i + step_size) > contour_vertices.size() -1 ) {
        break;
      }
      vec2 p0 = contour_vertices[i];
      vec2 p1 = contour_vertices[i+step_size];
      vec2 dir = p1 - p0;
      vec2 tan(-dir.y, dir.x);
      tan = normalized(tan) * 30;
      tan_vertices.push_back(p0);
      tan_vertices.push_back(p0 + tan);
    }

    std::copy(tan_vertices.begin(), tan_vertices.end(), std::back_inserter(contour_vertices));
    tan_count = tan_vertices.size();
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

  // Update the triangulate vertices.
  if(blob_vertices.size()) {

    glBindBuffer(GL_ARRAY_BUFFER, blob_vertices_vbo);
    size_t nbytes_needed = blob_vertices.size() * sizeof(BlobVertex);

    if(nbytes_needed > blob_vertices_allocated) {
      glBufferData(GL_ARRAY_BUFFER, nbytes_needed, blob_vertices[0].pos.ptr(), GL_STREAM_DRAW);
      blob_vertices_allocated = nbytes_needed;
    }
    else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, nbytes_needed, blob_vertices[0].pos.ptr());
    }
  }
}


void Tracking::clusterPoints() {
  
  prev_num_tracked = num_tracked;
  num_tracked = 0;
  
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
      matched->age++;
      num_tracked++;
    }
    else {
      Tracked* tr = new Tracked();
      tr->position = cluster_center;
      tr->matched = true;
      tr->id = ++last_id;
      tracked.push_back(tr);
      num_tracked++;
    }
  }

  // all non-matched will age and die
  std::vector<Tracked*>::iterator it = tracked.begin(); 
  while(it != tracked.end()) {
    Tracked* tr = *it;
    if(!tr->matched) {
      tr->lost++;
      if(tr->lost > 50) {
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

