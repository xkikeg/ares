#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <vector>
#include "ares.h"

class AresStationListView : public wxListView
{
public:
  AresStationListView(wxWindow * parent,
                      const wxWindowID id,
                      const wxPoint &pos=wxDefaultPosition,
                      const wxSize &size=wxDefaultSize);

  void OnSelected(wxListEvent& event);
  // void OnDeselected(wxListEvent& event);
  // void OnListKeyDown(wxListEvent& event);
  // void OnChar(wxKeyEvent& event);

  void setLineId(ares::line_id_t lineid);

private:
  DECLARE_NO_COPY_CLASS(AresStationListView);
  DECLARE_EVENT_TABLE();

  ares::line_id_t m_lineid;
};

class AresLineListBox : public wxListBox
{
public:
  AresLineListBox(wxWindow * parent,
                  const wxWindowID id);

  ares::line_id_t get_lineid(int selected) const;

  ares::line_id_t get_selected_lineid() const;

private:
  std::vector<ares::line_id_t> m_lineid_vec;
  DECLARE_NO_COPY_CLASS(AresLineListBox);
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
  void OnLineListbox(wxCommandEvent& event);

  DECLARE_NO_COPY_CLASS(AresListFrame);
  DECLARE_EVENT_TABLE();

private:
  wxPanel * m_panel;
  AresLineListBox * m_lineList;
  AresStationListView * m_stationList;
  ares::line_id_t m_lineid;

  void setLineId(ares::line_id_t lineid);
};

enum
{
  LINE_LIST_BOX = 100,
  STATION_LIST_VIEW,
  MENU_ABOUT = wxID_ABOUT,
  MENU_QUIT = wxID_EXIT,
};
