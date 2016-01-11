// DGPSUniServerBridgeDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "DGPSUniServerBridge.h"
#include "DGPSUniServerBridgeDlg.h"
#include "DGpsServerManager.h"
#include "GpsObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#define ID_TIMER_FLASH_POS 0x1003 //


typedef struct tagREPORT_ITEM
{
	LPCTSTR name;
	int     width;
} REPORT_ITEM, *LPREPORT_ITEM;

const REPORT_ITEM s_reportItems[] = 
{
	{_T("�ն�IP  "),     100},
	{_T("�ն˶˿�"),     100},
	{_T("����ת���˿�"), 100},
	{_T("�����˿� "),     100},
	{_T("״̬"), 110},
	//{_T("�ϴ�����(�ֽ���)"), 110},
	//{_T("������(�ֽ���)"),   110},
	//{_T("״̬"),             50},
} ;












// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDGPSUniServerBridgeDlg �Ի���




CDGPSUniServerBridgeDlg::CDGPSUniServerBridgeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDGPSUniServerBridgeDlg::IDD, pParent)
	, m_RecCstringT1(_T(""))
	, m_RecCstringT2(_T(""))
	, m_ShowSel(0)
	, m_ShowRec(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDGPSUniServerBridgeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TERMINAL_LIST, m_TerminalList);
	DDX_Text(pDX, IDC_REC, m_ShowRec);
}

BEGIN_MESSAGE_MAP(CDGPSUniServerBridgeDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_START, &CDGPSUniServerBridgeDlg::OnBnClickedStart)
	ON_WM_TIMER()
	ON_MESSAGE(WM_GET_TERMINAL_DATA, OnRecTerminalData)
	ON_MESSAGE(WM_GET_TERMINAL_DATA_2, OnRecTerminalData2)
	ON_NOTIFY(NM_CLICK, IDC_TERMINAL_LIST, &CDGPSUniServerBridgeDlg::OnNMClickTerminalList)
END_MESSAGE_MAP()


// CDGPSUniServerBridgeDlg ��Ϣ�������

BOOL CDGPSUniServerBridgeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	CDGpsServerManager::instance()->initialize();


	/*
	DWORD dwStyle = m_TerminalList.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	dwStyle |= LVS_EX_CHECKBOXES;//itemǰ����checkbox�ؼ�
	m_TerminalList.SetExtendedStyle(dwStyle); //������չ���

	m_TerminalList.InsertColumn(0,_T("�K��IP��ַ"),LVCFMT_IMAGE|LVCFMT_LEFT);
	m_TerminalList.InsertColumn(1,_T("ת���˿�"));
	//m_TerminalList.InsertColumn(2,_T("��Ŀ���"));
	for(int j=0;j<3;j++)
	{	m_TerminalList.SetColumnWidth(j ,100);}

	int nRow = m_TerminalList.InsertItem(0, _T("101.1.1.1"));//������
	m_TerminalList.SetItemText(nRow, 1, _T("jacky"));//��������


   */


	m_TerminalList.ModifyStyle(0, LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS);
	m_TerminalList.SetExtendedStyle(m_TerminalList.GetExtendedStyle()|LVS_EX_FLATSB|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	CImageList imgList;
	imgList.Create(1, 14, ILC_COLOR, 0, 0);

	m_TerminalList.SetImageList(&imgList, LVSIL_STATE);

	imgList.Detach();

	for (int i=0; i<sizeof(s_reportItems)/sizeof(REPORT_ITEM); ++i)
	{
		m_TerminalList.InsertColumn(i, s_reportItems[i].name, LVCFMT_LEFT, s_reportItems[i].width);
	}
	m_TerminalList.InsertItem(0, _T("202.120.1.1"));
    m_TerminalList.SetItemText(0, 1, _T("0"));    


	SetTimer(ID_TIMER_FLASH_POS,1000,NULL);

	CDGpsServerManager::instance()->SetGUIHandle(this->m_hWnd);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CDGPSUniServerBridgeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDGPSUniServerBridgeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CDGPSUniServerBridgeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDGPSUniServerBridgeDlg::OnBnClickedStart()
{
	// TODO: Add your control notification handler code here



}





void CDGPSUniServerBridgeDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	   case ID_TIMER_FLASH_POS:
		   ReDrawList();
		  break;
	   default:
	      break;
	}
	CDialog::OnTimer(nIDEvent);
}

void CDGPSUniServerBridgeDlg::ReDrawList(void)
{
    int i;
	m_TerminalList.SetRedraw(FALSE);
	for(i=m_TerminalList.GetItemCount( );i>=-1;i--)
	{
		m_TerminalList.DeleteItem(i);
	} 
	int index = 0;
	MapIDObj_it it = CDGpsServerManager::instance()->m_GpsObjMap.begin();
	while(it!= CDGpsServerManager::instance()->m_GpsObjMap.end())
	{
		GpsObj *terminal = it->second;

		
        char *Ip = inet_ntoa(terminal->m_clntAddr.sin_addr);
		USES_CONVERSION; 
		CString IPaddress=A2T(Ip);  
		CString PortTrans,PortLinked;
        PortTrans.Format(_T("%d"),terminal->m_LinkedPort);
		PortLinked.Format(_T("%d"),terminal->m_clntAddr.sin_port);
		m_TerminalList.InsertItem(index, IPaddress);
		m_TerminalList.SetItemText(index, 1,PortLinked);  
		m_TerminalList.SetItemText(index, 2,PortTrans);  
		m_TerminalList.SetItemText(index, 3,_T("6012"));  
		it++;

	}
	it = CDGpsServerManager::instance()->m_GpsObjMap2.begin();
	while(it!= CDGpsServerManager::instance()->m_GpsObjMap2.end())
	{
		GpsObj *terminal = it->second;


		char *Ip = inet_ntoa(terminal->m_clntAddr.sin_addr);
		USES_CONVERSION; 
		CString IPaddress=A2T(Ip);  
		CString PortTrans,PortLinked;
		PortTrans.Format(_T("%d"),terminal->m_LinkedPort);
		PortLinked.Format(_T("%d"),terminal->m_clntAddr.sin_port);
		m_TerminalList.InsertItem(index, IPaddress);
		m_TerminalList.SetItemText(index, 1,PortLinked);  
		m_TerminalList.SetItemText(index, 2,PortTrans);  
		m_TerminalList.SetItemText(index, 3,_T("6013"));  
		it++;

	}   
	
	
	m_TerminalList.SetRedraw(TRUE);
	m_TerminalList.Invalidate();
	m_TerminalList.UpdateWindow();

}


LRESULT CDGPSUniServerBridgeDlg::OnRecTerminalData(WPARAM wparam, LPARAM lparam)
{
	int len,copylen;
	CString NewRecCString;
    char *newRec = (char *)wparam;
	char buff[1024];
	len = (int)lparam;
    copylen  = len;
	int Skiplen = 4096 - m_RecCstringT1.GetLength();
	if (m_RecCstringT1.GetLength()+len> 4096)
	{
		
        memcpy(buff,&(newRec[Skiplen]),len-Skiplen);
		buff[len-Skiplen] = 0;
        m_RecCstringT1 = _T("");
		USES_CONVERSION; 
		NewRecCString=A2T(buff);
	}
    else
	{
		memcpy(buff,newRec,len);
		buff[len] = 0;
		USES_CONVERSION; 
		NewRecCString=A2T(buff);
	}
	
    m_RecCstringT1+=NewRecCString;
    if (m_ShowSel==0)
    {
		m_ShowRec = m_RecCstringT1;
		UpdateData(FALSE);
    }
    
	return 0;

}

LRESULT CDGPSUniServerBridgeDlg::OnRecTerminalData2(WPARAM wparam, LPARAM lparam)
{
	int len,copylen;
	CString NewRecCString;
	char *newRec = (char *)wparam;
	char buff[1024];
	len = (int)lparam;
	copylen  = len;
	int Skiplen = 4096 - m_RecCstringT2.GetLength();
	if (m_RecCstringT2.GetLength()+len> 4096)
	{

		memcpy(buff,&(newRec[Skiplen]),len-Skiplen);
		buff[len-Skiplen] = 0;
		m_RecCstringT2 = _T("");
		USES_CONVERSION; 
		NewRecCString=A2T(buff);
	}
	else
	{
		memcpy(buff,newRec,len);
		buff[len] = 0;
		USES_CONVERSION; 
		NewRecCString=A2T(buff);
	}

	m_RecCstringT2+=NewRecCString;

	if (m_ShowSel==1)
	{
		m_ShowRec = m_RecCstringT2;
		UpdateData(FALSE);
	}
	return 0;
}
void CDGPSUniServerBridgeDlg::OnNMClickTerminalList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	/*
	//��ȡ�������ڵ��к�   
	//�ҳ����λ��   
	DWORD dwPos = GetMessagePos();  
	CPoint point( LOWORD(dwPos), HIWORD(dwPos) );  
	m_TerminalList.ScreenToClient(&point);  
	//����ṹ��   
	LVHITTESTINFO lvinfo;  
	lvinfo.pt = point;  
	//��ȡ�к���Ϣ   
	int nItem = m_TerminalList.HitTest(&lvinfo);  
	if(nItem != -1)  
		m_itemSel = lvinfo.iItem;   //��ǰ�к�   

	//�ж��Ƿ�����CheckBox��   
	//if(lvinfo.flags == LVHT_ONITEMSTATEICON)  
    //		m_bHit = TRUE;  
    */
	*pResult = 0;  
	*pResult = 0;
}
