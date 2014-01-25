#include <assert.h>
#include <swnt/Water.h>
#include <swnt/HeightField.h>
#include <swnt/Settings.h>

Water::Water(HeightField& heightField, Settings& settings)
  :height_field(heightField)
  ,settings(settings)
  ,win_w(0)
  ,win_h(0)
  ,max_foam_depth(2.8)
  ,wind_level(0.0f)
  ,diffuse_tex(0)
  ,normal_tex(0)
  ,noise_tex(0)
  ,foam_tex(0)
  ,flow_tex(0)
  ,sand_tex(0)
  ,depth_ramp_tex(0)
  ,extra_flow_tex(0)
  ,force_tex(0)
  ,vortex_tex(0)
  ,u_max_foam_depth(0)
  ,u_vortex_intensity(0)
  ,u_sun_color(0)
  ,u_sun_intensity(0)
  ,u_sun_shininess(0)
  ,u_ambient_color(0)
  ,u_ambient_intensity(0)
  ,u_diffuse_intensity(0)
  ,u_foam_intensity(0)
  ,u_final_intensity(0)
  ,vortex_intensity(1.0)
  ,sun_intensity(0.15f)
  ,sun_shininess(45.0f)
  ,ambient_intensity(0.05f)
  ,diffuse_intensity(0.58f)
  ,foam_intensity(0.83f)
  ,final_intensity(1.0f)
  ,fbo(0)
{
  sun_pos[0] = 0.0;
  sun_pos[1] = 15.0;
  sun_pos[2] = -25.0;
  sun_color[0] = 4;
  sun_color[1] = 3;
  sun_color[2] = 1;
  ads_intensities[0] = -1.90f;  // ambient, the selected color
  ads_intensities[1] = 0.0f;    // diffuse
  ads_intensities[2] = 0.75f;   // spec
  ads_intensities[3] = 0.36;    // sun  
  ads_intensities[4] = 0.88;    // foam
  ads_intensities[5] = 0.33;    // texture
  ads_intensities[6] = 1.0;     // overall intensity
  ambient_color[0] = 46.0/255.0f;
  ambient_color[1] = 72.0/255.0f;
  ambient_color[2] = 96.0/255.0f;
}

bool Water::setup(int w, int h) {

  if(!w || !h) {
    printf("Error: invalid size: %d x %d\n", w, h);
    return false;
  }

  win_w = w;
  win_h = h;

  if(!setupShaders()) {
    printf("Error: cannot setup the water shaders.\n");
    return false;
  }

  if(!setupTextures()) {
    printf("Error: cannot setup water textures.\n");
    return false;
  }

  if(!setupFBO()) {
    printf("Error: cannot setup the water fbo.\n");
    return false;
  }

  return true;
}


bool Water::setupFBO() {
  glGenFramebuffers(1, &fbo); 
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
 
  glGenTextures(1, &extra_flow_tex);
  glBindTexture(GL_TEXTURE_2D, extra_flow_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, extra_flow_tex, 0);
 
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("Cannot create the framebuffer for the water diffuse capture.\n");
    return false;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}

bool Water::setupTextures() {
  diffuse_tex = rx_create_texture(rx_to_data_path("images/water_diffuse.png"));
  normal_tex = rx_create_texture(rx_to_data_path("images/water_normals.png"));
  noise_tex = rx_create_texture(rx_to_data_path("images/water_noise.png"));
  foam_tex = rx_create_texture(rx_to_data_path("images/water_foam.png"));
  flow_tex = rx_create_texture(rx_to_data_path("images/water_flow.png"));
  sand_tex = rx_create_texture(rx_to_data_path("images/sand.png"));
  force_tex = rx_create_texture(rx_to_data_path("images/force.png"));
  vortex_tex = rx_create_texture(rx_to_data_path("images/vortex.png"));

  // specify some tex params for the depth ramp. (we need nearest sampling)
  depth_ramp_tex = rx_create_texture(rx_to_data_path("images/depth_ramp.png"));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  return true;
}


#define WATER_CHECK_UNIFORM(u, msg) { if (u < 0) { printf("%s\n", msg); return false; } } 

bool Water::setupShaders() {

  const char* atts[] = { "a_tex" } ;
  water_prog.create(GL_VERTEX_SHADER, rx_to_data_path("shaders/water.vert"));
  water_prog.create(GL_FRAGMENT_SHADER, rx_to_data_path("shaders/water.frag"));
  water_prog.link(1, atts);

  glUseProgram(water_prog.id);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_pos_tex"), 0);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_norm_tex"), 1);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_diffuse_tex"), 2);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_normal_tex"), 3);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_noise_tex"), 4);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_foam_tex"), 5);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_flow_tex"), 6);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_sand_tex"), 7);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_depth_ramp_tex"), 8);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_extra_flow_tex"), 9);
  glUniform1i(glGetUniformLocation(water_prog.id, "u_vortex_tex"), 10);

  u_max_foam_depth = glGetUniformLocation(water_prog.id, "u_max_foam_depth");  
  u_vortex_intensity = glGetUniformLocation(water_prog.id, "u_vortex_intensity");  
  u_sun_color = glGetUniformLocation(water_prog.id, "u_sun_color");  
  u_sun_intensity = glGetUniformLocation(water_prog.id, "u_sun_intensity");  
  u_sun_shininess = glGetUniformLocation(water_prog.id, "u_sun_shininess");  
  u_ambient_color = glGetUniformLocation(water_prog.id, "u_ambient_color");  
  u_ambient_intensity = glGetUniformLocation(water_prog.id, "u_ambient_intensity");  
  u_diffuse_intensity = glGetUniformLocation(water_prog.id, "u_diffuse_intensity");  
  u_foam_intensity = glGetUniformLocation(water_prog.id, "u_foam_intensity");  
  u_final_intensity =glGetUniformLocation(water_prog.id, "u_final_intensity");  
  
  WATER_CHECK_UNIFORM(u_max_foam_depth, "Cannot find the foam depth uniform.");
  WATER_CHECK_UNIFORM(u_vortex_intensity, "Cannot find u_vortex_intensity.");
  WATER_CHECK_UNIFORM(u_sun_color, "Cannot find the sun color uniform");
  WATER_CHECK_UNIFORM(u_sun_intensity, "Cannot find the sun intensity uniform");
  WATER_CHECK_UNIFORM(u_sun_shininess, "Cannot find u_sun_shininess");
  WATER_CHECK_UNIFORM(u_ambient_color, "Cannot find u_ambient_color");
  WATER_CHECK_UNIFORM(u_ambient_intensity, "Cannot find u_ambient_intensity");
  WATER_CHECK_UNIFORM(u_diffuse_intensity, "Cannot find u_diffuse_intensity");
  WATER_CHECK_UNIFORM(u_foam_intensity, "Cannot find u_foam_intensity");
  WATER_CHECK_UNIFORM(u_final_intensity, "Cannot find u_final_intensity");
  return true;
}

void Water::update(float dt) {

}

#define WATER_USE_LINES 0

void Water::draw() {
  // @todo - do we want this to be related to real time?
  static float t = 0.0;
  float time0 = fmod(t, 1.0);
  float time1 = fmod(t + 0.5, 1.0);
  t += 0.002;
  
  glBindVertexArray(height_field.vertices_vao);

  glUseProgram(water_prog.id);
  glUniform1f(glGetUniformLocation(water_prog.id, "u_time0"), time0);
  glUniform1f(glGetUniformLocation(water_prog.id, "u_time1"), time1);
  glUniformMatrix4fv(glGetUniformLocation(water_prog.id, "u_vm"), 1, GL_FALSE, height_field.vm.ptr());
  glUniformMatrix4fv(glGetUniformLocation(water_prog.id, "u_pm"), 1, GL_FALSE, height_field.pm.ptr());
  glUniform1f(u_max_foam_depth, max_foam_depth);
  glUniform1f(u_vortex_intensity, vortex_intensity);
  glUniform3fv(u_sun_color, 1, sun_color);
  glUniform1f(u_sun_intensity, sun_intensity);
  glUniform1f(u_sun_shininess, sun_shininess);
  glUniform3fv(u_ambient_color, 1, ambient_color);
  glUniform1f(u_ambient_intensity, ambient_intensity);
  glUniform1f(u_diffuse_intensity, diffuse_intensity);
  glUniform1f(u_foam_intensity, foam_intensity);
  glUniform1f(u_final_intensity, final_intensity);

  glActiveTexture(GL_TEXTURE0);  glBindTexture(GL_TEXTURE_2D, height_field.tex_out_pos);
  glActiveTexture(GL_TEXTURE1);  glBindTexture(GL_TEXTURE_2D, height_field.tex_out_norm);
  glActiveTexture(GL_TEXTURE2);  glBindTexture(GL_TEXTURE_2D, diffuse_tex);
  glActiveTexture(GL_TEXTURE3);  glBindTexture(GL_TEXTURE_2D, normal_tex);
  glActiveTexture(GL_TEXTURE4);  glBindTexture(GL_TEXTURE_2D, noise_tex);
  glActiveTexture(GL_TEXTURE5);  glBindTexture(GL_TEXTURE_2D, foam_tex);
  glActiveTexture(GL_TEXTURE6);  glBindTexture(GL_TEXTURE_2D, flow_tex);
  glActiveTexture(GL_TEXTURE7);  glBindTexture(GL_TEXTURE_2D, sand_tex);
  glActiveTexture(GL_TEXTURE8);  glBindTexture(GL_TEXTURE_2D, depth_ramp_tex);
  glActiveTexture(GL_TEXTURE9);  glBindTexture(GL_TEXTURE_2D, extra_flow_tex);
  glActiveTexture(GL_TEXTURE10);  glBindTexture(GL_TEXTURE_2D, vortex_tex);

#if WATER_USE_LINES
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_TRIANGLES, 0, height_field.vertices.size());
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#else
  glDrawArrays(GL_TRIANGLES, 0, height_field.vertices.size());
#endif
}

void Water::setWeatherInfo(float wind) {
  wind_level = wind;
}
 
void Water::setTimeOfYear(float t) {
  ambient_color[0] = settings.curr_colors.water.x;
  ambient_color[1] = settings.curr_colors.water.y;
  ambient_color[2] = settings.curr_colors.water.z;
}
 
void Water::print() {
  /*
  printf("water.flow_tex: %d\n", flow_tex);
  printf("water.normals_tex: %d\n", normals_tex);
  printf("water.noise_tex: %d\n", noise_tex);
  printf("water.diffuse_tex: %d\n", diffuse_tex);
  printf("water.foam_tex: %d\n", foam_tex);
  printf("water.color_tex: %d\n", color_tex);
  printf("water.extra_flow_tex: %d\n", extra_flow_tex);
  */
}

void Water::beginGrabFlow() {
  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, win_w, win_h);
}
 
void Water::endGrabFlow() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
 
