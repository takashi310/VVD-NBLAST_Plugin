#ifndef _NBLASTGuiPluginWindow_H_
#define _NBLASTGuiPluginWindow_H_

#include <wxGuiPluginWindowBase.h>
#include <wx/filepicker.h>
#include <wx/progdlg.h>
#include <wx/listctrl.h>
#include <wx/stopwatch.h>
#include <wx/splitter.h>
#include <wx/sizer.h>
#include <wx/stopwatch.h>
#include "utility.h"

#define SYMBOL_NBLASTGuiPluginWindow_STYLE wxTAB_TRAVERSAL
#define SYMBOL_NBLASTGuiPluginWindow_TITLE _("NBLASTGuiPluginWindow")
#define SYMBOL_NBLASTGuiPluginWindow_IDNAME ID_NBLASTGuiPluginWindow
#define SYMBOL_NBLASTGuiPluginWindow_SIZE wxSize(400, 300)
#define SYMBOL_NBLASTGuiPluginWindow_POSITION wxDefaultPosition

class wxImagePanel : public wxPanel
{
	wxImage  m_image;
	wxImage  *m_orgimage;
	wxImage  m_olimage;
	wxBitmap *m_resized;
	int m_w, m_h;

public:
	wxImagePanel(wxWindow* parent, int w, int h);
	~wxImagePanel();
	void SetImage(wxString file, wxBitmapType format);
	void SetOverlayImage(wxString file, wxBitmapType format, bool show=true);
	void ResetImage();
	void ToggleOverlayVisibility(bool show);
	wxSize CalcImageSizeKeepAspectRatio(int w, int h);

	void OnDraw(wxPaintEvent & evt);
	void PaintNow();
	void OnSize(wxSizeEvent& event);
	void Render(wxDC& dc);
	void OnEraseBackground(wxEraseEvent& event);

	DECLARE_EVENT_TABLE()
};

class NBLASTListCtrl : public wxListCtrl, public Notifier
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

	void Append(wxString name, wxString score, int mipid=-1, int swcid=-1);
	wxString GetText(long item, int col);
	
	void LoadResults(wxString csvfilepath, wxString dbdir);

private:
	void OnSelect(wxListEvent &event);
	void OnAct(wxListEvent &event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnScroll(wxMouseEvent& event);
	void OnColBeginDrag(wxListEvent& event);
	
	DECLARE_EVENT_TABLE()
protected: //Possible TODO
	wxSize GetSizeAvailableForScrollTarget(const wxSize& size) {
		return size - GetEffectiveMinSize();
	}

private:
	wxImageList *m_images;
	wxStopWatch m_watch;
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
		ID_NB_ResultNumText,
		ID_NB_ResultPicker,
		ID_NB_OverlayCheckBox,
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

    void OnSENDEVENTBUTTONClick( wxCommandEvent& event );
	void OnReloadResultsButtonClick( wxCommandEvent& event );
	void OnSkeletonizeButtonClick( wxCommandEvent& event );
	void OnClose(wxCloseEvent& event);
	void OnInteropMessageReceived(wxCommandEvent & event);
	void OnOverlayCheck(wxCommandEvent& event);

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

	wxSplitterWindow* m_splitterWindow;
    wxFilePickerCtrl* m_RPickCtrl;
	wxFilePickerCtrl* m_nlibPickCtrl;
	wxDirPickerCtrl* m_outdirPickCtrl;
	wxTextCtrl* m_ofnameTextCtrl;
	wxTextCtrl* m_rnumTextCtrl;
	wxFilePickerCtrl* m_resultPickCtrl;
	NBLASTListCtrl* m_results;
	wxImagePanel* m_swcImagePanel;
	wxImagePanel* m_mipImagePanel;
	wxButton* m_SkeletonizeButton;
	wxButton* m_CommandButton;
	wxButton* m_ReloadResultButton;
	wxCheckBox* m_overlayChk;
	wxTimer* m_wtimer;
	wxProgressDialog* m_prg_diag;
	bool m_waitingforR;
	bool m_waitingforFiji;
};

#endif
    // _NBLASTGuiPluginWindow_H_
