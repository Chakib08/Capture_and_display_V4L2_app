# Capture_and_display_V4L2_app
An app to capture frames from a camera using V4L2 and display using OpenCV

## Prerequisites

You need to install OpenCV to be able to compile the application without having errors :

1. Install the required packages

```
$ sudo apt-get update
$ sudo apt-get install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
```
2. Clone OpenCV repository

`$ git clone https://github.com/opencv/opencv.git`


3. Build and install OpenCV
```
$ cd opencv
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
$ make -j8
$ sudo make install
```

4. Check OpenCV version
 
`$ pkg-config --modversion opencv4`


## Running the sample
### Command line (without using CMake)

Run the commands below

```
$ g++ -std=c++11 -Wall -I/usr/lib/opencv -I/usr/include/opencv4 v4l_capture.cpp -L/usr/lib -lopencv_core -lopencv_highgui -lopencv_videoio -o camera_cap
$ ./camera_cap
```

### Using Cmake

Run the commands below

```
$ cd Capture_and_display_V4L2_app
$ mkdir build && cd build
$ cmake ..
$ sudo make
$ ./Capture_and_display_V4L2_app
```

