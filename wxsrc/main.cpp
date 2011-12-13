#include <wx/intl.h>
#include "cdatabase.h"
#include "sqlite3_wrapper.h"

#include "main.h"
#include "listframe.h"
#include "filepath.h"

IMPLEMENT_APP(AresApp);

bool AresApp::OnInit()
{
  m_aresdb.reset(new ares::CDatabase(WXARES_DB_FILENAME));
  AresListFrame * main_frame = new AresListFrame(_("wxAres"),
                                                 wxDefaultPosition,
                                                 wxSize(600, 200));
  SetTopWindow(main_frame);
  return true;
}

std::shared_ptr<ares::CDatabase> AresApp::getdb() const
{
  return this->m_aresdb;
}
