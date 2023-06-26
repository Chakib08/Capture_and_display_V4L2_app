#include "v4l2Device.h"

v4l2Device::v4l2Device() : fd(-1) {
    memset(&cap, 0, sizeof(cap));
    memset(&fmt, 0, sizeof(fmt));
    memset(&req, 0, sizeof(req));
    memset(&buf, 0, sizeof(buf));
    memset(buffers, 0, sizeof(buffers));
    memset(buffer_sizes, 0, sizeof(buffer_sizes));
}

v4l2Device::~v4l2Device() {
    closeDevice();
}

bool v4l2Device::openDevice(const char* devicePath) {
    fd = open(devicePath, O_RDWR);
    if (fd < 0) {
        perror("open");
        return false;
    }
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("VIDIOC_QUERYCAP");
        closeDevice();
        return false;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "The device does not handle single-planar video capture.\n");
        closeDevice();
        return false;
    }
    return true;
}

bool v4l2Device::closeDevice() {
    if (fd != -1) {
        if (close(fd) == -1) {
            perror("close");
            return false;
        }
        fd = -1;
    }
    return true;
}

bool v4l2Device::setDeviceFormat(int width, int height) {
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    // fmt.fmt.pix.pixelformat = pixel_fmt;
    // fmt.fmt.pix.field = field;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        return false;
    }
    return true;
}

bool v4l2Device::startCapture() {
    memset(&req, 0, sizeof(req));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS");
        return false;
    }
    for (int i = 0; i < req.count; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("VIDIOC_QUERYBUF");
            return false;
        }
        buffer_sizes[i] = buf.length;
        buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i] == MAP_FAILED) {
            perror("mmap");
            return false;
        }
        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            return false;
        }
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON");
        return false;
    }
    return true;
}

bool v4l2Device::stopCapture() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        perror("VIDIOC_STREAMOFF");
        return false;
    }
    return true;
}

bool v4l2Device::dequeueBuffer(int* index) {
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
        perror("VIDIOC_DQBUF");
        return false;
    }
    *index = buf.index;
    return true;
}

bool v4l2Device::enqueueBuffer(int index) {
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;
    if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
        perror("VIDIOC_QBUF");
        return false;
    }
    return true;
}

void* v4l2Device::getBuffer(int index) {
    return buffers[index];
}

