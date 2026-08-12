#include<astrcmp.hpp>
#include<libraw/libraw.h>
#include<cstdio>
#include<cstring>

using namespace std;

namespace acmp {

    int __getPattern(LibRaw* proc) {
        if(proc->COLOR(0, 0) == 0) return BAYER_RGGB;
        else if(proc->COLOR(0, 0) == 2) return BAYER_BGGR;
        else if(proc->COLOR(0, 1) == 0) return BAYER_GRBG;
        else if(proc->COLOR(0, 1) == 2) return BAYER_GBRG;
        else return BAYER_UNKNOWN;
    }

    Processor::Processor(const ProcessorConf* conf) {
        if(conf) memcpy(&_config, conf, sizeof(ProcessorConf));
    }

    Processor::~Processor() {
        release();
    }

    void Processor::release() {
        _refKeyPoints.clear();
        _refDescriptor.release();
    }

    void Processor::setRefImg(const cv::Mat& img) {
        _refKeyPoints.clear();
        _refDescriptor.release();
        _detectAndCompute(img, _refKeyPoints, _refDescriptor);
    }

    int Processor::composite(char** files, int nfiles) {
        if(_config.printVerbose) printf("*** AstrCmp ver.%s ***\n", ACMP_VERSION);

        cv::Mat dst;
        int nComp = 0;

        for(int i = 0; i < nfiles; i++) {
            if(_config.printVerbose) printf("[*] (%d/%d) loading \"%s\"...\n", i+1, nfiles, files[i]);

            cv::Mat img;
            int err = -1;
            if(err = _openRaw(files[i], img)) {
                fprintf(stderr, "[!] skipped \"%s\": %s (%d)\n", files[i], acmp_err2str(err), err);
                continue;
            }

            if(_config.align) {
                if(_refKeyPoints.empty() || _refDescriptor.empty()) {
                    if(_config.printVerbose) printf("[*] getting reference...\n");

                    setRefImg(img);

                    if(!_refKeyPoints.size()) {
                        fprintf(stderr, "[!] no star detected. skipped \"%s\".\n", files[i]);
                        img.release();
                        continue;
                    }

                    if(_config.printVerbose) printf("      - keypoints = %ld\n", _refKeyPoints.size());
                }else {
                    if(_config.printVerbose) printf("[*] aligning...\n");

                    vector<cv::Point> kps;
                    cv::Mat desc;
                    _detectAndCompute(img, kps, desc);

                    if(!kps.size()) {
                        fprintf(stderr, "[!] no star detected. skipped \"%s\".\n", files[i]);
                        desc.release();
                        img.release();
                        continue;
                    }else if(_config.printVerbose) {
                        printf("      - keypoints = %ld\n", kps.size());
                    }

                    cv::Mat aln;
                    int nm = _align(img, aln, kps, desc);
                    kps.clear();
                    desc.release();

                    if(!nm) {
                        fprintf(stderr, "[!] failed to align image. skipped \"%s\".\n", files[i]);
                        continue;
                    }else if(_config.printVerbose) {
                        printf("      - match = %d\n", nm);
                    }
                    img.release();
                    img = aln.clone();
                    aln.release();
                }
            }

            _addImg(dst, img, nfiles);
            img.release();
            nComp++;
        }

        if(_config.printVerbose) printf("[*] %d images total.\n", nComp);

        if(_config.autoWb) {
            if(_config.printVerbose) printf("[*] processing awb...\n");
            _autoWb(dst);
        }

        if(_config.resizeHeight > 0) {
            if(_config.printVerbose) printf("[*] resizing image...\n");
            _resize(dst);
        }

        if(_config.printVerbose) printf("[*] saving result as \"%s\".\n", _config.fileName);
        vector<int> params = { cv::IMWRITE_TIFF_COMPRESSION, 1 };
        cv::imwrite(_config.fileName, dst, params);
        dst.release();

        return nComp;
    }

    int Processor::comasew(char** files, int nfiles) {
        if(_config.printVerbose) printf("*** AstrCmp Ver.%s ***\n", ACMP_VERSION);

        cv::VideoWriter vw;
        int nComp = 0;

        for(int i = 0; i < nfiles; i++) {
            if(_config.printVerbose) printf("[*] (%d/%d) loading \"%s\"...\n", i+1, nfiles, files[i]);

            cv::Mat img;
            cv::Mat img8u;
            int err = -1;
            if(err = _openRaw(files[i], img)) {
                fprintf(stderr, "[!] skipped \"%s\": %s (%d)\n", files[i], acmp_err2str(err), err);
                continue;
            }

            if(_config.align) {
                if(_refKeyPoints.empty() || _refDescriptor.empty()) {
                    if(_config.printVerbose) printf("[*] getting reference...\n");

                    setRefImg(img);

                    if(!_refKeyPoints.size()) {
                        fprintf(stderr, "[!] no star detected. skipped \"%s\".\n", files[i]);
                        img.release();
                        continue;
                    }

                    if(_config.printVerbose) printf("      - keypoints = %ld\n", _refKeyPoints.size());
                }else {
                    if(_config.printVerbose) printf("[*] aligning...\n");

                    vector<cv::Point> kps;
                    cv::Mat desc;
                    _detectAndCompute(img, kps, desc);

                    if(!kps.size()) {
                        fprintf(stderr, "[!] no star detected. skipped \"%s\".\n", files[i]);
                        desc.release();
                        img.release();
                        continue;
                    }else if(_config.printVerbose) {
                        printf("      - keypoints = %ld\n", kps.size());
                    }

                    cv::Mat aln;
                    int nm = _align(img, aln, kps, desc);
                    kps.clear();
                    desc.release();

                    if(!nm) {
                        fprintf(stderr, "[!] failed to align image. skipped \"%s\".\n", files[i]);
                        img.release();
                        continue;
                    }else if(_config.printVerbose) {
                        printf("      - match = %d\n", nm);
                    }
                    img.release();
                    img = aln.clone();
                    aln.release();
                }
            }

            if(_config.autoWb) {
                if(_config.printVerbose) printf("[*] processing awb...\n");
                _autoWb(img);
            }

            if(_config.resizeHeight > 0) {
                if(_config.printVerbose) printf("[*] resizing image...\n");
                _resize(img);
            }

            img.convertTo(img8u, CV_8UC3, 255, 0);
            img.release();

            if(!vw.isOpened()) {
                char gstStr[ACMP_CHAR_MAX];
                sprintf(
                    gstStr, "appsrc ! videoconvert ! jpegenc quality=%d ! avimux ! filesink location=%s",
                    _config.encodeQuality, _config.fileName
                );
                if(!vw.open(gstStr, cv::CAP_GSTREAMER, 0, _config.fps, img8u.size())) {
                    fprintf(stderr, "[!] failed to open video stream.\n");
                    img8u.release();
                    return nComp;
                }
            }
            vw.write(img8u);

            img8u.release();

            nComp++;
        }
        if(_config.printVerbose) printf("[*] %d images total.\n", nComp);

        if(_config.printVerbose) printf("[*] finishing encoding...\n");
        vw.release();

        return nComp;
    }

    void Processor::_initHprKernel() {
        _hprKernel = cv::Mat(3, 3, CV_32FC1);
        float* phpr = reinterpret_cast<float*>(_hprKernel.data);
        for(int i = 0; i < 9; i++) phpr[i] = (i == 4) ? 0 : 1/8.0;
    }

    int Processor::_openRaw(const char* file, cv::Mat& dst) {
        dst.release();
        LibRaw rproc;
        if(rproc.open_file(file)) {
            return _openImg(file, dst);
        }
        rproc.unpack();
        int xoff = rproc.imgdata.sizes.left_margin;
        int yoff = rproc.imgdata.sizes.top_margin;
        int pattern = __getPattern(&rproc);
        int rwidth = rproc.imgdata.sizes.raw_width;
        int rheight = rproc.imgdata.sizes.raw_height;
        int width = rproc.imgdata.sizes.width / 2;
        int height = rproc.imgdata.sizes.height / 2;
        int pMax = pow(2, rproc.imgdata.color.raw_bps) - 1;
        size_t rawSize = rwidth * rheight;
        size_t imgSize = width * height;

        float* ptr = new float[rawSize];
        for(size_t j = 0; j < rawSize; j++) {
            ptr[j] = (float)rproc.imgdata.rawdata.raw_image[j] - rproc.imgdata.color.black;
        }

        // split raw channel
        float mul_max = max(
            rproc.imgdata.color.cam_mul[0],
            max(rproc.imgdata.color.cam_mul[1],
                max(rproc.imgdata.color.cam_mul[2],
                    rproc.imgdata.color.cam_mul[3]
                )
            )
        );
        cv::Mat rawrz(height, width, CV_32FC3);
        float* prrz = reinterpret_cast<float*>(rawrz.data);
        for(size_t j = 0; j < imgSize; j++) {
            int x = j % width, y = j / width;
            size_t ptr_x = x * 2 + xoff;
            size_t ptr_y = y * 2 + yoff;
            float r = 0, g1 = 0, g2 = 0, b = 0;
            if(pattern == BAYER_BGGR) {
                b = ptr[ptr_x + rwidth * ptr_y];
                g1 = ptr[ptr_x + 1 + rwidth * ptr_y];
                g2 = ptr[ptr_x + rwidth * (ptr_y + 1)];
                r = ptr[ptr_x + 1 + rwidth * (ptr_y + 1)];
            }else if(pattern == BAYER_GBRG) {
                g1 = ptr[ptr_x + rwidth * ptr_y];
                b = ptr[ptr_x + 1 + rwidth * ptr_y];
                r = ptr[ptr_x + rwidth * (ptr_y + 1)];
                g2 = ptr[ptr_x + 1 + rwidth * (ptr_y + 1)];
            }else if(pattern == BAYER_GRBG) {
                g1 = ptr[ptr_x + rwidth * ptr_y];
                r = ptr[ptr_x + 1 + rwidth * ptr_y];
                b = ptr[ptr_x + rwidth * (ptr_y + 1)];
                g2 = ptr[ptr_x + 1 + rwidth * (ptr_y + 1)];
            }else if(pattern == BAYER_RGGB) {
                r = ptr[ptr_x + rwidth * ptr_y];
                g1 = ptr[ptr_x + 1 + rwidth * ptr_y];
                g2 = ptr[ptr_x + rwidth * (ptr_y + 1)];
                b = ptr[ptr_x + 1 + rwidth * (ptr_y + 1)];
            }else {
                rproc.recycle();
                return ACMP_ERROR_BAYERPATTERN;
            }
            prrz[j*3]   = 2.0 * b * rproc.imgdata.color.cam_mul[2];
            prrz[j*3+1] = (g1 * rproc.imgdata.color.cam_mul[1] + g2 * rproc.imgdata.color.cam_mul[3]);
            prrz[j*3+2] = 2.0 * r * rproc.imgdata.color.cam_mul[0];
        }
        free(ptr);

        // hot pixel reduction
        if(_config.hprThresh > 0) {
            cv::Mat nhp;
            if(_hprKernel.empty()) _initHprKernel();
            cv::filter2D(rawrz, nhp, -1, _hprKernel, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
            float* pnhp = reinterpret_cast<float*>(nhp.data);
            #pragma omp parallel for
            for(int j = 0; j < imgSize*3; j++) {
                if((prrz[j] - pnhp[j] * _config.hprThresh) > 0) {
                    prrz[j] = pnhp[j];
                }
            }
            nhp.release();
        }

        // demosaicing
        dst.create(height, width, CV_32FC3);
        float* pimg = reinterpret_cast<float*>(dst.data);
        #pragma omp parallel for
        for(size_t j = 0; j < imgSize; j++) {
            for(int c = 0; c < 3; c++) {
                pimg[j*3+c] = 0;
                for(int k = 0; k < 3; k++) pimg[j*3+c] += prrz[j*3+k] * rproc.imgdata.color.rgb_cam[2-c][2-k];
                if(pimg[j*3+c] < 0) pimg[j*3+c] = 0;
                pimg[j*3+c] /= pMax * 1024.0;
                pimg[j*3+c] = pow(pimg[j*3+c], _config.gamma);
            }
        }

        rproc.recycle();
        rawrz.release();

        return ACMP_SUCCESS;
    }

    int Processor::_openImg(const char* file, cv::Mat& dst) {
        if(!dst.empty()) dst.release();

        cv::Mat img = cv::imread(file, cv::IMREAD_COLOR | cv::IMREAD_ANYDEPTH | cv::IMREAD_IGNORE_ORIENTATION);

        if(img.empty()) return ACMP_ERROR_OPENFILE;

        int depth = img.depth();
        if(depth == CV_8U) img.convertTo(dst, CV_32FC3, 1.0 / 255, 0);
        else if(depth == CV_16S) img.convertTo(dst, CV_32FC3, 1.0 / 32767, 0);
        else if(depth == CV_16U) img.convertTo(dst, CV_32FC3, 1.0 / 65535, 0);
        else if(depth == CV_32F) dst = img.clone();
        else {
            img.release();
            return ACMP_ERROR_IMGDEPTH;
        }

        return ACMP_SUCCESS;
    }

    void Processor::_detectAndCompute(const cv::Mat& src, vector<cv::Point>& keypoints, cv::Mat& descriptor) {
        keypoints.clear();
        descriptor.release();

        cv::Mat gsrc, mgsrc, usrc, tsrc;
        cv::cvtColor(src, gsrc, cv::COLOR_BGR2GRAY);
        cv::medianBlur(gsrc, mgsrc, 5);
        mgsrc.convertTo(usrc, CV_8UC1, 255);
        cv::threshold(usrc, tsrc, _config.starThresh * 255, 255, cv::THRESH_BINARY);
        usrc.release();
        gsrc.release();
        mgsrc.release();

        vector<vector<cv::Point>> contours;
        cv::findContours(tsrc, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

        int nkp = contours.size();
        if(!nkp) return;
        keypoints.resize(nkp);

        descriptor.create(nkp, 3, CV_32FC1);
        float* pdsc = reinterpret_cast<float*>(descriptor.data);

        #pragma omp parallel for
        for(int i = 0; i < nkp; i++) {
            cv::Point2f pt(0, 0);
            float size = 0;
            int np = contours[i].size();

            for(int j = 0; j < np; j++) {
                pt.x += (float)contours[i][j].x / np;
                pt.y += (float)contours[i][j].y / np;
            }

            keypoints[i] = pt;
        }

        #pragma omp parallel for
        for(int i = 0; i < nkp; i++) {
            float ndist = 1e9, ndist2 = 1e9;
            int nidx = -1, nidx2 = -1;

            for(int j = 0; j < nkp; j++) if(i != j) {
                float dx = keypoints[j].x - keypoints[i].x;
                float dy = keypoints[j].y - keypoints[i].y;
                float dist = sqrt(dx * dx + dy * dy);
                if(dist < ndist) {
                    ndist2 = ndist;
                    ndist = dist;
                    nidx2 = nidx;
                    nidx = j;
                }
            }

            float dnx = keypoints[nidx2].x - keypoints[nidx].x;
            float dny = keypoints[nidx2].y - keypoints[nidx].y;
            float n2n = sqrt(dnx * dnx + dny * dny);

            pdsc[i * 3] = ndist;
            pdsc[i * 3 + 1] = ndist2;
            pdsc[i * 3 + 2] = n2n;
        }
        
        tsrc.release();
    }

    int Processor::_align(const cv::Mat& src, cv::Mat& dst, const vector<cv::Point>& keypoints, const cv::Mat& descriptor) const {
        dst.release();
        if(_refKeyPoints.empty() || _refDescriptor.empty()) return 0;

        cv::BFMatcher matcher(cv::NORM_L2, true);
        vector<cv::DMatch> matches;
        matcher.match(_refDescriptor, descriptor, matches);

        int nm = matches.size();
        if(nm < 4) {
            return 0;
        }
        vector<cv::Point2f> query(nm), train(nm);
        for(int i = 0; i < nm; i++) {
            query[i] = _refKeyPoints[matches[i].queryIdx];
            train[i] = keypoints[matches[i].trainIdx];
        }
        matches.clear();

        cv::Mat hom = cv::findHomography(train, query, cv::RANSAC);
        query.clear();
        train.clear();
        if(hom.cols != 3 || hom.rows != 3) {
            hom.release();
            return 0;
        }

        cv::warpPerspective(src, dst, hom, src.size(), cv::INTER_CUBIC);
        hom.release();

        return nm;
    }

    void Processor::_addImg(cv::Mat& img, const cv::Mat& added, int nfiles) const {
        if(img.empty()) {
            if(_config.compType == COMP_AVR) img = added / (float)nfiles;
            else img = added.clone();
        }else {
            float* pimg = reinterpret_cast<float*>(img.data);
            float* padd = reinterpret_cast<float*>(added.data);
            int npx = img.size().width * img.size().height * 3;
            
            switch(_config.compType) {
                case COMP_AVR:
                    #pragma omp parallel for
                    for(int i = 0; i < npx; i++) {
                        pimg[i] += padd[i] / nfiles;
                    }
                    break;
                case COMP_MIN:
                    #pragma omp parallel for
                    for(int i = 0; i < npx; i++) {
                        pimg[i] = min(pimg[i], padd[i]);
                    }
                    break;
                case COMP_MAX:
                    #pragma omp parallel for
                    for(int i = 0; i < npx; i++) {
                        pimg[i] = max(pimg[i], padd[i]);
                    }
                    break;
                default:
                    #pragma omp parallel for
                    for(int i = 0; i < npx; i++) {
                        pimg[i] += padd[i];
                    }
                    break;
            }
        }
    }

    void Processor::_autoWb(cv::Mat& img) const {
        int npx = img.size().width * img.size().height;
        int nq = npx / 4;
        float sr = 0, sg = 0, sb = 0;

        float* pimg = reinterpret_cast<float*>(img.data);

        float* red = new float[npx];
        float* green = new float[npx];
        float* blue = new float[npx];

        #pragma omp parallel for
        for(int i = 0; i < npx; i++) {
            blue[i] = pimg[i*3];
            green[i] = pimg[i*3+1];
            red[i] = pimg[i*3+2];
        }

        sort(red, red + npx);
        sort(green, green + npx);
        sort(blue, blue + npx);

        for(int i = 0; i < nq; i++) {
            sr += red[i];
            sg += green[i];
            sb += blue[i];
        }
        free(red);
        free(green);
        free(blue);

        float cmn = min(min(sr, sg), sb);

        #pragma omp parallel for
        for(int i = 0; i < npx; i++) {
            pimg[i*3] *= cmn / sb;
            pimg[i*3+1] *= cmn / sg;
            pimg[i*3+2] *= cmn / sr;
        }
    }

    void Processor::_resize(cv::Mat& img) const {
        int rwidth = (img.size().width * _config.resizeHeight) / img.size().height;
        int uw = rwidth % 4;
        if(uw) rwidth -= uw;

        cv::Size rsz(rwidth, _config.resizeHeight);
        cv::Mat dst;

        if(_config.printVerbose) printf("      - size: %d x %d\n", rsz.width, rsz.height);
        cv::resize(img, dst, rsz, 0, 0, cv::INTER_CUBIC);
        
        img.release();
        img = dst.clone();
        dst.release();
    }

    const char* acmp_err2str(int err) {
        switch(err) {
            case ACMP_SUCCESS: return "success";
            case ACMP_ERROR_OPENFILE: return "failed to open file";
            case ACMP_ERROR_BAYERPATTERN: return "invalid bayer pattern";
            case ACMP_ERROR_IMGDEPTH: return "invalid image depth";
        }
        return "unknown error";
    }
}