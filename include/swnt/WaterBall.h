#ifndef WATER_BALL_H
#define WATER_BALL_H

#define ROXLU_USE_ALL
#include <tinylib.h>

#define WATERDROP_STATE_NONE 0
#define WATERDROP_STATE_NORMAL 1
#define WATERDROP_STATE_FILL 2
#define WATERDROP_STATE_FLUSH 3
#define WATERDROP_STATE_FREE 4

class WaterDrop {
 public:
  WaterDrop();
  ~WaterDrop();

 public:
  vec2 position;     // 8 bytes
  vec2 forces;       // 8 bytes  - offset 8
  vec2 velocity;     // 8 bytes  - offset 16
  float mass;        // 4 bytes  - offset 24
  float inv_mass;    // 4 bytes  - offset 28 
  float size;        // 4 bytes  - offset 32
};

class WaterBall {

 public:
  WaterBall();
  ~WaterBall();
  bool setup(int w, int h);                   /* setup shaders/buffers etc.. */
  void update(float dt = 0.016f);             /* step the physics */
  void draw();                                /* render */
  void fill();                                /* start flushing the drops/waterball */
  void flush();                               /* start filling the waterball with drops */
  void addDrop(vec2 position, float mass);    /* adds a drop */
  /*
  void enable() { enabled = true; } 
  void disable() { enabled = false; } 
  bool isEnabled() { return enabled; } 
  */
  

 private:
  void addRandomDrop();                       /* adds a water drop somewhere on screen (0-win_w, 0-win_h) */

 public:
  std::vector<WaterDrop> drops;               /* the water drop particles */
  int win_w;
  int win_h;     
  vec2 position;                              /* the position of the ball, the drops are attracted to this position */
  uint64_t spawn_timeout;                     /* when we reach this timeout we will spawn another particle */
  uint64_t spawn_delay;                       /* delay X-millis between each spawn */
  uint64_t flush_timeout;                     /* when we need to remove another waterdrop, used while flushing */
  uint64_t flush_delay;                       /* delay X-millis between each flush */
  int state;                                  /* the waterball has a couple of different states, eg. to fill up, full, just animate etc.. */
  float attract_force;                        /* part of the force that we apply (based on distance) to attract to the position of the ball */
  float repel_force;                          /* each water drop is repelled from each other; */
  float max_speed;                            /* :-) don't go faster then this! */
  float radius_per_drop;                      /* the radius per water drop */
  float min_drop_size;                        /* minimum size of a water drop */
  float max_drop_size;                        /* maximum size of a water drop */
  float min_drop_mass;                        /* miminum random mass for a water drop; forces have more effect when the mass is low */
  float max_drop_mass;                        /* maximum random mass, used in addRandomDrop() */
  //bool enabled;                               /* only update/draw when enabled */
};

class WaterBallDrawer {

 public:
  WaterBallDrawer();
  ~WaterBallDrawer();
  bool setup(int w, int h);
  void update(float dt = 0.016f);
  void draw();
  void print();                               /* prints some debugging info */
  void addWaterBall(WaterBall* ball);   

 private:
  void drawParticlesWithAlpha();
  void drawParticlesWithWaterEffect();
  void drawRenderBuffers();

 public:
  std::vector<WaterBall*> balls;              /* the water balls which have been created and that we use to simulate/render */
  std::vector<WaterDrop> drops;               /* all collected water drops. see update() */
  Program basic_prog;                         /* basic shaders / prog which renders the water drops as alpha */
  Program water_prog;                         /* thresholds + applies refraction */
  int win_w;
  int win_h;     
  GLuint basic_vbo;                           /* the vbo that holds the water drop data */
  GLuint basic_vao;                           /* vertex array object */
  GLuint water_vao;                           /* vao used to do a full screen render pass */
  GLuint normals_tex; 
  GLuint alpha_tex;
  GLuint background_tex;                      /* the texture which is used to fetch colors from */
  GLuint fbo;                                 /* we render the particles first into a texture and then apply a threshold */
  GLuint scene_alpha_tex;                     /* the scen texture we captured. */
  GLuint scene_normals_tex;
  size_t bytes_allocated;                     /* number of bytes we allocted on gpu */
  mat4 pm;                                    /* projection matrix; ortho */
};


inline void WaterBall::flush() {
  if(state == WATERDROP_STATE_FLUSH) {
    printf("warning: Trying to set the state of a waterball to flush, but we're already flushing.\n");
    return;
  }
  state = WATERDROP_STATE_FLUSH;
}

inline void WaterBall::fill() {
  if(state == WATERDROP_STATE_FILL) {
    printf("warning: Trying to set the state of a waterball to flush, but we're already flushing.\n");
    return;
  }
  state = WATERDROP_STATE_FILL;
}

#endif
