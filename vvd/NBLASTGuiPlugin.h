#pragma once

#include <wxGuiPluginBase.h>
#include "utility.h"

#define NB_PLUGIN_VERSION "1.00"

#define NB_OPEN_FILE 1

class NBLASTGuiPlugin : public wxGuiPluginBase, public Notifier
{
	DECLARE_DYNAMIC_CLASS(NBLASTGuiPlugin)
public:
	NBLASTGuiPlugin();
	NBLASTGuiPlugin(wxEvtHandler * handler, wxWindow * vvd);
	virtual ~NBLASTGuiPlugin();

	virtual wxString GetName() const;
	virtual wxString GetId() const;
	virtual wxWindow * CreatePanel(wxWindow * parent);
	virtual void OnInit();
	virtual void OnDestroy();

	void SetRPath(wxString path) { m_R_path = path; }
	wxString GetRPath() { return m_R_path; }
	void SetNlibPath(wxString path) { m_nlib_path = path; }
	wxString GetNlibPath() { return m_nlib_path; }
	void SetOutDir(wxString path) { m_out_dir = path; }
	wxString GetOutDir() { return m_out_dir; }
	void SetFileName(wxString name) { m_ofname = name; }
	wxString GetFileName() { return m_ofname; }

	wxString GetPID() { return m_pid; }

	void LoadConfigFile();
	void SaveConfigFile();

	bool runNBLAST();
	bool runNBLAST(wxString rpath, wxString nlibpath, wxString outdir, wxString ofname);
	bool LoadSWC(wxString name, wxString swc_zip_path);

	void OnTimer(wxTimerEvent& event);

private:
	wxString m_R_path;
	wxString m_out_dir;
	wxString m_nlib_path;
	wxString m_ofname;
	wxTimer m_timer;
	wxStopWatch m_watch;
	wxString m_pid;
	wxProcess *m_R_process;
};