#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define BUFFER_COUNT 4

int main(int argc, char **argv)
{
    int fd, i, ret;
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    void *buffers[BUFFER_COUNT];
    size_t buffer_sizes[BUFFER_COUNT];

    // Check if video channel, frame rate and convert from JPEG flag are set
    if (argc != 2)
    {
        std::cout << "Error: No video node passes" << std::endl;
        return 1;
    }

    fd = open(argv[1], O_RDWR); //|O_NONBLOCK );
    if (fd < 0)
    {
        std::cout << "unable to open" << argv[1] << std::endl;
        return 1;
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("VIDIOC_QUERYCAP");
        return 1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "The device does not handle single-planar video capture.\n");
        return 1;
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 3848;
    fmt.fmt.pix.height = 2168;
    //fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        return 1;
    }

    memset(&req, 0, sizeof(req));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS");
        return 1;
    }

    for (i = 0; i < BUFFER_COUNT; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("VIDIOC_QUERYBUF");
            return 1;
        }
        buffer_sizes[i] = buf.length;
        buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i] == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            return 1;
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON");
        return 1;
    }

    cv::namedWindow("Camera Stream", cv::WINDOW_NORMAL);
    cv::resizeWindow("Camera Stream", 3848, 2168);

    while (true) 
    {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(fd + 1, &fds, NULL, NULL, &tv);

        if (r == -1) {
            perror("select");
            return 1;
        }

        if (r == 0) {
            fprintf(stderr, "select timeout\n");
            return 1;
        }

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            perror("VIDIOC_DQBUF");
            return 1;
        }

        cv::Mat rggb12_image(2168, 3848, CV_16UC1, buffers[buf.index]);
        //rggb12_image.convertTo(rggb12_image, CV_8UC3, 255.0/4095, 0);
        cv::Mat bgr_image(2168, 3848, CV_8UC3);

        // Check if the image is loaded successfully
        if (rggb12_image.empty()) {
            std::cout << "Could not open or find the image" << std::endl;
            return -1;
        }

        cv::cvtColor(rggb12_image, bgr_image, cv::COLOR_BayerGR2BGR);        
        cv::imshow("Camera Stream", bgr_image);
        cv::waitKey(1);

        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) 
        {
            perror("VIDIOC_QBUF");
            return 1;
        }
    }   
}