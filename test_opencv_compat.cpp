#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>

int main() {
    try {
        cv::dnn::Net net = cv::dnn::readNetFromONNX("../models/minimal_cnn.onnx");
        std::cout << "✅ Successfully loaded Minimal CNN ONNX" << std::endl;
        
        // Create a test 32x32 grayscale patch
        cv::Mat patch = cv::Mat::zeros(32, 32, CV_8UC1);
        // Add some pattern to make it more realistic
        cv::circle(patch, cv::Point(16, 16), 8, cv::Scalar(255), -1);
        
        // Convert to float [0,1]
        cv::Mat patchF;
        patch.convertTo(patchF, CV_32F, 1.0/255.0);
        
        // Create blob [1,1,32,32]
        cv::Mat blob = cv::dnn::blobFromImage(patchF);
        std::cout << "Input blob shape: " << blob.size << std::endl;
        
        net.setInput(blob);
        cv::Mat out = net.forward();
        std::cout << "✅ Forward pass successful!" << std::endl;
        std::cout << "Output shape: " << out.size << " dims: " << out.dims << std::endl;
        std::cout << "Output type: " << out.type() << " (CV_32F=" << CV_32F << ")" << std::endl;
        
        // Test if we can reshape and use the output
        if (out.dims == 4 && out.size[2] == 1 && out.size[3] == 1) {
            cv::Mat reshaped = out.reshape(1, out.size[1]);
            std::cout << "✅ Reshaped to: " << reshaped.size << std::endl;
        } else if (out.dims == 2) {
            std::cout << "✅ Already 2D: " << out.size << std::endl;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}