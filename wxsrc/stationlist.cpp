#include "stationlist.h"

#include <cdatabase.h>
#include <cstation.h>
#include <util.hpp>

#include "main.h"

BEGIN_EVENT_TABLE(AresStationListView, wxListView)
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
               wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL
               | wxLC_HRULES | wxLC_VRULES)
{
  wxListItem itemCol;

  for(size_t i=0; i < LENGTH(ARES_STATION_LIST_COLUMN); ++i)
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
  m_stations.clear();
  wxGetApp().getdb().get_stations_of_line(lineid, m_stations);
  SetItemCount(m_stations.size());
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

wxString AresStationListView::OnGetItemText(long item, long column) const
{
  assert(item >= 0 && static_cast<size_t>(item) < m_stations.size());
  switch(static_cast<ARES_STATION_LIST_COLUMN_TYPE>(column))
  {
  case ARES_STATION_LIST_COLUMN_NAME:
    return wxString::FromUTF8(m_stations[item].name.c_str());
  case ARES_STATION_LIST_COLUMN_YOMI:
    return wxString::FromUTF8(m_stations[item].yomi.c_str());
  case ARES_STATION_LIST_COLUMN_DENRYAKU:
    return wxString::FromUTF8(m_stations[item].denryaku.c_str());
  case ARES_STATION_LIST_COLUMN_REALKILO:
    return wxString::FromAscii(m_stations[item].realkilo.to_str().c_str());
  case ARES_STATION_LIST_COLUMN_FAKEKILO:
    return wxString::FromAscii(m_stations[item].fakekilo.to_str().c_str());
  default:
    throw std::runtime_error("no such column\n");
  }
}
