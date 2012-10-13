#include "main.h"

#include <wx/log.h>
#include <cdatabase.h>
#include <sqlite3_wrapper.h>

#include "listframe.h"
#include "filepath.h"

IMPLEMENT_APP(AresApp);

bool AresApp::OnInit()
{
  m_locale.Init();
  wxLocale::AddCatalogLookupPathPrefix(_T("locale"));
  m_locale.AddCatalog(_T("wxares"));
#ifdef __LINUX__
  {
    //wxLogNull noLog;
    m_locale.AddCatalog(_T("fileutils"));
  }
#endif

  m_aresdb.reset(new ares::CDatabase(WXARES_DB_FILENAME));
  AresListFrame * main_frame = new AresListFrame(_("wxAres"),
                                                 wxDefaultPosition,
                                                 wxSize(720, 200));
  SetTopWindow(main_frame);
  return true;
}

const ares::CDatabase & AresApp::getdb() const
{
  return *m_aresdb;
}
