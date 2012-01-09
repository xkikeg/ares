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

BEGIN_EVENT_TABLE(AresListFrame, wxFrame)
  EVT_MENU(MENU_ABOUT, AresListFrame::OnAbout)
  EVT_MENU(MENU_QUIT, AresListFrame::OnQuit)
  EVT_MENU(MENU_SEARCH_STATION, AresListFrame::OnSearchStationDialog)
  EVT_MENU(MENU_SEARCH_LINE, AresListFrame::OnSearchLineDialog)
  EVT_LISTBOX(LINE_LIST_BOX, AresListFrame::OnLineListbox)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresStationListView, wxListView)
  EVT_LIST_ITEM_SELECTED(STATION_LIST_VIEW, AresStationListView::OnSelected)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresLineListBox, wxListBox)
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
    65,
    65,
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
                       wxGetTranslation(ARES_STATION_LIST_COLUMN_STR[i]),
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
  m_stations.clear();
  wxGetApp().getdb()->get_stations_of_line(lineid, m_stations);
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
  auto itr = std::find_if(m_stations.begin(), m_stations.end(),
                          [idval](const ares::CStation & obj) -> bool
                          { return obj.id == idval; });
  assert(itr != m_stations.end());
  $.setSelection(itr - m_stations.begin());
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

void AresLineListBox::setSelectionWithId(ares::line_id_t idval)
{
  auto itr = std::find(m_lineid_vec.begin(), m_lineid_vec.end(), idval);
  assert(itr != m_lineid_vec.end());
  $.SetSelection(itr - m_lineid_vec.begin());
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
  m_result->SetSelection(0);
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

AresSearchStationDialog::AresSearchStationDialog(wxWindow * parent)
  : AresSearchDialog(parent, wxID_ANY, _("Search stations"))
{
}

AresSearchStationDialog::~AresSearchStationDialog() {}

void AresSearchStationDialog::OnOK(wxCommandEvent & WXUNUSED(event))
{
  // OK
  if(m_selected_station)
  {
    ares::line_vector result;
    wxGetApp().getdb()->get_lines_of_station(*m_selected_station,
                                             result);
    wxArrayString lines;
    const wxString & station = $.getSelectedString();
    for(auto itr=result.begin(); itr != result.end(); ++itr)
    {
      lines.Add(wxString::FromUTF8(wxGetApp().getdb()->get_line_name(*itr).c_str()));
    }
    assert(!lines.IsEmpty());
    if(lines.GetCount() == 1)
    {
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

void AresSearchStationDialog::OnSelected(wxCommandEvent& WXUNUSED(event))
{
  m_selected_station = $.getID();
}

void AresSearchStationDialog::getQueryResult(const wxString & query,
                                             std::vector<int> &result)
{
  wxGetApp().getdb()->find_stationid(query.ToUTF8(), ares::FIND_PREFIX, result);
}

wxString AresSearchStationDialog::getStringOfQueryId(const int id)
{
  return wxString::FromUTF8(wxGetApp().getdb()->get_station_name(id).c_str());
}

AresSearchLineDialog::AresSearchLineDialog(wxWindow * parent)
  : AresSearchDialog(parent, wxID_ANY, _("Search lines"))
{
}

AresSearchLineDialog::~AresSearchLineDialog() {}

void AresSearchLineDialog::OnSelected(wxCommandEvent& WXUNUSED(event))
{
  m_selected_line = $.getID();
}

void AresSearchLineDialog::getQueryResult(const wxString & query,
                                             std::vector<int> &result)
{
  wxGetApp().getdb()->find_lineid(query.ToUTF8(), ares::FIND_PREFIX, result);
}

wxString AresSearchLineDialog::getStringOfQueryId(const int id)
{
  return wxString::FromUTF8(wxGetApp().getdb()->get_line_name(id).c_str());
}

AresListFrame::AresListFrame(const wxString & title,
                             const wxPoint &pos,
                             const wxSize &size)
  : wxFrame(NULL, wxID_ANY, title, pos, size),
    m_panel(NULL), m_lineList(NULL), m_stationList(NULL)
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
}

void AresListFrame::OnSearchStationDialog(wxCommandEvent& WXUNUSED(event))
{
  OnSearchDialog<AresSearchStationDialog>();
}

void AresListFrame::OnSearchLineDialog(wxCommandEvent& WXUNUSED(event))
{
  OnSearchDialog<AresSearchLineDialog>();
}

void AresListFrame::OnLineListbox(wxCommandEvent& event)
{
  try
  {
    $.setLineId(m_lineList->get_lineid(event.GetInt()));
  }
  catch(const std::exception & e)
  {
    std::cerr << "Some Exception:" << e.what() << std::endl;
    throw;
  }
  catch(...)
  {
    std::cerr << "Unknown Exception Throwed\n";
    throw;
  }
}

void AresListFrame::setLineId(ares::line_id_t lineid)
{
  m_lineid = lineid;
  m_stationList->setLineId(lineid);
}
