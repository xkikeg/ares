#include "listframe.h"
#include "main.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/arrstr.h>
#include <wx/msgdlg.h>
#include <wx/aboutdlg.h>
#include <wx/choicdlg.h>
#include <vector>
#include <utility>
#include <exception>
#include "util.hpp"
#include "cdatabase.h"

#define ENABLE_DEBUG_PRINTING 0

#if ENABLE_DEBUG_PRINTING
#define DEBUG_PRT std::cerr
#else
struct NullOutputStream : public std::streambuf
{
} DEBUG_PRT;

template <class T>
inline NullOutputStream & operator<<(NullOutputStream & stream,
                              const T)
{
  return stream;
}
#endif

DEFINE_EVENT_TYPE(EVT_CONNECT_LIST_KILL_FOCUS)

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

BEGIN_EVENT_TABLE(AresStationListView, wxListView)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresLineListBox, wxListBox)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresAllLineListBox, wxListBox)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresConnectLineListBox, wxListBox)
  EVT_KILL_FOCUS(AresConnectLineListBox::OnKillFocus)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresSearchDialog, wxDialog)
  EVT_TEXT(SEARCH_QUERY_TEXT_CTRL, AresSearchDialog::OnUpdateQuery)
  EVT_TEXT_ENTER(SEARCH_QUERY_TEXT_CTRL, AresSearchDialog::focusResultList)
  EVT_SEARCHCTRL_CANCEL_BTN(SEARCH_QUERY_TEXT_CTRL, AresSearchDialog::clearQueryText)
  EVT_LISTBOX(SEARCH_RESULT_LIST_BOX, AresSearchDialog::OnSelected)
  EVT_LISTBOX_DCLICK(SEARCH_RESULT_LIST_BOX, AresSearchDialog::OnDblSelected)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresSearchStationDialog, AresSearchDialog)
  EVT_BUTTON(wxID_OK, AresSearchStationDialog::OnOK)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresSearchLineDialog, AresSearchDialog)
END_EVENT_TABLE();

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

namespace
{
  struct LISTVIEW_COLUMN_VALUES
  {
    wxString label;
    int format, width;
  };

  enum ARES_STATION_LIST_COLUMN_TYPE
  {
    ARES_STATION_LIST_COLUMN_NAME,
    ARES_STATION_LIST_COLUMN_YOMI,
    ARES_STATION_LIST_COLUMN_DENRYAKU,
    ARES_STATION_LIST_COLUMN_REALKILO,
    ARES_STATION_LIST_COLUMN_FAKEKILO,
    MAX_ARES_STATION_LIST_COLUMN_TYPE,
  };

  LISTVIEW_COLUMN_VALUES ARES_STATION_LIST_COLUMN[] =
  {
    {_("station") , wxLIST_FORMAT_LEFT , 100},
    {_("yomi")    , wxLIST_FORMAT_LEFT , 120},
    {_("denryaku"), wxLIST_FORMAT_LEFT ,  80},
    {_("realkilo"), wxLIST_FORMAT_RIGHT,  65},
    {_("fakekilo"), wxLIST_FORMAT_RIGHT,  65},
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

  for(int i=0; i < LENGTH(ARES_STATION_LIST_COLUMN); ++i)
  {
    this->InsertColumn(i,
                       wxGetTranslation(ARES_STATION_LIST_COLUMN[i].label),
                       ARES_STATION_LIST_COLUMN[i].format,
                       ARES_STATION_LIST_COLUMN[i].width);
  }
}

void AresStationListView::setLineId(ares::line_id_t lineid)
{
  m_lineid = lineid;
  this->Hide();
  this->DeleteAllItems();
  m_stations.clear();
  wxGetApp().getdb().get_stations_of_line(lineid, m_stations);
  for(auto itr=m_stations.begin(); itr != m_stations.end(); ++itr)
  {
    const long tmp = this->InsertItem
      (itr-m_stations.begin(), wxString::FromUTF8(itr->name.c_str()), 0);
    this->SetItemData(tmp, 0);
    this->SetItem(tmp, 1, wxString::FromUTF8(itr->yomi.c_str()));
    this->SetItem(tmp, 2, wxString::FromUTF8(itr->denryaku.c_str()));
    this->SetItem(tmp, 3, wxString::FromAscii(itr->realkilo.to_str().c_str()));
    this->SetItem(tmp, 4, wxString::FromAscii(itr->fakekilo.to_str().c_str()));
  }
  this->Show();
}

void AresStationListView::setSelection(long item)
{
  $.SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  $.EnsureVisible(item);
}

void AresStationListView::setSelectionWithId(ares::station_id_t idval)
{
  $.setSelection($.getIndexFromId(idval));
}

ares::station_id_t AresStationListView::getIdFromIndex(int index) const
{
  return m_stations[index].id;
}

int AresStationListView::getIndexFromId(ares::station_id_t idval) const
{
  auto itr = std::find_if(m_stations.begin(), m_stations.end(),
                          [idval](const ares::CStation & obj) -> bool
                          { return obj.id == idval; });
  assert(itr != m_stations.end());
  return itr - m_stations.begin();
}

AresLineListBox::~AresLineListBox() {}

ares::line_id_t AresLineListBox::getLineId(int selected) const
{
  return m_lineid_vec[selected];
}

ares::line_id_t AresLineListBox::getSelectedLineId() const
{
  int selected = this->GetSelection();
  assert(selected != wxNOT_FOUND);
  return $.getLineId(selected);
}

void AresLineListBox::setSelectionWithId(ares::line_id_t idval)
{
  auto itr = std::find(m_lineid_vec.begin(), m_lineid_vec.end(), idval);
  assert(itr != m_lineid_vec.end());
  $.SetSelection(itr - m_lineid_vec.begin());
}

AresAllLineListBox::AresAllLineListBox(wxWindow * parent,
                                       const wxWindowID id)
  : AresLineListBox::AresLineListBox(parent, id)
{
  std::vector<std::pair<int, std::string> > u8lines;
  wxGetApp().getdb().get_all_lines_name(u8lines);
  for(auto itr=u8lines.begin(); itr != u8lines.end(); ++itr)
  {
    m_lineid_vec.push_back(itr->first);
    this->Append(wxString::FromUTF8(itr->second.c_str(),
                                    itr->second.size()));
  }
}

AresAllLineListBox::~AresAllLineListBox() {}

AresConnectLineListBox::~AresConnectLineListBox() {}

ares::line_id_t AresConnectLineListBox::getLineId(int selected) const
{
  auto itr=m_connectLineMap.find(m_current_station);
  if(itr == m_connectLineMap.end()) { return 0; }
  return itr->second.first[selected];
}

void AresConnectLineListBox::setSelectionWithId(ares::line_id_t idval)
{
  auto itr=m_connectLineMap.find(m_current_station);
  if(itr == m_connectLineMap.end()) { return; }
  auto itr2=std::find(itr->second.first.begin(), itr->second.first.end(), idval);
  assert(itr2 != itr->second.first.end());
  $.SetSelection(itr2 - itr->second.first.end());
}

void AresConnectLineListBox::setStation(const ares::station_id_t station)
{
  m_current_station = station;
  auto itr=m_connectLineMap.find(station);
  if(itr == m_connectLineMap.end())
  {
    $.Set(0, nullptr, nullptr);
  }
  else
  {
    $.Set(itr->second.second, nullptr);
  }
}

void AresConnectLineListBox::setLineId(const ares::line_id_t line)
{
  m_connectLineMap.clear();
  ares::connect_vector result;
  wxGetApp().getdb().get_connect_line(line, result);
  for(auto itr=result.begin(); itr != result.end(); ++itr)
  {
    if(m_connectLineMap[itr->second].first.empty())
    {
      m_connectLineMap[itr->second].first.push_back(line);
      m_connectLineMap[itr->second].second.Add(
        wxString::FromUTF8(wxGetApp().getdb().get_line_name(line).c_str()));
    }
    m_connectLineMap[itr->second].first.push_back(itr->first);
    m_connectLineMap[itr->second].second.Add(
      wxString::FromUTF8(wxGetApp().getdb().get_line_name(itr->first).c_str()));
  }
}

void AresConnectLineListBox::OnKillFocus(wxFocusEvent& WXUNUSED(event))
{
  if($.GetParent())
  {
    wxCommandEvent newEvent(EVT_CONNECT_LIST_KILL_FOCUS, $.GetId());
    $.GetParent()->ProcessEvent(newEvent);
  }
}

AresSearchDialog::AresSearchDialog(wxWindow * parent,
                                   const wxWindowID id,
                                   const wxString & title)
  : wxDialog(parent, id, title),
    m_query(nullptr), m_button_ok(nullptr), m_button_cancel(nullptr)
{
  wxBoxSizer * bottomsizer = new wxBoxSizer(wxHORIZONTAL);
  m_button_ok = new wxButton(this, wxID_OK);
  m_button_cancel = new wxButton(this, wxID_CANCEL);
  bottomsizer->Add(m_button_cancel,
                   wxSizerFlags().Expand().Border(wxALL, 4).Proportion(0));
  bottomsizer->Add(m_button_ok,
                   wxSizerFlags().Expand().Border(wxALL, 4).Proportion(0));

  wxBoxSizer * topsizer = new wxBoxSizer(wxVERTICAL);
  m_query = new wxSearchCtrl(this, SEARCH_QUERY_TEXT_CTRL, _T(""),
                             wxDefaultPosition, wxDefaultSize,
                             wxTE_PROCESS_ENTER);
  m_query->ShowSearchButton(true);
  m_query->ShowCancelButton(true);
  // m_query->SetMenu();
  m_result = new wxListBox(this, SEARCH_RESULT_LIST_BOX);
  topsizer->Add(m_query,
                wxSizerFlags().Expand().Border(wxALL, 4).Proportion(0));
  topsizer->Add(m_result,
                wxSizerFlags().Expand().Border(wxALL, 4).Proportion(1));
  topsizer->Add(bottomsizer,
                wxSizerFlags().Expand().Border(wxALL, 4).Proportion(0));
  $.SetSizer(topsizer);
  m_query->SetFocus();
}

AresSearchDialog::~AresSearchDialog() {}

void AresSearchDialog::clearQueryText(wxCommandEvent& WXUNUSED(event))
{
  m_query->SetValue(_T(""));
}

void AresSearchDialog::focusResultList(wxCommandEvent& WXUNUSED(event))
{
  m_result->SetFocus();
  $.setSelection(0);
}

void AresSearchDialog::OnUpdateQuery(wxCommandEvent& WXUNUSED(event))
{
  m_result_id_vec.clear();
  m_result->Clear();

  // Empty query => none results.
  if(m_query->IsEmpty()) { return; }
  $.getQueryResult(m_query->GetValue(), m_result_id_vec);
  for(auto itr=m_result_id_vec.begin();
      itr != m_result_id_vec.end(); ++itr)
  {
    m_result->Append($.getStringOfQueryId(*itr));
  }
}

void AresSearchDialog::OnDblSelected(wxCommandEvent& WXUNUSED(event))
{
  wxCommandEvent dummy(wxEVT_COMMAND_BUTTON_CLICKED, $.GetAffirmativeId());
  m_button_ok->Command(dummy);
}

wxString AresSearchDialog::getSelectedString()
{
  assert(m_result->GetSelection() != wxNOT_FOUND);
  return m_result->GetString(m_result->GetSelection());
}

boost::optional<ares::line_id_t> AresSearchDialog::getLine()
{
  return m_selected_line;
}

boost::optional<ares::station_id_t> AresSearchDialog::getStation()
{
  return m_selected_station;
}

void AresSearchDialog::setSelection(long item)
{
  m_result->SetSelection(item);
  $.updateSelection();
}

void AresSearchDialog::OnSelected(wxCommandEvent& WXUNUSED(event))
{
  $.updateSelection();
}

AresSearchStationDialog::AresSearchStationDialog(wxWindow * parent)
  : AresSearchDialog(parent, wxID_ANY, _("Search stations"))
{
}

AresSearchStationDialog::~AresSearchStationDialog() {}

void AresSearchStationDialog::OnOK(wxCommandEvent & WXUNUSED(event))
{
  // OK
  DEBUG_PRT << "search station: station is already set: "
            << PRT(m_selected_station) << "\n";
  if(m_selected_station)
  {
    ares::line_vector result;
    wxGetApp().getdb().get_lines_of_station(*m_selected_station,
                                             result);
    wxArrayString lines;
    const wxString & station = $.getSelectedString();
    for(auto itr=result.begin(); itr != result.end(); ++itr)
    {
      lines.Add(wxString::FromUTF8(wxGetApp().getdb().get_line_name(*itr).c_str()));
    }
    assert(!lines.IsEmpty());
    if(lines.GetCount() == 1)
    {
      DEBUG_PRT << "station search only find " << PRT(result[0]) << "\n";
      m_selected_line = result[0];
    }
    else
    {
      wxSingleChoiceDialog dlg(this, wxString::Format(
                                 _("Choose the line of %s"),
                                 station.c_str()),
                               _("Choose line"),
                               lines);
      m_selected_line = (wxID_OK == dlg.ShowModal())
        ? boost::make_optional(result[dlg.GetSelection()])
        : boost::none;
    }
  }
  $.EndModal(wxID_OK);
}

void AresSearchStationDialog::updateSelection()
{
  m_selected_station = $.getID();
  DEBUG_PRT << "station selected " << *m_selected_station << "\n";
}

void AresSearchStationDialog::getQueryResult(const wxString & query,
                                             std::vector<int> &result)
{
  wxGetApp().getdb().find_stationid(query.ToUTF8(), ares::FIND_PREFIX, result);
}

wxString AresSearchStationDialog::getStringOfQueryId(const int id)
{
  return wxString::FromUTF8(wxGetApp().getdb().get_station_name(id).c_str());
}

AresSearchLineDialog::AresSearchLineDialog(wxWindow * parent)
  : AresSearchDialog(parent, wxID_ANY, _("Search lines"))
{
}

AresSearchLineDialog::~AresSearchLineDialog() {}

void AresSearchLineDialog::updateSelection()
{
  m_selected_line = $.getID();
}

void AresSearchLineDialog::getQueryResult(const wxString & query,
                                             std::vector<int> &result)
{
  wxGetApp().getdb().find_lineid(query.ToUTF8(), ares::FIND_PREFIX, result);
}

wxString AresSearchLineDialog::getStringOfQueryId(const int id)
{
  return wxString::FromUTF8(wxGetApp().getdb().get_line_name(id).c_str());
}

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
