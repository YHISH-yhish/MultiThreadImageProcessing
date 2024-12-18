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

    void ProcessImage();  // ����ͼ����
    void CancelProcessing();  // ȡ������
    bool IsProcessing() const;  // ����Ƿ����ڴ���
    void SetWndHandle(HWND hWnd);  // ���ô��ھ��
    int m_selecttype;
    float m_inputParam;  // ���ڱ����UI���ݹ����Ĳ���

    void UpdateProgress(int progress);  // ���½���

private:
    void ProcessGrayscale(const cv::Mat& image);  // �Ҷȴ���
    void ProcessEnlarge(const cv::Mat& image);  // �Ŵ���
    void ProcessShrink(const cv::Mat& image);  // ��С����
    void ProcessRotateClockwise(const cv::Mat& image);  // ˳ʱ����ת
    void ProcessRotateCounterClockwise(const cv::Mat& image);  // ��ʱ����ת

    // ʹ���߳�������
    static UINT ThreadProcessGrayscale(LPVOID pParam);
    static UINT ThreadProcessEnlarge(LPVOID pParam);
    static UINT ThreadProcessShrink(LPVOID pParam);
    static UINT ThreadProcessRotateClockwise(LPVOID pParam);
    static UINT ThreadProcessRotateCounterClockwise(LPVOID pParam);
	CImage m_processedimage;  // ͼ��
private:
    std::string m_imagePath;  // ͼ��·��
    bool m_isProcessing;  // �Ƿ����ڴ���
    int m_curfilenum;
    HWND m_hWnd;  // ���ھ��

};
