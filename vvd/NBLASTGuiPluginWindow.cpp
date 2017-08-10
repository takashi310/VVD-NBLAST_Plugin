
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "NBLASTGuiPlugin.h"
#include "NBLASTGuiPluginWindow.h"
#include "VRenderFrame.h"
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>


BEGIN_EVENT_TABLE(NBLASTListCtrl, wxListCtrl)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, NBLASTListCtrl::OnAct)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, NBLASTListCtrl::OnSelect)
	EVT_KEY_DOWN(NBLASTListCtrl::OnKeyDown)
	EVT_KEY_UP(NBLASTListCtrl::OnKeyUp)
	EVT_MOUSE_EVENTS(NBLASTListCtrl::OnMouse)
	EVT_SCROLLWIN(NBLASTListCtrl::OnScroll)
	EVT_MOUSEWHEEL(NBLASTListCtrl::OnScroll)
END_EVENT_TABLE()

NBLASTListCtrl::NBLASTListCtrl(
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, id, pos, size, style)
{
	wxListItem itemCol;
	itemCol.SetText("Name");
	this->InsertColumn(0, itemCol);
	
	itemCol.SetText("Score");
	itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
	this->InsertColumn(1, itemCol);

	SetColumnWidth(0, 400);
	SetColumnWidth(1, 130);
}

NBLASTListCtrl::~NBLASTListCtrl()
{

}

void NBLASTListCtrl::LoadResults(wxString csvfilepath)
{
	wxFileInputStream input(csvfilepath);
	wxTextInputStream text(input, wxT("\t"), wxConvUTF8 );
	SetEvtHandlerEnabled(false);

	DeleteAllItems();

	while(input.IsOk() && !input.Eof() )
	{
		wxString line = text.ReadLine();
		wxStringTokenizer tkz(line, wxT(","));
		
		wxArrayString con;
		while(tkz.HasMoreTokens())
			con.Add(tkz.GetNextToken());

		if (con.Count() >= 2)
			Append(con[0], con[1]);
	}
	SetEvtHandlerEnabled(true);
}

void NBLASTListCtrl::Append(wxString name, wxString score)
{
	long tmp = InsertItem(GetItemCount(), name);
	SetItem(tmp, 1, score);
}

wxString NBLASTListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void NBLASTListCtrl::OnSelect(wxListEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxString name = GetText(item, 1);
	}
}

void NBLASTListCtrl::OnAct(wxListEvent &event)
{
	int index = 0;
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxString name = GetText(item, 0);
		notifyAll(NB_OPEN_FILE, name.ToStdString().c_str(), name.ToStdString().length()+1);
	}
}

void NBLASTListCtrl::OnKeyDown(wxKeyEvent& event)
{
	//event.Skip();
}

void NBLASTListCtrl::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
}

void NBLASTListCtrl::OnMouse(wxMouseEvent &event)
{
	event.Skip();
}

void NBLASTListCtrl::OnScroll(wxScrollWinEvent& event)
{
	event.Skip(true);
}

void NBLASTListCtrl::OnScroll(wxMouseEvent& event)
{
	event.Skip(true);
}

/*
 * NBLASTGuiPluginWindow type definition
 */

IMPLEMENT_DYNAMIC_CLASS( NBLASTGuiPluginWindow, wxGuiPluginWindowBase )


/*
 * NBLASTGuiPluginWindow event table definition
 */

BEGIN_EVENT_TABLE( NBLASTGuiPluginWindow, wxGuiPluginWindowBase )

////@begin NBLASTGuiPluginWindow event table entries
    EVT_BUTTON( ID_SEND_EVENT_BUTTON, NBLASTGuiPluginWindow::OnSENDEVENTBUTTONClick )
	EVT_BUTTON( ID_RELOAD_RESULTS_BUTTON, NBLASTGuiPluginWindow::OnReloadResultsButtonClick )
	EVT_CLOSE(NBLASTGuiPluginWindow::OnClose)
////@end NBLASTGuiPluginWindow event table entries

END_EVENT_TABLE()


/*
 * NBLASTGuiPluginWindow constructors
 */

NBLASTGuiPluginWindow::NBLASTGuiPluginWindow()
{
    Init();
}

NBLASTGuiPluginWindow::NBLASTGuiPluginWindow( wxGuiPluginBase * plugin, 
											   wxWindow* parent, wxWindowID id, 
											   const wxPoint& pos, const wxSize& size, 
											   long style )
{
    Init();
    Create(plugin, parent, id, pos, size, style);
}


/*
 * NBLASTGuiPluginWindow creator
 */

bool NBLASTGuiPluginWindow::Create(wxGuiPluginBase * plugin, 
									wxWindow* parent, wxWindowID id, 
									const wxPoint& pos, const wxSize& size, 
									long style )
{
    wxGuiPluginWindowBase::Create(plugin, parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

	((NBLASTGuiPlugin *)GetPlugin())->addObserver(this);

    return true;
}


/*
 * NBLASTGuiPluginWindow destructor
 */

NBLASTGuiPluginWindow::~NBLASTGuiPluginWindow()
{
	NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();
	if (plugin)
	{
		wxString path;
		if (m_RPickCtrl)
		{
			path = m_RPickCtrl->GetPath();
			plugin->SetRPath(path);
		}
		if (m_nlibPickCtrl)
		{
			path = m_nlibPickCtrl->GetPath();
			plugin->SetNlibPath(path);
		}
		if (m_outdirPickCtrl)
		{
			path = m_outdirPickCtrl->GetPath();
			plugin->SetOutDir(path);
		}

		plugin->SaveConfigFile();
	}
}


/*
 * Member initialisation
 */

void NBLASTGuiPluginWindow::Init()
{
	m_RPickCtrl = NULL;
	m_nlibPickCtrl = NULL;
	m_outdirPickCtrl = NULL;
	m_ofnameTextCtrl = NULL;
	m_CommandButton = NULL;
	m_prg_diag = NULL;
	m_waitingforR = false;
	m_wtimer = new wxTimer(this, ID_WaitTimer);
}


/*
 * Control creation for NBLASTGuiPluginWindow
 */

void NBLASTGuiPluginWindow::CreateControls()
{    
	wxString rpath, nlibpath, outdir;
	NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();
	if (plugin)
	{
		rpath = plugin->GetRPath();
		nlibpath = plugin->GetNlibPath();
		outdir = plugin->GetOutDir();
	}

	SetEvtHandlerEnabled(false);
	Freeze();

	////@begin NBLASTGuiPluginWindow content construction
	wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
	wxStaticText *st;

	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Rscript.exe:", wxDefaultPosition, wxSize(120, -1), wxALIGN_RIGHT);
	m_RPickCtrl = new wxFilePickerCtrl( this, ID_NB_RPicker, rpath, _("Set a path to Rscript.exe"), wxFileSelectorDefaultWildcardStr, wxDefaultPosition, wxSize(400, -1));
	sizer1->Add(5, 10);
	sizer1->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer1->Add(5, 10);
	sizer1->Add(m_RPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer1->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer1, 0, wxEXPAND);

	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Target Neurons (.rds):", wxDefaultPosition, wxSize(120, -1), wxALIGN_RIGHT);
	m_nlibPickCtrl = new wxFilePickerCtrl( this, ID_NB_RPicker, nlibpath, _("Choose a target neuron library"), "*.rds", wxDefaultPosition, wxSize(400, -1));
	sizer2->Add(5, 10);
	sizer2->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer2->Add(5, 10);
	sizer2->Add(m_nlibPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer2->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer2, 0, wxEXPAND);

	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Output Directory:", wxDefaultPosition, wxSize(120, -1), wxALIGN_RIGHT);
	m_outdirPickCtrl = new wxDirPickerCtrl( this, ID_NB_OutputPicker, outdir, _("Choose an output directory"), wxDefaultPosition, wxSize(400, -1));
	sizer3->Add(5, 10);
	sizer3->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer3->Add(5, 10);
	sizer3->Add(m_outdirPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer3->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer3, 0, wxEXPAND);

	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Output File Name:", wxDefaultPosition, wxSize(120, -1), wxALIGN_RIGHT);
	m_ofnameTextCtrl = new wxTextCtrl( this, ID_NB_OutFileText, "", wxDefaultPosition, wxSize(200, -1));
	sizer4->Add(5, 10);
	sizer4->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer4->Add(5, 10);
	sizer4->Add(m_ofnameTextCtrl, 1, wxRIGHT);
	sizer4->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer4, 0, wxALIGN_LEFT);
    
	m_CommandButton = new wxButton( this, ID_SEND_EVENT_BUTTON, _("Run NBLAST"), wxDefaultPosition, wxDefaultSize, 0 );
	itemBoxSizer2->Add(10, 5);
	itemBoxSizer2->Add(m_CommandButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	m_results = new NBLASTListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(500, 500));
	m_results->addObserver(this);

	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(m_results, 1, wxEXPAND);

	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Result File:", wxDefaultPosition, wxSize(120, -1), wxALIGN_RIGHT);
	m_resultPickCtrl = new wxFilePickerCtrl( this, ID_NB_ResultPicker, "", _("Choose a search result file"), "*.txt", wxDefaultPosition, wxSize(400, -1));
	sizer5->Add(5, 10);
	sizer5->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer5->Add(5, 10);
	sizer5->Add(m_resultPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer5->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer5, 0, wxEXPAND);

	m_ReloadResultButton = new wxButton( this, ID_RELOAD_RESULTS_BUTTON, _("Reload Results"), wxDefaultPosition, wxDefaultSize, 0 );
	itemBoxSizer2->Add(10, 5);
	itemBoxSizer2->Add(m_ReloadResultButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	this->SetSizer(itemBoxSizer2);
	this->Layout();

	////@end NBLASTGuiPluginWindow content construction

	Thaw();
	SetEvtHandlerEnabled(true);

	//m_wtimer->Start(50);
}

void NBLASTGuiPluginWindow::EnableControls(bool enable)
{
	if (enable)
	{
		if (m_RPickCtrl) m_RPickCtrl->Enable();
		if (m_nlibPickCtrl) m_nlibPickCtrl->Enable();
		if (m_outdirPickCtrl) m_outdirPickCtrl->Enable();
		if (m_results) m_results->Enable();
		if (m_CommandButton) m_CommandButton->Enable();
	}
	else 
	{
		if (m_RPickCtrl) m_RPickCtrl->Disable();
		if (m_nlibPickCtrl) m_nlibPickCtrl->Disable();
		if (m_outdirPickCtrl) m_outdirPickCtrl->Disable();
		if (m_results) m_results->Disable();
		if (m_CommandButton) m_CommandButton->Disable();
	}
}

bool NBLASTGuiPluginWindow::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap NBLASTGuiPluginWindow::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin NBLASTGuiPluginWindow bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end NBLASTGuiPluginWindow bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon NBLASTGuiPluginWindow::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin NBLASTGuiPluginWindow icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end NBLASTGuiPluginWindow icon retrieval
}

void NBLASTGuiPluginWindow::doAction(ActionInfo *info)
{
	if (!info)
		return;
	int evid = info->id;
	
	NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();

	switch (evid)
	{
	case NB_OPEN_FILE:
		if (plugin && m_resultPickCtrl)
		{
			wxString str = wxString((char *)info->data);
			wxString zip =  m_resultPickCtrl->GetPath().BeforeLast(L'.', NULL) + _(".zip");
			plugin->LoadSWC(str, zip);
		}
		break;
	default:
		break;
	}
}


void NBLASTGuiPluginWindow::OnSENDEVENTBUTTONClick( wxCommandEvent& event )
{
	wxString rpath = m_RPickCtrl->GetPath();
	wxString nlibpath = m_nlibPickCtrl->GetPath();
	wxString outdir = m_outdirPickCtrl->GetPath();
	wxString ofname = m_ofnameTextCtrl->GetValue();
	NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();
	if (plugin)
	{
		plugin->runNBLAST(rpath, nlibpath, outdir, ofname);

		wxString respath = outdir + wxFILE_SEP_PATH + ofname + _(".txt");
		if (wxFileExists(respath))
		{
			m_resultPickCtrl->SetPath(respath);
			wxCommandEvent e;
			OnReloadResultsButtonClick(e);
		}
	}

//	wxCommandEvent e(wxEVT_GUI_PLUGIN_INTEROP);
//	e.SetString(m_RPickCtrl->GetPath());
//	GetPlugin()->GetEventHandler()->AddPendingEvent(e);

    event.Skip();
}

void NBLASTGuiPluginWindow::OnReloadResultsButtonClick( wxCommandEvent& event )
{
	wxString respath = m_resultPickCtrl->GetPath();
	
	if (m_results) m_results->LoadResults(respath);

    event.Skip();
}

void NBLASTGuiPluginWindow::OnClose(wxCloseEvent& event)
{
	event.Skip();
}
