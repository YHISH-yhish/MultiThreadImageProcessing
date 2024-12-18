#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include <thread>
#define WM_PROCESS_UPDATE WM_USER+100
#define WM_USER_ERROR WM_USER+200
class MultiThreadImageProcessor {
public:
    MultiThreadImageProcessor(const std::string& imagePath, CString m_inputParam);
    MultiThreadImageProcessor(const CString& imagePath, int processingType,int curfilenum, float para);

    void ProcessImage();  // 启动图像处理
    void CancelProcessing();  // 取消处理
    bool IsProcessing() const;  // 检查是否正在处理
    void SetWndHandle(HWND hWnd);  // 设置窗口句柄
    int m_selecttype;
    float m_inputParam;  // 用于保存从UI传递过来的参数

    void UpdateProgress(int progress);  // 更新进度

private:
    void ProcessGrayscale(const cv::Mat& image);  // 灰度处理
    void ProcessEnlarge(const cv::Mat& image);  // 放大处理
    void ProcessShrink(const cv::Mat& image);  // 缩小处理
    void ProcessRotateClockwise(const cv::Mat& image);  // 顺时针旋转
    void ProcessRotateCounterClockwise(const cv::Mat& image);  // 逆时针旋转

    // 使用线程来处理
    static UINT ThreadProcessGrayscale(LPVOID pParam);
    static UINT ThreadProcessEnlarge(LPVOID pParam);
    static UINT ThreadProcessShrink(LPVOID pParam);
    static UINT ThreadProcessRotateClockwise(LPVOID pParam);
    static UINT ThreadProcessRotateCounterClockwise(LPVOID pParam);
	CImage m_processedimage;  // 图像
private:
    std::string m_imagePath;  // 图像路径
    bool m_isProcessing;  // 是否正在处理
    int m_curfilenum;
    HWND m_hWnd;  // 窗口句柄

};
