#ifndef _MAIN_WINDOW_
#define _MAIN_WINDOW_

#include <gtkmm.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include "options-dialog.h"
#include "tp-canvas.h"
#include "gcode-grid.h"




class Point
{
 public:
  double x;
  double y;
};

class Polygon
{
 public:
  std::vector<Point> points;
  int m_nverts;

  Polygon() {}
  Polygon(int nverts, double* verts)
    {
      // nverts = number of vertices
      // loop verts array by 2's,
      // convert to points and load vector
		
      m_nverts = nverts;
      for(int i = 0; i < nverts*2 ; i=i+2)
	{
	  Point p;
	  p.x = verts[i];
	  p.y = verts[i+1];
	  points.push_back(p);
	}
    }
};


class MainWindow: public Gtk::Window
{

 public:
  MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
  virtual ~MainWindow();

 protected:
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
  Glib::RefPtr<Gio::SimpleAction> m_refActionOptions;
  Glib::RefPtr<Gio::SimpleAction> m_refActionFileOpen;

 private:
  Glib::RefPtr<Gtk::Builder> m_refGlade;

  Gtk::Box *m_pControlBox;
  Gtk::CheckButton *m_pTraceCB;
  Gtk::CheckButton *m_pAnimateModeCB;
  Gtk::CheckButton *m_pXHairCB;
  Gtk::Label *m_pScaleFactorLbl;
  
  Gtk::EventBox *m_pZoomInEventBox;
  Gtk::Image *m_pZoomInImage;
  Gtk::EventBox *m_pZoomOutEventBox;
  Gtk::Image *m_pZoomOutImage;

  Gtk::EventBox *m_pJogEventBox;
  Gtk::Image *m_pJogButtonImage;

  Gtk::Label *m_pAllocWidthLbl;
  Gtk::Label *m_pAllocHeightLbl;

  Gtk::Entry *m_pScaleWidthText;
  Gtk::Entry *m_pScaleHeightText;
  Gtk::Entry *m_pTransWidthText;
  Gtk::Entry *m_pTransHeightText;
  Gtk::Button *m_pSetBtn;

  Gtk::Label *m_pMaxXLbl;
  Gtk::Label *m_pMinXLbl;
  Gtk::Label *m_pMaxYLbl;
  Gtk::Label *m_pMinYLbl;

  Gtk::CheckButton *m_pReverseXCB;
  Gtk::CheckButton *m_pReverseYCB;
  
  Polygon *m_pJogLeft;
  Polygon *m_pJogRight;
  Polygon *m_pJogUp;
  Polygon *m_pJogDown;
  Polygon *m_pSetEngPos;



  //  Gtk::Entry *m_pFooEntry;


  TPCanvas *m_pCanvas;
  Gtk::Viewport *m_pGCViewport;
  GCodeGrid m_GCodeGrid;
  string_vector m_pJobFileLines;

  void connectWidgets();
  
  //handlers
  void on_set_btn_clicked();
  void on_file_open();
  void on_options_action();
  void on_trace_btn_clicked();
  void on_animate_btn_toggled();
  void on_xhair_btn_clicked();
  bool on_zoom_in_btn_clicked(GdkEventButton*);
  bool on_zoom_out_btn_clicked(GdkEventButton*);
  bool on_zoom_in_btn_release(GdkEventButton*);
  bool on_zoom_out_btn_release(GdkEventButton*);
  bool on_jog_eventbox_btn_press(GdkEventButton*);
  bool on_jog_eventbox_btn_release(GdkEventButton*);
  void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
  
  void jog_left_clicked();
  void jog_right_clicked();
  void jog_up_clicked();
  void jog_down_clicked();
  void set_home_clicked();
  void set_btn_clicked();
  void on_rev_x_btn_clicked();
  void on_rev_y_btn_clicked();
  void on_my_size_allocate(Gtk::Allocation& allocation);
  Glib::ustring int_to_string(int iVal);
  Glib::ustring double_to_string(double dVal);
  
  void buildPolygons();
  bool pointInPolygon (Point* point, Polygon* polygon);

};

#endif // _MAIN_WINDOW_
