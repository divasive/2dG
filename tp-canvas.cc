
#include <cmath>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include "tp-canvas.h"
#include "main-window.h"

TPCanvas::TPCanvas(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
  : Gtk::DrawingArea(cobject),
  m_GCM(),
  m_refGlade(refGlade)
{
  // Cairo's set_dash method takes a vector as an arg,
  // with a double for the dash length and one for the
  // space. This might be made adjustable as an option
  m_dashes = new std::vector<double>;
  m_dashes->push_back(0.01);
  m_dashes->push_back(0.01);
  
  m_current_line_index = 0;
  animate = false;
}

TPCanvas::~TPCanvas()
{
}


/**
 *
 * Called by main UI 'Animate' checkbox event handler
 * Sets animate mode flag, resets upper GCode command bound to 0,
 * and activates timer
 *
**/
void TPCanvas::on_animate(bool state)
{
  if(state)
    {
      animate = true;
      Glib::signal_timeout().connect( sigc::mem_fun(*this, &TPCanvas::on_timeout), 300 );
      m_current_line_index = 0;
    }
  else
    {
      animate = false;
    }
}


/**
 *
 * For Gtkmm 4, the drawing function can be programatically 
 * assigned/changed to whatever. We're still waiting for that
 * glorious update to be ready for primetime. For Gtkmm 3, 
 * it has to be this signature
 *
 **/
bool TPCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
  double m_new_ratio = static_cast<double>(width)/static_cast<double>(height);

  // scale to unit square and translate (0, 0) to be 
  // (0.5, 0.5), i.e. the center of the window. If the
  // main UI jog control has different ideas, do that
  
  if(m_scale_width && m_scale_height) {
    cr->scale(-m_scale_width, m_scale_height);
  } else {
    cr->scale(height, height);
  }
  
  if(m_trans_y && m_trans_x && m_old_ratio == m_new_ratio) {
    cr->translate(m_trans_x, m_trans_y);
    //    std::cout << "using stored trans coords: " << m_old_ratio << std::endl;
  } else {
    //    std::cout << "recalculating trans coords: " << m_new_ratio << std::endl;
    m_trans_x = 0.5 * m_new_ratio;
    m_trans_y = 0.5;
    cr->translate(m_trans_y, m_trans_y);
    m_old_ratio = m_new_ratio;
    queue_draw();
  }

  // Start by painting the drawing widget all white
  cr->set_source_rgb(1.0, 1.0, 1.0);
  cr->paint();

  if(animate)
    animate_toolpath(cr);
  else
    draw_toolpath(cr);

  return true;
}


/**
 *
 * Called when the the crosshair mode flag is set
 *
 **/
void TPCanvas::draw_crosshairs(const Cairo::RefPtr<Cairo::Context>& cr)
{

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
  double ratio = static_cast<double>(width)/static_cast<double>(height);

  double l_bound_x = floor(-m_trans_x * 10 / m_scale)/10;
  double r_bound_x = ceil((ratio - m_trans_x) * 10 / m_scale) / 10;  
  double u_bound_y = ceil((1.0 - m_trans_y) * 10 / m_scale) / 10;
  double l_bound_y = floor(-m_trans_y * 10 / m_scale) / 10 ;

  // this is a visual judgement call about whether
  // the tick numbering is too cluttered
  int max_ticks = 20;
  bool too_many_x = abs(r_bound_x - l_bound_x)/0.1 > max_ticks;
  bool too_many_y = abs(l_bound_y - u_bound_y)/0.1 > max_ticks;

  // floating point arithmetic is not dependable, so bump 
  // the ranges up to ints, but display the double
  int left = (int)(l_bound_x * 10);
  int right = (int)(r_bound_x * 10);
  int top = (int)(u_bound_y * 10);
  int bottom = (int)(l_bound_y * 10);
  double j;

  
  cr->set_font_size(0.025);
  cr->set_source_rgb(0.3,0.3,0.8);
  cr->set_line_width(m_line_width * 0.6);

  // draw the bare crosshairs
  cr->move_to(0.0, l_bound_y * m_scale);
  cr->line_to(0.0, u_bound_y * m_scale);
  cr->move_to(l_bound_x * m_scale, 0.0);
  cr->line_to(r_bound_x * m_scale, 0.0);

  // draw and number the x ticks
  for(int k = left; k < right; k++) {
    j = (double)(k / 10.0);
    cr->move_to(j * m_scale,0.02);
    cr->line_to(j * m_scale,-0.02);
    cr->move_to((j * m_scale) - 0.02, 0.06);
      
    if(too_many_x) {
      if( k % 5 == 0 && k)
	cr->show_text(double_to_string(j));
    } else if (k)
      cr->show_text(double_to_string(j));
  }

  // y ticks
  for(int k = bottom; k < top; k++) {
    j = (double)(k / 10.0);
    cr->move_to(0.02, j * m_scale);
    cr->line_to(-0.02, j * m_scale);
    cr->move_to(0.04, (j * m_scale) + 0.01);
    
    if(too_many_y) {
      if( k % 5 == 0 && k)
	cr->show_text(double_to_string(j));
    } else if (k)
      cr->show_text(double_to_string(j));

  }

  cr->stroke();
}


/**
 * 
 * Called on redraw when the animate flag is set. The member 
 * variable m_current_line_index tracks the current upper 
 * bound of the GCodeCommand vector. When the timer fires, 
 * the handler (on_timer) increments this value and the path
 * is rendered to that point, giving the effect that the path
 * is being animated. The 'tool up' lines (G00/move to) are 
 * always rendered (red dashed lines), since that's the point
 *
 **/
void TPCanvas::animate_toolpath(const Cairo::RefPtr<Cairo::Context>& cr)
{

  // This was a bear to figure out. When a command mode
  // (G00/G01) transition occurs (therefore requiring a
  // pen color change), the only way I could get it to
  // stroke the whole path was to do a 'move' to wherever
  // the pen already is, THEN change the color. So we
  // need this to record where the last line_to went,
  // i.e. where the next line will start
  GCodeCommand prev_cmd;
  prev_cmd.command = "";
  prev_cmd.scaled_x = 0.0;
  prev_cmd.scaled_y = 0.0;

  if(m_xhairs_on) {
    draw_crosshairs(cr);
  }

  cr->begin_new_path();
  cr->set_line_width(0.003);

  cr->set_source_rgb(1.0,0.0,0.0);

  GCodeCommand cmd;

  for(uint i = 0; i < m_current_line_index; i++)
    {
      cmd = m_GCM.m_coords->at(i);

      if(cmd.command.compare("G00") == 0) {
	if(cmd.command.compare(prev_cmd.command) != 0) {
	  //std::cout << "command changed: " << cmd.command << ": " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	  cr->stroke();
	  cr->set_source_rgb(1.0,0.0,0.0);
	  cr->set_dash(*m_dashes,0.0);
	  cr->move_to(prev_cmd.scaled_x, prev_cmd.scaled_y);
	}
	  
	//std::cout << "     dashing line: " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	cr->line_to(cmd.scaled_x, cmd.scaled_y);
	prev_cmd = cmd;
      
      } else if(cmd.command.compare("G01") == 0) {
	if(cmd.command.compare(prev_cmd.command) != 0) {
	  //std::cout << "command changed: " << cmd.command << ": " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	  cr->stroke();
	  cr->set_source_rgb(0.0,0.3,0.0);
	  cr->unset_dash();
	  cr->move_to(prev_cmd.scaled_x, prev_cmd.scaled_y);
	}
	
	//std::cout << "     solid line: " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	cr->line_to(cmd.scaled_x, cmd.scaled_y);
	prev_cmd = cmd;
	//std::cout << "stroking " << cmd.scaled_x << "," << cmd.scaled_y << "(" << up_count++ << ") blue" << std::endl;
      }
      
    }
    
  cr->stroke();

  // When the complete path has rendered, unset animate
  // flag and the checkbox. This will result in the
  // handler killing its timer
  if(m_current_line_index == m_GCM.m_coords->size()) {
    animate = false;
    Gtk::CheckButton *pAnimateModeCB;
    m_refGlade->get_widget("cb_animate", pAnimateModeCB);
    pAnimateModeCB->set_active(false);
  }
  
}


/**
 * 
 * Called on redraw when the animate flag is NOT set. Renders
 * the full contents of the GCodeCommand vector in the loop.
 * If the Show Pen Up checkbox (m_trace_on flag) is set, the
 * G00/move commands are rendered, in red dashed lines.
 * If the Show Crosshairs checkbox (m_xhairs_on flag) is set,
 * draw_crosshairs is called from here first.
 *
 **/  
void TPCanvas::draw_toolpath(const Cairo::RefPtr<Cairo::Context>& cr)
{

  GCodeCommand prev_cmd;
  prev_cmd.command = "";
  prev_cmd.scaled_x = 0.0;
  prev_cmd.scaled_y = 0.0;

  if(m_xhairs_on) {
    draw_crosshairs(cr);
  }

  cr->begin_new_path();
  cr->set_line_width(0.003);

  if(m_trace_on) {  // Show the stylus-up lines in red dashes

    for(GCodeCommand cmd : *(m_GCM.m_coords)) {

      if(cmd.command.compare("G00") == 0) {
	if(cmd.command.compare(prev_cmd.command) != 0) {
	  //std::cout << "command changed: " << cmd.command << ": " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	  cr->stroke();
	  cr->set_source_rgb(1.0,0.0,0.0);
	  cr->set_dash(*m_dashes,0.0);
	  cr->move_to(prev_cmd.scaled_x, prev_cmd.scaled_y);
	}
	  
	//std::cout << "     dashing line: " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	cr->line_to(cmd.scaled_x, cmd.scaled_y);
	prev_cmd = cmd;
      
      } else if(cmd.command.compare("G01") == 0) {
	if(cmd.command.compare(prev_cmd.command) != 0) {
	  //std::cout << "command changed: " << cmd.command << ": " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	  cr->stroke();
	  cr->set_source_rgb(0.0,0.3,0.0);
	  cr->unset_dash();
	  cr->move_to(prev_cmd.scaled_x, prev_cmd.scaled_y);
	}
	
	//std::cout << "     solid line: " << cmd.scaled_x << "," << cmd.scaled_y << std::endl;
	cr->line_to(cmd.scaled_x, cmd.scaled_y);
	prev_cmd = cmd;
      }
            
    }

    cr->stroke();

  } else { //m_trace_on = false

    cr->set_source_rgb(0.0,0.3,0.1);

    for(GCodeCommand cmd : *(m_GCM.m_coords)) {
      if(cmd.command.compare("G00") == 0) {
	//std::cout << "move " << cmd.scaled_x << " / " << cmd.scaled_y << std::endl;
	cr->move_to(cmd.scaled_x, cmd.scaled_y);
      } else if(cmd.command.compare("G01") == 0) {
	//std::cout << "line " << cmd.scaled_x << " / " << cmd.scaled_y << std::endl;
	cr->line_to(cmd.scaled_x, cmd.scaled_y);
      }
    }

    cr->stroke();
  }

  // TODO: fix this, bidirectionally
  // 1. clicking a line in the grid highlights it in the drawingarea
  // 2. highlight the m_current_line_index on the grid 
  //    for each animation pass

  if(m_has_highlight) {

    cr->set_source_rgba(0.96,0.93,0.26,0.5);
    cr->set_line_width(0.01);

    GCodeCommand _start = m_GCM.m_coords->at(m_selected_line_index - 1);
    GCodeCommand _end = m_GCM.m_coords->at(m_selected_line_index);
    cr->move_to(_start.scaled_x, _start.scaled_y);
    cr->line_to(_end.scaled_x, _end.scaled_y);
    //    std::cout << "highlighting " <<  _start.scaled_x << " / " <<  _start.scaled_y;
    //    std::cout << " to " <<  _end.scaled_x << " / " <<  _end.scaled_y << std::endl;
    cr->stroke();
  }

  else {
    // first point of path, draw a small circle
  }

}


void TPCanvas::show_crosshairs(bool on) {

  m_xhairs_on = on;
}

  
void TPCanvas::show_trace(bool on) {

  m_trace_on = on;
}

void TPCanvas::zoom_in() {
  m_scale = m_scale * m_scale_zoom_factor;
  m_GCM.m_scale = m_scale;
}

void TPCanvas::zoom_out() {
  m_scale = m_scale / m_scale_zoom_factor;
  m_GCM.m_scale = m_scale;
}

void TPCanvas::context_scale(double sx, double sy)
{
  m_scale_width = sx;
  m_scale_height = sy;
}


void TPCanvas::context_translate(double sx, double sy)
{
  m_trans_y = sx;
  m_trans_y = sy;
}


void TPCanvas::increment_trans_x()
{
  m_trans_x += m_trans_delta;
}

void TPCanvas::decrement_trans_x()
{
  m_trans_x -= m_trans_delta;
}

void TPCanvas::increment_trans_y()
{
  m_trans_y += m_trans_delta;
}

void TPCanvas::decrement_trans_y()
{
  m_trans_y -= m_trans_delta;
}

Glib::ustring TPCanvas::double_to_string(double dVal)
{
    std::ostringstream ssIn;
    ssIn << std::setprecision(2) << dVal;
    Glib::ustring strOut = ssIn.str();

    return strOut;
}

double TPCanvas::get_trans_x()
{
  if(m_trans_x == 0.0)
    recalc_translation();

  return m_trans_x;   
}


double TPCanvas::get_trans_y()
{
  if(m_trans_y == 0.0)
    recalc_translation();

  return m_trans_y;
}

void TPCanvas::recalc_translation()
{
  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();
  m_new_ratio = static_cast<double>(width)/static_cast<double>(height);
  m_trans_x = 0.5 * m_new_ratio;
  m_trans_y = 0.5;
  m_old_ratio = m_new_ratio;
}


double TPCanvas::get_scale_factor()
{
  return m_scale;
}


void TPCanvas::set_to_home()
{
  recalc_translation();
}

std::pair<double,double> TPCanvas::get_coord_at_index(int index) {

  GCodeCommand coord = m_GCM.m_coords->at(index);
  std::pair<double,double> coord_pair(coord.x, coord.y);
  return coord_pair;
}

void TPCanvas::highlight_line(int index)
{
  m_has_highlight = true;
  m_selected_line_index = index;
  queue_draw();
}

/**
 *
 * Handler for the timer. 'Invalidates' the drawingarea
 * widget (this thing) and increments the GCodeCommand 
 * vector upper bound when it is fired. When animate flag
 * is unset, it returns false, destroying the timer.
 *
 **/
bool TPCanvas::on_timeout()
{
  auto win = get_window();

  if (win && animate)
    {
      Gdk::Rectangle r(0, 0, get_allocation().get_width(),
		       get_allocation().get_height());
      win->invalidate_rect(r, false);
      m_signal_highlight_grid.emit(m_current_line_index++);
      return true;
    }
  
  else return false;
}


type_signal_highlight_grid TPCanvas::signal_highlight_grid()
{
  return m_signal_highlight_grid;
}

