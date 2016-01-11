// DGPSUniServerBridgeDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"


#define WM_GET_TERMINAL_DATA               (WM_USER + 5)  // 
#define WM_GET_TERMINAL_DATA_2             (WM_USER + 6)  // 

// CDGPSUniServerBridgeDlg �Ի���
class CDGPSUniServerBridgeDlg : public CDialog
{
// ����
public:
	CDGPSUniServerBridgeDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DGPSUNISERVERBRIDGE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	afx_msg LRESULT OnRecTerminalData(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnRecTerminalData2(WPARAM wparam, LPARAM lparam);
	CListCtrl m_TerminalList;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void ReDrawList(void);
	CString m_RecCstringT1;
	CString m_RecCstringT2;
	int m_ShowSel;
	CString m_ShowRec;
	afx_msg void OnNMClickTerminalList(NMHDR *pNMHDR, LRESULT *pResult);
};
