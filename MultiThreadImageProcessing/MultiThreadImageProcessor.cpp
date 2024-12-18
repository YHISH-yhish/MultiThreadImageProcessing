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
//    // 确保图像有效
//    if (mat.empty()) {
//        AfxMessageBox(_T("输入图像无效"));
//        return;
//    }
//
//    int width = mat.cols;
//    int height = mat.rows;
//
//    // 创建 CImage 对象，假设目标是 RGB 图像
//    if (!img.Create(width, height, 24)) { // 24 位 RGB 图像
//        AfxMessageBox(_T("CImage 创建失败"));
//        return;
//    }
//
//    // 获取 CImage 的像素数据
//    BYTE* pData = static_cast<BYTE*>(img.GetBits());
//    if (pData == nullptr) {
//        AfxMessageBox(_T("无法获取 CImage 数据"));
//        return;
//    }
//
//    int channels = mat.channels();
//    if (channels == 1) {  // 灰度图像 (单通道)
//        for (int y = 0; y < height; ++y) {
//            for (int x = 0; x < width; ++x) {
//                uchar gray = mat.at<uchar>(y, x);
//                pData[(y * width + x) * 3 + 0] = gray; // Blue
//             //   pData[(y * width + x) * 3 + 1] = gray; // Green
//              //  pData[(y * width + x) * 3 + 2] = gray; // Red
//            }
//        }
//    }
//    else if (channels == 3) {  // 彩色图像 (三通道)
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
//        AfxMessageBox(_T("不支持的图像格式"));
//    }
//}

void MatToCImage( cv::Mat &mat, CImage& cImage)
{
    //create new CImage
    int width = mat.cols;
    int height = mat.rows;
    int channels = mat.channels();

    cImage.Destroy(); //clear
    cImage.Create(width, height, 8 * channels); //默认图像像素单通道占用1个字节

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

    // 检查图像是否加载成功
    if (image.empty()) {
        PostMessage(m_hWnd, WM_USER_ERROR, (WPARAM)(L"[无法加载图像]"), 0);
        m_isProcessing = false;
        return;
    }

    // 获取下拉框选择的处理类型
   /* CComboBox* pComboBox = (CComboBox*)::GetDlgItem(m_hWnd, IDC_PROCESSING_TYPE);
    if (pComboBox == nullptr || pComboBox->GetSafeHwnd() == NULL) {
        PostMessage(m_hWnd, WM_USER_ERROR, (WPARAM)(L"[无法找到下拉框控件]"), 0);
        m_isProcessing = false;
        return;
    }*/

    int selectedIndex = m_selecttype;
    // 根据选择的类型执行不同的图像处理
    switch (selectedIndex) {
	//case 0:  // 未选择
	//	AfxMessageBox(_T("请选择处理类型"));
	//	break;
    case 0:  // 灰度处理
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessGrayscale, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 1:  // 放大处理
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessEnlarge, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 2:  // 缩小处理
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessShrink, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 3:  // 顺时针旋转
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessRotateClockwise, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    case 4:  // 逆时针旋转
        AfxBeginThread(&MultiThreadImageProcessor::ThreadProcessRotateCounterClockwise, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;

    default:
        PostMessage(m_hWnd, WM_USER_ERROR, (WPARAM)(L"[未知处理类型]"), 0);
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
    size_t pos = basePath.find_last_of("\\.");  // 查找路径最后的分隔符
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // 获取路径部分
    }
    std::string outputPath = basePath + "_out.jpg";  // 拼接输出文件路径
    cv::imwrite(outputPath, grayImage);
    //std::string  outputPath = pProcessor->m_imagePath + "outputImage.jpg";
	//cv::imwrite(outputPath, grayImage);
    //bool result = cv::imwrite("output_image.jpg", grayImage);

   /* if (result) {
        std::cout << "图像保存成功!" << std::endl;
    }
    else {
        std::cerr << "图像保存失败!" << std::endl;
    }*/
    
    //for (int i = 1; i <= 100; ++i) {
    //    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)i, (LPARAM)("[正在进行灰度处理]"));
    //    //pProcessor->UpdateProgress(i);
    //    std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 延时模拟处理
    //}
   // ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, 100, (LPARAM)("[灰度处理完成]"));
    pProcessor->m_isProcessing = false;
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 2);
    delete pProcessor;
    return 0;
}

UINT MultiThreadImageProcessor::ThreadProcessEnlarge(LPVOID pParam) {
    MultiThreadImageProcessor* pProcessor = reinterpret_cast<MultiThreadImageProcessor*>(pParam);
    // 获取输入参数
    float inputParam = pProcessor->m_inputParam;
   
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 1);
    cv::Mat image = cv::imread(pProcessor->m_imagePath);
    cv::Mat enlargedImage;
    cv::resize(image, enlargedImage, cv::Size(), inputParam, inputParam, cv::INTER_LINEAR);
    std::string basePath = pProcessor->m_imagePath;
    size_t pos = basePath.find_last_of("\\.");  // 查找路径最后的分隔符
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // 获取路径部分
    }
    std::string outputPath = basePath + "_out.jpg";  // 拼接输出文件路径
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
    size_t pos = basePath.find_last_of("\\.");  // 查找路径最后的分隔符
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // 获取路径部分
    }
    std::string outputPath = basePath + "_out.jpg";  // 拼接输出文件路径
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
    size_t pos = basePath.find_last_of("\\.");  // 查找路径最后的分隔符
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // 获取路径部分
    }
    std::string outputPath = basePath + "_out.jpg";  // 拼接输出文件路径
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
    size_t pos = basePath.find_last_of("\\.");  // 查找路径最后的分隔符
    if (pos != std::string::npos) {
        basePath = basePath.substr(0, pos);  // 获取路径部分
    }
    std::string outputPath = basePath + "_out.jpg";  // 拼接输出文件路径
    cv::imwrite(outputPath, rotatedCounterClockwise);
    pProcessor->m_isProcessing = false;
    ::PostMessage(pProcessor->m_hWnd, WM_PROCESS_UPDATE, (WPARAM)pProcessor->m_curfilenum, 2);
    delete pProcessor; 
    return 0;
}


void MultiThreadImageProcessor::CancelProcessing() {
    // 停止处理
    m_isProcessing = false;
}

bool MultiThreadImageProcessor::IsProcessing() const {
    return m_isProcessing;
}

void MultiThreadImageProcessor::SetWndHandle(HWND hWnd) {
    m_hWnd = hWnd;
}
