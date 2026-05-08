#include <chrono>
#include <iostream>
#include <string>


#include "./algorithm/Partition.hpp"
#include "./utils/DataProcess.hpp"
#include "./utils/SpanningForest.hpp"
#include "./utils/Timer.hpp"
#include "./algorithm/ComputeResult.hpp"

std::chrono::time_point<std::chrono::high_resolution_clock> GlobalTimer::start_time;
double GlobalTimer::limit_seconds = 3600.0;

int main(int argc, char* argv[]) {
    string alg, strInputFile, strOutputFile;
    
    if (argc < 3) {
        throw std::runtime_error("Not enough parameters have been passed. \n");
    } else {
        strInputFile = argv[1];
        strOutputFile = argv[2];
        alg = argv[3];
    }

    // strInputFile="../dataset/P_set/p04.txt";
    // alg = "Match";
    // strOutputFile = "../output_p04.txt";
    std::ofstream file(strOutputFile);
    if (!file) {
        throw std::runtime_error("open file error\n");
    }

    std::streambuf* original_cout_buffer = std::cout.rdbuf();

    std::cout.rdbuf(file.rdbuf());

    try {

        GlobalTimer::start(3600.0);

        if (strInputFile.find("P_set") != string::npos) {
            read_P_set_data(strInputFile);
        } else if (strInputFile.find("SD_set") != string::npos) {
            read_SD_set_data(strInputFile);
        }else if(strInputFile.find("G_set") != string::npos) {
            read_Generate_data(strInputFile);
        }
        else {
            throw runtime_error("input file error!\n");
        }

        auto start = std::chrono::high_resolution_clock::now();

        spanningForest();
        getPartitions();

        if (alg == "Match") {
            computeResultByMinCostPerfectMatching();
        } else if (alg == "GW") {
            computeResultByGW();
        } else {
            throw runtime_error("alg input error\n");
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "The running time is : " << duration.count() << " s" << std::endl;

    } catch (const std::exception& e) {
        std::cout.rdbuf(original_cout_buffer);
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout.rdbuf(original_cout_buffer);
    return 0;
}