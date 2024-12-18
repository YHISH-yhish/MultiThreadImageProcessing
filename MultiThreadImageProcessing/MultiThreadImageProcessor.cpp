#include "pch.h"
#include "resource.h"
#include "MultiThreadImageProcessor.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <thread>

//void MatToCImage(const cv::Mat& mat, CImage& img) {
//    // ȷ��ͼ����Ч
//    if (mat.empty()) {
//        AfxMessageBox(_T("����ͼ����Ч"));
//        return;
//    }
//
//    int width = mat.cols;
//    int height = mat.rows;
//
//    // ���� CImage ���󣬼���Ŀ���� RGB ͼ��
//    if (!img.Create(width, height, 24)) { // 24 λ RGB ͼ��
//        AfxMessageBox(_T("CImage ����ʧ��"));
//        return;
//    }
//
//    // ��ȡ CImage ����������
//    BYTE* pData = static_cast<BYTE*>(img.GetBits());
//    if (pData == nullptr) {
//        AfxMessageBox(_T("�޷���ȡ CImage ����"));
//        return;
//    }
//
//    int channels = mat.channels();
//    if (channels == 1) {  // �Ҷ�ͼ�� (��ͨ��)
//        for (int y = 0; y < height; ++y) {
//            for (int x = 0; x < width; ++x) {
//                uchar gray = mat.at<uchar>(y, x);
//                pData[(y * width + x) * 3 + 0] = gray; // Blue
//             //   pData[(y * width + x) * 3 + 1] = gray; // Green
//              //  pData[(y * width + x) * 3 + 2] = gray; // Red
//            }
//        }
//    }
//    else if (channels == 3) {  // ��ɫͼ�� (��ͨ��)
//        for (int y = 0; y < height; ++y) {
//            for (int x = 0; x < width; ++x) {
//                cv::Vec3b color = mat.at<cv::Vec3b>(y, x);
//                pData[(y * width + x) * 3 + 0] = color[0]; // Blue
//                pData[(y * width + x) * 3 + 1] = color[1]; // Green
//                pData[(y * width + x) * 3 + 2] = color[2]; // Red
//            }
//        }
//    }
//    else {
//        AfxMessageBox(_T("��֧�ֵ�ͼ���ʽ"));
//    }
//}

void MatToCImage( cv::Mat &mat, CImage& cImage)
{
    //create new CImage
    int width = mat.cols;
    int height = mat.rows;
    int channels = mat.channels();

    cImage.Destroy(); //clear
    cImage.Create(width, height, 8 * channels); //Ĭ��ͼ�����ص�ͨ��ռ��1���ֽ�

    //copy values
    uchar* ps;
    uchar* pimg = (uchar*)cImage.GetBits(); //A pointer to the bitmap buffer
    int step = cImage.GetPitch();

    for (int i = 0; i < height; ++i)
    {
        ps = (mat.ptr<uchar>(i));
        for (int j = 0; j < width; ++j)
        {
            if (channels == 1) //gray
            {
                *(pimg + i * step + j) = ps[j];
            }
            else if (channels == 3) //color
            {
                for (int k = 0; k < 3; ++k)
                {
                    *(pimg + i * step + j * 3 + k) = ps[j * 3 + k];
                }
            }
        }
    }
}




MultiThreadImageProcessor::MultiThreadImageProcessor(const std::string& imagePath, CString m_inputParam)
    : m_imagePath(imagePath), m_isProcessing(false), m_hWnd(nullptr) {
}

MultiThreadImageProcessor::MultiThreadImageProcessor(const CString& imagePath, int processingType,int curfilenum,float para)
    : m_imagePath(imagePath), m_isProcessing(false), m_hWnd(nullptr) {
    m_selecttype = processingType;
    m_curfilenum = curfilenum;
    m_inputParam = para;
}

void MultiThreadImageProcessor::UpdateProgress(int progress) {
    if (m_hWnd != nullptr) {
        ::PostMessage(m_hWnd, WM_PROCESS_UPDATE, progress, 0);
    }
}

void MultiThreadImageProcessor::ProcessImage() {
    m_isProcessing = true;
    cv::Mat image = cv::imread(m_imagePath);

    // ���ͼ���Ƿ���سɹ�
    if (image.empty()) {
        PostMessage(m_hWnd, WM_USER_ERROR, (WPARAM)(L"[�޷�����ͼ��]"), 0);
        m_isProcessing = false;
        return;
    }

    // ��ȡ������ѡ��Ĵ�������
   /* CComboBox* pComboBox = (CComboBox*)::GetDlgItem(m_hWnd, IDC_PROCESSING_TYPE);
    if (pComboBox == nullptr || pComboBox->GetSafeHwnd() == NULL) {
        PostMessage(m_hWnd, WM_USER_ERROR, (WPARAM)(L"[�޷��ҵ�������ؼ�]"), 0);
        m_isProcessing = false;
        return;
    }*/

    int selectedIndex = m_selecttype;
    // ����ѡ�������ִ�в�ͬ��ͼ����
    switch (selectedIndex) {
	//case 0:  // δѡ��
	//	AfxMessageBox(_T("��ѡ��������"));
	//	break;
    case 0:  // �Ҷȴ���
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessGrayscale, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 1:  // �Ŵ���
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessEnlarge, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 2:  // ��С����
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessShrink, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 3:  // ˳ʱ����ת
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessRotateClockwise, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 4:  // ��ʱ����ת
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessRotateCounterClockwise, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    default:
        PostMessage(m_hWnd, WM_USER_ERROR, (WPARAM)(L"[δ֪��������]"), 0);
        break;
    }
}


UINT MultiThreadImageProcessor::ThreadProcessGrayscale(LPVOID pParam) {
    MultiThreadImageProcessor* pProcessor = reinterpret_cast<MultiThreadImageProcessor*>(pParam);
    cv::Mat image = cv::imread(pProcessor->m_imagePath);
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 1);
   // 
   // cv::imshow("Origin Image", image);	cv::waitKey(0);
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
	//cv::imshow("Gray Image", grayImage);
    // cv::waitKey(0);
	//MatToCImage(grayImage, pProcessor->m_processedimage);
    std::string basePath = pProcessor->m_imagePath;
    size_t pos = basePath.find_last_of("\\.");  // ����·�����ķָ���
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // ��ȡ·������
    }
    std::string outputPath = basePath + "_out.jpg";  // ƴ������ļ�·��
    cv::imwrite(outputPath, grayImage);
    //std::string  outputPath = pProcessor->m_imagePath + "outputImage.jpg";
	//cv::imwrite(outputPath, grayImage);
    //bool result = cv::imwrite("output_image.jpg", grayImage);

   /* if (result) {
        std::cout << "ͼ�񱣴�ɹ�!" << std::endl;
    }
    else {
        std::cerr << "ͼ�񱣴�ʧ��!" << std::endl;
    }*/
    
    //for (int i = 1; i <= 100; ++i) {
    //    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)i, (LPARAM)("[���ڽ��лҶȴ���]"));
    //    //pProcessor->UpdateProgress(i);
    //    std::this_thread::sleep_for(std::chrono::milliseconds(20));  // ��ʱģ�⴦��
    //}
   // ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, 100, (LPARAM)("[�Ҷȴ������]"));
    pProcessor->m_isProcessing = false;
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 2);
    delete pProcessor;
    return 0;
}

UINT MultiThreadImageProcessor::ThreadProcessEnlarge(LPVOID pParam) {
    MultiThreadImageProcessor* pProcessor = reinterpret_cast<MultiThreadImageProcessor*>(pParam);
    // ��ȡ�������
    float inputParam = pProcessor->m_inputParam;
   
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 1);
    cv::Mat image = cv::imread(pProcessor->m_imagePath);
    cv::Mat enlargedImage;
    cv::resize(image, enlargedImage, cv::Size(), inputParam, inputParam, cv::INTER_LINEAR);
    std::string basePath = pProcessor->m_imagePath;
    size_t pos = basePath.find_last_of("\\.");  // ����·�����ķָ���
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // ��ȡ·������
    }
    std::string outputPath = basePath + "_out.jpg";  // ƴ������ļ�·��
    cv::imwrite(outputPath, enlargedImage);
    pProcessor->m_isProcessing = false; 
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 2);
    delete pProcessor;
    return 0;
}

UINT MultiThreadImageProcessor::ThreadProcessShrink(LPVOID pParam) {
    MultiThreadImageProcessor* pProcessor = reinterpret_cast<MultiThreadImageProcessor*>(pParam);
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 1); 
    float inputParam = 1.0/pProcessor->m_inputParam;
    cv::Mat image = cv::imread(pProcessor->m_imagePath);
    cv::Mat shrunkImage;
    cv::resize(image, shrunkImage, cv::Size(), inputParam, inputParam, cv::INTER_LINEAR);
    std::string basePath = pProcessor->m_imagePath;
    size_t pos = basePath.find_last_of("\\.");  // ����·�����ķָ���
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // ��ȡ·������
    }
    std::string outputPath = basePath + "_out.jpg";  // ƴ������ļ�·��
    cv::imwrite(outputPath, shrunkImage);
    pProcessor->m_isProcessing = false;
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 2);
    delete pProcessor;
    return 0;
}

UINT MultiThreadImageProcessor::ThreadProcessRotateClockwise(LPVOID pParam) {
    MultiThreadImageProcessor* pProcessor = reinterpret_cast<MultiThreadImageProcessor*>(pParam);
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 1); 
    int inputParam = pProcessor->m_inputParam;
   
    cv::Mat image = cv::imread(pProcessor->m_imagePath);
    cv::Mat rotatedClockwise;
    if (inputParam <= 90)
        cv::rotate(image, rotatedClockwise, cv::ROTATE_90_CLOCKWISE);
    else if(inputParam >90)
        cv::rotate(image, rotatedClockwise, cv::ROTATE_180);
  
    std::string basePath = pProcessor->m_imagePath;
    size_t pos = basePath.find_last_of("\\.");  // ����·�����ķָ���
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // ��ȡ·������
    }
    std::string outputPath = basePath + "_out.jpg";  // ƴ������ļ�·��
    cv::imwrite(outputPath, rotatedClockwise);
    pProcessor->m_isProcessing = false;
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 2);
    delete pProcessor;
    return 0;
}

UINT MultiThreadImageProcessor::ThreadProcessRotateCounterClockwise(LPVOID pParam) {
    MultiThreadImageProcessor* pProcessor = reinterpret_cast<MultiThreadImageProcessor*>(pParam);
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 1); 
    int inputParam = pProcessor->m_inputParam;
    cv::Mat image = cv::imread(pProcessor->m_imagePath);
    cv::Mat rotatedCounterClockwise;

    if (inputParam <= 90)
         cv::rotate(image, rotatedCounterClockwise, cv::ROTATE_90_COUNTERCLOCKWISE);
    else if (inputParam > 90)
        cv::rotate(image, rotatedCounterClockwise, cv::ROTATE_180);

    std::string basePath = pProcessor->m_imagePath;
    size_t pos = basePath.find_last_of("\\.");  // ����·�����ķָ���
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // ��ȡ·������
    }
    std::string outputPath = basePath + "_out.jpg";  // ƴ������ļ�·��
    cv::imwrite(outputPath, rotatedCounterClockwise);
    pProcessor->m_isProcessing = false;
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 2);
    delete pProcessor; 
    return 0;
}


void MultiThreadImageProcessor::CancelProcessing() {
    // ֹͣ����
    m_isProcessing = false;
}

bool MultiThreadImageProcessor::IsProcessing() const {
    return m_isProcessing;
}

void MultiThreadImageProcessor::SetWndHandle(HWND hWnd) {
    m_hWnd = hWnd;
}
