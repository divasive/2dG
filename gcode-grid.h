
#ifndef GCODE_GRID_H
#define GCODE_GRID_H

#include <gtkmm.h>
#include <vector>
#include "tp-canvas.h"
#include <boost/algorithm/string/predicate.hpp>


class GCodeGrid : public Gtk::TreeView
{
public:
  GCodeGrid();
  virtual ~GCodeGrid();
  void load_code_lines(string_vector jobLines);
  Glib::RefPtr<Gtk::ListStore> getModel() {return m_refTreeModel;}
  Glib::ustring getSelectedCode(const Gtk::TreeModel::Path& path);
  int getSelectedLineId(const Gtk::TreeModel::Path& path);
  
protected:
  //Signal handlers:


  //Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_code_line); add(m_col_line_nmbr);}

      Gtk::TreeModelColumn<Glib::ustring> m_col_code_line;
      Gtk::TreeModelColumn<int> m_col_line_nmbr;
    };

  ModelColumns m_Columns;

  //Child widgets:
  Glib::RefPtr<Gtk::ListStore> m_refTreeModel;

private:

  
};

#endif //GCODE_GRID_H
