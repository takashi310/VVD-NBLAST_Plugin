#pragma once

#include <wxGuiPluginBase.h>
#include "utility.h"

#define NB_PLUGIN_VERSION "1.00"

#define NB_OPEN_FILE 1
#define NB_SET_IMAGE 2

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
	virtual bool OnRun(wxString options);

	void SetRPath(wxString path) { m_R_path = path; }
	wxString GetRPath() { return m_R_path; }
	void SetNlibPath(wxString path) { m_nlib_path = path; }
	wxString GetNlibPath() { return m_nlib_path; }
	void SetOutDir(wxString path) { m_out_dir = path; }
	wxString GetOutDir() { return m_out_dir; }
	void SetFileName(wxString name) { m_ofname = name; }
	wxString GetFileName() { return m_ofname; }
	void SetResultNum(wxString num) { m_rnum = num; }
	wxString GetResultNum() { return m_rnum; }
	void SetDatabaseNames(wxString num) { m_db_names = num; }
	wxString GetDatabaseNames() { return m_db_names; }

	wxString GetPID() { return m_pid; }

	void LoadConfigFile();
	void SaveConfigFile();

	bool runNBLAST();
	bool runNBLAST(wxString rpath, wxString nlibpath, wxString outdir, wxString ofname, wxString rnum, wxString db_names=wxString());
	bool LoadSWC(wxString name, wxString swc_zip_path);
	bool LoadSWC(wxString path);
	bool skeletonizeMask();

	void OnTimer(wxTimerEvent& event);

private:
	wxString m_R_path;
	wxString m_out_dir;
	wxString m_nlib_path;
	wxString m_ofname;
	wxString m_rnum;
	wxString m_db_names;
	wxTimer m_timer;
	wxStopWatch m_watch;
	wxString m_pid;
	wxProcess *m_R_process;
};