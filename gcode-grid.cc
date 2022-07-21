#include "gcode-grid.h"

GCodeGrid::GCodeGrid()
{

  //Adjust properties
  this->set_size_request(160, -1);

  
  //Create the Tree model:
  m_refTreeModel = Gtk::ListStore::create(m_Columns);
  this->set_model(m_refTreeModel);
  
  //Add the TreeView's view columns:
  this->append_column(" ", m_Columns.m_col_line_nmbr);
  this->append_column(" ", m_Columns.m_col_code_line);

  show_all_children();
}

void GCodeGrid::load_code_lines(string_vector jobLines)
{
  m_refTreeModel->clear();

  std::cout << "work_lines size: " << jobLines->size() << std::endl;
  Gtk::TreeModel::Row row;
  
  std::vector<Glib::ustring>::iterator line = jobLines->begin();
  for (int i = 0; line!= jobLines->end(); ++line) {
    if(boost::starts_with(*line, "G00") || boost::starts_with(*line, "G01")) {
    row = *(m_refTreeModel->append());
    //    std::cout << line->find('\r') << std::endl;
    //std::cout << line->find('\n') << std::endl;
    if(line->find('\r') <= line->length() || line->find('\n') <= line->length()) {
      if(line->find('\r') <= line->length()) line->erase(line->find('\r'));
      if(line->find('\n') <= line->length()) line->erase(line->find('\n'));
    } 

    row[m_Columns.m_col_code_line] = *line;
    
    row[m_Columns.m_col_line_nmbr] = i++;
    }
  }
  
  //see what was actually loaded into model
  /*
  Gtk::TreeModel::Children children = m_refTreeModel->children();
    std::cout << "=========================================" << std::endl;
  for(Gtk::TreeModel::Children::iterator itr = children.begin(); itr != children.end(); ++itr) {
    Gtk::TreeModel::Row row = *itr;
    std::cout << row[m_Columns.m_col_code_line] << std::endl;
  }
  std::cout << "=========================================" << std::endl;
  */
}

Glib::ustring GCodeGrid::getSelectedCode(const Gtk::TreeModel::Path& path)
{
  Glib::ustring codeLine = "oops";
  Gtk::TreeModel::iterator iter = m_refTreeModel->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    codeLine = row[m_Columns.m_col_code_line];
  }

  return codeLine;

}

int GCodeGrid::getSelectedLineId(const Gtk::TreeModel::Path& path)
{
  int id = 0;
  Gtk::TreeModel::iterator iter = m_refTreeModel->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    id = row[m_Columns.m_col_line_nmbr];
  }

  return id;

}

/**
 *
 * So here's how you find a row and select it. I was intending to
 * highlight in yellow, but that looked like an elaborate combination
 * of programatically setting the CSS for the selected row. Since this
 * works with the default blue color, good enough for now.
 * 
 * get_selection is a member of TreeView, returning the TreeSelection
 * instance, whose select(&TreePath) method does the selecting. To get
 * a path to the row we want to select, there's a constructor that takes
 * a string as an argument. If it's an actual tree with child branches,
 * it's a colon separated traversal list, ala '4:2:7'. For this simple
 * list, the position in the list (row_index, e.g. '3' works)
 *
 * This approach gets you access to the Row object, but that doesn't 
 * seem to be all that useful.
 *    Gtk::TreeModel::Row row = (m_refTreeModel->children())[row_index]
 *
 **/
void GCodeGrid::on_highlight_row(uint row_index)
{
  Glib::ustring path_string(std::to_string(row_index));
  Gtk::TreePath path(path_string);
  get_selection()->select(path);
}

GCodeGrid::~GCodeGrid()
{
}
