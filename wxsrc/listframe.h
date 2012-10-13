#pragma once

#include <wx/frame.h>
#include <wx/listctrl.h>

#include <ares.h>

class wxPanel;
class AresLineListBox;
class AresStationListView;
class AresConnectLineListBox;

class AresListFrame : public ::wxFrame
{
public:
  AresListFrame(const wxString &title,
                const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize);
  virtual ~AresListFrame() {}

  // notifications
  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnSearchStationDialog(wxCommandEvent& event);
  void OnSearchLineDialog(wxCommandEvent& event);
  void OnAllLineListBox(wxCommandEvent& event);
  void OnConnectLineListBox(wxCommandEvent& event);
  void OnKillFocusConnectList(wxCommandEvent& event);
  void OnSelectStationListView(wxListEvent& event);

  DECLARE_NO_COPY_CLASS(AresListFrame);
  DECLARE_EVENT_TABLE();

private:
  wxPanel * m_panel;
  AresLineListBox * m_lineList;
  AresStationListView * m_stationList;
  AresConnectLineListBox * m_connectList;
  ares::line_id_t m_lineid;
  ares::station_id_t m_stationid;

  template<class SearchDialog>
  void OnSearchDialog();
  void setLineId(ares::line_id_t lineid,
                 bool delay_connect_linebox=false);
};

enum
{
  ALL_LINE_LIST_BOX = 100,
  STATION_LIST_VIEW,
  CONNECT_LINE_LIST_BOX,
  SEARCH_QUERY_TEXT_CTRL,
  SEARCH_RESULT_LIST_BOX,
  MENU_SEARCH_STATION,
  MENU_SEARCH_LINE,
  MENU_ABOUT = wxID_ABOUT,
  MENU_QUIT = wxID_EXIT,
};
