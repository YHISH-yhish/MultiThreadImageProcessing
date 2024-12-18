// MultiThreadImageProcessingDlg.h

#pragma once

#include "afxwin.h"
#include <opencv2/opencv.hpp>
#include "MultiThreadImageProcessor.h"

struct ImagedDta {
	CString path;
	int type;
	float para;
	int progress;//0-待处理；1-	处理中；2 - 处理完成；
};
// CMultiThreadImageProcessingDlg 对话框
class CMultiThreadImageProcessingDlg : public CDialogEx
{
	// 构造
public:
	CMultiThreadImageProcessingDlg(CWnd* pParent = nullptr);	// 标准构造函数

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MULTITHREADIMAGEPROCESSING_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	// 实现
protected:
	HICON m_hIcon;
	CImage m_image;                    // 当前显示的图像
	CStatic m_imageCtrl;               // 显示图像的控件
	CComboBox m_processingTypeCombo;   // 处理类型下拉框
	CButton m_addButton;               // 添加图像按钮
	CButton m_deleteButton;            // 删除图像按钮
	CButton m_viewResultButton;        // 查看结果按钮
	CButton m_startButton;             // 开始处理按钮
	CButton m_cancelButton;            // 取消处理按钮
	CButton m_saveButton;            // 保存按钮
	CListBox m_processListBox;   // 文件列表框
	CEdit m_inputParamEdit;          // 输入参数编辑框
	CProgressCtrl m_progressBar;         // 进度条
	BOOL m_isProcessing;               // 是否正在处理图像
	MultiThreadImageProcessor* m_pProcessor;  // 图像处理器
	int m_selectedProcessingType;      // 当前选择的处理类型
	ImagedDta m_imageinfo[200];
	int m_filenum;
	int m_CompleteFile;

	// 生成的消息映射函数
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedAddImage();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedDeleteImage();
	afx_msg void OnBnClickedStartProcessing();
	afx_msg void OnBnClickedCancelProcessing();
	afx_msg void OnBnClickedViewResult();
	afx_msg void OnSelectProcessingType();
	afx_msg void OnNMCustomdrawProgress1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserError(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeListProcess();
	int m_Process_Type;
	afx_msg void OnDestroy();
	void UpdateListBox();

	afx_msg void OnEnChangeEdit1();
};
