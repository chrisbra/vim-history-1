#include "stdafx.h"
#include <comdef.h>	// For _bstr_t
#include "VisVim.h"
#include "Commands.h"
#include "OleAut.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

#endif


// Change directory before opening file?
#define CD_SOURCE		0	// Cd to source path
#define CD_SOURCE_PARENT	1	// Cd to parent directory of source path
#define CD_NONE			2	// No cd


static BOOL g_bEnableVim = TRUE;	// Vim enabled
static BOOL g_bDevStudioEditor = FALSE;	// Open file in Dev Studio editor simultaneously
static int g_ChangeDir = CD_NONE;	// CD after file open?

static void VimSetEnableState (BOOL bEnableState);
static BOOL VimOpenFile (BSTR& FileName, long LineNr);
static DISPID VimGetDispatchId (COleAutomationControl& VimOle, char* Method);
static void VimErrDiag (COleAutomationControl& VimOle);
static void VimChangeDir (COleAutomationControl& VimOle, DISPID DispatchId, BSTR& FileName);
static void DebugMsg (char* Msg, char* Arg = NULL);


/////////////////////////////////////////////////////////////////////////////
// CCommands

CCommands::CCommands ()
{
	// m_pApplication == NULL; M$ Code generation bug!!!
	m_pApplication = NULL;
	m_pApplicationEventsObj = NULL;
	m_pDebuggerEventsObj = NULL;
}

CCommands::~CCommands ()
{
	ASSERT (m_pApplication != NULL);
	if (m_pApplication)
	{
		m_pApplication->Release ();
		m_pApplication = NULL;
	}
}

void CCommands::SetApplicationObject (IApplication * pApplication)
{
	// This function assumes pApplication has already been AddRef'd
	// for us, which CDSAddIn did in its QueryInterface call
	// just before it called us.
	m_pApplication = pApplication;
	if (! m_pApplication)
		return;

	// Create Application event handlers
	XApplicationEventsObj::CreateInstance (&m_pApplicationEventsObj);
	if (! m_pApplicationEventsObj)
	{
		ReportInternalError ("XApplicationEventsObj::CreateInstance");
		return;
	}
	m_pApplicationEventsObj->AddRef ();
	m_pApplicationEventsObj->Connect (m_pApplication);
	m_pApplicationEventsObj->m_pCommands = this;

#ifdef NEVER
	// Create Debugger event handler
	CComPtr < IDispatch > pDebugger;
	if (SUCCEEDED (m_pApplication->get_Debugger (&pDebugger))
	    && pDebugger != NULL)
	{
		XDebuggerEventsObj::CreateInstance (&m_pDebuggerEventsObj);
		m_pDebuggerEventsObj->AddRef ();
		m_pDebuggerEventsObj->Connect (pDebugger);
		m_pDebuggerEventsObj->m_pCommands = this;
	}
#endif

	// Get settings from registry HKEY_CURRENT_USER\Software\Vim\VisVim
	HKEY hAppKey = GetAppKey ("Vim");
	if (hAppKey)
	{
		HKEY hSectionKey = GetSectionKey (hAppKey, "VisVim");
		if (hSectionKey)
		{
			g_bEnableVim = GetRegistryInt (hSectionKey, "EnableVim",
						       g_bEnableVim);
			g_bDevStudioEditor = GetRegistryInt(hSectionKey,"DevStudioEditor",
							    g_bDevStudioEditor);
			g_ChangeDir = GetRegistryInt (hSectionKey, "ChangeDir",
						      g_ChangeDir);
			RegCloseKey (hSectionKey);
		}
		RegCloseKey (hAppKey);
	}
}

void CCommands::UnadviseFromEvents ()
{
	ASSERT (m_pApplicationEventsObj != NULL);
	if (m_pApplicationEventsObj)
	{
		m_pApplicationEventsObj->Disconnect (m_pApplication);
		m_pApplicationEventsObj->Release ();
		m_pApplicationEventsObj = NULL;
	}

#ifdef NEVER
	if (m_pDebuggerEventsObj)
	{
		// Since we were able to connect to the Debugger events, we
		// should be able to access the Debugger object again to
		// unadvise from its events (thus the VERIFY_OK below--see
		// stdafx.h).
		CComPtr < IDispatch > pDebugger;
		VERIFY_OK (m_pApplication->get_Debugger (&pDebugger));
		ASSERT (pDebugger != NULL);
		m_pDebuggerEventsObj->Disconnect (pDebugger);
		m_pDebuggerEventsObj->Release ();
		m_pDebuggerEventsObj = NULL;
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
// Event handlers

// Application events

HRESULT CCommands::XApplicationEvents::BeforeBuildStart ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BuildFinish (long nNumErrors, long nNumWarnings)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeApplicationShutDown ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

// The open document event handle is the place where the real interface work
// is done.
// Vim gets called from here.
//
HRESULT CCommands::XApplicationEvents::DocumentOpen (IDispatch * theDocument)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	if (! g_bEnableVim)
		// Vim not enabled or empty command line entered
		return S_OK;

	// First get the current file name and line number

	// Get the document object
	CComQIPtr < ITextDocument, &IID_ITextDocument > pDoc (theDocument);
	if (! pDoc)
		return S_OK;

	BSTR FileName;
	long LineNr = -1;

	// Get the document name
	if (FAILED (pDoc->get_FullName (&FileName)))
		return S_OK;

	LPDISPATCH pDispSel;

	// Get a selection object dispatch pointer
	if (SUCCEEDED (pDoc->get_Selection (&pDispSel)))
	{
		// Get the selection object
		CComQIPtr < ITextSelection, &IID_ITextSelection > pSel (pDispSel);

		if (pSel)
			// Get the selection line number
			pSel->get_CurrentLine (&LineNr);

		pDispSel->Release ();
	}

	// Open the file in Vim and position to the current line
	if (VimOpenFile (FileName, LineNr))
	{
		if (! g_bDevStudioEditor)
		{
			// Close the document in developer studio
			CComVariant vSaveChanges = dsSaveChangesPrompt;
			DsSaveStatus Saved;

			pDoc->Close (vSaveChanges, &Saved);
		}
	}

	// We're done here
	SysFreeString (FileName);
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::BeforeDocumentClose (IDispatch * theDocument)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::DocumentSave (IDispatch * theDocument)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewDocument (IDispatch * theDocument)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	if (! g_bEnableVim)
		// Vim not enabled or empty command line entered
		return S_OK;

	// First get the current file name and line number

	CComQIPtr < ITextDocument, &IID_ITextDocument > pDoc (theDocument);
	if (! pDoc)
		return S_OK;

	BSTR FileName;
	HRESULT hr;

	hr = pDoc->get_FullName (&FileName);
	if (FAILED (hr))
		return S_OK;

	// Open the file in Vim and position to the current line
	if (VimOpenFile (FileName, 0))
	{
		if (! g_bDevStudioEditor)
		{
			// Close the document in developer studio
			CComVariant vSaveChanges = dsSaveChangesPrompt;
			DsSaveStatus Saved;

			pDoc->Close (vSaveChanges, &Saved);
		}
	}

	SysFreeString (FileName);
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowActivate (IDispatch * theWindow)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WindowDeactivate (IDispatch * theWindow)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceOpen ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::WorkspaceClose ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

HRESULT CCommands::XApplicationEvents::NewWorkspace ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}

// Debugger event

HRESULT CCommands::XDebuggerEvents::BreakpointHit (IDispatch * pBreakpoint)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// VisVim dialog

class CMainDialog : public CDialog
{
    public:
	CMainDialog (CWnd * pParent = NULL);	// Standard constructor

	//{{AFX_DATA(CMainDialog)
	enum { IDD = IDD_ADDINMAIN };
	int	m_ChangeDir;
	BOOL	m_bDevStudioEditor;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CMainDialog)
    protected:
	virtual void DoDataExchange (CDataExchange * pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

    protected:
	//{{AFX_MSG(CMainDialog)
	afx_msg void OnEnable();
	afx_msg void OnDisable();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP ()
};

CMainDialog::CMainDialog (CWnd * pParent /* =NULL */ )
	: CDialog (CMainDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDialog)
	m_ChangeDir = -1;
	m_bDevStudioEditor = FALSE;
	//}}AFX_DATA_INIT
}

void CMainDialog::DoDataExchange (CDataExchange * pDX)
{
	CDialog::DoDataExchange (pDX);
	//{{AFX_DATA_MAP(CMainDialog)
	DDX_Radio(pDX, IDC_CD_SOURCE_PATH, m_ChangeDir);
	DDX_Check (pDX, IDC_DEVSTUDIO_EDITOR, m_bDevStudioEditor);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP (CMainDialog, CDialog)
	//{{AFX_MSG_MAP(CMainDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ()


/////////////////////////////////////////////////////////////////////////////
// CCommands methods

STDMETHODIMP CCommands::VisVimDialog ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	// Use m_pApplication to access the Developer Studio Application
	// object,
	// and VERIFY_OK to see error strings in DEBUG builds of your add-in
	// (see stdafx.h)

	VERIFY_OK (m_pApplication->EnableModeless (VARIANT_FALSE));

	CMainDialog Dlg;

	Dlg.m_bDevStudioEditor = g_bDevStudioEditor;
	Dlg.m_ChangeDir = g_ChangeDir;
	if (Dlg.DoModal () == IDOK)
	{
		g_bDevStudioEditor = Dlg.m_bDevStudioEditor;
		g_ChangeDir = Dlg.m_ChangeDir;

		// Save settings to registry HKEY_CURRENT_USER\Software\Vim\VisVim
		HKEY hAppKey = GetAppKey ("Vim");
		if (hAppKey)
		{
			HKEY hSectionKey = GetSectionKey (hAppKey, "VisVim");
			if (hSectionKey)
			{
				WriteRegistryInt (hSectionKey, "DevStudioEditor",
						  g_bDevStudioEditor);
				WriteRegistryInt (hSectionKey, "ChangeDir", g_ChangeDir);
				RegCloseKey (hSectionKey);
			}
			RegCloseKey (hAppKey);
		}
	}

	VERIFY_OK (m_pApplication->EnableModeless (VARIANT_TRUE));
	return S_OK;
}

STDMETHODIMP CCommands::VisVimEnable ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	VimSetEnableState (true);
	return S_OK;
}

STDMETHODIMP CCommands::VisVimDisable ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	VimSetEnableState (false);
	return S_OK;
}

STDMETHODIMP CCommands::VisVimToggle ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	VimSetEnableState (! g_bEnableVim);
	return S_OK;
}

STDMETHODIMP CCommands::VisVimLoad ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	// Use m_pApplication to access the Developer Studio Application object,
	// and VERIFY_OK to see error strings in DEBUG builds of your add-in
	// (see stdafx.h)

	CComBSTR bStr;
	// Define dispatch pointers for document and selection objects
	CComPtr < IDispatch > pDispDoc, pDispSel;

	// Get a document object dispatch pointer
	VERIFY_OK (m_pApplication->get_ActiveDocument (&pDispDoc));
	if (! pDispDoc)
		return S_OK;

	BSTR FileName;
	long LineNr = -1;

	// Get the document object
	CComQIPtr < ITextDocument, &IID_ITextDocument > pDoc (pDispDoc);

	if (! pDoc)
		return S_OK;

	// Get the document name
	if (FAILED (pDoc->get_FullName (&FileName)))
		return S_OK;

	// Get a selection object dispatch pointer
	if (SUCCEEDED (pDoc->get_Selection (&pDispSel)))
	{
		// Get the selection object
		CComQIPtr < ITextSelection, &IID_ITextSelection > pSel (pDispSel);

		if (pSel)
			// Get the selection line number
			pSel->get_CurrentLine (&LineNr);
	}

	// Open the file in Vim
	VimOpenFile (FileName, LineNr);

	SysFreeString (FileName);
	return S_OK;
}


//
// Here we do the actual processing and communication with Vim
//

// Set the enable state and save to registry
//
static void VimSetEnableState (BOOL bEnableState)
{
	g_bEnableVim = bEnableState;
	HKEY hAppKey = GetAppKey ("Vim");
	if (hAppKey)
	{
		HKEY hSectionKey = GetSectionKey (hAppKey, "VisVim");
		if (hSectionKey)
			WriteRegistryInt (hSectionKey, "EnableVim", g_bEnableVim);
		RegCloseKey (hAppKey);
	}
}

// Open the file 'FileName' in Vim and goto line 'LineNr'
// 'FileName' is expected to contain an absolute DOS path including the drive
// letter.
// 'LineNr' must contain a valid line number or 0, e. g. for a new file
//
static BOOL VimOpenFile (BSTR& FileName, long LineNr)
{

	// OLE automation object for com. with Vim
	// When the object goes out of scope, it's desctructor destroys the OLE connection;
	// This is imortant to avoid blocking the object
	// (in this memory corruption would be likely when terminating Vim
	// while still running DevStudio).
	// So keep this object local!
	COleAutomationControl VimOle;

	// :cd D:/Src2/VisVim/
	// 
	// Get a dispatch id for the SendKeys method of Vim;
	// enables connection to Vim if necessary
	DISPID DispatchId;
	DispatchId = VimGetDispatchId (VimOle, "SendKeys");
	if (! DispatchId)
		// OLE error, can't obtain dispatch id
		goto OleError;

	OLECHAR Buf[MAX_OLE_STR];
	char VimCmd[MAX_OLE_STR];
	char* VimCmdStart;
	char* s;

	// Prepend CTRL-\ CTRL-N to exit insert mode
	VimCmd[0] = 0x1c;
	VimCmd[1] = 0x0e;
	VimCmdStart = VimCmd + 2;

	// Update the current file in Vim if it has been modified
	sprintf (VimCmdStart, ":up\n", (char*) FileName);
	if (! VimOle.Method (DispatchId, "s", TO_OLE_STR_BUF (VimCmd, Buf)))
		goto OleError;

	// Change Vim working directory to where the file is if desired
	if (g_ChangeDir != CD_NONE)
		VimChangeDir (VimOle, DispatchId, FileName);

	// Make Vim open the file
	sprintf (VimCmd, ":e %S\n", (char*) FileName);
	// Convert all \ to / 
	for (s = VimCmd; *s; ++s)
		if (*s == '\\')
			*s = '/';
	if (! VimOle.Method (DispatchId, "s", TO_OLE_STR_BUF (VimCmd, Buf)))
		goto OleError;

	if (LineNr > 0)
	{
		// Goto line
		sprintf (VimCmd, ":%d\n", LineNr);
		if (! VimOle.Method (DispatchId, "s", TO_OLE_STR_BUF (VimCmd, Buf)))
			goto OleError;
	}

	// Make Vim come to the foreground
	if (! VimOle.Method ("SetForeground"))
		VimOle.ErrDiag ();

	// We're done
	return true;

    OleError:
	// There was an OLE error
	// Check if it's the "unknown class string" error
	VimErrDiag (VimOle);
	return false;
}

// Return the dispatch id for the Vim method 'Method'
// Create the Vim OLE object if necessary
// Returns a valid dispatch id or null on error
//
static DISPID VimGetDispatchId (COleAutomationControl& VimOle, char* Method)
{
	// Initialize Vim OLE connection if not already done
	if (! VimOle.IsCreated ())
	{
		if (! VimOle.CreateObject ("Vim.Application"))
			return NULL;
	}

	// Get the dispatch id for the SendKeys method.
	// By doing this, we are checking if Vim is still there...
	DISPID DispatchId = VimOle.GetDispatchId ("SendKeys");
	if (! DispatchId)
	{
		// We can't get a dispatch id.
		// This means that probably Vim has been terminated.
		// Don't issue an error message here, instead
		// destroy the OLE object and try to connect once more
		//
		// In fact, this should never happen, because the OLE aut. object
		// should not be kept long enough to allow the user to terminate Vim
		// to avoid memory corruption (why the heck is there no system garbage
		// collection for those damned OLE memory chunks???).
		VimOle.DeleteObject ();
		if (! VimOle.CreateObject ("Vim.Application"))
			// If this create fails, it's time for an error msg
			return NULL;

		if (! (DispatchId = VimOle.GetDispatchId ("SendKeys")))
			// There is something wrong...
			return NULL;
	}

	return DispatchId;
}

// Output an error message for an OLE error
// Check on the classstring error, which probably means Vim wasn't registered.
//
static void VimErrDiag (COleAutomationControl& VimOle)
{
	SCODE sc = GetScode (VimOle.GetResult ());
	if (sc == CO_E_CLASSSTRING)
	{
		char Buf[256];
		sprintf (Buf, "There is no registered OLE automation server named "
			 "\"Vim.Application\".\n"
			 "Use the OLE-enabled version of Vim with VisVim and "
			 "make sure to register Vim by running \"vim -register\".");
		MessageBox (NULL, Buf, "OLE Error", MB_OK);
	}
	else
		VimOle.ErrDiag ();
}

// Change directory to the directory the file 'FileName' is in or it's parent
// directory according to the setting of the global 'g_ChangeDir':
// 'FileName' is expected to contain an absolute DOS path including the drive
// letter.
//	CD_NONE
//	CD_SOURCE_PATH
//	CD_SOURCE_PARENT
//
static void VimChangeDir (COleAutomationControl& VimOle, DISPID DispatchId, BSTR& FileName)
{
	// Do a :cd first

	// Get the path name of the file ("dir/")
	CString StrFileName = FileName;
	char Drive[_MAX_DRIVE];
	char Dir[_MAX_DIR];
	_splitpath (StrFileName, Drive, Dir, NULL, NULL);
	// Convert to unix path name format
	for (char* s = Dir; *s; ++s)
		if (*s == '\\')
			*s = '/';

	// Construct the cd command; append /.. if cd to parent
	// directory and not in root directory
	OLECHAR Buf[MAX_OLE_STR];
	char VimCmd[MAX_OLE_STR];

	sprintf (VimCmd, ":cd %s%s%s\n", Drive, Dir,
		 g_ChangeDir == CD_SOURCE_PARENT && Dir[1] ? ".." : "");
	VimOle.Method (DispatchId, "s", TO_OLE_STR_BUF (VimCmd, Buf));
}

#ifdef _DEBUG
// Print out a debug message
//
static void DebugMsg (char* Msg, char* Arg)
{
	char Buf[400];
	sprintf (Buf, Msg, Arg);
	AfxMessageBox (Buf);
}
#endif
