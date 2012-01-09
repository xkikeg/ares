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

class AresStationListView : public wxListView
{
public:
  AresStationListView(wxWindow * parent,
                      const wxWindowID id,
                      const wxPoint &pos=wxDefaultPosition,
                      const wxSize &size=wxDefaultSize);
  virtual ~AresStationListView() {}

  void OnSelected(wxListEvent& event);
  // void OnDeselected(wxListEvent& event);
  // void OnListKeyDown(wxListEvent& event);
  // void OnChar(wxKeyEvent& event);

  void setLineId(ares::line_id_t lineid);

  void setSelection(long item);
  void setSelectionWithId(ares::station_id_t idval);

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
                  const wxWindowID id);
  virtual ~AresLineListBox() {}

  ares::line_id_t get_lineid(int selected) const;

  ares::line_id_t get_selected_lineid() const;

  void setSelectionWithId(ares::line_id_t idval);

private:
  std::vector<ares::line_id_t> m_lineid_vec;
  DECLARE_NO_COPY_CLASS(AresLineListBox);
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
  virtual void OnSelected(wxCommandEvent & event) = 0;
  void OnDblSelected(wxCommandEvent & event);
  virtual void getQueryResult(const wxString & query,
                              std::vector<int> &result) = 0;
  wxString getSelectedString();
  virtual wxString getStringOfQueryId(const int id) = 0;
  boost::optional<ares::line_id_t> getLine();
  boost::optional<ares::station_id_t> getStation();

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
  virtual void OnSelected(wxCommandEvent& event);
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
  virtual void OnSelected(wxCommandEvent& event);
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
  void OnLineListbox(wxCommandEvent& event);

  DECLARE_NO_COPY_CLASS(AresListFrame);
  DECLARE_EVENT_TABLE();

private:
  wxPanel * m_panel;
  AresLineListBox * m_lineList;
  AresStationListView * m_stationList;
  ares::line_id_t m_lineid;

  template<class SearchDialog>
  void OnSearchDialog();
  void setLineId(ares::line_id_t lineid);
};

enum
{
  LINE_LIST_BOX = 100,
  STATION_LIST_VIEW,
  SEARCH_QUERY_TEXT_CTRL,
  SEARCH_RESULT_LIST_BOX,
  MENU_SEARCH_STATION,
  MENU_SEARCH_LINE,
  MENU_ABOUT = wxID_ABOUT,
  MENU_QUIT = wxID_EXIT,
};
