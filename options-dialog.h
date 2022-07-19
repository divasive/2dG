#ifndef _OPTIONS_DIALOG_
#define _OPTIONS_DIALOG_

#include <gtkmm.h>
#include <iostream>

class OptionsDialog: public Gtk::Dialog
{

 public:
  OptionsDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
  virtual ~OptionsDialog();
  std::string fart;
 protected:


 private:
  Glib::RefPtr<Gtk::Builder> m_refGlade;

  Gtk::Button *m_pOKBtn;
  Gtk::Button *m_pCancelBtn;

  void connectWidgets();
  
  //handlers
  void on_ok_btn_clicked();
  void on_cancel_btn_clicked();

};

#endif // _OPTIONS_DIALOG_
