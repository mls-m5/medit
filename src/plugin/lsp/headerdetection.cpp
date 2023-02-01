#include "headerdetection.h"
// #include "files/popenstream.h"
// #include <cstdlib>
// #include <sstream>

// std::vector<std::string> detectStandardHeaders(std::string compiler) {
//     auto stream = POpenStream{compiler + " -E -xc++ -v /dev/null 2>&1"};

//    for (std::string line; std::getline(stream, line);) {
//        if (line.find("#include <...> search starts here:") !=
//            std::string::npos) {
//            break;
//        }
//    }

//    auto ret = std::vector<std::string>{};

//    for (std::string line; std::getline(stream, line);) {
//        if (line.empty()) {
//            continue;
//        }
//        if (!std::isspace(line.front())) {
//            break;
//        }
//        while (!line.empty() && std::isspace(line.front())) {
//            line.erase(0, 1);
//        }

//        ret.push_back(line);
//    }

//    return ret;
//}

// void setStandardHeaders(std::string sourceCompiler) {
//     auto headers = detectStandardHeaders(sourceCompiler);

//    auto ss = std::ostringstream{};
//    //    ss << "CPLUS_INCLUDE_PATH=";

//    for (auto &header : headers) {
//        ss << header << ":";
//    }

//    auto str = ss.str();
//    str.pop_back();

//    //    putenv(str.data()); // Note: Non const pointer argument
//    setenv("CPLUS_INCLUDE_PATH", str.data(), 1);
//    std::system("echo $CPLUS_INCLUDE_PATH");
//}
