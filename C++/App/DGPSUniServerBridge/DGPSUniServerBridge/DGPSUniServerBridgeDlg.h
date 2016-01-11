// DGPSUniServerBridgeDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"


#define WM_GET_TERMINAL_DATA               (WM_USER + 5)  // 
#define WM_GET_TERMINAL_DATA_2             (WM_USER + 6)  // 

// CDGPSUniServerBridgeDlg 对话框
class CDGPSUniServerBridgeDlg : public CDialog
{
// 构造
public:
	CDGPSUniServerBridgeDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DGPSUNISERVERBRIDGE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
