#include "pch.h"
#include "MultiThreadImageProcessing.h"
#include "ImageProcessingDlg.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

MultiThreadImageProcessor::MultiThreadImageProcessor(
    const CString& imagePath,
    ImageProcessingType processingType
) :
    m_imagePath(imagePath),
    m_processingType(processingType),
    m_cancelRequested(false)
{
}

MultiThreadImageProcessor::~MultiThreadImageProcessor() {
    CancelProcessing();
}

void MultiThreadImageProcessor::ProcessImage() {
    try {
        // 使用OpenCV读取图像
        cv::Mat inputImage = cv::imread(CT2A(m_imagePath));  // Convert CString to const char*
        if (inputImage.empty()) {
            NotifyProgress(_T("图像加载失败"));
            return;
        }

        if (m_cancelRequested) {
            NotifyProgress(_T("处理已取消"));
            return;
        }

        // 处理图像
        cv::Mat processedImage = ProcessImageInternal(inputImage);

        // 保存处理后的图像
        CString outputPath = m_imagePath + _T("_processed.jpg");
        cv::imwrite(CT2A(outputPath), processedImage); // Convert CString to const char*

        NotifyProgress(_T("图像处理完成"));
    }
    catch (const cv::Exception& e) {
        NotifyProgress(CString(e.what()));
    }
    catch (...) {
        NotifyProgress(_T("未知错误"));
    }
}

cv::Mat MultiThreadImageProcessor::ProcessImageInternal(const cv::Mat& input) {
    cv::Mat result;
    switch (m_processingType) {
    case ImageProcessingType::GRAYSCALE:
        cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
        break;
    case ImageProcessingType::ZOOM_200_PERCENT:
        cv::resize(input, result, cv::Size(), 2.0, 2.0);
        break;
    case ImageProcessingType::ZOOM_50_PERCENT:
        cv::resize(input, result, cv::Size(), 0.5, 0.5);
        break;
    case ImageProcessingType::ROTATE_90_CLOCKWISE:
        cv::rotate(input, result, cv::ROTATE_90_CLOCKWISE);
        break;
    case ImageProcessingType::ROTATE_90_COUNTERCLOCKWISE:
        cv::rotate(input, result, cv::ROTATE_90_COUNTERCLOCKWISE);
        break;
    default:
        result = input.clone();
    }
    return result;
}

void MultiThreadImageProcessor::CancelProcessing() {
    m_cancelRequested = true;
}

void MultiThreadImageProcessor::NotifyProgress(const CString& message) {
    ::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_PROCESS_UPDATE,
        reinterpret_cast<WPARAM>(CT2A(message)), 0);
}

// CImageProcessingDlg 实现
BEGIN_MESSAGE_MAP(CImageProcessingDlg, CDialogEx)
    ON_BN_CLICKED(ID_ADD_IMAGE, &CImageProcessingDlg::OnAddImage)
    ON_BN_CLICKED(ID_DELETE_IMAGE, &CImageProcessingDlg::OnDeleteImage)
    ON_BN_CLICKED(ID_VIEW_RESULT, &CImageProcessingDlg::OnViewResult)
    ON_BN_CLICKED(ID_START_PROCESSING, &CImageProcessingDlg::OnStartProcessing)
    ON_BN_CLICKED(ID_CANCEL_PROCESSING, &CImageProcessingDlg::OnCancelProcessing)
    ON_MESSAGE(WM_PROCESS_UPDATE, &CImageProcessingDlg::OnUpdateProgress)
    ON_CBN_SELCHANGE(IDC_PROCESSING_TYPE, &CImageProcessingDlg::OnSelectProcessingType)
END_MESSAGE_MAP()

CImageProcessingDlg::CImageProcessingDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_IMAGEPROCESSING_DIALOG, pParent),
    m_isProcessing(false),
    m_selectedProcessingType(ImageProcessingType::GRAYSCALE)
{
}

CImageProcessingDlg::~CImageProcessingDlg() {
    if (m_pProcessor) {
        m_pProcessor->CancelProcessing();
    }
}

BOOL CImageProcessingDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // 初始化控件
    m_imageCtrl.SubclassDlgItem(IDC_IMAGE, this);
    m_startButton.SubclassDlgItem(ID_START_PROCESSING, this);
    m_cancelButton.SubclassDlgItem(ID_CANCEL_PROCESSING, this);
    m_addButton.SubclassDlgItem(ID_ADD_IMAGE, this);
    m_deleteButton.SubclassDlgItem(ID_DELETE_IMAGE, this);
    m_viewResultButton.SubclassDlgItem(ID_VIEW_RESULT, this);
    m_processingTypeCombo.SubclassDlgItem(IDC_PROCESSING_TYPE, this);

    m_processingTypeCombo.AddString(_T("灰度处理"));
    m_processingTypeCombo.AddString(_T("放大至200%"));
    m_processingTypeCombo.AddString(_T("缩小至50%"));
    m_processingTypeCombo.AddString(_T("顺时针旋转90°"));
    m_processingTypeCombo.AddString(_T("逆时针旋转90°"));
    m_processingTypeCombo.SetCurSel(0);

    UpdateUIState();

    return TRUE;
}

void CImageProcessingDlg::UpdateUIState() {
    m_startButton.EnableWindow(!m_isProcessing && !m_image.IsNull());
    m_cancelButton.EnableWindow(m_isProcessing);
    m_deleteButton.EnableWindow(!m_isProcessing && !m_image.IsNull());
    m_viewResultButton.EnableWindow(!m_isProcessing && !m_image.IsNull());
}

void CImageProcessingDlg::OnAddImage() {
    CFileDialog fileDlg(TRUE, _T("Image Files"), NULL,
        OFN_FILEMUSTEXIST, _T("Image Files (*.jpg; *.png; *.bmp)|*.jpg; *.png; *.bmp||"));

    if (fileDlg.DoModal() == IDOK) {
        if (LoadImage(fileDlg.GetPathName())) {
            m_imageCtrl.SetBitmap((HBITMAP)m_image.Detach());
            UpdateUIState();
        }
    }
}

bool CImageProcessingDlg::LoadImage(const CString& filePath) {
    try {
        m_image.Destroy();
        if (FAILED(m_image.Load(filePath))) {
            AfxMessageBox(_T("图像加载失败"));
            return false;
        }
        m_imagePath = filePath; // 保存文件路径
        return true;
    }
    catch (const CException* e) {
        e->ReportError();
        return false;
    }
}

void CImageProcessingDlg::OnDeleteImage() {
    m_image.Destroy();
    m_imageCtrl.SetBitmap(NULL);
    UpdateUIState();
}

void CImageProcessingDlg::OnStartProcessing() {
    if (m_isProcessing || m_image.IsNull()) return;

    m_isProcessing = true;
    UpdateUIState();

    // 创建处理器
    m_pProcessor = std::make_unique<MultiThreadImageProcessor>(
        m_imagePath, // 使用保存的路径
        m_selectedProcessingType
    );

    std::thread processingThread([this]() {
        m_pProcessor->ProcessImage();
        PostMessage(WM_PROCESS_UPDATE, reinterpret_cast<WPARAM>(_T("处理完成").GetBuffer()), 0);
        });
    processingThread.detach();
}

void CImageProcessingDlg::OnCancelProcessing() {
    if (m_pProcessor) {
        m_pProcessor->CancelProcessing();
    }
    m_isProcessing = false;
    UpdateUIState();
}

LRESULT CImageProcessingDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam) {
    CString msg((LPCSTR)wParam);
    AfxMessageBox(msg);
    m_isProcessing = false;
    UpdateUIState();
    return 0;
}

void CImageProcessingDlg::OnSelectProcessingType() {
    int selectedIndex = m_processingTypeCombo.GetCurSel();
    if (selectedIndex != CB_ERR) {
        m_selectedProcessingType = static_cast<ImageProcessingType>(selectedIndex);
    }
    else {
        m_selectedProcessingType = ImageProcessingType::GRAYSCALE;
        m_processingTypeCombo.SetCurSel(0);
    }
}

void CImageProcessingDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}
