#ifndef _NBLASTGuiPluginWindow_H_
#define _NBLASTGuiPluginWindow_H_

#include <wxGuiPluginWindowBase.h>
#include <wx/filepicker.h>
#include <wx/progdlg.h>
#include <wx/listctrl.h>
#include <wx/stopwatch.h>
#include "utility.h"

#define SYMBOL_NBLASTGuiPluginWindow_STYLE wxTAB_TRAVERSAL
#define SYMBOL_NBLASTGuiPluginWindow_TITLE _("NBLASTGuiPluginWindow")
#define SYMBOL_NBLASTGuiPluginWindow_IDNAME ID_NBLASTGuiPluginWindow
#define SYMBOL_NBLASTGuiPluginWindow_SIZE wxSize(400, 300)
#define SYMBOL_NBLASTGuiPluginWindow_POSITION wxDefaultPosition

class NBLASTListCtrl: public wxListCtrl, public Notifier
{
	enum
	{
		Menu_AddTo = wxID_HIGHEST+5100,
		Menu_Save
	};

public:
	NBLASTListCtrl(wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=wxLC_REPORT|wxLC_SINGLE_SEL);
	~NBLASTListCtrl();

	void Append(wxString name, wxString score);
	wxString GetText(long item, int col);
	
	void LoadResults(wxString csvfilepath);

private:
	void OnSelect(wxListEvent &event);
	void OnAct(wxListEvent &event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnScroll(wxMouseEvent& event);

	DECLARE_EVENT_TABLE()
protected: //Possible TODO
	wxSize GetSizeAvailableForScrollTarget(const wxSize& size) {
		return size - GetEffectiveMinSize();
	}
};


class NBLASTGuiPluginWindow: public wxGuiPluginWindowBase, public Observer
{    
    DECLARE_DYNAMIC_CLASS( NBLASTGuiPluginWindow )
    DECLARE_EVENT_TABLE()

	enum
	{
		ID_NBLASTGuiPluginWindow = wxID_HIGHEST+10001,
		ID_NB_RPicker,
		ID_NB_NlibPicker,
		ID_NB_OutputPicker,
		ID_NB_OutFileText,
		ID_NB_ResultPicker,
		ID_SEND_EVENT_BUTTON,
		ID_SKELETONIZE_BUTTON,
		ID_RELOAD_RESULTS_BUTTON,
		ID_WaitTimer
	};

public:
    /// Constructors
    NBLASTGuiPluginWindow();
    NBLASTGuiPluginWindow(wxGuiPluginBase * plugin, wxWindow* parent, wxWindowID id = SYMBOL_NBLASTGuiPluginWindow_IDNAME, const wxPoint& pos = SYMBOL_NBLASTGuiPluginWindow_POSITION, const wxSize& size = SYMBOL_NBLASTGuiPluginWindow_SIZE, long style = SYMBOL_NBLASTGuiPluginWindow_STYLE );

    /// Creation
    bool Create(wxGuiPluginBase * plugin, wxWindow* parent, wxWindowID id = SYMBOL_NBLASTGuiPluginWindow_IDNAME, const wxPoint& pos = SYMBOL_NBLASTGuiPluginWindow_POSITION, const wxSize& size = SYMBOL_NBLASTGuiPluginWindow_SIZE, long style = SYMBOL_NBLASTGuiPluginWindow_STYLE );

    /// Destructor
    ~NBLASTGuiPluginWindow();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin NBLASTGuiPluginWindow event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEND_EVENT_BUTTON
    void OnSENDEVENTBUTTONClick( wxCommandEvent& event );
	
	void OnReloadResultsButtonClick( wxCommandEvent& event );

	void OnSkeletonizeButtonClick( wxCommandEvent& event );

	void OnClose(wxCloseEvent& event);

////@end NBLASTGuiPluginWindow event handler declarations

////@begin NBLASTGuiPluginWindow member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

	void doAction(ActionInfo *info);

	void EnableControls(bool enable=true);

////@end NBLASTGuiPluginWindow member function declarations

    static bool ShowToolTips();

private:

    wxFilePickerCtrl* m_RPickCtrl;
	wxFilePickerCtrl* m_nlibPickCtrl;
	wxDirPickerCtrl* m_outdirPickCtrl;
	wxTextCtrl* m_ofnameTextCtrl;
	wxFilePickerCtrl* m_resultPickCtrl;
	NBLASTListCtrl* m_results;
	wxButton* m_SkeletonizeButton;
	wxButton* m_CommandButton;
	wxButton* m_ReloadResultButton;
	wxTimer* m_wtimer;
	wxProgressDialog* m_prg_diag;
	bool m_waitingforR;
};

#endif
    // _NBLASTGuiPluginWindow_H_
