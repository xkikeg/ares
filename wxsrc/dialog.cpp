#include "dialog.h"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/srchctrl.h>
#include <wx/textctrl.h>
#include <wx/choicdlg.h>

#include <util.hpp>
#include <cdatabase.h>

#include "main.h"
#include "listframe.h"
#include "wxares_debug.h"

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
