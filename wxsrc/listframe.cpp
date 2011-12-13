#include "listframe.h"
#include "main.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <vector>
#include <utility>
#include "util.hpp"
#include "cstation.h"
#include "cdatabase.h"

BEGIN_EVENT_TABLE(AresListFrame, wxFrame)
  EVT_MENU(MENU_ABOUT, AresListFrame::OnAbout)
  EVT_MENU(MENU_QUIT, AresListFrame::OnQuit)
  EVT_LISTBOX(LINE_LIST_BOX, AresListFrame::OnLineListbox)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresStationListView, wxListView)
  EVT_LIST_ITEM_SELECTED(STATION_LIST_VIEW, AresStationListView::OnSelected)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresLineListBox, wxListBox)
END_EVENT_TABLE();

namespace
{
  enum ARES_STATION_LIST_COLUMN_TYPE
  {
    ARES_STATION_LIST_COLUMN_NAME,
    ARES_STATION_LIST_COLUMN_YOMI,
    ARES_STATION_LIST_COLUMN_DENRYAKU,
    ARES_STATION_LIST_COLUMN_REALKILO,
    ARES_STATION_LIST_COLUMN_FAKEKILO,
    MAX_ARES_STATION_LIST_COLUMN_TYPE,
  };

  wxString ARES_STATION_LIST_COLUMN_STR[] =
  {
    _("station"),
    _("yomi"),
    _("denryaku"),
    _("realkilo"),
    _("fakekilo"),
  };

  int ARES_STATION_LIST_COLUMN_FORMAT[] =
  {
    wxLIST_FORMAT_LEFT,
    wxLIST_FORMAT_LEFT,
    wxLIST_FORMAT_LEFT,
    wxLIST_FORMAT_RIGHT,
    wxLIST_FORMAT_RIGHT,
  };

  int ARES_STATION_LIST_COLUMN_WIDTH[] =
  {
    100,
    120,
    80,
    60,
    60,
  };
}

AresStationListView::AresStationListView(wxWindow * parent,
                                         const wxWindowID id,
                                         const wxPoint &pos,
                                         const wxSize &size)
  : wxListView(parent, id, pos, size,
               wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES)
{
  wxListItem itemCol;

  for(int i=0; i<MAX_ARES_STATION_LIST_COLUMN_TYPE; ++i)
  {
    this->InsertColumn(i,
                       ARES_STATION_LIST_COLUMN_STR[i],
                       ARES_STATION_LIST_COLUMN_FORMAT[i],
                       ARES_STATION_LIST_COLUMN_WIDTH[i]);
  }
}

void AresStationListView::OnSelected(wxListEvent& event)
{
  wxMessageBox(event.GetText(),
               _T("test"),
               wxICON_INFORMATION, this);
}

void AresStationListView::setLineId(ares::line_id_t lineid)
{
  m_lineid = lineid;
  this->Hide();
  this->DeleteAllItems();
  std::vector<ares::CStation> result;
  wxGetApp().getdb()->get_stations_of_line(lineid, result);
  for(size_t i=0; i < result.size(); ++i)
  {
    const long tmp = this->InsertItem
      (i, wxString::FromUTF8(result[i].name.c_str()), 0);
    this->SetItemData(tmp, 0);
    this->SetItem(tmp, 1, wxString::FromUTF8(result[i].yomi.c_str()));
    this->SetItem(tmp, 2, wxString::FromUTF8(result[i].denryaku.c_str()));
    this->SetItem(tmp, 3, wxString::FromAscii(result[i].realkilo.to_str().c_str()));
    this->SetItem(tmp, 4, wxString::FromAscii(result[i].fakekilo.to_str().c_str()));
  }
  this->Show();
}

AresLineListBox::AresLineListBox(wxWindow * parent,
                                 const wxWindowID id)
  : wxListBox(parent, id, wxDefaultPosition, wxDefaultSize,
              0, NULL, wxLB_SINGLE)
{
  std::vector<std::pair<int, std::string> > u8lines;
  wxGetApp().getdb()->get_all_lines_name(u8lines);
  for(auto itr=u8lines.begin(); itr != u8lines.end(); ++itr)
  {
    m_lineid_vec.push_back(itr->first);
    this->Append(wxString::FromUTF8(itr->second.c_str(),
                                    itr->second.size()));
  }
}

ares::line_id_t AresLineListBox::get_lineid(int selected) const
{
  return m_lineid_vec[selected];
}

ares::line_id_t AresLineListBox::get_selected_lineid() const
{
  int selected = this->GetSelection();
  assert(selected != wxNOT_FOUND);
  return $.get_lineid(selected);
}

// void AresLineListBox::OnSelected(wxListEvent& event)
// {
//   wxMessageBox(event.GetText(),
//                _T("test"),
//                wxICON_INFORMATION, this);
// }

AresListFrame::AresListFrame(const wxString & title,
                             const wxPoint &pos,
                             const wxSize &size)
  : wxFrame(NULL, wxID_ANY, title, pos, size),
    m_panel(NULL), m_lineList(NULL), m_stationList(NULL)
{
  wxMenu *menuFile = new wxMenu;
  menuFile->Append(MENU_QUIT, _("E&xit\tCtrl-Q"));

  wxMenu *menuHelp = new wxMenu;
  menuHelp->Append(MENU_ABOUT, _("&About...\tF1"));

  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(menuFile, _("&File"));
  menu_bar->Append(menuHelp, _("&Help"));
  SetMenuBar(menu_bar);

  m_panel = new wxPanel(this, wxID_ANY);

  m_stationList = new AresStationListView(m_panel, STATION_LIST_VIEW);
  m_lineList = new AresLineListBox(m_panel, LINE_LIST_BOX);

  wxBoxSizer * leftsizer = new wxBoxSizer(wxVERTICAL);
  leftsizer->Add(m_lineList,
                 wxSizerFlags().Expand().Proportion(1).Top());
  wxBoxSizer * mainsizer = new wxBoxSizer(wxHORIZONTAL);
  mainsizer->Add(leftsizer,
                 wxSizerFlags().Expand().Border(wxALL, 8).Proportion(0));
  mainsizer->Add(m_stationList,
                 wxSizerFlags().Expand().Border(wxALL, 8).Proportion(1));

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
  wxMessageBox(_("wxAres for JR fare calculate\n"
                 "(c) liquid_amber 2010-2011"),
               _("About wxAres"),
               wxICON_INFORMATION, this);
}

void AresListFrame::OnLineListbox(wxCommandEvent& event)
{
  $.setLineId(m_lineList->get_lineid(event.GetInt()));
}

void AresListFrame::setLineId(ares::line_id_t lineid)
{
  m_lineid = lineid;
  m_stationList->setLineId(lineid);
}
