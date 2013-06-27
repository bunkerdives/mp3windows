
// PlayMp3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlayMp3.h"
#include "PlayMp3Dlg.h"
#include "afxdialogex.h"



// The following include block is for I/O redirect capability
#include <iostream>
#include <fstream>
#include <conio.h>
#include <stdio.h>
#ifndef _USE_OLD_OSTREAMS
using namespace std;
#endif
#include "guicon.h"
#include <crtdbg.h>




#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Define SLIDER constants
#define SLIDER_TIME_ELAPSE_MAX_POS 100000
#define SLIDER_TIME_INIT 0
#define SLIDER_TIME_PLAY_INIT 0







// Matlab Mex Functionality

//#include <matrix.h>
//#include "mex.h"   

/* Definitions to keep compatibility with earlier versions of ML */
#ifndef MWSIZE_MAX
typedef int mwSize;
typedef int mwIndex;
typedef int mwSignedIndex;

#if (defined(_LP64) || defined(_WIN64)) && !defined(MX_COMPAT_32)
/* Currently 2^48 based on hardware limitations */
# define MWSIZE_MAX    281474976710655UL
# define MWINDEX_MAX   281474976710655UL
# define MWSINDEX_MAX  281474976710655L
# define MWSINDEX_MIN -281474976710655L
#else
# define MWSIZE_MAX    2147483647UL
# define MWINDEX_MAX   2147483647UL
# define MWSINDEX_MAX  2147483647L
# define MWSINDEX_MIN -2147483647L
#endif
#define MWSIZE_MIN    0UL
#define MWINDEX_MIN   0UL
#endif







// CPlayMp3Dlg dialog


CPlayMp3Dlg::CPlayMp3Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPlayMp3Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPlayMp3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_MP3_FILE, m_edtMp3File);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_BTN_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_sliderVolume);
	DDX_Control(pDX, IDC_STATIC_DURATION, m_staticDuration);
	DDX_Control(pDX, IDC_SLIDER_TIME_ELAPSED, m_sliderTimeElapsed);
}

BEGIN_MESSAGE_MAP(CPlayMp3Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CPlayMp3Dlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CPlayMp3Dlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_STOP, &CPlayMp3Dlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CPlayMp3Dlg::OnBnClickedBtnBrowse)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CPlayMp3Dlg message handlers

BOOL CPlayMp3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_playerDll.LoadDLL(L"LibMP3DLL.dll");
	playing = false;
	pause = false;

	m_sliderVolume.SetRangeMin(0);
	m_sliderVolume.SetRangeMax(10000);
	m_sliderVolume.SetPos(7000);

	m_sliderTimeElapsed.SetRangeMin(0);
	m_sliderTimeElapsed.SetRangeMax(SLIDER_TIME_ELAPSE_MAX_POS);
	m_sliderTimeElapsed.SetPos(SLIDER_TIME_INIT);

	m_btnPlay.EnableWindow(TRUE);
	m_btnPause.EnableWindow(FALSE);
	m_btnStop.EnableWindow(FALSE);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlayMp3Dlg::OnPaint()
{	


	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlayMp3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlayMp3Dlg::OnDestroy()
{
	CDialogEx::OnDestroy();


	if(playing)
	{
		m_playerDll.Stop();
	}

	m_playerDll.Cleanup();
	m_playerDll.UnloadDLL();
}


void CPlayMp3Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 0)
	{
		if(playing)
		{
			long EvCode = 0;
			bool b = m_playerDll.WaitForCompletion(0, &EvCode);

			if(b)
			{
				playing = false;
				KillTimer(0);
				TRACE(L"Playing stopped");

				m_btnPlay.EnableWindow(TRUE);
				m_btnPause.EnableWindow(FALSE);
				m_btnStop.EnableWindow(FALSE);
			}

			if(pause==false)
			{
				CString sDuration;
				int duration = m_playerDll.GetDuration()/10000000;
				__int64 curPos = m_playerDll.GetCurrentPosition();

				int timeElapsedSec = curPos/10000000;
				sDuration.Format(L"%d:%02d/%d:%02d", timeElapsedSec/60,  timeElapsedSec%60, duration/60, duration%60);
				m_staticDuration.SetWindowTextW(sDuration);
				__int64 pos = (curPos * SLIDER_TIME_ELAPSE_MAX_POS)/m_playerDll.GetDuration();
				m_sliderTimeElapsed.SetPos((int)(pos));
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CPlayMp3Dlg::OnBnClickedBtnPlay()
{
	__int64 duration;
	__int64 curPos;
	__int64 sliderPos;


	/* __NOTE:	Need to see what's going on! */
	/*			Redirect IO to console (guicon.h & guicon.cpp) for debugging
	*/
	static bool toggle=true;
	if(toggle){!toggle;RedirectIOToConsole2();}


	if(pause)
	{
		m_playerDll.Play();
		pause = false;

		m_btnPlay.EnableWindow(FALSE);
		m_btnPause.EnableWindow(TRUE);
		m_btnStop.EnableWindow(TRUE);


		/* __NOTE:	Output current volume value; Demonstrate manual volume set */
		std::cout << "Volume = " << m_playerDll.GetVolume() << std::endl;
		m_playerDll.SetVolume(m_playerDll.GetVolume()); /*manually set the volume -10,000<=vol<=0 */



	}
	else
	{
		CString file;

		/*	Demonstrate user file selection (browse), and manual file selection
			(modify) via hardcode
		*/


		/* __NOTE :	output filename chosen by user 
					Populate filename w/user selection */
		m_edtMp3File.GetWindowTextW(file);
		std::wcout << file.GetString() << endl;

			/* std::wcout << file.GetBuffer(file.GetLength()) << endl;
			file.ReleaseBuffer();*/

		/* Manually hardcode filename via _T string literal, print & play */
		file = _T("C:\\Users\\310126802\\Music\\16 - Come To Dust.mp3");
		std::wcout << file.GetString() << endl;

		/* Get the file position and duration in the MP3 */
		// __int64 duration = m_playerDll.GetDuration();
		// __int64 pos =  m_sliderTimeElapsed.GetPos()* duration/SLIDER_TIME_ELAPSE_MAX_POS;



		





		// curPos = 1942373669;
		// duration = 2487910000;
		// m_playerDll.SetPositions(&curPos, &duration, true);


		//m_sliderTimeElapsed.SetPos(0)

		/* m_playerDll.SetPositions(&pos, &duration, true);*/
		

		// std::cout << "Pos = " << pos << std::endl;
		// std::cout << "Duration = " << duration << std::endl;
		// pos = 1942373669;
		// m_playerDll.SetPositions(&pos, &duration, true);

		// __int64 curPos = m_playerDll.GetCurrentPosition();
		// __int64 sliderPos = (curPos * SLIDER_TIME_ELAPSE_MAX_POS)/m_playerDll.GetDuration();
		// m_sliderTimeElapsed.SetPos((int)(sliderPos));

		if(file==L"")
		{
			MessageBox(L"No MP3 file is specified!", L"Error");
			return;
		}

		if(FileExists(file)==false)
		{
			MessageBox(L"MP3 file does not exists!", L"Error");
			return;
		}

		if(m_playerDll.Load(file)&&m_playerDll.Play())
		{


			playing = true;
			pause = false;
			this->SetTimer(0, 500, NULL);

			m_playerDll.SetVolume(GetVolume());

			m_sliderTimeElapsed.SetPos( SLIDER_TIME_PLAY_INIT );

			m_btnPlay.EnableWindow(FALSE);
			m_btnPause.EnableWindow(TRUE);
			m_btnStop.EnableWindow(TRUE);


			
			curPos = 1942373669;
			duration = m_playerDll.GetDuration();
			m_playerDll.SetPositions(&curPos, &duration, true);

			curPos = m_playerDll.GetCurrentPosition();
			sliderPos = (curPos * SLIDER_TIME_ELAPSE_MAX_POS)/duration;
			std::cout << "=PLAY= Actual Pos: " << curPos << std::endl;
			std::cout << "=PLAY= Slider Pos: " << sliderPos << std::endl;
			std::cout << "=PLAY= Volume    : " << m_playerDll.GetVolume() << std::endl;
			std::cout << "=PLAY= Duration  : " << duration << std::endl << std::endl;


		}
		else
		{
			MessageBox(L"Cannot play this file!", L"Error");
			return;
		}
	}

}


void CPlayMp3Dlg::OnBnClickedBtnPause()
{	__int64 duration;
	__int64 curPos;
	__int64 sliderPos;

	if(m_playerDll.Pause())
	{
		pause = true;

		m_btnPlay.EnableWindow(TRUE);
		m_btnPause.EnableWindow(FALSE);
		m_btnStop.EnableWindow(FALSE);


		
		
		/* __NOTE: Get current dur, pos, slider, and print */
		duration = m_playerDll.GetDuration();
		curPos = m_playerDll.GetCurrentPosition();
		sliderPos = (curPos * SLIDER_TIME_ELAPSE_MAX_POS)/duration;

		std::cout << "=PAUSE= Actual Pos: " << curPos << std::endl;
		std::cout << "=PAUSE= Slider Pos: " << sliderPos << std::endl;
		std::cout << "=PLAY= Volume     : " << m_playerDll.GetVolume() << std::endl;
		std::cout << "=PAUSE= Duration  : " << duration << std::endl << std::endl;


	}
}


void CPlayMp3Dlg::OnBnClickedBtnStop()
{
	__int64 duration;
	__int64 curPos;
	__int64 sliderPos;

	if(m_playerDll.Stop())
	{
		playing = false;
		pause = false;
		this->KillTimer(0);

		m_btnPlay.EnableWindow(TRUE);
		m_btnPause.EnableWindow(FALSE);
		m_btnStop.EnableWindow(FALSE);


		/* __NOTE: Get current dur, pos, slider, and print */
		duration = m_playerDll.GetDuration();
		curPos = m_playerDll.GetCurrentPosition();
		sliderPos = (curPos * SLIDER_TIME_ELAPSE_MAX_POS)/duration;

		std::cout << "=STOP= Actual Pos: " << curPos << std::endl;
		std::cout << "=STOP= Slider Pos: " << sliderPos << std::endl;
		std::cout << "=PLAY= Volume    : " << m_playerDll.GetVolume() << std::endl;
		std::cout << "=STOP= Duration  : " << duration << std::endl << std::endl;
	}
}


void CPlayMp3Dlg::OnBnClickedBtnBrowse()
{
	CFileDialog fOpenDlg(
		TRUE, _T("mp3"), _T("*.mp3"), OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, 
		_T("MP3 files (*.mp3)|*.mp3|All files (*.*)|*.*||"), this);

	fOpenDlg.m_pOFN->lpstrTitle=_T("MP3 file");

	CString szMkvPath;
	if(fOpenDlg.DoModal()==IDOK)
	{
		szMkvPath = fOpenDlg.GetPathName();
		m_edtMp3File.SetWindowText(szMkvPath);
	}
}

/* Return TRUE if file 'fileName' exists */
bool CPlayMp3Dlg::FileExists(const TCHAR *fileName)
{
	DWORD       fileAttr;

	fileAttr = GetFileAttributes(fileName);
	if (0xFFFFFFFF == fileAttr)
		return false;
	return true;
}

int CPlayMp3Dlg::GetVolume()
{
	int vol = m_sliderVolume.GetPos();
	vol = 10000 - vol;
	vol = -vol;

	return vol;
}

void CPlayMp3Dlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	static int toggle = 0;
	CScrollBar* VolScrollBar = (CScrollBar*)(&m_sliderVolume);
	CScrollBar* PosScrollBar = (CScrollBar*)(&m_sliderTimeElapsed);
	if(pScrollBar==VolScrollBar)
		m_playerDll.SetVolume(GetVolume());
	else if(pScrollBar==PosScrollBar)
	{
		if(playing && pause==false)
		{
			__int64 duration = m_playerDll.GetDuration();
			__int64 pos =  m_sliderTimeElapsed.GetPos()* duration/SLIDER_TIME_ELAPSE_MAX_POS;
			m_playerDll.SetPositions(&pos, &duration, true);


			// /* __NOTE:	Output current mp3 file position; Demonstrate manual position set
			// */
			// std::cout << "Pos = " << pos << std::endl;
			// pos = 1942373669;
			// m_playerDll.SetPositions(&pos, &duration, true);

		}
		else
		{
			__int64 duration = m_playerDll.GetDuration();
			__int64 pos =  m_sliderTimeElapsed.GetPos()* duration/SLIDER_TIME_ELAPSE_MAX_POS;
			__int64 posStop =  pos;
			m_playerDll.SetPositions(&pos, &posStop, true);

			if(pause)
			{
				m_playerDll.SetPositions(&pos, &duration, true);
				m_playerDll.Pause();
			}
			else
			{
				m_playerDll.SetPositions(&pos, &posStop, true);
			}
		}
		CString sDuration;
		int duration = m_playerDll.GetDuration()/10000000;
		__int64 curPos = m_playerDll.GetCurrentPosition();

		int timeElapsedSec = curPos/10000000;
		sDuration.Format(L"%d:%02d/%d:%02d", timeElapsedSec/60,  timeElapsedSec%60, duration/60, duration%60);
		m_staticDuration.SetWindowTextW(sDuration);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
