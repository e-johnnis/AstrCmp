#include<astrcmp.hpp>
#include<cstdio>
#include<cstdlib>
#include<unistd.h>

using namespace std;

void showHelp();

int main(int argc, char** argv) {
    acmp::ProcessorConf config;
    int copt;
    sprintf(config.fileName, "acmp_result.tif");

    if(argc < 2) {
        printf("*** AstrCmp ver.%s ***\n", ACMP_VERSION);
        printf("\n");
        showHelp();
        return 0;
    }

    if(!strcmp(argv[optind], "sum")) config.compType = acmp::COMP_SUM;
    else if(!strcmp(argv[optind], "avr")) config.compType = acmp::COMP_AVR;
    else if(!strcmp(argv[optind], "min")) config.compType = acmp::COMP_MIN;
    else if(!strcmp(argv[optind], "max")) config.compType = acmp::COMP_MAX;
    else if(!strcmp(argv[optind], "timelapse") || !strcmp(argv[optind], "tl")) {
        config.compType = acmp::COMP_TIMELAPSE;
        sprintf(config.fileName, "acmp_result.webm");
    }else {
        fprintf(stderr, "[!] invalid composite type \"%s\".\n", argv[optind]);
        showHelp();
        return 2;
    }
    optind++;

    while((copt = getopt(argc, argv, "o:war:g:t:h:f:v")) != -1) {
        switch(copt) {
            case 'o':
                strcpy(config.fileName, optarg);
                break;
            case 'w':
                config.autoWb = 1;
                break;
            case 'a':
                config.align = 1;
                break;
            case 'r':
                if(!sscanf(optarg, "%f", &config.hprThresh)) {
                    fprintf(stderr, "[!] invalid value for key 'r': \"%s\"\n", optarg);
                    return 3;
                }
                break;
            case 'g':
                if(!sscanf(optarg, "%f", &config.gamma)) {
                    fprintf(stderr, "[!] invalid value for key 'g': \"%s\"\n", optarg);
                    return 3;
                }
                break;
            case 't':
                if(!sscanf(optarg, "%f", &config.starThresh)) {
                    fprintf(stderr, "[!] invalid value for key 't': \"%s\"\n", optarg);
                    return 3;
                }
                break;
            case 'h':
                if(!sscanf(optarg, "%d", &config.resizeHeight)) {
                    fprintf(stderr, "[!] invalid value for key 'h': \"%s\"\n", optarg);
                    return 3;
                }
                break;
            case 'f':
                if(!sscanf(optarg, "%d", &config.fps)) {
                    fprintf(stderr, "[!] invalid value for key 'f': \"%s\"\n", optarg);
                    return 3;
                }else if(config.compType != acmp::COMP_TIMELAPSE) {
                    fprintf(stderr, "[?] fps is available only in timelapse mode.\n");
                }
                break;
            case 'v':
                config.printVerbose = 1;
                break;
            case '?':
                fprintf(stderr, "[!] invalid option '%c'.\n", optopt);
                showHelp();
                return 3;
        }
    }

    acmp::Processor proc(&config);
    if(config.compType == acmp::COMP_TIMELAPSE) proc.comasew(argv + optind, argc - optind);
    else proc.composite(argv + optind, argc - optind);

    return 0;
}

void showHelp() {
    printf("useage         : acmp sum|avr|min|max [options...] <raw_image_files...>\n");
    printf("timelapse mode : acmp timelapse|tl [options...] <raw_image_files...>\n");
    printf("\n");
    printf("options:\n");
    printf("  -o [string] : output file name (*.tif, default=acmp_result.tif)\n");
    printf("                default=acmp_result.webm in timelapse mode\n");
    printf("  -w          : enable auto wb\n");
    printf("  -a          : enable alignment\n");
    printf("  -r [float]  : enable hot pixel reduction and set threshold (recommended=10)\n");
    printf("  -t [float]  : set star detection threshold for alignment (0-1, default=0.35)\n");
    printf("  -g [float]  : set gamma value (default=0.45)\n");
    printf("  -h [int]    : enable resize and set height pixels\n");
    printf("  -f [int]    : set fps (timelapse mode only, default=15)\n");
    printf("  -v          : print progress while processing\n");
    printf("\n");
}