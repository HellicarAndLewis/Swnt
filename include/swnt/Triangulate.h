#ifndef ROXLU_TRIANGULATE_H
#define ROXLU_TRIANGULATE_H

/*

  Triangulate
  ---------------------------------------

  - ! ADD THESE TO YOUR PREPROCESSOR FLAGS:

         -DTRILIBRARY
         -DSINGLE

  - ! AFTER CALLING `Triangulate::triangulate()` MAKE SURE TO CALL `Triangulate::free(out)` TO CLEAN MEMORY!

  - ! TRIANGLE MAY CRASH, SEE THIS PAGE: http://www.cs.cmu.edu/~quake/triangle.trouble.html
      WHEN YOU WANT TO TRIANGULATE A CONTOUR, A QUICK FIX WOULD BE TO NOT ADD EVERY 
      POINT, BUT E.G. EVERY 5TH ELEMENT.

  <example>

        Triangulate tri;
        struct triangulateio out;
        tri.triangulate("zpQ", out);
        
        std::vector<vec2> vertices;
        
        // Extract the triangles (you could use a GL element buffer 
        for(int i = 0; i < out.numberoftriangles; ++i) {
           int a = out.trianglelist[i * 3 + 0];
           int b = out.trianglelist[i * 3 + 1];
           int c = out.trianglelist[i * 3 + 2];
           vec2 pa(out.pointlist[(a * 2) + 0], out.pointlist[(a * 2) + 1]);
           vec2 pb(out.pointlist[(b * 2) + 0], out.pointlist[(b * 2) + 1]);
           vec2 pc(out.pointlist[(c * 2) + 0], out.pointlist[(c * 2) + 1]);
           vertices.push_back(pa);
           vertices.push_back(pb);
           vertices.push_back(pc);
        }
        
        tri.free(out);

  </example>

  Very thin wrapper around the [Triangle library](http://www.cs.cmu.edu/~quake/triangle.html).

     Important:
     ----------
   
     After calling Triangualate::triangulate(out), make sure to call Triangulate::free(out)
     to free up all allocated memory. Of course call Triangulate::free(out), after you've used
     the data.

     Usefull options
     ---------------

     The triangulate() options expects a string with a couple of characters; where each character 
     defines how the triangulation is done. These are the ones I use most often:

     - Q         Suppress verbose output
     - p         Use this when you want to create "hand" like shapes instead; it will fit the contour basically
     - a         Imposes a maximum triangle area. 
     
     See: http://www.cs.cmu.edu/~quake/triangle.switch.html for more infor.

     Examples:   
     ---------
     - tri.triangulate("zpQ", out);             See: http://farm6.staticflickr.com/5504/11995424136_da15c9c991.jpg
     - tri.triangulate("zpa100", out);          See: http://farm6.staticflickr.com/5530/11994959684_43678280ea.jpg
     - tri.triangulate("zpa10000", out);        See: http://farm4.staticflickr.com/3814/11994889643_2d5275765e.jpg
     - tri.triangulate("za100", out);           See: http://farm4.staticflickr.com/3828/11994889833_05cf0984a6.jpg


     Notes:
     -----
     You need to look into the source to figure out how to use the triangulation library in an application 
     as an library and not just the executable. Before you include the `triangle.h` file, you need to set some
     #defines, so include like this:

     ````c++
         extern "C" {
         # define REAL float
         # define ANSI_DECLARATORS
         # define VOID void
         # include "triangle.h"
         }
     ````

     Also add these to you compiler flags:

     ````
     -DTRILIBRARY
     -DSINGLE
     ````

     References:
     -----------
     [0] http://www.cs.cmu.edu/~quake/triangle.html
     [1] http://www.cs.cmu.edu/~quake/triangle.switch.html
     [2] http://www.math.umbc.edu/~rouben/2011-01-math625/triangle-demo2.c

 */

extern "C" {
# define REAL float
# define ANSI_DECLARATORS
# define VOID void
# include "triangle.h"
}

#include <vector>

class Triangulate {
 public:
  Triangulate();

  void add(float x, float y);                                     /* Add a point that you want to use to triangulate */
  void triangulate(std::string opt, struct triangulateio& out);   /* Triangulate the points you added. We set the `struct triangulateio& out`. See Triangle documentation for the data which is set; or you see the example above */
  void free(struct triangulateio& out);                           /* Free the `struct triangulateio` that you passed to `triangulate()`. Make sure to call this!! */
  size_t size();                                                  /* Return the number of points added */
  void clear();                                                   /* Clear the points you've added */

 public:
  std::vector<float> points;                                       /* The points you added [x0,y0,x1,y1,...] */
};


inline size_t Triangulate::size() {
  return points.size() / 2;
}

inline void Triangulate::clear() {
  points.clear();
}
#endif
