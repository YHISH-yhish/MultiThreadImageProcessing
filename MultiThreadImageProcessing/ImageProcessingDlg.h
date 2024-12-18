// ImageProcessingDlg.h
#pragma once

#include <thread>
#include <future>
#include <atomic>
#include <memory>
#include <opencv2/opencv.hpp>

// 定义图像处理类型枚举
enum class ImageProcessingType {
    GRAYSCALE = 0,
    ZOOM_200_PERCENT,
    ZOOM_50_PERCENT,
    ROTATE_90_CLOCKWISE,
    ROTATE_90_COUNTERCLOCKWISE
};

class MultiThreadImageProcessor {
public:
    MultiThreadImageProcessor(const CString& imagePath, ImageProcessingType processingType);
    ~MultiThreadImageProcessor();

    void ProcessImage();
    void CancelProcessing();

private:
    CString m_imagePath;
    ImageProcessingType m_processingType;
    std::atomic<bool> m_cancelRequested;

    cv::Mat ProcessImageInternal(const cv::Mat& input);
    void NotifyProgress(const CString& message);
};

class CImageProcessingDlg : public CDialogEx {
public:
    CImageProcessingDlg(CWnd* pParent = nullptr);
    ~CImageProcessingDlg();

    enum { IDD = IDC_LIST_PROCESS};

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

protected:
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnAddImage();
    afx_msg void OnDeleteImage();
    afx_msg void OnViewResult();
    afx_msg void OnStartProcessing();
    afx_msg void OnCancelProcessing();
    afx_msg LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSelectProcessingType();

private:
    CImage m_image;
    CStatic m_imageCtrl;
    CButton m_startButton;
    CButton m_cancelButton;
    CButton m_addButton;
    CButton m_deleteButton;
    CButton m_viewResultButton;
    CComboBox m_processingTypeCombo;

    std::atomic<bool> m_isProcessing;
    ImageProcessingType m_selectedProcessingType;
    std::unique_ptr<MultiThreadImageProcessor> m_pProcessor;

    bool LoadImage(const CString& filePath);
    void UpdateUIState();
};