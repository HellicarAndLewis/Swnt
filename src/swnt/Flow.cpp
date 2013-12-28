#include <assert.h>
#include <swnt/Flow.h>
#include <swnt/Settings.h>
#include <swnt/Graphics.h>

Flow::Flow(Settings& settings, Graphics& graphics) 
  :settings(settings)
  ,graphics(graphics)
  ,prev_image(NULL)
  ,field_vao(0)
  ,field_vbo(0)
  ,field_bytes_allocated(0)
  ,field_size(128)
  ,perlin(4, 4, TWO_PI, 94)
  ,flow_tex(0)
{
}

bool Flow::setup() {
  
  size_t nbytes = settings.image_processing_w * settings.image_processing_h ;
  prev_image = new unsigned char[nbytes];
  
  if(!prev_image) {
    printf("Error: cannot allocate the bytes for the previous image.\n");
    return false;
  }

  memset(prev_image, 0x00, nbytes);

  if(!setupGraphics()) {
    printf("Error: cannot setup the GL state in Flow.\n");
    return false;
  }

  if(field_size <= 0) {
    printf("Error: invalid field size.\n");
    return false;
  }

  velocities.assign((field_size * field_size), vec2());
  heights.assign((field_size * field_size), 0.0f);

  glGenTextures(1, &flow_tex);
  glBindTexture(GL_TEXTURE_2D, flow_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, field_size, field_size, 0, GL_RG, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  printf("flow.flow_tex: %d\n", flow_tex);
  return true;
}

bool Flow::setupGraphics() {
  glGenVertexArrays(1, &field_vao);
  glBindVertexArray(field_vao);
  
  glGenBuffers(1, &field_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, field_vbo);

  glEnableVertexAttribArray(0); // pos
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)0); // pos
  return true;
}

void Flow::updateFlowTexture() {
  glBindTexture(GL_TEXTURE_2D, flow_tex);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, field_size, field_size, GL_RG, GL_FLOAT, velocities[0].ptr());
}

void Flow::draw(){

  assert(settings.color_dx < settings.colors.size());

  if(field_vertices.size()) {
    glBindVertexArray(field_vao);
    glUseProgram(graphics.v_prog);

    mat4 mm;
    mm.translate(0, 0.0, 0);
    glUniform3fv(glGetUniformLocation(graphics.v_prog, "u_color"), 1, settings.colors[settings.color_dx].flow_lines.ptr());
    glUniformMatrix4fv(glGetUniformLocation(graphics.v_prog, "u_mm"), 1, GL_FALSE, mm.ptr());
    glUniformMatrix4fv(glGetUniformLocation(graphics.v_prog, "u_pm"), 1, GL_FALSE, settings.ortho_matrix.ptr());
    glDrawArrays(GL_LINES, 0, field_vertices.size());
    glDrawArrays(GL_POINTS, 0, field_vertices.size());
  }
}

/*
  
  This function will find good points to track in the previous frame
  and then tries to find those points in the current image (curr). The first
  time this function is ran we get wrong results as there is no "previous" image
  from which we can get good points to track.

  After finding good features in the previous frame, we use cv::calcOpticalFlowPyrLK
  to find those points in the current image. 

 */
void Flow::calc(unsigned char* curr) {
  calcFlow(curr);
  updateVelocityField();
  updateFieldVertices();
  dampVelocities();
}

void Flow::calcFlow(unsigned char* curr) {
  assert(prev_image);

  prev_good_points.clear();
  curr_good_points.clear();
  status.clear();

  int w = settings.image_processing_w;
  int h = settings.image_processing_h;
  size_t nbytes = w * h;

  cv::Mat mat_curr(h, w, CV_8UC1, curr, cv::Mat::AUTO_STEP);
  cv::Mat mat_prev(h, w, CV_8UC1, prev_image, cv::Mat::AUTO_STEP);

  cv::goodFeaturesToTrack(mat_prev,            // input, the image from which we want to know good features to track
                          prev_good_points,    // output, the points will be stored in this output vector
                          10,                  // max points, maximum number of good features to track
                          0.05,                // quality level, "minimal accepted quality of corners", the lower the more points we will get
                          10,                  // minDistance, minimum distance between points
                          cv::Mat(),           // mask
                          4,                   // block size
                          false,               // useHarrisDetector, makes tracking a bit better when set to true
                          0.04                 // free parameter for harris detector
                          );
  
  
  if(!prev_good_points.size()) {
    memcpy(prev_image, curr, nbytes);
    return;
  }

  cv::TermCriteria termcrit(cv::TermCriteria::COUNT|cv::TermCriteria::EPS,prev_good_points.size(),0.03);
  std::vector<float> error;

  curr_good_points.assign(prev_good_points.size(), cv::Point2f());

  cv::calcOpticalFlowPyrLK(mat_prev,             // prev image 
                           mat_curr,             // curr image
                           prev_good_points,     // find these points in the new image
                           curr_good_points,     // result of found points
                           status,               // output status vector, found points are set to 1
                           error,                // each point gets an error value (see flag)
                           cv::Size(21, 21),     // size of the window at each pyramid level 
                           0,                    // maxLevel - 0 = no pyramids, > 0 use this level of pyramids
                           termcrit,             // termination criteria
                           0,                    // flags OPTFLOW_USE_INITIAL_FLOW or OPTFLOW_LK_GET_MIN_EIGENVALS
                           0.1                   // minEigThreshold 
                           );

  memcpy(prev_image, curr, nbytes);
}

void Flow::updateFieldVertices() {
  field_vertices.clear();
  
  float scale_x = (float(settings.win_w) / (field_size-1));
  float scale_y = (float(settings.win_h) / (field_size-1));

  for(size_t j = 0; j < field_size; ++j) {
    for(size_t i = 0; i < field_size; ++i) {
      size_t dx = j * field_size + i;
      vec2 pos(i * scale_x, j * scale_y);
      vec2 vel = velocities[dx];
      field_vertices.push_back(pos);
      field_vertices.push_back(pos + vel);
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, field_vbo);
  size_t bytes_needed = sizeof(vec2) * field_vertices.size();
  if(bytes_needed > field_bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, bytes_needed, field_vertices[0].ptr(), GL_STREAM_DRAW);
    field_bytes_allocated = bytes_needed;
  }
  else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, bytes_needed, field_vertices[0].ptr());
  }
}

void Flow::updateVelocityField() {

  if(!curr_good_points.size()) {
    return;
  }

  if(curr_good_points.size() != prev_good_points.size()) {
    printf("Curr good points and prev good points are not same size but the should be!\n");
    return;
  }

  int w = settings.image_processing_w;
  int h = settings.image_processing_h;
  float influence = 1.5f;

  for(size_t i = 0; i < curr_good_points.size(); ++i) {
    cv::Point2f& c = curr_good_points[i];
    cv::Point2f& p = prev_good_points[i];
    vec2 dir(influence * (c.x - p.x), influence * (c.y - p.y));
    int col = MIN(field_size-1, (c.x/w) * field_size);
    int row = MIN(field_size-1, (c.y/h) * field_size);
    int dx = row * field_size + col;
    velocities[dx] += dir;
  }
}

void Flow::dampVelocities() {
  for(size_t j = 0; j < field_size; ++j) {
    for(size_t i = 0; i < field_size; ++i) {
      size_t dx = j * field_size + i;
      velocities[dx] *= 0.95f;
    }
  }
}

void Flow::createVortex(float px, float py) {
  int num = 5;
  float step = 1.0/(float)field_size;
  for(int j = -num; j < num; ++j) {
    for(int i = -num; i < num; ++i) {
      float perc_x = px + float(i) * step;
      float perc_y = py + float(j) * step;
      vec2 dir(-(py - perc_y), px - perc_x);
      int col = MIN(field_size-1, (perc_x * field_size));
      int row = MIN(field_size-1, (perc_y * field_size));
      int dx = row * field_size + col;
      velocities[dx] += (dir * 50.0);
    }
  }
}

void Flow::applyPerlinToField() {
  size_t s = field_size;
  float influence = 1.01; // how much influence does the current velocity has?
  float scale = 0.5;
  static float t = 0.001;

  for(size_t j = 0; j < s; ++j) {
    for(size_t i = 0; i < s; ++i) {
      size_t dx = j * s + i;
      vec2& v = velocities[dx];
      float a = perlin.get(float(i) * (1.0f/s) + t, float(j) * (1.0f/s) + t);
      float vx = cos(a) * scale;
      float vy = sin(a) * scale;
      vec2 force(vx, vy);
      velocities[dx] += force;
      heights[dx] = (TWO_PI + a) / (TWO_PI * 2);
    }
  }
  t += 0.001;
}
