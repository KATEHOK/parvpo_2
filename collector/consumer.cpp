#include <iostream>
#include "crow_all.h"
#include <vector>
#include "json.hpp"
#include <cmath>
#include <list>

std::vector<int> matrix1_demo;
std::vector<int> matrix2_demo;
int got_data = 0;

template<typename T>
std::list<T> vectorToList(const std::vector<T>& vec) {
    return std::list<T>(vec.begin(), vec.end());
}

// std::vector<int> matrixMultiply(const std::vector<int>& matrix1, const std::vector<int>& matrix2, int size) {
//     std::list<int> result(size * size, 0);



//     for (int i = 0; i < size; ++i) {
//         for (int j = 0; j < size; ++j) {
//             for (int k = 0; k < size; ++k) {
//                 result[i * size + j] += matrix1[i * size + k] * matrix2[k * size + j];
//             }
//         }
//     }
    
//     return result;
// }

std::list<int> matrixMultiply(const std::list<int>& matrix1, const std::list<int>& matrix2, int size) {
    std::list<int> result(size * size, 0);

    auto it = result.begin();

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            auto it1 = matrix1.begin();
            std::advance(it1, i * size);
            
            auto it2 = matrix2.begin();
            std::advance(it2, j);
            
            for (int k = 0; k < size; ++k) {
                *it += *it1 * *it2;
                std::advance(it1, 1);
                std::advance(it2, size);
            }
            
            ++it;
        }
    }
    
    return result;
}
bool isPerfectSquare(int num) {
    int squareRoot = std::sqrt(num);
    return squareRoot * squareRoot == num;
}

int handleProducerMessage(int num, std::vector<int> buf){
    if (num == -1){
        return 1;
    }
    else{
        buf.push_back(num);
        return 0;
    }
}

int* vectorToArray(const std::vector<int>& vec) {
    int* arr = new int[vec.size()]; // Create a new array with the same size as the vector
    
    // Copy the elements from the vector to the array
    for (size_t i = 0; i < vec.size(); i++) {
        arr[i] = vec[i];
    }
    
    return arr; // Return the pointer to the array
}

std::pair<int, int> getData(const std::string& requestContent) {
    std::string buffer;

    //std::size_t closingCurlyBracePos = requestContent.find("{");

    //std::cout << "closingCurlyBracePos: " << closingCurlyBracePos  << std::endl;

    if (1) {

        for (int i = 0; i < requestContent.length(); i++) {
            buffer += requestContent[i];
        }

        //buffer = requestContent.substr(0, closingCurlyBracePos + 1);
        //std::cout << buffer << " " << buffer.length() <<  std::endl;
    }
    else {
        return std::pair<int, int>(-1, -520);
    }

    nlohmann::json json = nlohmann::json::parse(buffer);

    int messageType = json["message_type"];
    int messageContent = json["message_content"];

    //std::cout << messageType << " " << messageContent << std::endl;

    return std::pair<int, int>(messageType, messageContent);
}

int main() {
    crow::SimpleApp app;
    crow::logger::setLogLevel(crow::LogLevel::Warning);

    CROW_ROUTE(app, "/")
    .methods("POST"_method)(
        [&](const crow::request& req, crow::response& res) {
            // Read the HTTP request
            std::string requestContent = req.body;

            //std::cout << requestContent << std::endl;
            
            // Check the type of producer
            auto tmp = getData(requestContent);
            int prod_type = tmp.first, prod_data = tmp.second;

            //std::cout << prod_type << prod_data;

            if (prod_type == 1) {
                if (prod_data == -1) {
                    got_data += 1;
                }
                else {
                    matrix1_demo.push_back(prod_data);
                }
            } 
            else if (prod_type == 2) {
                if (prod_data == -1) {
                    got_data += 1;
                }
                else {
                    matrix2_demo.push_back(prod_data);
                }
            } 
            else {
                // Unknown producer type
                // ...
                ;
            }

            // Construct an HTTP response
            std::string responseContent = std::to_string(prod_data); // Placeholder for the response content
            res.code = 200;
            res.set_header("Content-Type", "text/plain");
            res.body = responseContent;

            res.end();
        }
    );

    CROW_ROUTE(app, "/end").methods("POST"_method)(
        [&](const crow::request& req, crow::response& res) {
            got_data += 1;

            if (got_data == 2) {
                auto timer1 = std::chrono::high_resolution_clock::now();

                if (isPerfectSquare(matrix1_demo.size()) && isPerfectSquare(matrix2_demo.size())) {
                    //std::cout << "Result: " << std::endl;
                    std::list<int> result = matrixMultiply(vectorToList(matrix1_demo), vectorToList(matrix2_demo), sqrt(matrix1_demo.size()));
                    // for (int i = 0; i < result.size(); i++) {
                    //     std::cout << result[i] << " ";
                    // }
                    //std::cout << std::endl;
                }

                auto timer2 = std::chrono::high_resolution_clock::now();   

                std::chrono::duration<double, std::milli> duration = timer2 - timer1;

                std::cout << "Time: " << duration.count() << " ms" << std::endl;

                exit(0);
            }
        }
    );

    // while (true && got_data != 2) {

    //     if ((matrix1_demo.size() + matrix2_demo.size()) % 1000 == 0) {
    //         std::cout << (matrix1_demo.size() + matrix2_demo.size()) << " messages received" << std::endl;
    //     }

    //     tcp::socket socket(io);
    //     acceptor.accept(socket);

    //     // Read the HTTP request
    //     boost::asio::streambuf request;
    //     boost::asio::read_until(socket, request, "\r\n\r\n");
    //     std::string requestContent = boost::asio::buffer_cast<const char*>(request.data());
    //     //std::cout << "Received HTTP request:\n" << requestContent << std::endl << std::endl;

    //     // Check the type of producer
    //     auto tmp = getData(requestContent);
    //     int type = tmp.first, data = tmp.second;

    //     //std::cout << "Type: " << type << " Data: " << data << std::endl;

    //     if (type == 1) {
    //         if (data == -1){
    //            got_data += 1;
    //         }
    //         else{
    //             matrix1_demo.push_back(data);
    //         }
    //     } else if (type == 2) {
    //         if (data == -1){
    //            got_data += 1;
    //         }
    //         else{
    //             matrix2_demo.push_back(data);
    //         }
    //     } else {
    //         // Unknown producer type
    //         //std::cout << "Unknown reuest type: " << type << std::endl << std::endl;
    //         ;
    //     }

    //     // Construct an HTTP response
    //     std::string responseContent = std::to_string(data); // Placeholder for the response content
    //     std::string response = "HTTP/1.1 200 OK\r\n";
    //     response += "Content-Length: " + std::to_string(responseContent.length()) + "\r\n";
    //     response += "Content-Type: text/plain\r\n";
    //     response += "\r\n";
    //     response += responseContent;

    //     // Send the response back to the sender
    //     boost::asio::write(socket, boost::asio::buffer(response));
    // }

    // std::cout << "Matrix 1: " << matrix1_demo.size() << std::endl;
    // for (int i = 0; i < matrix1_demo.size(); i++) {
    //     //std::cout << matrix1_demo[i];
    // }

    // std::cout << std::endl;

    // std::cout << "Matrix 2: " << matrix2_demo.size() << std::endl;
    // for (int i = 0; i < matrix2_demo.size(); i++) {
    //     //std::cout << matrix2_demo[i];
    // }

    //std::cout << std::endl;
    //std::cout << std::endl;

    std::cout << "Starting server..." << std::endl;

    app.port(8080).multithreaded().run();

}
