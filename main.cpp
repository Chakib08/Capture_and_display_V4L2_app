#include <opencv2/opencv.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaimgproc.hpp>

#include "v4l2Device.h"
#include <opencv2/opencv.hpp>

#define WIDTH 3848
#define HEIGHT 2168

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Error: No video node passed" << std::endl;
        return 1;
    }

    const char* devicePath = argv[1];

    // Create an instance of v4l2Device
    v4l2Device device;

    // Open the device
    if (!device.openDevice(devicePath)) {
        return 1; // Failed to open device
    }

    // Set device format
    int width = WIDTH;
    int height = HEIGHT;
    if (!device.setDeviceFormat(width, height)) {
        device.closeDevice();
        return 1; // Failed to set device format
    }

    // Start capture
    if (!device.startCapture()) {
        device.closeDevice();
        return 1; // Failed to start capture
    }

    // Create OpenCV windows for displaying the captured frames
    cv::namedWindow("Camera Stream", cv::WINDOW_NORMAL);
    cv::resizeWindow("Camera Stream", WIDTH, HEIGHT);

    // Capture loop
    while (true) {
        int index;
        if (!device.dequeueBuffer(&index)) {
            device.stopCapture();
            device.closeDevice();
            return 1; // Failed to dequeue buffer
        }

        // Process the captured frame
        cv::Mat rggb12_image(HEIGHT, WIDTH, CV_16UC1, device.getBuffer(index));
        cv::Mat bgr_image(HEIGHT, WIDTH, CV_8UC3);

        // Check if the image is loaded successfully
        if (rggb12_image.empty()) {
            std::cout << "Could not open or find the image" << std::endl;
            device.stopCapture();
            device.closeDevice();
            return -1;
        }
        cv::cvtColor(rggb12_image, bgr_image, cv::COLOR_BayerGR2BGR);

        // Display the processed frame
        cv::imshow("Camera Stream", bgr_image);

        // Requeue the buffer
        if (!device.enqueueBuffer(index)) {
            device.stopCapture();
            device.closeDevice();
            return 1; // Failed to enqueue buffer
        }

        // Wait for a key press to exit the loop
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    // Stop capture
    if (!device.stopCapture()) {
        device.closeDevice();
        return 1; // Failed to stop capture
    }

    // Close the device
    if (!device.closeDevice()) {
        return 1; // Failed to close device
    }

    return 0;
}

