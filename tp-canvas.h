
#ifndef TP_CANVAS_H
#define TP_CANVAS_H

#include <gtkmm/drawingarea.h>
#include <gtkmm/builder.h>
#include <boost/algorithm/string/predicate.hpp>
#include <string>
#include <iostream>
#include "gcode-manager.h"
#include <iomanip>
//#include <queue>
//#include <cmath>
//#include <utility>
//#include <thread>
//#include <chrono>

//typedef std::shared_ptr<std::vector<std::string>> string_vector;
//typedef std::shared_ptr<std::vector<Glib::ustring>> string_vector;
  
//typedef std::shared_ptr<std::vector<double>> dash_vector;
//typedef std::shared_ptr<std::vector<GCodeCommand>> coordinate_vector;
typedef sigc::signal<void, uint> type_signal_highlight_grid;

class TPCanvas : public Gtk::DrawingArea
{
public:
  TPCanvas(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
  virtual ~TPCanvas();
  //string_vector set_control_data(string_vector lines);
  void on_animate(bool state);
  GCodeManager m_GCM; // = new GCodeManager();

  void show_crosshairs(bool on);
  void show_trace(bool on);
  void zoom_in();
  void zoom_out();
  void context_scale(double sx, double sy);
  void context_translate(double sx, double sy);
  void increment_trans_x();
  void decrement_trans_x();
  void increment_trans_y();
  void decrement_trans_y();
  double get_trans_x();
  double get_trans_y();
  double get_scale_factor();
  double get_min_x(){return m_min_x;}
  double get_max_x(){return m_max_x;}
  double get_min_y(){return m_min_y;}
  double get_max_y(){return m_max_y;}
  void reverse_x(bool b) { m_GCM.m_polarity_x = (b ? -1 : 1); }
  void reverse_y(bool b) { m_GCM.m_polarity_y = (b ? -1 : 1); }
  void set_to_home();
  std::pair<double,double> get_coord_at_index(int index);
  void highlight_line(int index);
  type_signal_highlight_grid signal_highlight_grid();
  
protected:
  //Override default signal handler:
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
  bool _on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
  uint m_selected_line_index;
  uint m_current_line_index; 
  bool animate;

  Glib::ustring double_to_string(double dVal);
  void draw_crosshairs(const Cairo::RefPtr<Cairo::Context>& cr);
  void draw_toolpath(const Cairo::RefPtr<Cairo::Context>& cr);
  void animate_toolpath(const Cairo::RefPtr<Cairo::Context>& cr);
  
  void draw_toolpath_(const Cairo::RefPtr<Cairo::Context>& cr);
  void _draw_toolpath(const Cairo::RefPtr<Cairo::Context>& cr);
  
  void recalc_translation();
  double m_line_width = 0.005;
  bool on_timeout();
  
  type_signal_highlight_grid m_signal_highlight_grid;
  
private:
  Glib::RefPtr<Gtk::Builder> m_refGlade;
  double m_scale = 1.0; //0.08;//1.3;
  double m_offset = 0.0; //0.16;
  bool m_trace_on = false;
  bool m_xhairs_on = false;
  //  coordinate_vector m_coords;
  std::vector<double> *m_dashes;
  
  double m_scale_width = 0;
  double m_scale_height = 0;
  double m_trans_x = 0.0;
  double m_trans_y = 0.0;
  double m_trans_delta = 0.2;
  double m_scale_zoom_factor = 1.2;
  double m_new_ratio = 0.0;
  double m_old_ratio = 0.0;
  double m_min_y = 0.0;
  double m_max_y = 0.0;
  double m_min_x = 0.0;
  double m_max_x = 0.0;

  bool m_has_highlight = false;
  //  int min_of(int l, int z, int e, int f);
};

#endif // TP_CANVAS_H
