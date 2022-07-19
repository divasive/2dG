#include "gcode-manager.h"


GCodeManager::GCodeManager() :
  m_scale(1.0)
{
  m_coords.reset(new std::vector<GCodeCommand>);
}

GCodeManager::~GCodeManager()
{

}


/**
 *
 * The original purpose of this class was to get this 
 * monstrosity out of the TPCanvas class to clean it up.
 * At some point, other functionality could be moved/added 
 * here as well. All the work to convert a toolpath file 
 * to a 'usable' coordinate vector for Cairo is done here
 *
 **/
string_vector GCodeManager::set_control_data(string_vector lines)
{
  string_vector work_lines;
  
  m_coords->clear();
  work_lines.reset(new std::vector<Glib::ustring>());
  
  // Use this for reversing the order of the coords - needs some work
  //std::vector<std::string>::reverse_iterator line = lines->rbegin();
  //  for (; line!= lines->rend(); ++line) {
  //int i = 1;


  Glib::ustring work_line;
  Glib::ustring x_val;
  Glib::ustring y_val;
  Glib::ustring x_val_prev;
  Glib::ustring y_val_prev;
  bool has_x = true;
  Glib::ustring cmd_prev = "";

  //  std::cout << "input lines: " << lines->size() << std::endl;
  std::vector<Glib::ustring>::iterator line = lines->begin();

  
  for (; line!= lines->end(); ++line) {
    Glib::ustring cmd = "";
    if(boost::starts_with(*line, "G00")) {

      if(line->substr(3,1).compare("Z") == 0) {
	cmd_prev = "G00";
	continue;

      } else if(line->substr(3,1).compare("G") == 0) {

	if(line->substr(6,1).compare("Z") == 0) {
	  cmd_prev = "G00";
	  continue;
	}

	else {
	  line->assign(line->substr(6, line->length() - 1));
	}
      } 
      cmd = "G00";

    } else if(boost::starts_with(*line, "G01")) {
      if(line->substr(3,1).compare("Z") == 0) {
	cmd_prev = "G01";
	continue;
      } else if(line->substr(3,1).compare("G") == 0) {

	if(line->substr(6,1).compare("Z") == 0) {
	  cmd_prev = "G01";
	  continue;
	}

	else {
	  line->assign(line->substr(6, line->length() - 1));
	}
      } 
      cmd = "G01";

    } else if(boost::starts_with(*line, "X")) {
      cmd = cmd_prev;
    } else if(boost::starts_with(*line, "Y")) {
      //std::cout << "setting has_x to false" << std::endl;
      has_x = false;
      cmd = cmd_prev;
    } else {
      //std::cout << "not a coordinate: " << *line; // << std::endl;
      continue;
    }
    
    //std::cout << "\n" << i++ << ": " << *line; // << std::endl; 

    int x_pos = line->find_first_of('X');
    int y_pos = line->find_first_of('Y');
    int z_pos = line->find_first_of('Z');
    int e_pos = line->find_first_of('E');
    int f_pos = line->find_first_of('F');

    int line_end = min_of(line->length(), z_pos, e_pos, f_pos);
    //    std::cout << "z_pos: " << z_pos << "  e_pos: " << e_pos << "   f_pos: " << f_pos << std::endl;
    //std::cout << "length: " << line->length() << "   line_end: " << line_end << "   y_pos: " << y_pos << std::endl;
													 
    if(has_x)
      x_val = line->substr(x_pos + 1, y_pos - x_pos - 1);
    else
      x_val = x_val_prev;

    if(y_pos < 0)
      y_val = y_val_prev;
    else
      y_val = line->substr(y_pos + 1, line_end - y_pos - 1);


    try {
      GCodeCommand gcode;
      gcode.command = cmd;
      //      std::cout << "scaling " << x_val << " / " << y_val << " with " << m_scale << " * " << m_polarity_x << std::endl;
      gcode.scaled_x = std::stod(x_val) * m_scale * m_polarity_x;
      gcode.scaled_y = std::stod(y_val) * m_scale * m_polarity_y;
      gcode.x = std::stod(x_val);
      gcode.y = std::stod(y_val);
      m_coords->push_back(gcode);
    
      if(round(gcode.x * 1000) > round(m_max_x * 1000)) m_max_x = gcode.x;
      if(round(gcode.x * 1000) < round(m_min_x * 1000)) m_min_x = gcode.x;
      if(gcode.y > m_max_y) m_max_y = gcode.y;
      if(gcode.y < m_min_y) m_min_y = gcode.y;
    } catch (const std::exception&) {
      std::cout << "barfed on " << *line << " while command is " << cmd << std::endl;
      continue;
    }

    work_line = cmd;
    work_line.append("X");
    work_line.append(x_val);
    work_line.append("Y");
    work_line.append(y_val);
    //std::cout << work_line << std::endl;
    work_lines->push_back(work_line);
    
    x_val_prev = x_val;
    y_val_prev = y_val;

    has_x = true;
    cmd_prev = cmd;
  }

  //  std::cout << "work_lines size: " << work_lines->size() << std::endl;

  return work_lines;
}

int GCodeManager::min_of(int length, int z, int e, int f)
{
  int the_min = length;
  if(z > 0 && z < the_min) the_min = z;
  if(e > 0 && e < the_min) the_min = e;
  if(f > 0 && f < the_min) the_min = f;

  return the_min;
}


