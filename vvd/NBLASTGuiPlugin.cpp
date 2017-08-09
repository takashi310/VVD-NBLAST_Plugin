#include "stdwx.h"
#include "NBLASTGuiPlugin.h"
#include "NBLASTGuiPluginWindow.h"
#include "VRenderFrame.h"
#include "wx/process.h"
#include "wx/mstream.h"
#include "wx/filename.h"
#include "compatibility.h"

IMPLEMENT_DYNAMIC_CLASS(NBLASTGuiPlugin, wxObject)


NBLASTGuiPlugin::NBLASTGuiPlugin()
	: wxGuiPluginBase(NULL, NULL), m_R_process(NULL)
{

}

NBLASTGuiPlugin::NBLASTGuiPlugin(wxEvtHandler * handler, wxWindow * vvd)
	: wxGuiPluginBase(handler, vvd), m_R_process(NULL)
{
}

NBLASTGuiPlugin::~NBLASTGuiPlugin()
{

}

bool NBLASTGuiPlugin::runNBLAST()
{

	return runNBLAST(m_R_path, m_nlib_path, m_out_dir, m_ofname);
}

bool NBLASTGuiPlugin::runNBLAST(wxString rpath, wxString nlibpath, wxString outdir, wxString ofname)
{
	if (rpath.IsEmpty() || nlibpath.IsEmpty() || outdir.IsEmpty() || ofname.IsEmpty())
		return false;
	
	VRenderFrame *vframe = (VRenderFrame *)m_vvd;
	if (!vframe) return false;

	VolumeData *vd = vframe->GetCurSelVol();
	if (!vd) return false;

#ifdef _WIN32
	wchar_t slash = L'\\';
#else
	wchar_t slash = L'/';
#endif

	wxString tempvdpath = wxStandardPaths::Get().GetTempDir() + slash + "vvdnbtmpv.nrrd";
	vd->Save(tempvdpath, 2, false, true, false, false); 

	m_R_path = rpath;
	m_nlib_path = nlibpath;
	m_out_dir = outdir;
	m_ofname = ofname;

	wxString rscript;
#ifdef _WIN32
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = expath.BeforeLast(slash, NULL);
	rscript = expath + "\\nblast_search.R";
	tempvdpath.Replace("\\", "\\\\");
	m_nlib_path.Replace("\\", "\\\\");
	m_out_dir.Replace("\\", "\\\\");
	m_ofname.Replace("\\", "\\\\");
#else
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = expath.BeforeLast(slash, NULL);
	rscript = expath + "/../Resources/nblast_search.R";
#endif

	wxString com = _("\"")+m_R_path+_("\" ") + _("\"")+rscript+_("\" ") + _("\"")+tempvdpath+_("\" ") +
					_("\"")+m_nlib_path+_("\" ") + _("\"")+m_ofname+_("\" ") + _("\"")+m_out_dir+_("\" ");
	wxExecuteEnv env;
	wxString envpath;
	wxGetEnv(_("PATH"), &envpath);
	env.env["PATH"] = envpath;
	wxExecute(com, wxEXEC_BLOCK, NULL, &env);

	return true;
}

wxString NBLASTGuiPlugin::GetName() const
{
	return _("NBLAST Search");
}

wxString NBLASTGuiPlugin::GetId() const
{
	return wxT("{EEFDF66-5FBB-4719-AF17-76C1C82D3FE1}");
}

wxWindow * NBLASTGuiPlugin::CreatePanel(wxWindow * parent)
{
	return new NBLASTGuiPluginWindow(this, parent);
}

void NBLASTGuiPlugin::OnInit()
{
	LoadConfigFile();
}

void NBLASTGuiPlugin::OnDestroy()
{

}

void NBLASTGuiPlugin::OnTimer(wxTimerEvent& event)
{

}

void NBLASTGuiPlugin::LoadConfigFile()
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = expath.BeforeLast(GETSLASH(), NULL);
#ifdef _WIN32
	wxString dft = expath + "\\NBLAST_plugin_settings.dft";
	if (!wxFileExists(dft))
		dft = wxStandardPaths::Get().GetUserConfigDir() + "\\NBLAST_plugin_settings.dft";
#else
	wxString dft = expath + "/../Resources/NBLAST_plugin_settings.dft";
#endif
	if (wxFileExists(dft))
	{
		wxFileInputStream is(dft);
		if (is.IsOk())
		{
			wxFileConfig fconfig(is);
			wxString str;
			if (fconfig.Read("R_path", &str))
				m_R_path = str;
			if (fconfig.Read("neuronlib_path", &str))
				m_nlib_path = str;
			if (fconfig.Read("output_dir", &str))
				m_out_dir = str;
		}
	}
}

void NBLASTGuiPlugin::SaveConfigFile()
{
	wxFileConfig fconfig("NBLAST plugin default settings");

	fconfig.Write("R_path", m_R_path);
	fconfig.Write("neuronlib_path", m_nlib_path);
	fconfig.Write("output_dir", m_out_dir);

	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = expath.BeforeLast(GETSLASH(),NULL);
#ifdef _WIN32
	wxString dft = expath + "\\NBLAST_plugin_settings.dft";
	wxString dft2 = wxStandardPaths::Get().GetUserConfigDir() + "\\NBLAST_plugin_settings.dft";
	if (!wxFileExists(dft) && wxFileExists(dft2))
		dft = dft2;
#else
	wxString dft = expath + "/../Resources/NBLAST_plugin_settings.dft";
#endif
	wxFileOutputStream os(dft);
	fconfig.Save(os);
}
