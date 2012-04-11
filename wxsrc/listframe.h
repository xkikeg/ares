#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <vector>
#include <boost/optional.hpp>
#include "ares.h"
#include "cstation.h"

DECLARE_EVENT_TYPE(EVT_CONNECT_LIST_KILL_FOCUS, -1)

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

private:
  DECLARE_NO_COPY_CLASS(AresStationListView);
  DECLARE_EVENT_TABLE();

  ares::line_id_t m_lineid;
  std::vector<ares::CStation> m_stations;
};

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

class AresSearchDialog : public wxDialog
{
public:
  AresSearchDialog(wxWindow * parent,
                   const wxWindowID id,
                   const wxString & title);
  virtual ~AresSearchDialog();
  void clearQueryText(wxCommandEvent & event);
  void focusResultList(wxCommandEvent & event);
  void OnUpdateQuery(wxCommandEvent & event);
  void OnSelected(wxCommandEvent & event);
  void OnDblSelected(wxCommandEvent & event);
  virtual void getQueryResult(const wxString & query,
                              std::vector<int> &result) = 0;
  wxString getSelectedString();
  virtual wxString getStringOfQueryId(const int id) = 0;
  boost::optional<ares::line_id_t> getLine();
  boost::optional<ares::station_id_t> getStation();
  virtual void setSelection(long item);
  virtual void updateSelection() = 0;

private:
  wxSearchCtrl * m_query;
  wxListBox * m_result;
  wxButton * m_button_ok, * m_button_cancel;

  std::vector<int> m_result_id_vec;

protected:
  boost::optional<ares::line_id_t> m_selected_line;
  boost::optional<ares::station_id_t> m_selected_station;

  int getID()
  {
    return m_result_id_vec[m_result->GetSelection()];
  }

  DECLARE_EVENT_TABLE();
};

class AresSearchStationDialog : public AresSearchDialog
{
public:
  explicit AresSearchStationDialog(wxWindow * parent);
  virtual ~AresSearchStationDialog();
  void OnOK(wxCommandEvent & event);
  virtual void updateSelection();
  virtual void getQueryResult(const wxString & query,
                              std::vector<int> &result);
  virtual wxString getStringOfQueryId(const int id);

private:
  DECLARE_EVENT_TABLE();
};

class AresSearchLineDialog : public AresSearchDialog
{
public:
  explicit AresSearchLineDialog(wxWindow * parent);
  virtual ~AresSearchLineDialog();
  virtual void updateSelection();
  virtual void getQueryResult(const wxString & query,
                              std::vector<int> &result);
  virtual wxString getStringOfQueryId(const int id);

private:
  DECLARE_EVENT_TABLE();
};

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
