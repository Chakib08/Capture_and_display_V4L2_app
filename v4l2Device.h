#ifndef V4L2DEVICE_H
#define V4L2DEVICE_H

#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ostream>
#include <iostream>
#include <cstring>

#define BUFFER_COUNT 4

class v4l2Device {
public:
    v4l2Device();
    ~v4l2Device();

    bool openDevice(const char* devicePath);
    bool closeDevice();
    bool setDeviceFormat(int width, int height);
    bool startCapture();
    bool stopCapture();
    bool dequeueBuffer(int* index);
    bool enqueueBuffer(int index);
    void* getBuffer(int index);

private:
    int fd;
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    void* buffers[BUFFER_COUNT];
    size_t buffer_sizes[BUFFER_COUNT];
};
#endif // V4L2DEVICE_H