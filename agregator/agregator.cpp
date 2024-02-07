#include <iostream>
#include "crow_all.h"
#include <cpr/cpr.h>
#include <vector>
#include "json.hpp"
#include <cmath>
#include <unistd.h>

// agregator(debian_12<c++>:81):
// 	1) Прием сообщений { producer_number, number } пока number - неотрицательное:
// 		Добавление number в матрицу, соответствующую producer_number (1 -> a; 2 -> b)
// 	2) Вывод в file_input.txt матриц
// 	3) Отправка на collector текущего времени time_start
// 	4) Формирование блоков для перемножения и отправка их на balancer пока существуют неотправленные блоки
// 	5) Завершение работы

struct Block {
    std::vector<std::vector<int>> block;    // Элементы
    int row_id;                             // Индекс первой строки из большой матрицы
    int col_id;                             // Индекс первого столбца из большой матрицы
    int matrix_id;                          // Индекс матрицы
};

std::vector<int> matrix1_demo;
std::vector<int> matrix2_demo;
std::vector<std::vector<int>> matrix1;
std::vector<std::vector<int>> matrix2;
int got_data = 0;

std::vector<struct Block> separate_by_blocks(int p, int matr_id, std::vector<std::vector<int>> matrix)
{
    int size = matrix.size();
    int block_size = int((double)size / sqrt(p));
    std::vector<struct Block> blocks;

    for (int i = 0; i < size; i += block_size) {
        // блочные строки
        for (int j = 0; j < size; j += block_size) {
            // блочные столбцы
            struct Block my_block;
            for (int row_id = i; row_id < i + block_size && row_id < size; ++row_id) {
                std::vector<int> row;
                for (int col_id = j; col_id < j + block_size && col_id < size; ++col_id) row.push_back(matrix[row_id][col_id]);
                my_block.block.push_back(row);
            }
            my_block.row_id = i;
            my_block.col_id = j;
            my_block.matrix_id = matr_id;
            blocks.push_back(my_block);
        }
    }

    return blocks;
}

void convert_matrix(std::vector<int>& demo, std::vector<std::vector<int>>& matrix) {
    int size = std::sqrt(demo.size());
    for (int i = 0; i < size; ++i) {
        std::vector<int> row;
        for (int j = 0; j < size; ++j) row.push_back(demo[size * i + j]);
        matrix.push_back(row);
    }
}

int output_matrix(std::vector<std::vector<int>>& matrix, char* filename, char mode) {
    FILE* file = fopen(filename, &mode);
    if (file == NULL) {
        perror("Error with openning file_input.txt");
        return 1;
    }

    for (auto row : matrix) {
        for (auto item : row)
            if (fprintf(file, "%d ", item) < 0) {
                perror("Error with writing into file_input.txt");
                fclose(file);
                return 1;
            }
        if (fprintf(file, "\n") < 0) {
            perror("Error with writing into file_input.txt");
            fclose(file);
            return 1;
        }
    }
    if (fprintf(file, "\n") < 0) {
        perror("Error with writing into file_input.txt");
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

bool isPerfectSquare(int num) {
    int squareRoot = std::sqrt(num);
    return squareRoot * squareRoot == num;
}

std::pair<int, int> getData(const std::string& requestContent) {
    std::string buffer;
    if (1) {
        for (int i = 0; i < requestContent.length(); i++) {
            buffer += requestContent[i];
        }
    }
    else {
        return std::pair<int, int>(-1, -520);
    }

    nlohmann::json json = nlohmann::json::parse(buffer);

    int messageType = json["message_type"];
    int messageContent = json["message_content"];

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
            
            // Check the type of producer
            auto tmp = getData(requestContent);
            int prod_type = tmp.first, prod_data = tmp.second;

            if (prod_type == 1) matrix1_demo.push_back(prod_data);
            else if (prod_type == 2) matrix2_demo.push_back(prod_data);

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
            
            // Construct an HTTP response
            std::string responseContent = "-1";
            res.code = 200;
            res.set_header("Content-Type", "text/plain");
            res.body = responseContent;
            res.end();
            
            ++got_data;
            if (got_data == 2) {

                if (!isPerfectSquare(matrix1_demo.size()) || !isPerfectSquare(matrix2_demo.size())) {
                    std::cout << "Wrong data! Required square matrixes!" << std::endl;
                    exit(0);
                }

                convert_matrix(matrix1_demo, matrix1);
                convert_matrix(matrix2_demo, matrix2);

                char filename[] = "file_input.txt";
                if (output_matrix(matrix1, filename, 'w') + output_matrix(matrix2, filename, 'a') != 0) {
                    std::cout << "file_input.txt has incorrect data" << std::endl;
                }

                auto time = std::chrono::system_clock::now();
                auto duration = time.time_since_epoch();
                auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                nlohmann::json jsonTime;
                jsonTime["1_point_milliseconds_since_epoch"] = millis;

                while (1 == 1) {
                    cpr::Response r = cpr::Post(
                    cpr::Url{"http://collector:84/time"},
                    cpr::Body{jsonTime.dump()},
                    cpr::Header{{"Content-Type", "application/json"}}
                    );

                    if (r.status_code == 200) exit(0);
                    std::cout << "Failed to send the JSON data to collector/time. Status code: " << r.status_code << std::endl;

                    sleep(1);
                }

                std::vector<struct Block> blocks_1 = separate_by_blocks(2, 1, matrix_1);
                std::vector<struct Block> blocks_2 = separate_by_blocks(2, 2, matrix_2);

                for (auto block_1 : blocks_1) {
                    // ищем оппонента из blocks_2
                    // преобразуем в json
                    // отправляем на nginx:8080
                }
            }
        }
    );

    std::cout << "Starting server..." << std::endl;
    app.port(81).multithreaded().run();
    return 0;
}
