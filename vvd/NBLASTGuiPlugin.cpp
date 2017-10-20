#include "stdwx.h"
#include "NBLASTGuiPlugin.h"
#include "NBLASTGuiPluginWindow.h"
#include "VRenderFrame.h"
#include <wx/process.h>
#include <wx/mstream.h>
#include <wx/filename.h>
#include <wx/zipstrm.h>
#include "compatibility.h"

IMPLEMENT_DYNAMIC_CLASS(NBLASTGuiPlugin, wxObject)

bool NBLASTGuiPlugin::m_exp_swc = false;
bool NBLASTGuiPlugin::m_exp_swcprev = false;
bool NBLASTGuiPlugin::m_exp_mip = false;
bool NBLASTGuiPlugin::m_exp_vol = false;
bool NBLASTGuiPlugin::m_pfx_score = true;
bool NBLASTGuiPlugin::m_pfx_db = true;

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

	return runNBLAST(m_R_path, m_nlib_path, m_out_dir, m_ofname, m_rnum, m_db_names);
}

bool NBLASTGuiPlugin::runNBLAST(wxString rpath, wxString nlibpath, wxString outdir, wxString ofname, wxString rnum, wxString db_names)
{
	if (!wxFileExists(rpath) || outdir.IsEmpty() || ofname.IsEmpty() || rnum.IsEmpty())
		return false;
	
	VRenderFrame *vframe = (VRenderFrame *)m_vvd;
	if (!vframe) return false;

	VolumeData *vd = vframe->GetCurSelVol();
	if (!vd) return false;

	wxString tempvdpath = wxStandardPaths::Get().GetTempDir() + wxFILE_SEP_PATH + "vvdnbtmpv.nrrd";
	vd->Save(tempvdpath, 2, false, true, true, false); 
	wxString maskfname = wxStandardPaths::Get().GetTempDir() + wxFILE_SEP_PATH + "vvdnbtmpv.msk";
	if (wxFileExists(maskfname))
		wxRenameFile(maskfname, tempvdpath, true);

	m_R_path = rpath;
	m_nlib_path = nlibpath;
	m_out_dir = outdir;
	m_ofname = ofname;
	m_rnum = rnum;
	m_db_names = db_names;

	wxString rscript;
#ifdef _WIN32
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = expath.BeforeLast(wxFILE_SEP_PATH, NULL);
	rscript = expath + "\\nblast_search.R";
	//tempvdpath.Replace("\\", "\\\\");
	//m_nlib_path.Replace("\\", "\\\\");
	//m_out_dir.Replace("\\", "\\\\");
	//m_ofname.Replace("\\", "\\\\");
    
    wxString com = "call " + _("\"")+m_R_path+_("\" ") + _("\"")+rscript+_("\" ") + _("\"")+tempvdpath+_("\" ") +
    _("\"")+m_nlib_path+_("\" ") + _("\"")+m_ofname+_("\" ") + _("\"")+m_out_dir+_("\" ") + _("\"")+m_rnum+_("\" ");
	if (!m_db_names.IsEmpty())
		com += _("\"")+m_db_names+_("\"");
    wxExecuteEnv env;
    wxString envpath;
    wxGetEnv(_("PATH"), &envpath);
    env.env["PATH"] = envpath;
    //wxExecute(com, wxEXEC_SYNC, NULL, &env);
	wxShell(com);
#else
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = expath.BeforeLast(wxFILE_SEP_PATH, NULL);
	rscript = expath + "/../Resources/nblast_search.R";
    wxString term = expath + "/../Resources/term.sh";
    wxString com = _("bash \'")+term+_("\' ") + _("\"") + _("\'")+m_R_path+_("\' ") + _("\'")+rscript+_("\' ") + _("\'")+tempvdpath+_("\' ") +
    _("\'")+m_nlib_path+_("\' ") + _("\'")+m_ofname+_("\' ") + _("\'")+m_out_dir+_("\' ") + _("\'")+m_rnum+_("\' ");
	if (!m_db_names.IsEmpty())
		com += _("\'")+m_db_names+_("\' ");
	com += _("\"");
    wxExecute(com, wxEXEC_SYNC);
    wxString act = "osascript -e 'tell application \"System Events\" to set frontmost of the first process whose unix id is "+wxString::Format("%lu", wxGetProcessId())+" to true'";
    wxExecute(act, wxEXEC_HIDE_CONSOLE|wxEXEC_ASYNC);
#endif

	return true;
}

bool NBLASTGuiPlugin::OnRun(wxString options)
{

	return true;
}

bool NBLASTGuiPlugin::LoadSWC(wxString name, wxString swc_zip_path)
{
	wxFFileInputStream fis(swc_zip_path);
	if (!fis.IsOk()) return false;

	wxConvAuto conv;
	wxZipInputStream zis(fis, conv);
	if (!zis.IsOk()) return false;

	VRenderFrame *vframe = (VRenderFrame *)m_vvd;
	if (!vframe) return false;

	wxZipEntry *entry;
	wxString ename = name;

	if (ename.AfterLast(L'.') != wxT("swc"))
		ename += wxT(".swc");

	size_t esize = 0;
	while (entry = zis.GetNextEntry())
	{
		if (ename == entry->GetName())
		{
			esize = entry->GetSize();
			break;
		}
	}

	if (esize <= 0) return false;

	char *buff = new char[esize];
	zis.Read(buff, esize);

	wxString tempswcpath = wxStandardPaths::Get().GetTempDir() + wxFILE_SEP_PATH + ename;

	wxFile tmpswc(tempswcpath, wxFile::write);

	if (tmpswc.IsOpened())
	{
		tmpswc.Write(buff, esize);
		tmpswc.Close();
		wxArrayString arr;
		arr.Add(tempswcpath);
		vframe->LoadMeshes(arr);
		wxRemoveFile(tempswcpath);
	}

	delete [] buff;

	return true;
}

bool NBLASTGuiPlugin::LoadFiles(wxString path)
{
	VRenderFrame *vframe = (VRenderFrame *)m_vvd;
	if (!vframe) return false;

	wxArrayString arr;
	arr.Add(path);
	vframe->StartupLoad(arr);

	return true;
}

bool NBLASTGuiPlugin::skeletonizeMask()
{
	wxString fi_name = _("Fiji Interface");

	VRenderFrame *vframe = (VRenderFrame *)m_vvd;
	if (!vframe) return false;

	if (!vframe->PluginExists(fi_name))
	{
		wxMessageBox("ERROR: Could not found Fiji interface", "NBLAST Plugin");
		return false;
	}

	if (!vframe->IsCreatedPluginWindow(fi_name))
	{
		vframe->CreatePluginWindow(fi_name);
		vframe->ToggleVisibilityPluginWindow(fi_name, false);
	}

	vframe->RunPlugin(fi_name, "NBLAST Skeletonize,true,false");

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

#ifdef _WIN32
	if (m_R_path.IsEmpty())
	{
		WCHAR szp64[MAX_PATH], szp[MAX_PATH];
		ExpandEnvironmentStrings(L"%ProgramW6432%", szp64, ARRAYSIZE(szp64));
		ExpandEnvironmentStrings(L"%programfiles(x86)%", szp, ARRAYSIZE(szp));
		
		wxString r64dir = wxString(szp64) + wxFILE_SEP_PATH + "R";
		wxString rdir = wxString(szp) + wxFILE_SEP_PATH + "R";
		wxString rpath;
		
		if (wxDirExists(r64dir))
		{
			wxDir dir(r64dir);
			wxString subdir;
			wxArrayString list;
			for ( bool cont = dir.GetFirst(&subdir, wxString("R-*"), wxDIR_DIRS); cont; cont = dir.GetNext(&subdir) )
				list.Add(subdir);
			list.Sort(true);

			for (int i = 0 ; i < list.GetCount(); i++)
			{
				wxArrayString result;
				wxString bindir = r64dir + wxFILE_SEP_PATH + list[i] + wxFILE_SEP_PATH + "bin";
				wxDir::GetAllFiles(bindir, &result, wxT("*Rscript*"));
				if (result.GetCount() > 0)
				{
					rpath = result[0];
					break;
				}
			}
		}
		else if (rpath.IsEmpty() && wxDirExists(rdir))
		{
			wxDir dir(rdir);
			wxString subdir;
			wxArrayString list;
			for ( bool cont = dir.GetFirst(&subdir, wxString("R-*"), wxDIR_DIRS); cont; cont = dir.GetNext(&subdir) )
				list.Add(subdir);
			list.Sort(true);

			for (int i = 0 ; i < list.GetCount(); i++)
			{
				wxArrayString result;
				wxString bindir = rdir + wxFILE_SEP_PATH + list[i] + wxFILE_SEP_PATH + "bin";
				wxDir::GetAllFiles(bindir, &result, wxT("*Rscript*"));
				if (result.GetCount() > 0)
				{
					rpath = result[0];
					break;
				}
			}
		}

        m_R_path = rpath;
	}
#else
	if (m_R_path.IsEmpty() && wxFileExists("/Library/Frameworks/R.framework/Resources/bin/Rscript"))
        m_R_path = _("/Library/Frameworks/R.framework/Resources/bin/Rscript");
#endif

	if (m_out_dir.IsEmpty())
		m_out_dir = wxStandardPaths::Get().GetTempDir();
	if (m_rnum.IsEmpty())
		m_rnum = wxT("100");
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
			
			if (fconfig.Read("rnum", &str))
				m_rnum = str;
			else
				m_rnum = "10";

			bool bval = false;
			if (fconfig.Read("export_swc", &bval))
				m_exp_swc = bval;
			if (fconfig.Read("export_swcprev", &bval))
				m_exp_swcprev = bval;
			if (fconfig.Read("export_mip", &bval))
				m_exp_mip = bval;
			if (fconfig.Read("export_vol", &bval))
				m_exp_vol = bval;
			if (fconfig.Read("prefix_score", &bval))
				m_pfx_score = bval;
			if (fconfig.Read("prefix_database", &bval))
				m_pfx_db = bval;
		}
	}
}

void NBLASTGuiPlugin::SaveConfigFile()
{
	wxFileConfig fconfig("NBLAST plugin default settings");

	fconfig.Write("R_path", m_R_path);
	fconfig.Write("neuronlib_path", m_nlib_path);
	fconfig.Write("output_dir", m_out_dir);
	fconfig.Write("rnum", m_rnum);
	fconfig.Write("export_swc", m_exp_swc);
	fconfig.Write("export_swcprev", m_exp_swcprev);
	fconfig.Write("export_mip", m_exp_mip);
	fconfig.Write("export_vol", m_exp_vol);
	fconfig.Write("prefix_score", m_pfx_score);
	fconfig.Write("prefix_database", m_pfx_db);

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
