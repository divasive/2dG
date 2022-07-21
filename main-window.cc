#include "main-window.h"


MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
  : Gtk::Window(cobject),
  m_refGlade(refGlade)
{

  connectWidgets();
  m_pScaleFactorLbl->set_text(double_to_string(m_pCanvas->get_scale_factor()));
  buildPolygons();

}

void MainWindow::connectWidgets()
{
  // Widgets
  m_refGlade->get_widget_derived("toolpath_canvas", m_pCanvas);
  m_pCanvas->signal_size_allocate().connect( sigc::mem_fun(*this, &MainWindow::on_my_size_allocate));
  m_pCanvas->signal_highlight_grid().connect(sigc::mem_fun(m_GCodeGrid, &GCodeGrid::on_highlight_row) );
  m_refGlade->get_widget("gcode_viewport", m_pGCViewport);
  m_pGCViewport->add(m_GCodeGrid);
  m_GCodeGrid.signal_row_activated().connect(sigc::mem_fun(*this, &MainWindow::on_row_activated) );
  
  // Menu Bar
  m_refActionGroup = Gio::SimpleActionGroup::create();
  m_refActionGroup->add_action("options", sigc::mem_fun(*this, &MainWindow::on_options_action) );
  m_refActionGroup->add_action("open", sigc::mem_fun(*this, &MainWindow::on_file_open) );
  insert_action_group("app", m_refActionGroup);

  // Control Bar
  m_refGlade->get_widget("control_box", m_pControlBox);
  m_pControlBox->set_sensitive(false);
  
  m_refGlade->get_widget("cb_trace", m_pTraceCB);
  m_refGlade->get_widget("cb_animate", m_pAnimateModeCB);
  m_refGlade->get_widget("cb_xhair", m_pXHairCB);
  m_refGlade->get_widget("scale_factor_lbl", m_pScaleFactorLbl);
  
  m_refGlade->get_widget("zoom_in_event_box", m_pZoomInEventBox);
  m_refGlade->get_widget("zoom_in_image", m_pZoomInImage);
  m_pZoomInImage->set("images/zoom_in.png");

  m_refGlade->get_widget("zoom_out_event_box", m_pZoomOutEventBox);
  m_refGlade->get_widget("zoom_out_image", m_pZoomOutImage);
  m_pZoomOutImage->set("images/zoom_out.png");

  m_refGlade->get_widget("jog_button_image", m_pJogButtonImage);

  m_pJogButtonImage->set("images/jog_panel_off.png");
  m_refGlade->get_widget("jog_event_box", m_pJogEventBox);
  
  m_pJogEventBox->set_events(Gdk::BUTTON_PRESS_MASK);
  m_pJogEventBox->signal_button_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_jog_eventbox_btn_press) );
  m_pJogEventBox->signal_button_release_event().connect(sigc::mem_fun(*this, &MainWindow::on_jog_eventbox_btn_release) );

  
  m_pTraceCB->signal_clicked().connect( sigc::mem_fun(*this, &MainWindow::on_trace_btn_clicked) );
  m_pAnimateModeCB->signal_toggled().connect( sigc::mem_fun(*this, &MainWindow::on_animate_btn_toggled) );
  m_pXHairCB->signal_clicked().connect( sigc::mem_fun(*this, &MainWindow::on_xhair_btn_clicked) );
  m_pZoomInEventBox->signal_button_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_zoom_in_btn_clicked) );
  m_pZoomInEventBox->signal_button_release_event().connect(sigc::mem_fun(*this, &MainWindow::on_zoom_in_btn_release) );
  m_pZoomOutEventBox->signal_button_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_zoom_out_btn_clicked) );
  m_pZoomOutEventBox->signal_button_release_event().connect(sigc::mem_fun(*this, &MainWindow::on_zoom_out_btn_release) );

  m_refGlade->get_widget("alloc_width_lbl", m_pAllocWidthLbl);
  m_refGlade->get_widget("alloc_height_lbl", m_pAllocHeightLbl);
  m_refGlade->get_widget("scale_width_text", m_pScaleWidthText);   
  m_refGlade->get_widget("scale_height_text", m_pScaleHeightText);
  m_refGlade->get_widget("translate_width_text", m_pTransWidthText);
  m_refGlade->get_widget("translate_height_text", m_pTransHeightText);

  m_refGlade->get_widget("set_parms_btn", m_pSetBtn);
  m_pSetBtn->signal_clicked().connect( sigc::mem_fun(*this, &MainWindow::on_set_btn_clicked) );

  m_refGlade->get_widget("max_x_lbl", m_pMaxXLbl);
  m_refGlade->get_widget("min_x_lbl", m_pMinXLbl);
  m_refGlade->get_widget("max_y_lbl", m_pMaxYLbl);
  m_refGlade->get_widget("min_y_lbl", m_pMinYLbl);

  m_refGlade->get_widget("cb_reverse_x", m_pReverseXCB);
  m_refGlade->get_widget("cb_reverse_y", m_pReverseYCB);
  m_pReverseXCB->signal_clicked().connect( sigc::mem_fun(*this, &MainWindow::on_rev_x_btn_clicked) );
  m_pReverseYCB->signal_clicked().connect( sigc::mem_fun(*this, &MainWindow::on_rev_y_btn_clicked) );


  //CSS Style
  /*
  auto css = Gtk::CssProvider::create();
  if(not css->load_from_path("style/boilerplate.css")) {
    std::cerr << "Failed to load css\n";
    std::exit(1);
  }
	

  auto screen = Gdk::Screen::get_default();
  auto ctx = m_pBarBtn->get_style_context();
  ctx->add_provider_for_screen(screen, css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  */
  show_all_children();
}

void MainWindow::on_options_action()
{
  /*
  std::cout << "options dialog" << std::endl;
  OptionsDialog* dlg = 0;
  m_refGlade->get_widget_derived("options_dlg", dlg);
  dlg->set_transient_for(*this);

  int result = dlg->run();
  switch(result)
    {
    case Gtk::RESPONSE_APPLY:
      {
	std::cout << "apply me" << std::endl;
	break;
      }
      
    case Gtk::RESPONSE_CANCEL:
      {
	std::cout << "Cancel clicked." << std::endl;
	break;
      }
    
    default:
      break;
    }

  dlg->hide();

  std::string fileName = "test.GM";
  std::ifstream jobFile(fileName);
  m_pJobFileLines.reset(new std::vector<std::string>());
  for (std::string line; std::getline(jobFile, line ); )
    m_pJobFileLines->push_back( line );

  m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
*/  
}

void MainWindow::on_file_open()
{
  Gtk::FileChooserDialog dialog("Select Job File");
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();

  filter->add_pattern("*.t05");
  filter->add_pattern("*.t06");
  filter->add_pattern("*.GM");
  filter->add_pattern("*.WCF");
  filter->set_name("G-Code files");
  dialog.set_filter(filter);
    
  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Select", Gtk::RESPONSE_OK);
    
  int result = dialog.run();
  switch(result)
    {
    case Gtk::RESPONSE_OK:
      {
	std::string filename = dialog.get_filename();
	//	std::cout << "opening " << filename << std::endl;
	//	std::ifstream jobFile(fileName);
	Glib::RefPtr<Glib::IOChannel> io = Glib::IOChannel::create_from_file(filename,"r");
	m_pJobFileLines.reset(new std::vector<Glib::ustring>());

	//	for (Glib::ustring& line; io->read_line(line ); /**/ ) {
	Glib::ustring line;
	while(io->read_line(line) != Glib::IOStatus::IO_STATUS_EOF) {
	  m_pJobFileLines->push_back( line );
	  //	  std::cout << line << std::endl;
	}

	string_vector work_lines = m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
	m_GCodeGrid.load_code_lines(work_lines);
	m_pControlBox->set_sensitive(true);
	m_pMaxXLbl->set_text(double_to_string(m_pCanvas->get_max_x()));
	m_pMinXLbl->set_text(double_to_string(m_pCanvas->get_min_x()));
	m_pMaxYLbl->set_text(double_to_string(m_pCanvas->get_max_y()));
	m_pMinYLbl->set_text(double_to_string(m_pCanvas->get_min_y()));

	break;
      }
      
    case Gtk::RESPONSE_CANCEL:
      {

	if(!m_pJobFileLines || m_pJobFileLines->size() == 0)
	  m_pControlBox->set_sensitive(false);

	std::cout << "Cancel clicked." << std::endl;
	break;
      }
    
    default:
      break;
    }
}


void MainWindow::on_trace_btn_clicked()
{

  if(m_pTraceCB->get_active()) {
    m_pCanvas->show_trace(true);
  } else {
    m_pCanvas->show_trace(false);
  }

  m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
  m_pCanvas->queue_draw();
}

void MainWindow::on_animate_btn_toggled()
{
  m_pCanvas->on_animate(m_pAnimateModeCB->get_active());
  
}

void MainWindow::on_xhair_btn_clicked()
{

  if(m_pXHairCB->get_active()) {
    m_pCanvas->show_crosshairs(true);

  } else {
    m_pCanvas->show_crosshairs(false);
  }

  m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
  m_pCanvas->queue_draw();
}


bool MainWindow::on_zoom_in_btn_clicked(GdkEventButton* eb)
{
  m_pZoomInImage->set("images/zoom_in_on.png");
  m_pCanvas->zoom_in();
  m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
  m_pScaleFactorLbl->set_text(double_to_string(m_pCanvas->get_scale_factor()));
  m_pCanvas->queue_draw();
  return true;
}


bool MainWindow::on_zoom_in_btn_release(GdkEventButton* eb)
{
  m_pZoomInImage->set("images/zoom_in.png");
  return true;
}


bool MainWindow::on_zoom_out_btn_clicked(GdkEventButton* eb)
{
  m_pZoomOutImage->set("images/zoom_out_on.png");
  m_pCanvas->zoom_out();
  m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
  m_pScaleFactorLbl->set_text(double_to_string(m_pCanvas->get_scale_factor()));
  m_pCanvas->queue_draw();
  return true;
}


bool MainWindow::on_zoom_out_btn_release(GdkEventButton* eb)
{
  m_pZoomOutImage->set("images/zoom_out.png");
  return true;
}


bool MainWindow::on_jog_eventbox_btn_press(GdkEventButton* eb)
{

  Point p;
  p.x = eb->x;
  p.y = eb->y;

  if(pointInPolygon(&p, m_pJogLeft))
    jog_left_clicked();
  else if(pointInPolygon(&p, m_pJogRight))
    jog_right_clicked();
  else if(pointInPolygon(&p, m_pJogUp))
    jog_up_clicked();
  else if(pointInPolygon(&p, m_pJogDown))
    jog_down_clicked();
  else if(pointInPolygon(&p, m_pSetEngPos))
    set_home_clicked();
  else
    std::cout << "got an undefined click at x: " << eb->x << "  y: " << eb->y << std::endl;

  //  std::cout << "got a click at x: " << eb->x << "  y: " << eb->y << std::endl;
  return true;
}

bool MainWindow::pointInPolygon(Point *point, Polygon *polygon) {

  bool c = false;
  if(polygon) {
    std::vector<Point> points = polygon->points;
    int i, j, nvert = polygon->m_nverts;

    for(i = 0, j = nvert - 1; i < nvert; j = i++) {
      if( ( (points[i].y >= point->y ) != (points[j].y >= point->y) ) &&
	  (point->x <= (points[j].x - points[i].x) * (point->y - points[i].y) / (points[j].y - points[i].y) + points[i].x)
	  )
	c = !c;
    }

  } else {
      std::cout << "this polygon is nought" << std::endl;
  }

  return c;


//  return false;
}

bool MainWindow::on_jog_eventbox_btn_release(GdkEventButton* eb)
{
  m_pJogButtonImage->set("images/jog_panel_off.png");

  return true;
}


void MainWindow::jog_left_clicked()
{
  m_pJogButtonImage->set("images/jog_panel_left.png");
  m_pCanvas->increment_trans_x();
  double trans_x = m_pCanvas->get_trans_x();
  m_pTransWidthText->set_text(double_to_string(trans_x));
  m_pCanvas->queue_draw();
}

void MainWindow::jog_right_clicked()
{
  m_pJogButtonImage->set("images/jog_panel_right.png");
  m_pCanvas->decrement_trans_x();
  double trans_x = m_pCanvas->get_trans_x();
  m_pTransWidthText->set_text(double_to_string(trans_x));
  m_pCanvas->queue_draw();

}

void MainWindow::jog_up_clicked()
{
  m_pJogButtonImage->set("images/jog_panel_up.png");
  m_pCanvas->decrement_trans_y();
  double trans_y = m_pCanvas->get_trans_y();
  m_pTransHeightText->set_text(double_to_string(trans_y));
  m_pCanvas->queue_draw();
}

void MainWindow::jog_down_clicked()
{
  m_pJogButtonImage->set("images/jog_panel_down.png");
  m_pCanvas->increment_trans_y();
  double trans_y = m_pCanvas->get_trans_y();
  m_pTransHeightText->set_text(double_to_string(trans_y));
  m_pCanvas->queue_draw();

}


void MainWindow::buildPolygons()
{
  //Home Position
  double leftCoords[] = {3,56,13,41,32,41,32,70,12,70};
  double rightCoords[] = {76,40,95,40,104,55,95,70,76,70}; 
  double upCoords[] = {38,34,38,14,54,3,70,14,70,34}; 
  double downCoords[] = {40,78,68,78,68,96,54,106,40,96};
  double setEnPosCoords[] = {37,71,37,40,71,40,71,73};

  m_pJogLeft = new Polygon(5, leftCoords);
  m_pJogRight = new Polygon(5, rightCoords);
  m_pJogUp = new Polygon(5, upCoords);
  m_pJogDown = new Polygon(5, downCoords);
  m_pSetEngPos = new Polygon(4, setEnPosCoords);

}


void MainWindow::set_home_clicked()
{
  //  std::cout << "set home clicked" << std::endl;
  m_pCanvas->set_to_home();
  double trans_y = m_pCanvas->get_trans_y();
  double trans_x = m_pCanvas->get_trans_x();
  m_pTransHeightText->set_text(double_to_string(trans_y));
  m_pTransWidthText->set_text(double_to_string(trans_x));
  m_pCanvas->queue_draw();

}

void MainWindow::on_my_size_allocate(Gtk::Allocation& allocation)
{
  //  std::cout << "inside on_my_size_allocate " << std::endl;
  m_pAllocWidthLbl->set_text(int_to_string(allocation.get_width()));
  m_pAllocHeightLbl->set_text(int_to_string(allocation.get_height()));
  m_pScaleWidthText->set_text(int_to_string(allocation.get_width()));
  m_pScaleHeightText->set_text(int_to_string(allocation.get_height()));
  m_pTransWidthText->set_text(double_to_string(m_pCanvas->get_trans_x()));
  m_pTransHeightText->set_text(double_to_string(m_pCanvas->get_trans_y()));

}

Glib::ustring MainWindow::int_to_string(int iVal)
{
    std::ostringstream ssIn;
    ssIn << iVal;
    Glib::ustring strOut = ssIn.str();

    return strOut;
}

Glib::ustring MainWindow::double_to_string(double dVal)
{
  dVal = round(dVal * 100.0)/100.0;
  std::ostringstream ssIn;
  ssIn << dVal;
  Glib::ustring strOut = ssIn.str();

  return strOut;
}

void MainWindow::on_set_btn_clicked()
{
  Glib::ustring scale_width = m_pScaleWidthText->get_text();
  Glib::ustring scale_height = m_pScaleHeightText->get_text();
  Glib::ustring trans_width = m_pTransWidthText->get_text();
  Glib::ustring trans_height = m_pTransHeightText->get_text();

  if(!scale_height.empty() && !scale_width.empty()) {
    m_pCanvas->context_scale(Glib::Ascii::strtod(scale_width),Glib::Ascii::strtod(scale_height));
  }

  if(!trans_height.empty() && !trans_width.empty()) {
    m_pCanvas->context_translate(Glib::Ascii::strtod(trans_width),Glib::Ascii::strtod(trans_height));
  }

  m_pCanvas->queue_draw();
    
}

void MainWindow::on_rev_x_btn_clicked()
{
  
  if(m_pReverseXCB->get_active()) 
    m_pCanvas->reverse_x(true);

  else
    m_pCanvas->reverse_x(false);

  m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
  m_pCanvas->queue_draw();

}


void MainWindow::on_rev_y_btn_clicked()
{

  if(m_pReverseYCB->get_active()) 
    m_pCanvas->reverse_y(true);
    
  else 
    m_pCanvas->reverse_y(false);

  m_pCanvas->m_GCM.set_control_data(m_pJobFileLines);
  m_pCanvas->queue_draw();

}

void MainWindow::on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column)
{

  Glib::ustring codeLine = m_GCodeGrid.getSelectedCode(path);
  int lineId = m_GCodeGrid.getSelectedLineId(path);
  std::cout << "you selected " << lineId << "  "  << codeLine << std::endl;
  std::pair <double,double> coords = m_pCanvas->get_coord_at_index(lineId);
  std::cout << "tp canvas says: " << coords.first << " - " << coords.second << std::endl;
  m_pCanvas->highlight_line(lineId);
}

MainWindow::~MainWindow()
{

}

