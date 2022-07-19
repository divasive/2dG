#ifndef GCODE_COMMAND_H
#define GCODE_COMMAND_H

#include <memory>
#include <glibmm/ustring.h>
#include <vector>

struct GCodeCommand
{
  Glib::ustring command;
  double x;
  double y;
  double scaled_x;
  double scaled_y;
};

typedef std::shared_ptr<std::vector<GCodeCommand>> coordinate_vector;
typedef std::shared_ptr<std::vector<Glib::ustring>> string_vector;

#endif
