# AstrCmp
Composite Processor for Astrnomical Raw Images

## Installation

``` bash
# Install tools
sudo apt install git g++

# Install dependencies
sudo apt install libopencv-dev libraw-dev

# Clone, build and install repository
git clone https://github.com/e-johnnis/AstrCmp
cd AstrCmp
make build
sudo make install

# Check installation: help will be shown if installed successfully
acmp
```

## Useage

Output image will be:

- named "acmp_result.tif" (configurable with '-o' option)
- sized with the half of original height / width
- formatted 32-bit float TIFF

In this example, you may have Canon RAW images (``.CR2``) in the directory ``raw-images``.

``` bash
# Recommended: composite raw images with star-detection alignment, auto white balace and hot pixel reduction
acmp avr -w -a -r 10 raw-images/*.CR2

# Configured output file name
acmp avr -o rawimg.tif -w -a -r 10 raw-images/*.CR2

# Star trails
acmp max -o trails.tif -w -r 10 raw-imgages/*.CR2

# Resize output
acmp avr -w -a -r 10 -h 1080 raw-images/*.CR2

# Timelapse mode (default file name = "acmp_result.avi")
acmp tl -w -r 10 -h 1080 raw-images/*.CR2
```