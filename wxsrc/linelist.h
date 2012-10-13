#pragma once

#include <wx/listbox.h>

#include <map>
#include <vector>

#include <ares.h>
#include <util.hpp>

DECLARE_EVENT_TYPE(EVT_CONNECT_LIST_KILL_FOCUS, -1)

class AresLineListBox : public wxListBox
{
public:
  AresLineListBox(wxWindow * parent,
                  const wxWindowID id,
                  const wxPoint & pos=wxDefaultPosition,
                  const wxSize & size=wxDefaultSize)
    : wxListBox(parent, id, pos, size,
                0, NULL, wxLB_SINGLE) {}
  virtual ~AresLineListBox();
  virtual ares::line_id_t getLineId(int selected) const;
  virtual ares::line_id_t getSelectedLineId() const;
  virtual void setSelectionWithId(ares::line_id_t idval);

protected:
  std::vector<ares::line_id_t> m_lineid_vec;

private:
  DECLARE_NO_COPY_CLASS(AresLineListBox);
  DECLARE_EVENT_TABLE();
};

class AresAllLineListBox : public AresLineListBox
{
public:
  AresAllLineListBox(wxWindow * parent,
                     const wxWindowID id);
  virtual ~AresAllLineListBox();
private:
  DECLARE_NO_COPY_CLASS(AresAllLineListBox);
  DECLARE_EVENT_TABLE();
};

class AresConnectLineListBox : public AresLineListBox
{
public:
  AresConnectLineListBox(wxWindow * parent,
                         const wxWindowID id)
    : AresLineListBox(parent, id, wxDefaultPosition,
                      wxSize(120, -1)) {}
  virtual ~AresConnectLineListBox();
  virtual ares::line_id_t getLineId(int selected) const;
  virtual void setSelectionWithId(ares::line_id_t idval);
  void setStation(const ares::station_id_t station);
  void setLineId(const ares::line_id_t line);
  void OnKillFocus(wxFocusEvent& event);

private:
  ares::station_id_t m_current_station;
  std::map<ares::station_id_t,
           std::pair<std::vector<ares::line_id_t>,
                     wxArrayString> > m_connectLineMap;
  DECLARE_NO_COPY_CLASS(AresConnectLineListBox);
  DECLARE_EVENT_TABLE();
};
