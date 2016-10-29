#include <Tensor.h>
#include <TensorUtil.h>
#include <opencv2\opencv.hpp>

#define KERNEL_SIZE 10

int main(){
    // Reading image.
    cv::Mat m = cv::imread("img.jpg", 1);
    cv::resize(m, m, cv::Size(100, 100));

    // Checking image.
    cv::imshow("m", m);cv::waitKey(1);

    // Convert from cv::Mat to Tensor<T>
    auto mt = tu::Mat2Tensor<unsigned char>(m, 
                                  tu::s("i", m.rows, "j", m.cols, "k", 3, ""),
                                  tu::s("i", 0,      "j", 0,      "k", 0, "")
                                  );

    // Generate index tensor.
    auto& i = tu::rangeTensor<double>(0, m.rows, 1, "i", 0);
    auto& j = tu::rangeTensor<double>(0, m.cols, 1, "j", 0);
    auto& k = tu::rangeTensor<double>(0, 3, 1, "k", 0);
    auto& l = tu::rangeTensor<double>(0, KERNEL_SIZE, 1, "l", 0);
    auto& o = tu::rangeTensor<double>(0, KERNEL_SIZE, 1, "m", 0);

    Tensor<double>& p = (l * o) * 0 + 1. / (l.size() * o.size());
    p = p.cud("l", 1, "m", 1, "");

    Tensor<int>& idx = (i * (double)m.cols * 3   + 
                        j * 3 + k + l * (double)m.cols * 3 + o * 3)
                        .convertTo<int, ConcreteTensor<int>>();

    // Substitute indextensor to .
    Tensor<double>& mtIndexed = mt[idx].convertTo<double, ConcreteTensor<double>>();

    Tensor<unsigned char>& mean = (mtIndexed * p)
                                  .convertTo<unsigned char, ConcreteTensor<unsigned char>>();

    cv::Mat ret = tu::Tensor2Mat<unsigned char>(mean, CV_8UC3, "ijk"); 

    cv::imshow("ret", ret);cv::waitKey();
    cv::imwrite("ret.jpg", ret);
}