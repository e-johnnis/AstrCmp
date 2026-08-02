#include<astrcmp.hpp>
#include<cstdio>
#include<cstdlib>
#include<unistd.h>

using namespace std;

void showHelp();

int main(int argc, char** argv) {
    acmp::ProcessorConf config;
    int copt;
    sprintf(config.fileName, "acp_result.tif");

    if(argc < 2) {
        fprintf(stderr, "[!] no argument.\n");
        showHelp();
        return 1;
    }

    if(!strcmp(argv[1], "sum")) config.compType = acmp::COMP_SUM;
    else if(!strcmp(argv[1], "avr")) config.compType = acmp::COMP_AVR;
    else if(!strcmp(argv[1], "min")) config.compType = acmp::COMP_MIN;
    else if(!strcmp(argv[1], "max")) config.compType = acmp::COMP_MAX;
    else {
        fprintf(stderr, "[!] invalid composite type \"%s\".\n", argv[1]);
        showHelp();
        return 2;
    }

    optind = 2;
    while((copt = getopt(argc, argv, "o:war:g:t:vh")) != -1) {
        switch(copt) {
            case 'h':
                showHelp();
                return 0;
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
    proc.composite(argv + optind, argc - optind);

    return 0;
}

void showHelp() {
    printf("useage: acp sum|avr|min|max [options...] <raw_image_files...>\n");
    printf("\n");
    printf("options:\n");
    printf("  -o [string] : output file name (*.tif, default=acp_result.tif)\n");
    printf("  -w          : enable auto wb\n");
    printf("  -a          : enable alignment\n");
    printf("  -r [float]  : enable hot pixel reduction and set threshold (default=10)\n");
    printf("  -t [float]  : set star detection threshold for alignment (0-1, default=0.35)\n");
    printf("  -g [float]  : set gamma value (default=0.45)\n");
    printf("  -v          : print progress while processing\n");
    printf("  -h          : show this help and exit\n");
}