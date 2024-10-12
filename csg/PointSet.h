#ifndef POINTSET_H 
#define POINTSET_H 

#include <stdio.h>
#include <math.h>
#include <vector>

#define PI 3.1415926535897932384626433832795


class PointCoord
{
public:
	float axis[3];
	bool _bound;

};

class PointSet{

public:
  std::vector<PointCoord> _point;
  bool *_bound;
  
public:
  PointSet(){
  };
  
  ~PointSet(){
  }
  
  void setPointSize(int N){
	  _point.resize(N);
  }
  
  inline void swapIndex(int i, int j){
	  float tmp = _point[i].axis[0];
    _point[i].axis[0] = _point[j].axis[0];
    _point[j].axis[0] = tmp;
    
    tmp = _point[i].axis[1];
    _point[i].axis[1] = _point[j].axis[1];
    _point[j].axis[1] = tmp;
    
    tmp = _point[i].axis[2];
    _point[i].axis[2] = _point[j].axis[2];
    _point[j].axis[2] = tmp;
    
  }
  
  void setPoint(int i, float x, float y, float z){
    _point[i].axis[0] = x;
    _point[i].axis[1] = y;
    _point[i].axis[2] = z;
  }
   
  void bound(float min[], float max[]){
	  const int _pointN = _point.size();
    min[0] = min[1] = min[2] = 10000000000;
    max[0] = max[1] = max[2] = -10000000000;
    for(int i=0; i<_pointN; i++){
      PointCoord *p = &_point[i];
	  if(p->axis[0] < min[0])
        min[0] = p->axis[0];
      if(p->axis[0] > max[0])
        max[0] = p->axis[0];
      
      if(p->axis[1] < min[1])
        min[1] = p->axis[1];
      if(p->axis[1] > max[1])
        max[1] = p->axis[1];
      
      if(p->axis[2] < min[2])
        min[2] = p->axis[2];
      if(p->axis[2] > max[2])
        max[2] = p->axis[2];
    }
  }
  
  void bound(float min[], float max[], int start, int end){
    min[0] = min[1] = min[2] = 10000000000;
    max[0] = max[1] = max[2] = -10000000000;
    for(int i=start; i<end; i++){
      PointCoord *p = &_point[i];
	  if(p->axis[0] < min[0])
        min[0] = p->axis[0];
      if(p->axis[0] > max[0])
        max[0] = p->axis[0];
      
      if(p->axis[1] < min[1])
        min[1] = p->axis[1];
      if(p->axis[1] > max[1])
        max[1] = p->axis[1];
      
      if(p->axis[2] < min[2])
        min[2] = p->axis[2];
      if(p->axis[2] > max[2])
        max[2] = p->axis[2];
	}
  }
};

#endif