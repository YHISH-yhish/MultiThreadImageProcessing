#include "pch.h"
#include "framework.h"
#include "MultiThreadImageProcessing.h"
#include "MultiThreadImageProcessingDlg.h"
#include "afxdialogex.h"
#include "MultiThreadImageProcessor.h"
#include <opencv2/opencv.hpp>
#include <atlstr.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CMultiThreadImageProcessingDlg 对话框
CMultiThreadImageProcessingDlg::CMultiThreadImageProcessingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MULTITHREADIMAGEPROCESSING_DIALOG, pParent),
	m_isProcessing(false), m_pProcessor(nullptr)
	, m_Process_Type(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiThreadImageProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMAGE, m_imageCtrl);              // 图像显示控件
	DDX_Control(pDX, IDC_PROCESSING_TYPE, m_processingTypeCombo);    // 处理类型下拉框
	DDX_Control(pDX, ID_ADD_IMAGE, m_addButton);            // 添加图像按钮
	DDX_Control(pDX, ID_DELETE_IMAGE, m_deleteButton);      // 删除图像按钮
	DDX_Control(pDX, ID_VIEW_RESULT, m_viewResultButton);   // 查看结果按钮
	DDX_Control(pDX, ID_START_PROCESSING, m_startButton);   // 开始按钮
	DDX_Control(pDX, ID_CANCEL_PROCESSING, m_cancelButton);  // 取消按钮
	DDX_Control(pDX, IDC_SAVE, m_saveButton);               // 保存按钮
	DDX_Control(pDX, IDC_LIST_PROCESS, m_processListBox);   // 文件列表框
	DDX_Control(pDX, IDC_EDIT1, m_inputParamEdit);          // 输入参数编辑框
	DDX_Control(pDX, IDC_PROGRESS1, m_progressBar);         // 进度条
	DDX_CBIndex(pDX, IDC_PROCESSING_TYPE, m_Process_Type);
	DDV_MinMaxInt(pDX, m_Process_Type, 0, INT_MAX);
}

BEGIN_MESSAGE_MAP(CMultiThreadImageProcessingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_ADD_IMAGE, &CMultiThreadImageProcessingDlg::OnBnClickedAddImage)
	ON_BN_CLICKED(ID_DELETE_IMAGE, &CMultiThreadImageProcessingDlg::OnBnClickedDeleteImage)
	ON_BN_CLICKED(ID_VIEW_RESULT, &CMultiThreadImageProcessingDlg::OnBnClickedViewResult)
	ON_BN_CLICKED(ID_START_PROCESSING, &CMultiThreadImageProcessingDlg::OnBnClickedStartProcessing)
	ON_BN_CLICKED(ID_CANCEL_PROCESSING, &CMultiThreadImageProcessingDlg::OnBnClickedCancelProcessing)
	ON_BN_CLICKED(IDC_SAVE, &CMultiThreadImageProcessingDlg::OnBnClickedSave)
	ON_CBN_SELCHANGE(IDC_PROCESSING_TYPE, &CMultiThreadImageProcessingDlg::OnSelectProcessingType)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS1, &CMultiThreadImageProcessingDlg::OnNMCustomdrawProgress1)
	ON_LBN_SELCHANGE(IDC_LIST_PROCESS, &CMultiThreadImageProcessingDlg::OnLbnSelchangeListProcess)
	ON_MESSAGE(WM_PROCESS_UPDATE, &CMultiThreadImageProcessingDlg::OnUpdateProgress)
	ON_MESSAGE(WM_USER_ERROR, &CMultiThreadImageProcessingDlg::OnUserError)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_EDIT1, &CMultiThreadImageProcessingDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()

// CMultiThreadImageProcessingDlg 消息处理程序

BOOL CMultiThreadImageProcessingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置对话框的图标
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 初始化控件
	//m_processingTypeCombo.AddString(_T("（请选择）"));
	m_processingTypeCombo.AddString(_T("灰度处理"));
	m_processingTypeCombo.AddString(_T("放大"));
	m_processingTypeCombo.AddString(_T("缩小"));
	m_processingTypeCombo.AddString(_T("顺时针旋转"));
	m_processingTypeCombo.AddString(_T("逆时针旋转"));
	m_processingTypeCombo.SetCurSel(0);  // 默认选择第一个

	// 强制刷新组合框显示
	m_processingTypeCombo.Invalidate();  // 刷新控件显示
	m_processingTypeCombo.UpdateWindow(); // 确保控件状态更新
	m_progressBar.SetRange(0, 100);  // 设置进度条范围

	m_filenum = 0; m_CompleteFile = 0;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMultiThreadImageProcessingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CMultiThreadImageProcessingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CMultiThreadImageProcessingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMultiThreadImageProcessingDlg::OnBnClickedAddImage()
{
	// 创建一个文件对话框对象，参数表示这是一个打开文件的对话框
	CFileDialog fileDlg(TRUE, _T("png"), NULL, 0, _T("Image Files(*.bmp; *.jpg;*.png)|*.BMP;*.JPG;*.PNG|All Files (*.*)|*.*||"), this);

	// 如果用户在对话框中选择了一个文件并点击了OK按钮
	if (fileDlg.DoModal() == IDOK)
	{
		// 获取用户选择的文件路径
		CString filePath = fileDlg.GetPathName();

		// 创建并加载图像
		CImage myImage;  // 创建一个新的 CImage 对象
		if (myImage.Load(filePath) == S_OK) // 如果加载成功
		{
			m_image = myImage;
			m_imageinfo[m_filenum].path = filePath;
			m_imageinfo[m_filenum].progress = 0;
			m_imageinfo[m_filenum].type = m_selectedProcessingType;
			m_imageinfo[m_filenum].para = 1.0;
			m_filenum++;
			UpdateListBox();
			
			//m_processListBox.AddString(filePath+"[待处理]");  // 将文件路径添加到列表框
			HBITMAP hBitmap = (HBITMAP)myImage.Detach();
			//CBitmap bitmap;
			//bitmap.LoadBitmap(IDB_YOUR_BITMAP);
			m_imageCtrl.SetBitmap(hBitmap);  // 在图像控件中显示图像
			Invalidate();  // 刷新窗口
			UpdateWindow();  // 更新窗口
			
			
		}
		else // 如果加载图像失败，显示一个消息框通知用户
		{
			MessageBox(_T("图片加载失败"), _T("提示"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		}
	}
}


void CMultiThreadImageProcessingDlg::OnBnClickedDeleteImage()
{
	// 获取当前选中的列表框项索引
	int selIndex = m_processListBox.GetCurSel();

	// 判断是否有选中的项
	if (selIndex != LB_ERR)
	{
		// 删除列表框中的选中项
		m_processListBox.DeleteString(selIndex);
		for (int i = selIndex; i < m_filenum; i++)
		{
			m_imageinfo[i] = m_imageinfo[i + 1];
		}
		m_filenum--;
		// 清空图像控件中的图像
		m_imageCtrl.SetBitmap(NULL);

		Invalidate();  // 刷新窗口
		UpdateWindow();  // 更新窗口

		// 提示用户图像已删除
		AfxMessageBox(_T("图像已删除"));
	}
	else
	{
		// 如果没有选中的列表项，提示用户
		AfxMessageBox(_T("请先选择一个列表项"));
	}
}


void CMultiThreadImageProcessingDlg::OnBnClickedStartProcessing()
{
	//if (m_isProcessing) {
	//	AfxMessageBox(_T("图像正在处理，请稍候..."));
	//	return;
	//}

	//if (m_imageCtrl.GetBitmap() == NULL) {
	//	AfxMessageBox(_T("没有加载图像！"));
	//	return;
	//}

	//// 获取下拉框的选项
	//m_selectedProcessingType = m_processingTypeCombo.GetCurSel();

	//// 创建图像处理对象并启动多线程处理
	//CString selfile;
	//m_processListBox.GetText(m_processListBox.GetCurSel(), selfile);
	for (int i = 0; i < m_filenum; i++)
	{
		CString selfile = m_imageinfo[i].path;
		// 创建图像处理对象并启动多线程处理
		m_pProcessor = new MultiThreadImageProcessor((LPCTSTR)selfile, m_imageinfo[i].type,i, m_imageinfo[i].para);
		m_pProcessor->SetWndHandle(this->GetSafeHwnd());
		std::thread processingThread(&MultiThreadImageProcessor::ProcessImage, m_pProcessor);
		processingThread.detach();
	}
	/*m_pProcessor = new MultiThreadImageProcessor((LPCTSTR)selfile, m_selectedProcessingType);
	m_pProcessor->SetWndHandle(this->GetSafeHwnd());
	std::thread processingThread(&MultiThreadImageProcessor::ProcessImage, m_pProcessor);
	processingThread.detach();*/
	m_progressBar.SetRange(0, m_filenum);  // 设置进度条范围
	m_progressBar.SetPos(0);  // 进度条清零

	// 更新状态为处理进行中
	//m_isProcessing = true;
    
}


void CMultiThreadImageProcessingDlg::OnBnClickedCancelProcessing()
{
	int selIndex = m_processListBox.GetCurSel();  // 获取当前选中的项的索引
	m_imageinfo[selIndex].progress = 0;
	m_CompleteFile -= 1;
	::PostMessage(m_hWnd, WM_PROCESS_UPDATE, selIndex, 0);
	UpdateListBox();
	
	/*if (m_isProcessing) {
		m_pProcessor->CancelProcessing();
		m_isProcessing = false;
		AfxMessageBox(_T("图像处理已取消。"));
	}
	else {
		AfxMessageBox(_T("没有正在处理的图像。"));
	}*/
}

LRESULT CMultiThreadImageProcessingDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam)
{
	// 更新UI，显示处理进度
	int w = (int)wParam, l = (int)lParam;
	if (l == 2) { m_CompleteFile++; }
	m_progressBar.SetPos(m_CompleteFile);
	m_imageinfo[w].progress = l;
	UpdateListBox();
	CString status((LPCSTR)lParam);
	status.Format("%d%%完成", m_CompleteFile*100/m_filenum);
	SetDlgItemText(IDC_STATUS, status);  // 更新状态文本
	return 0;
}

LRESULT CMultiThreadImageProcessingDlg::OnUserError(WPARAM wParam, LPARAM lParam)
{
	// 更新UI，显示处理进度
	CString status((LPCSTR)wParam);
	SetDlgItemText(IDC_STATUS, status);  // 更新状态文本
	return 0;
}

void CMultiThreadImageProcessingDlg::OnSelectProcessingType()
{
	m_selectedProcessingType = m_processingTypeCombo.GetCurSel();
	int selIndex = m_processListBox.GetCurSel();  // 获取当前选中的项的索引
	if (selIndex != -1)
	{
		m_imageinfo[selIndex].type = m_selectedProcessingType;
		CString sss;
		sss.Format("%3.1f", m_imageinfo[selIndex].para);
		m_inputParamEdit.SetWindowTextA(sss); //m_inputParamEdit.UpdateData(false);
		m_processingTypeCombo.SetCurSel(m_selectedProcessingType);
	}
	
	UpdateListBox();
}

void CMultiThreadImageProcessingDlg::OnBnClickedViewResult()
{
	int selIndex = m_processListBox.GetCurSel();  // 获取当前选中的项的索引
	CString filePath;
	m_processListBox.GetText(selIndex, filePath);  // 获取选中项的文本内容（文件路径）
	int nPos = filePath.ReverseFind('\.'); // 文件路径，以'\'斜杠分隔的路径
	CString csFileFullName;
	csFileFullName = filePath.Left(nPos ); // 获取文件全名，包括文件名和扩展名
	// 去掉文件名部分，只保留目录路径	

	// 拼接新的文件名
	CString processedImagePath = csFileFullName + _T("_out.jpg");
	CImage myImage;  // 创建一个新的 CImage 对象
	if (myImage.Load(processedImagePath) == S_OK) // 如果加载成功
	{
		m_image = myImage;
		//m_processListBox.AddString(processedImagePath);  // 将文件路径添加到列表框
		HBITMAP hBitmap = (HBITMAP)myImage.Detach();
		//CBitmap bitmap;
		//bitmap.LoadBitmap(IDB_YOUR_BITMAP);
		m_imageCtrl.SetBitmap(hBitmap);  // 在图像控件中显示图像
		Invalidate();  // 刷新窗口
		UpdateWindow();  // 更新窗口
	}
	else // 如果加载图像失败，显示一个消息框通知用户
	{
		MessageBox(_T("图片加载失败"), _T("提示"), MB_OK | MB_ICONERROR | MB_TOPMOST);
	}
	//m_imageCtrl.SetBitmap((HBITMAP)m_processedimage);
}

void CMultiThreadImageProcessingDlg::OnNMCustomdrawProgress1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

    switch (pNMCD->dwDrawStage)
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;

    case CDDS_ITEMPREPAINT:
    {
        // 获取进度条的当前进度
        int pos = m_progressBar.GetPos();
        int min, max;
        m_progressBar.GetRange(min, max);

        // 计算进度百分比
        int percent = (pos - min) * 100 / (max - min);

        // 设置进度条颜色
        COLORREF clrText;
        if (percent < 50)
        {
            clrText = RGB(255, 0, 0); // 红色
        }
        else if (percent < 80)
        {
            clrText = RGB(255, 255, 0); // 黄色
        }
        else
        {
            clrText = RGB(0, 255, 0); // 绿色
        }

        // 设置文本颜色
        CDC* pDC = CDC::FromHandle(pNMCD->hdc);
        pDC->SetTextColor(clrText);

        *pResult = CDRF_DODEFAULT;
        break;
    }

    default:
        *pResult = CDRF_DODEFAULT;
        break;
    }
}

void CMultiThreadImageProcessingDlg::OnBnClickedSave()
{
	// 获取当前选中的项的索引
	int selIndex = m_processListBox.GetCurSel();
	if (selIndex != LB_ERR) {
		// 创建文件对话框对象
		CFileDialog saveFileDlg(FALSE, _T("jpg"), NULL, OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, _T("Image Files (*.jpg;*.png;*.bmp)|*.jpg;*.png;*.bmp|All Files (*.*)|*.*"), this);

		// 显示对话框并检查返回值
		if (saveFileDlg.DoModal() == IDOK) {
			CString savePath = saveFileDlg.GetPathName();

			// 确保路径不为空
			if (savePath.IsEmpty()) {
				AfxMessageBox(_T("保存路径无效"));
				return;
			}

			// 如果 m_image 是 CImage 对象
			
			HRESULT hr = m_image.Save(savePath);
			if (FAILED(hr)) {
				AfxMessageBox(_T("保存图像失败！"));
			}
			else {
				AfxMessageBox(_T("图像已保存！"));
			}

			// 如果是 OpenCV的cv::Mat，则应使用 cv::imwrite
			// 例：
			// std::string outputPath = CW2A(savePath.GetString());
			// if (!cv::imwrite(outputPath, image)) {
			//     AfxMessageBox(_T("保存图像失败！"));
			// } else {
			//     AfxMessageBox(_T("图像已保存！"));
			// }
		}
	}
	else {
		AfxMessageBox(_T("没有图像可以保存"));
	}
}




void CMultiThreadImageProcessingDlg::OnLbnSelchangeListProcess()
{
	int selIndex = m_processListBox.GetCurSel();  // 获取当前选中的项的索引

	if (selIndex != LB_ERR)  // 确保有项被选中
	{
		CString filePath=m_imageinfo[selIndex].path;
		m_processingTypeCombo.SetCurSel(m_imageinfo[selIndex].type);
		CString sss;
		sss.Format("%3.1f",m_imageinfo[selIndex].para);
		m_inputParamEdit.SetWindowTextA(sss); //m_inputParamEdit.UpdateData(false);
		m_processingTypeCombo.SetCurSel(m_imageinfo[selIndex].type);

		//m_processListBox.GetText(selIndex, filePath);  // 获取选中项的文本内容（文件路径）
		// 现在 filePath 变量包含选中的文件路径
		// 创建并加载图像
		CImage myImage;  // 创建一个新的 CImage 对象
		if (myImage.Load(filePath) == S_OK) // 如果加载成功
		{
			m_image = myImage;
			HBITMAP hBitmap = (HBITMAP)myImage.Detach();
			//CBitmap bitmap;
			//bitmap.LoadBitmap(IDB_YOUR_BITMAP);
			m_imageCtrl.SetBitmap(hBitmap);  // 在图像控件中显示图像
			Invalidate();  // 刷新窗口
			UpdateWindow();  // 更新窗口
		}
		else // 如果加载图像失败，显示一个消息框通知用户
		{
			MessageBox(_T("图片加载失败"), _T("提示"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		}
	}
	else
	{
		MessageBox(_T("没有选中任何项"), _T("提示"), MB_OK | MB_ICONWARNING);
	}

	
}


void CMultiThreadImageProcessingDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	

}


void CMultiThreadImageProcessingDlg::UpdateListBox()
{
	int selIndex = m_processListBox.GetCurSel();  // 获取当前选中的项的索引
	m_processListBox.ResetContent();
	for (int i = 0; i< m_filenum;i++)
	{
		CString str1,str2;
		switch (m_imageinfo[i].progress)
		{
		case 0: str1 = "待处理"; break;
		case 1: str1 ="处理中"; break;
		case 2: str1 = "处理完成"; break;
		}
		switch (m_imageinfo[i].type	)
		{
		case 0: str2 = "灰度"; break;
		case 1: str2 = "放大"; break;
		case 2: str2 = "缩小"; break;
		case 3: str2 = "顺时针旋转"; break;
		case 4: str2 = "逆时针旋转"; break;
		}
	
		m_processListBox.AddString(m_imageinfo[i].path+"["+str2+str1+"]");
	}
	
	m_processListBox.SetCurSel(selIndex);
	//m_processListBox.UpdateWindow();


}


void CMultiThreadImageProcessingDlg::OnEnChangeEdit1()
{
	CString inputParam;
	int comsel = m_processingTypeCombo.GetCurSel();  // 获取当前选中的项的索引
	UpdateData(false);
	m_inputParamEdit.GetWindowText(inputParam); // 获取输入框内容

	// 创建 MultiThreadImageProcessor 对象并将参数传递进去
	int selIndex = m_processListBox.GetCurSel();  // 获取当前选中的项的索引
	
	if(selIndex!=LB_ERR)
	{
		CString filePath = m_imageinfo[selIndex].path;
		std::string str = CT2A(filePath.GetString());
		m_imageinfo[selIndex].para = atof(inputParam);
	}
	m_processingTypeCombo.SetCurSel(comsel);
	//MultiThreadImageProcessor* pProcessor = new MultiThreadImageProcessor(str,inputParam);
	//pProcessor->m_inputParam = inputParam;  // 将输入参数保存到对象中

}


