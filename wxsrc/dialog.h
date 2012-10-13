#pragma once

#include <wx/dialog.h>
#include <wx/listbox.h>

#include <boost/optional.hpp>

#include <ares.h>

class wxSearchCtrl;
class wxButton;

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

  inline int getID()
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
