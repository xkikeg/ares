#include "listframe.h"

#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/aboutdlg.h>

#include <vector>
#include <boost/optional.hpp>

#include <cstation.h>

#include "linelist.h"
#include "stationlist.h"
#include "dialog.h"
#include "wxares_debug.h"

BEGIN_EVENT_TABLE(AresListFrame, wxFrame)
  EVT_MENU(MENU_ABOUT, AresListFrame::OnAbout)
  EVT_MENU(MENU_QUIT, AresListFrame::OnQuit)
  EVT_MENU(MENU_SEARCH_STATION, AresListFrame::OnSearchStationDialog)
  EVT_MENU(MENU_SEARCH_LINE, AresListFrame::OnSearchLineDialog)
  EVT_LISTBOX(ALL_LINE_LIST_BOX, AresListFrame::OnAllLineListBox)
  EVT_LISTBOX(CONNECT_LINE_LIST_BOX, AresListFrame::OnConnectLineListBox)
  EVT_COMMAND(CONNECT_LINE_LIST_BOX, EVT_CONNECT_LIST_KILL_FOCUS,
              AresListFrame::OnKillFocusConnectList)
  EVT_LIST_ITEM_SELECTED(STATION_LIST_VIEW, AresListFrame::OnSelectStationListView)
END_EVENT_TABLE();


AresListFrame::AresListFrame(const wxString & title,
                             const wxPoint &pos,
                             const wxSize &size)
  : wxFrame(nullptr, wxID_ANY, title, pos, size),
    m_panel(nullptr),
    m_lineList(nullptr), m_stationList(nullptr), m_connectList(nullptr)
{
  wxMenu *menuFile = new wxMenu;
  menuFile->Append(MENU_QUIT, _("E&xit\tCtrl-Q"));

  wxMenu *menuSearch = new wxMenu;
  menuSearch->Append(MENU_SEARCH_STATION, _("Search &Station\t/"));
  menuSearch->Append(MENU_SEARCH_LINE, _("Search &Line\t?"));

  wxMenu *menuHelp = new wxMenu;
  menuHelp->Append(MENU_ABOUT, _("&About...\tF1"));

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(menuFile, _("&File"));
  menu_bar->Append(menuSearch, _("&Search"));
  menu_bar->Append(menuHelp, _("&Help"));
  SetMenuBar(menu_bar);

  m_panel = new wxPanel(this, wxID_ANY);

  m_lineList = new AresAllLineListBox(m_panel, ALL_LINE_LIST_BOX);
  m_stationList = new AresStationListView(m_panel, STATION_LIST_VIEW);
  m_connectList = new AresConnectLineListBox(m_panel, CONNECT_LINE_LIST_BOX);

  wxBoxSizer * leftsizer = new wxBoxSizer(wxVERTICAL);
  leftsizer->Add(m_lineList,
                 wxSizerFlags().Expand().Proportion(1).Top());
  wxBoxSizer * mainsizer = new wxBoxSizer(wxHORIZONTAL);
  mainsizer->Add(leftsizer,
                 wxSizerFlags().Expand().Border(wxALL, 8).Proportion(0));
  mainsizer->Add(m_stationList,
                 wxSizerFlags().Expand().Border(wxTOP
                                                | wxBOTTOM, 8).Proportion(1));
  mainsizer->Add(m_connectList,
                 wxSizerFlags().Expand().Border(wxALL, 8).Proportion(0));

  m_panel->SetSizer(mainsizer);
  m_panel->SetFocus();
  Show(true);
}

void AresListFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
  Close(true);
}

void AresListFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxAboutDialogInfo aboutInfo;
  aboutInfo.SetName(_T("wxAres"));
  // aboutInfo.SetVersion("");
  aboutInfo.SetDescription(_("wxAres for JR fare calculate"));
  aboutInfo.SetCopyright(_T("(c) 2010-2011"));
  aboutInfo.SetWebSite(_T("http://github.com/liquidamber/ares/"));
  aboutInfo.AddDeveloper(_T("liquid_amber"));

  wxAboutBox(aboutInfo);
}

template<class DialogType>
void AresListFrame::OnSearchDialog()
{
  DialogType dlg(this);
  if(wxID_OK == dlg.ShowModal())
  {
    DEBUG_PRT << PRT(dlg.getLine())
              << PRT(dlg.getStation()) << "\n";
    DEBUG_PRT << PRT(boost::optional<int>())
              << PRT(boost::optional<int>(123)) << "\n";
    if(auto line = dlg.getLine())
    {
      m_lineList->setSelectionWithId(*line);
      m_lineList->SetFocus();
      $.setLineId(*line);
      if(auto station = dlg.getStation())
      {
        m_stationList->setSelectionWithId(*station);
        m_stationList->SetFocus();
      }
    }
  }
  else
  {
    DEBUG_PRT << PRT(wxID_OK) << "\n";
  }
}

void AresListFrame::OnSearchStationDialog(wxCommandEvent& WXUNUSED(event))
{
  OnSearchDialog<AresSearchStationDialog>();
}

void AresListFrame::OnSearchLineDialog(wxCommandEvent& WXUNUSED(event))
{
  OnSearchDialog<AresSearchLineDialog>();
}

void AresListFrame::OnAllLineListBox(wxCommandEvent& event)
{
  try
  {
    $.setLineId(m_lineList->getLineId(event.GetInt()));
  }
  catch(const std::exception & e)
  {
    DEBUG_PRT << "Some Exception:" << e.what() << "\n";
    throw;
  }
  catch(...)
  {
    DEBUG_PRT << "Unknown Exception Throwed\n";
    throw;
  }
}

void AresListFrame::OnConnectLineListBox(wxCommandEvent& event)
{
  if(event.GetInt() == wxNOT_FOUND) { return; }
  ares::line_id_t nlineid = m_connectList->getLineId(event.GetInt());
  if(nlineid == m_lineid) { return; }
  $.setLineId(nlineid, true);
  m_lineList->setSelectionWithId(nlineid);
}

void AresListFrame::OnKillFocusConnectList(wxCommandEvent& WXUNUSED(event))
{
  m_stationList->setSelectionWithId(m_stationid);
  m_connectList->setLineId(m_lineid);
  m_connectList->setStation(m_stationid);
  m_connectList->setSelectionWithId(m_lineid);
}

void AresListFrame::OnSelectStationListView(wxListEvent& event)
{
  m_stationid = m_stationList->getIdFromIndex(event.GetIndex());
  m_connectList->setStation(m_stationid);
  m_connectList->setSelectionWithId(m_lineid);
}

void AresListFrame::setLineId(ares::line_id_t lineid,
                              bool delay_connect_linebox)
{
  m_lineid = lineid;
  m_stationList->setLineId(lineid);
  if(!delay_connect_linebox)
  {
    m_stationList->setSelection(0);
    m_stationid = m_stationList->getIdFromIndex(0);
    m_connectList->setLineId(lineid);
    m_connectList->setStation(m_stationid);
    m_connectList->setSelectionWithId(lineid);
  }
}
