#ifndef GCODE_MANAGER_H
#define GCODE_MANAGER_H

#include "gcode-command.h"
#include <iostream>
#include <cmath>
#include <boost/algorithm/string/predicate.hpp>


class GCodeManager
{
 public:
  GCodeManager();
  virtual ~GCodeManager();
  string_vector set_control_data(string_vector lines);
  coordinate_vector m_coords;
  double m_scale;
  int m_polarity_x = 1;
  int m_polarity_y = 1;

 protected:
  double m_min_y = 0.0;
  double m_max_y = 0.0;
  double m_min_x = 0.0;
  double m_max_x = 0.0;
  
  int min_of(int l, int z, int e, int f);

};  

#endif //GCODE_MANAGER_H
