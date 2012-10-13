#pragma once

#include <wx/listctrl.h>
#include <ares.h>

namespace ares {
  class CStation;
}

class AresStationListView : public wxListView
{
public:
  AresStationListView(wxWindow * parent,
                      const wxWindowID id,
                      const wxPoint &pos=wxDefaultPosition,
                      const wxSize &size=wxDefaultSize);
  virtual ~AresStationListView() {}

  void setLineId(ares::line_id_t lineid);

  void setSelection(long item);
  void setSelectionWithId(ares::station_id_t idval);

  ares::station_id_t getIdFromIndex(int index) const;
  int getIndexFromId(ares::station_id_t idval) const;

  virtual wxString OnGetItemText(long item, long column) const;

private:
  DECLARE_NO_COPY_CLASS(AresStationListView);
  DECLARE_EVENT_TABLE();

  ares::line_id_t m_lineid;
  std::vector<ares::CStation> m_stations;
};
