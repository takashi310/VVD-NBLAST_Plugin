
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
#include <wx/dcbuffer.h>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(wxImagePanel, wxPanel)
EVT_PAINT(wxImagePanel::OnDraw)
EVT_SIZE(wxImagePanel::OnSize)
END_EVENT_TABLE()

wxImagePanel::wxImagePanel(wxWindow* parent, int w, int h) :
wxPanel(parent)
{
	m_orgimage = NULL;
	m_resized = NULL;
	m_w = -1;
	m_h = -1;
	SetBackgroundStyle(wxBG_STYLE_PAINT);
}

wxImagePanel::~wxImagePanel()
{
	wxDELETE(m_orgimage);
	wxDELETE(m_resized);
	m_image.Destroy();
	m_olimage.Destroy();
}

void wxImagePanel::SetImage(wxString file, wxBitmapType format)
{
	wxDELETE(m_orgimage);
	wxDELETE(m_resized);
	m_image.Destroy();

	if (!wxFileExists(file))
		return;

	m_orgimage = new wxImage(file, format);
	if (m_orgimage && m_orgimage->IsOk())
	{
		m_resized = new wxBitmap();
		m_image = m_orgimage->Copy();
	}
	else
		wxDELETE(m_orgimage);
}

void wxImagePanel::SetOverlayImage(wxString file, wxBitmapType format, bool show)
{
	if (!wxFileExists(file) || !m_orgimage || !m_orgimage->IsOk())
		return;

	m_olimage.Destroy();
	m_olimage.LoadFile(file, format);
	if (!m_olimage.IsOk())
		return;

	ToggleOverlayVisibility(show);
}

void wxImagePanel::ToggleOverlayVisibility(bool show)
{
	if (!m_orgimage || !m_orgimage->IsOk())
		return;

	if (!m_olimage.IsOk())
		return;

	if (!show)
	{
		m_image = m_orgimage->Copy();
		return;
	}

	int w = m_orgimage->GetSize().GetWidth();
	int h = m_orgimage->GetSize().GetHeight();

	if (m_olimage.GetSize() != m_orgimage->GetSize())
		m_olimage = m_olimage.Scale( w, h, wxIMAGE_QUALITY_HIGH);

	if (!m_olimage.IsOk())
		return;

	m_image = m_orgimage->Copy();
	
	unsigned char *oldata = m_olimage.GetData();
	unsigned char *imdata = m_image.GetData();

	double alpha = 0.6;
	double alpha2 = 0.8;

	m_olimage.Blur(1);
	
	for (int y = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++)
		{
			if (oldata[(y*w+x)*3] >= 64)
			{
				imdata[(y*w+x)*3]   = (unsigned char)(alpha*255.0 + (1.0-alpha)*(double)imdata[(y*w+x)*3]);
				imdata[(y*w+x)*3+1] = (unsigned char)(alpha*0.0   + (1.0-alpha)*(double)imdata[(y*w+x)*3+1]);
				imdata[(y*w+x)*3+2] = (unsigned char)(alpha*0.0   + (1.0-alpha)*(double)imdata[(y*w+x)*3+2]);
			}
			else if (oldata[(y*w+x)*3] > 0)
			{
				imdata[(y*w+x)*3]   = (unsigned char)(alpha2*0.0 + (1.0-alpha2)*(double)imdata[(y*w+x)*3]);
				imdata[(y*w+x)*3+1] = (unsigned char)(alpha2*0.0 + (1.0-alpha2)*(double)imdata[(y*w+x)*3+1]);
				imdata[(y*w+x)*3+2] = (unsigned char)(alpha2*0.0 + (1.0-alpha2)*(double)imdata[(y*w+x)*3+2]);
			}
		}
	}
}

void wxImagePanel::ResetImage()
{
	if (!m_orgimage || !m_orgimage->IsOk())
		return;

	m_image = m_orgimage->Copy();
}

void wxImagePanel::OnDraw(wxPaintEvent & evt)
{
	wxAutoBufferedPaintDC dc(this);
	Render(dc);
}

void wxImagePanel::PaintNow()
{
	wxClientDC dc(this);
	Render(dc);
}

wxSize wxImagePanel::CalcImageSizeKeepAspectRatio(int w, int h)
{
	if (!m_orgimage || !m_orgimage->IsOk())
		return wxSize(-1, -1);

	int orgw = m_orgimage->GetWidth();
	int orgh = m_orgimage->GetHeight();

	double nw = w;
	double nh = (double)w*((double)orgh/(double)orgw);
	if (nh <= h)
		return wxSize((int)nw, (int)nh);

	nw = (double)h*((double)orgw/(double)orgh);
	nh = h;
	return wxSize((int)nw, (int)nh);
}

void wxImagePanel::Render(wxDC& dc)
{
	dc.Clear();

	if (!m_image.IsOk())
		return;

	int neww, newh;
	this->GetSize( &neww, &newh );

	wxSize ns = CalcImageSizeKeepAspectRatio(neww, newh);
	if (ns.GetWidth() <= 0 || ns.GetHeight() <= 0)
		return;

	int posx = neww > ns.GetWidth() ? (neww-ns.GetWidth())/2 : 0;
	int posy = newh > ns.GetHeight() ? (newh-ns.GetHeight())/2 : 0;

	wxDELETE(m_resized);
	m_resized = new wxBitmap( m_image.Scale( ns.GetWidth(), ns.GetHeight(), wxIMAGE_QUALITY_HIGH));
	if (m_resized && m_resized->IsOk())
	{
		m_w = ns.GetWidth();
		m_h = ns.GetHeight();
		dc.DrawBitmap(*m_resized, posx, posy, false);
	}
}

void wxImagePanel::OnEraseBackground(wxEraseEvent& event)
{
	event.Skip();
}

void wxImagePanel::OnSize(wxSizeEvent& event)
{
	Refresh(true);
	event.Skip();
}


BEGIN_EVENT_TABLE(NBLASTListCtrl, wxListCtrl)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, NBLASTListCtrl::OnAct)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, NBLASTListCtrl::OnSelect)
	EVT_LIST_COL_BEGIN_DRAG(wxID_ANY, NBLASTListCtrl::OnColBeginDrag)
	EVT_KEY_DOWN(NBLASTListCtrl::OnKeyDown)
	EVT_KEY_UP(NBLASTListCtrl::OnKeyUp)
	EVT_MOUSE_EVENTS(NBLASTListCtrl::OnMouse)

END_EVENT_TABLE()

NBLASTListCtrl::NBLASTListCtrl(
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, id, pos, size, style)
{
	m_images = NULL;

	wxListItem itemCol;
	itemCol.SetText("");
	this->InsertColumn(0, itemCol);

	itemCol.SetText("Name");
	this->InsertColumn(1, itemCol);

	itemCol.SetText("Preview");
	this->InsertColumn(2, itemCol);

	itemCol.SetText("Color-MIP");
	this->InsertColumn(3, itemCol);
	
	itemCol.SetText("Score");
	itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
	this->InsertColumn(4, itemCol);

	SetColumnWidth(0, 0);
	SetColumnWidth(1, 100);
	SetColumnWidth(2, 150);
	SetColumnWidth(3, 150);
	SetColumnWidth(4, 150);
}

NBLASTListCtrl::~NBLASTListCtrl()
{
	wxDELETE(m_images);
}

void NBLASTListCtrl::LoadResults(wxString csvfilepath, wxString dbdir)
{
	wxFileInputStream input(csvfilepath);
	wxTextInputStream text(input, wxT("\t"), wxConvUTF8 );
	SetEvtHandlerEnabled(false);

	DeleteAllItems();

	bool show_image = false;
	int w = 0, h = 0;

	wxString thumbdir = dbdir + wxFILE_SEP_PATH + _("MIP_thumb");
	wxString prevdir = dbdir + wxFILE_SEP_PATH + _("swc_thumb");

	wxDir dir1(thumbdir);
	if (dir1.IsOpened())
	{
		wxString fimgpath;
		if (dir1.GetFirst(&fimgpath, "*.png"))
		{
			wxBitmap img(_(thumbdir+wxFILE_SEP_PATH+fimgpath), wxBITMAP_TYPE_PNG);
			if (img.IsOk())
			{
				w = img.GetWidth();
				h = img.GetHeight();
                
#ifdef _DARWIN
                SetColumnWidth(2, w+8);
                SetColumnWidth(3, w+8);
#else
                SetColumnWidth(2, w+2);
                SetColumnWidth(3, w+2);
#endif
				show_image = true;
			}
		}
	}

	wxArrayString names, scores;
	while(input.IsOk() && !input.Eof() )
	{
		wxString line = text.ReadLine();
		wxStringTokenizer tkz(line, wxT(","));
		
		wxArrayString con;
		while(tkz.HasMoreTokens())
			con.Add(tkz.GetNextToken());

		if (con.Count() >= 2)
		{
			names.Add(con[0]);
			scores.Add(con[1]);
		}
	}

	if (show_image)
	{
		if (m_images) wxDELETE(m_images);
		m_images = new wxImageList(w, h, false);
        SetImageList(m_images, wxIMAGE_LIST_SMALL);
		
        wxString imgpath;
		
		char *dummy8 = new (std::nothrow) char[w*h];
		memset((void*)dummy8, 0, sizeof(char)*w*h);
		wxBitmap dummy(dummy8, w, h);
		m_images->Add(dummy, wxBITMAP_TYPE_BMP);

		int imgcount = 0;
		for (int i = 0; i < names.GetCount(); i++)
		{
			int mipid = 0;
			int swcid = 0;
			if (wxFileExists(thumbdir+wxFILE_SEP_PATH+names[i]+_(".png")))
			{
				wxBitmap img(thumbdir+wxFILE_SEP_PATH+names[i]+_(".png"), wxBITMAP_TYPE_PNG);
				if (img.IsOk())
				{
					imgcount++;
					m_images->Add(img, wxBITMAP_TYPE_PNG);
					mipid = imgcount;
				}
			}
			if (wxFileExists(prevdir+wxFILE_SEP_PATH+names[i]+_(".png")))
			{
				wxBitmap img(prevdir+wxFILE_SEP_PATH+names[i]+_(".png"), wxBITMAP_TYPE_PNG);
				if (img.IsOk())
				{
					imgcount++;
					m_images->Add(img, wxBITMAP_TYPE_PNG);
					swcid = imgcount;
				}
			}

			Append(names[i], scores[i], mipid, swcid);
		}
	}
	else
	{
		for (int i = 0; i < names.GetCount(); i++)
			Append(names[i], scores[i]);
	}

	SetEvtHandlerEnabled(true);
    Update();

	long item = GetNextItem(-1);
	if (item != -1)
		SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void NBLASTListCtrl::OnColBeginDrag(wxListEvent& event)
{
	if ( event.GetColumn() == 0 )
    {
        event.Veto();
    }
}

void NBLASTListCtrl::Append(wxString name, wxString score, int mipid, int swcid)
{
	long tmp = InsertItem(GetItemCount(), _(""));
	SetItem(tmp, 1, name);
	SetItem(tmp, 2, _(""), swcid);
	SetItem(tmp, 3, _(""), mipid);
	SetItem(tmp, 4, score);
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
		notifyAll(NB_SET_IMAGE, name.ToStdString().c_str(), name.ToStdString().length()+1);
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
		wxString name = GetText(item, 1);
		notifyAll(NB_OPEN_FILE, name.ToStdString().c_str(), name.ToStdString().length()+1);
	}
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

void NBLASTListCtrl::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

void NBLASTListCtrl::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
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
	EVT_BUTTON( ID_SKELETONIZE_BUTTON, NBLASTGuiPluginWindow::OnSkeletonizeButtonClick )
	EVT_BUTTON( ID_RELOAD_RESULTS_BUTTON, NBLASTGuiPluginWindow::OnReloadResultsButtonClick )
	EVT_CHECKBOX(ID_NB_OverlayCheckBox, NBLASTGuiPluginWindow::OnOverlayCheck)
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
	m_splitterWindow = NULL;
	m_RPickCtrl = NULL;
	m_nlibPickCtrl = NULL;
	m_outdirPickCtrl = NULL;
	m_ofnameTextCtrl = NULL;
	m_CommandButton = NULL;
	m_swcImagePanel = NULL;
	m_mipImagePanel = NULL;
	m_overlayChk = NULL;
	m_prg_diag = NULL;
	m_waitingforR = false;
	m_waitingforFiji = false;
	m_wtimer = new wxTimer(this, ID_WaitTimer);
}


/*
 * Control creation for NBLASTGuiPluginWindow
 */

void NBLASTGuiPluginWindow::CreateControls()
{    
	wxString rpath, nlibpath, outdir, rnum;
	NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();
	if (plugin)
	{
		rpath = plugin->GetRPath();
		nlibpath = plugin->GetNlibPath();
		outdir = plugin->GetOutDir();
		rnum = plugin->GetResultNum();
	}

	SetEvtHandlerEnabled(false);
	Freeze();

	m_splitterWindow = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1));
	wxPanel* nbpanel = new wxPanel(m_splitterWindow, wxID_ANY);
	nbpanel->SetWindowStyle(wxBORDER_SIMPLE);
	wxPanel* imgpanel = new wxPanel(m_splitterWindow, wxID_ANY);
	imgpanel->SetWindowStyle(wxBORDER_SIMPLE);

	////@begin NBLASTGuiPluginWindow content construction
	wxBoxSizer* itemBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* itemBoxSizer2_1 = new wxBoxSizer(wxVERTICAL);

	wxIntegerValidator<unsigned int> vald_int;
	vald_int.SetMin(1);

	wxStaticText *st;
#ifdef _WIN32
    int stsize = 120;
#else
    int stsize = 130;
    if (rpath.IsEmpty() && wxFileExists("/Library/Frameworks/R.framework/Resources/bin/Rscript"))
        rpath = _("/Library/Frameworks/R.framework/Resources/bin/Rscript");
#endif

	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(nbpanel, 0, "Rscript:", wxDefaultPosition, wxSize(stsize, -1), wxALIGN_RIGHT);
	m_RPickCtrl = new wxFilePickerCtrl( nbpanel, ID_NB_RPicker, rpath, _("Set a path to Rscript"), wxFileSelectorDefaultWildcardStr, wxDefaultPosition, wxSize(400, -1));
	sizer1->Add(5, 10);
	sizer1->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer1->Add(5, 10);
	sizer1->Add(m_RPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer1->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer1, 0, wxEXPAND);

	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(nbpanel, 0, "Target Neurons (.rds):", wxDefaultPosition, wxSize(stsize, -1), wxALIGN_RIGHT);
	m_nlibPickCtrl = new wxFilePickerCtrl( nbpanel, ID_NB_RPicker, nlibpath, _("Choose a target neuron library"), "*.rds", wxDefaultPosition, wxSize(400, -1));
	sizer2->Add(5, 10);
	sizer2->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer2->Add(5, 10);
	sizer2->Add(m_nlibPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer2->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer2, 0, wxEXPAND);

	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(nbpanel, 0, "Output Directory:", wxDefaultPosition, wxSize(stsize, -1), wxALIGN_RIGHT);
	m_outdirPickCtrl = new wxDirPickerCtrl( nbpanel, ID_NB_OutputPicker, outdir, _("Choose an output directory"), wxDefaultPosition, wxSize(400, -1));
	sizer3->Add(5, 10);
	sizer3->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer3->Add(5, 10);
	sizer3->Add(m_outdirPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer3->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer3, 0, wxEXPAND);

	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(nbpanel, 0, "Project Name:", wxDefaultPosition, wxSize(stsize, -1), wxALIGN_RIGHT);
	m_ofnameTextCtrl = new wxTextCtrl( nbpanel, ID_NB_OutFileText, "", wxDefaultPosition, wxSize(200, -1));
	sizer4->Add(5, 10);
	sizer4->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer4->Add(5, 10);
	sizer4->Add(m_ofnameTextCtrl, 1, wxRIGHT);
	sizer4->Add(20, 10);
	st = new wxStaticText(nbpanel, 0, "Save Top", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	m_rnumTextCtrl = new wxTextCtrl( nbpanel, ID_NB_ResultNumText, rnum, wxDefaultPosition, wxSize(30, -1), wxTE_RIGHT, vald_int);
	sizer4->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer4->Add(5, 10);
	sizer4->Add(m_rnumTextCtrl, 0, wxRIGHT);
	sizer4->Add(5, 10);
	st = new wxStaticText(nbpanel, 0, "Neurons", wxDefaultPosition, wxSize(70, -1), wxALIGN_LEFT);
	sizer4->Add(st, 0, wxALIGN_CENTER_VERTICAL);

	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer4, 0, wxALIGN_LEFT);
    
	wxBoxSizer *sizerb = new wxBoxSizer(wxHORIZONTAL);
	m_SkeletonizeButton = new wxButton( nbpanel, ID_SKELETONIZE_BUTTON, _("Skeletonize"), wxDefaultPosition, wxDefaultSize, 0 );
	m_CommandButton = new wxButton( nbpanel, ID_SEND_EVENT_BUTTON, _("Run NBLAST"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerb->Add(m_SkeletonizeButton);
//	sizerb->Add(75, 10);
	sizerb->Add(m_CommandButton);
	itemBoxSizer2->Add(10, 5);
	itemBoxSizer2->Add(sizerb, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	m_SkeletonizeButton->Hide();

    wxBoxSizer *sizerl = new wxBoxSizer(wxHORIZONTAL);
	m_results = new NBLASTListCtrl(nbpanel, wxID_ANY, wxDefaultPosition, wxSize(590, 500));
	m_results->addObserver(this);
    sizerl->Add(5,10);
    sizerl->Add(m_results, 1, wxEXPAND);
    sizerl->Add(5,10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizerl, 1, wxEXPAND);

	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(nbpanel, 0, "Result File:", wxDefaultPosition, wxSize(stsize, -1), wxALIGN_RIGHT);
	m_resultPickCtrl = new wxFilePickerCtrl( nbpanel, ID_NB_ResultPicker, "", _("Choose a search result file"), "*.txt", wxDefaultPosition, wxSize(400, -1));
	sizer5->Add(5, 10);
	sizer5->Add(st, 0, wxALIGN_CENTER_VERTICAL);
	sizer5->Add(5, 10);
	sizer5->Add(m_resultPickCtrl, 1, wxRIGHT|wxEXPAND);
	sizer5->Add(10, 10);
	itemBoxSizer2->Add(5, 5);
	itemBoxSizer2->Add(sizer5, 0, wxEXPAND);

	m_ReloadResultButton = new wxButton( nbpanel, ID_RELOAD_RESULTS_BUTTON, _("Reload Results"), wxDefaultPosition, wxDefaultSize, 0 );
	itemBoxSizer2->Add(10, 5);
	itemBoxSizer2->Add(m_ReloadResultButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	wxBoxSizer *sizerchk = new wxBoxSizer(wxHORIZONTAL);
	m_overlayChk = new wxCheckBox(imgpanel, ID_NB_OverlayCheckBox, "Overlay search query");
	m_overlayChk->SetValue(true);
	sizerchk->Add(20, 10);
	sizerchk->Add(m_overlayChk, 0, wxALIGN_CENTER_VERTICAL);
	m_swcImagePanel = new wxImagePanel( imgpanel, 500, 250);
	m_mipImagePanel = new wxImagePanel( imgpanel, 500, 250);
	itemBoxSizer2_1->Add(5, 5);
	itemBoxSizer2_1->Add(sizerchk, 0, wxLEFT);
	itemBoxSizer2_1->Add(5, 5);
	itemBoxSizer2_1->Add(m_swcImagePanel, 1, wxEXPAND);
	itemBoxSizer2_1->Add(5, 5);
	itemBoxSizer2_1->Add(m_mipImagePanel, 1, wxEXPAND);

	nbpanel->SetSizer(itemBoxSizer2);
	imgpanel->SetSizer(itemBoxSizer2_1);

	nbpanel->SetMinSize(wxSize(610,750));
	imgpanel->SetMinSize(wxSize(510,750));
	
	m_splitterWindow->SplitVertically(nbpanel, imgpanel);

	itemBoxSizer->Add(m_splitterWindow, 1, wxEXPAND); 
	this->SetSizer(itemBoxSizer);
	this->Layout();

	////@end NBLASTGuiPluginWindow content construction

	plugin->GetEventHandler()->Bind(wxEVT_GUI_PLUGIN_INTEROP, 
		wxCommandEventHandler(NBLASTGuiPluginWindow::OnInteropMessageReceived), this);

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
	case NB_SET_IMAGE:
		if (plugin && m_nlibPickCtrl && m_resultPickCtrl && m_swcImagePanel && m_mipImagePanel)
		{
			wxString prjimg = m_resultPickCtrl->GetPath().BeforeLast(L'.', NULL) + _(".png");

			wxString imgpath1 = m_nlibPickCtrl->GetPath().BeforeLast(wxFILE_SEP_PATH, NULL) + wxFILE_SEP_PATH +
				_("swc_prev") + wxFILE_SEP_PATH + wxString((char *)info->data) + _(".png");
			m_swcImagePanel->SetImage(imgpath1, wxBITMAP_TYPE_PNG);
			if (m_overlayChk->GetValue()) m_swcImagePanel->SetOverlayImage(prjimg, wxBITMAP_TYPE_PNG);
			m_swcImagePanel->Refresh();
			
			wxString imgpath2 = m_nlibPickCtrl->GetPath().BeforeLast(wxFILE_SEP_PATH, NULL) + wxFILE_SEP_PATH +
				_("MIP") + wxFILE_SEP_PATH + wxString((char *)info->data) + _(".png");
			m_mipImagePanel->SetImage(imgpath2, wxBITMAP_TYPE_PNG);
			if (m_overlayChk->GetValue()) m_mipImagePanel->SetOverlayImage(prjimg, wxBITMAP_TYPE_PNG);
			m_mipImagePanel->Refresh();
		}
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
	wxString rnum = m_rnumTextCtrl->GetValue();
	long lval = -1;
	NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();
	if (plugin)
	{
		if (!wxFileExists(rpath))
			{wxMessageBox("Could not find Rscript binary", "NBLAST Plugin"); event.Skip(); return;}
		if (!wxFileExists(nlibpath))
			{wxMessageBox("Could not find a target neuron file", "NBLAST Plugin");; event.Skip(); return;}
		if (outdir.IsEmpty())
			{wxMessageBox("Set an output directory", "NBLAST Plugin");; event.Skip(); return;}
		if (ofname.IsEmpty())
			{wxMessageBox("Set a project name", "NBLAST Plugin");; event.Skip(); return;}
		if (!rnum.ToLong(&lval))
			{wxMessageBox("Invalid number", "NBLAST Plugin");; event.Skip(); return;}

		VRenderFrame *vframe = (VRenderFrame *)plugin->GetVVDMainFrame();
		if (vframe)
		{
			VolumeData *vd = vframe->GetCurSelVol();
			if (vd)
			{
				if (vd->GetName().Find("skeleton") == wxNOT_FOUND)
				{
					if (plugin->skeletonizeMask())
					{
						plugin->SetRPath(rpath);
						plugin->SetNlibPath(nlibpath);
						plugin->SetOutDir(outdir);
						plugin->SetFileName(ofname);
						plugin->SetResultNum(rnum);
						m_waitingforFiji = true;
					}
				}
				else if (!m_waitingforFiji)
				{
					plugin->runNBLAST(rpath, nlibpath, outdir, ofname, rnum);

					wxString respath = outdir + wxFILE_SEP_PATH + ofname + _(".txt");
					if (wxFileExists(respath))
					{
						m_resultPickCtrl->SetPath(respath);
						wxCommandEvent e;
						OnReloadResultsButtonClick(e);
					}
				}
			}
		}

	}

//	wxCommandEvent e(wxEVT_GUI_PLUGIN_INTEROP);
//	e.SetString(m_RPickCtrl->GetPath());
//	GetPlugin()->GetEventHandler()->AddPendingEvent(e);

    event.Skip();
}

void NBLASTGuiPluginWindow::OnSkeletonizeButtonClick( wxCommandEvent& event )
{
	NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();
	if (plugin) plugin->skeletonizeMask();
}

void NBLASTGuiPluginWindow::OnReloadResultsButtonClick( wxCommandEvent& event )
{
	wxString respath = m_resultPickCtrl->GetPath();
	
	if (m_results) m_results->LoadResults(respath, m_nlibPickCtrl->GetPath().BeforeLast(wxFILE_SEP_PATH, NULL));

    event.Skip();
}

void NBLASTGuiPluginWindow::OnOverlayCheck( wxCommandEvent& event )
{
	if (m_swcImagePanel && m_mipImagePanel)
	{
		m_swcImagePanel->ToggleOverlayVisibility(m_overlayChk->GetValue());
		m_mipImagePanel->ToggleOverlayVisibility(m_overlayChk->GetValue());
		m_swcImagePanel->Refresh();
		m_mipImagePanel->Refresh();
	}
}

void NBLASTGuiPluginWindow::OnClose(wxCloseEvent& event)
{
	event.Skip();
}

void NBLASTGuiPluginWindow::OnInteropMessageReceived(wxCommandEvent & event)
{
	if (m_waitingforFiji)
	{
		NBLASTGuiPlugin* plugin = (NBLASTGuiPlugin *)GetPlugin();
		if (!plugin) return;

		wxString message = event.GetString();
		wxStringTokenizer tkz(message, wxT(","));

		wxArrayString args;
		while(tkz.HasMoreTokens())
			args.Add(tkz.GetNextToken());

		if (args[0] == _("Fiji Interface") && args[1] == _("finished") && args[2] == _("NBLAST Skeletonize"))
		{
			m_waitingforFiji = false;
			plugin->runNBLAST();

			wxString respath = plugin->GetOutDir() + wxFILE_SEP_PATH + plugin->GetFileName() + _(".txt");
			if (wxFileExists(respath))
			{
				m_resultPickCtrl->SetPath(respath);
				wxCommandEvent e;
				OnReloadResultsButtonClick(e);
			}
		}
	}
}
