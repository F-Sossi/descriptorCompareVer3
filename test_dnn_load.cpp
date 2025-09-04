#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>

int main() {
    try {
        cv::dnn::Net net = cv::dnn::readNetFromONNX("../models/hardnet.onnx");
        std::cout << "✅ Successfully loaded HardNet ONNX model" << std::endl;
        
        // Test forward pass
        cv::Mat dummy_input = cv::Mat::zeros(32, 32, CV_32F);
        cv::Mat blob = cv::dnn::blobFromImage(dummy_input);
        net.setInput(blob);
        cv::Mat out = net.forward();
        std::cout << "✅ Forward pass successful, output shape: " << out.size() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}