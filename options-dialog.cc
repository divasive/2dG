#include "options-dialog.h"


OptionsDialog::OptionsDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
  : Gtk::Dialog(cobject),
  m_refGlade(refGlade)
{

  connectWidgets();
  fart = "stinky";
}

void OptionsDialog::connectWidgets()
{
  //Widgets
  m_refGlade->get_widget("options_ok_btn", m_pOKBtn);
  m_refGlade->get_widget("options_cancel_btn", m_pCancelBtn);
}

void OptionsDialog::on_ok_btn_clicked()
{

}
void OptionsDialog::on_cancel_btn_clicked()
{

}

OptionsDialog::~OptionsDialog()
{

}

