#include "linelist.h"

#include <cdatabase.h>

#include "main.h"

DEFINE_EVENT_TYPE(EVT_CONNECT_LIST_KILL_FOCUS)

BEGIN_EVENT_TABLE(AresLineListBox, wxListBox)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresAllLineListBox, wxListBox)
END_EVENT_TABLE();

BEGIN_EVENT_TABLE(AresConnectLineListBox, wxListBox)
  EVT_KILL_FOCUS(AresConnectLineListBox::OnKillFocus)
END_EVENT_TABLE();

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
