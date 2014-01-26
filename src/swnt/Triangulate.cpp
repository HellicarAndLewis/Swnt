#include <swnt/Triangulate.h>
#include <string>
#include <algorithm>
#include <stdlib.h>

Triangulate::Triangulate() {
}

void Triangulate::add(float x, float y) {
  points.push_back(x);
  points.push_back(y);
}

void Triangulate::triangulate(std::string opt, struct triangulateio& out) {

  if(!points.size()) {
    return;
  }

  std::vector<int> edges;

#if 1
  int num_points = (int)(points.size() / 2) - 1;

  for(int i = 0; i < num_points; ++i) {
    edges.push_back(i);
    edges.push_back(i + 1);
  }

  edges.push_back(num_points);
  edges.push_back(0);
#endif

  struct triangulateio in = { 0 } ;

  // initialize a nice clean out list
  out.pointlist = NULL;
  out.pointattributelist = NULL;
  out.pointmarkerlist = NULL;
  out.numberofpoints = 0;
  out.numberofpointattributes = 0;
  out.trianglelist = NULL;
  out.triangleattributelist = NULL;
  out.trianglearealist = NULL;
  out.neighborlist = NULL;
  out.numberoftriangles = 0;
  out.numberofcorners = 0;
  out.numberoftriangleattributes = 0;
  out.segmentlist = NULL;
  out.segmentmarkerlist = NULL;
  out.numberofsegments = 0;
  out.holelist = NULL;
  out.numberofholes = 0;
  out.regionlist = NULL;
  out.numberofregions = 0;
  out.edgelist = NULL;
  out.edgemarkerlist = NULL;
  out.normlist = NULL;
  out.numberofedges = 0;
  out.pointlist = NULL;
  out.pointmarkerlist = NULL;
  
  in.pointlist = &points.front(); 
  in.numberofpoints = int(points.size()) / 2;
  
  if(edges.size()) {
    in.segmentlist = &edges.front();
    in.numberofsegments = edges.size() / 2;
  }

  ::triangulate((char*)opt.c_str(), &in, &out, NULL);
}

#define TRIANGULATE_FREE(e) { if (e) { ::free(e); e = NULL; } }

void Triangulate::free(struct triangulateio& out) {

  TRIANGULATE_FREE(out.pointlist);
  TRIANGULATE_FREE(out.pointmarkerlist);
  TRIANGULATE_FREE(out.pointattributelist);
  TRIANGULATE_FREE(out.trianglelist);                                         
  TRIANGULATE_FREE(out.triangleattributelist);
  TRIANGULATE_FREE(out.trianglearealist);
  TRIANGULATE_FREE(out.neighborlist);
  TRIANGULATE_FREE(out.segmentlist);
  TRIANGULATE_FREE(out.segmentmarkerlist);
  TRIANGULATE_FREE(out.holelist);
  TRIANGULATE_FREE(out.regionlist);
  TRIANGULATE_FREE(out.edgelist);
  TRIANGULATE_FREE(out.edgemarkerlist);
  TRIANGULATE_FREE(out.normlist);

  out.numberofpoints = 0;
  out.numberofpointattributes = 0;
  out.numberoftriangles = 0;
  out.numberofcorners = 0;
  out.numberoftriangleattributes = 0;
  out.numberofsegments = 0;
  out.numberofholes = 0;
  out.numberofregions = 0;
  out.numberofedges = 0;
}
