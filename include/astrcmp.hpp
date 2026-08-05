#ifndef __ASTRCMP_HPP
#define __ASTRCMP_HPP 0

#include<opencv2/opencv.hpp>
#include<vector>

#define ACMP_VERSION "0.4.0.0"
#define ACMP_CHAR_MAX 512

namespace acmp {

    enum ErrorType {
        ACMP_SUCCESS = 0,
        ACMP_ERROR_OPENFILE,
        ACMP_ERROR_BAYERPATTERN
    };

    enum CompositeType {
        COMP_SUM = 0,
        COMP_AVR,
        COMP_MIN,
        COMP_MAX,
        COMP_TIMELAPSE
    };

    enum BayerPattern {
        BAYER_UNKNOWN = 0,
        BAYER_BGGR,
        BAYER_GBRG,
        BAYER_GRBG,
        BAYER_RGGB
    };

    struct ProcessorConf {
        int compType = COMP_SUM;
        int align = 0;
        float hprThresh = 10;
        int autoWb = 0;
        float starThresh = 0.35;
        float gamma = 0.45;
        int resizeHeight = 0;
        int fps = 15;
        int encodeQuality = 50;
        int printVerbose = 0;
        char fileName[512];
    };

    class Processor {
    public:
        Processor(const ProcessorConf*);
        ~Processor();
        void release();
        void setRefImg(const cv::Mat&);
        int composite(char**, int);
        int comasew(char**, int);

    private:
        void _initHprKernel();
        int _openRaw(const char*, cv::Mat&);
        void _detectAndCompute(const cv::Mat&, std::vector<cv::KeyPoint>&, cv::Mat&);
        int _align(const cv::Mat&, cv::Mat&, const std::vector<cv::KeyPoint>&, const cv::Mat&) const;
        void _addImg(cv::Mat&, const cv::Mat&, int) const;
        void _autoWb(cv::Mat&) const;
        void _resize(cv::Mat&) const;

        ProcessorConf _config;
        std::vector<cv::KeyPoint> _refKeyPoints;
        cv::Mat _refDescriptor;
        cv::Mat _hprKernel;
    };

    const char* acmp_err2str(int);
}

#endif