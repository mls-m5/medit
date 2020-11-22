//! Copyright Mattias Larsson Sköld 2020

#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

class POpenStream : public std::iostream {
private:
    struct POpenStreamBuf : public std::streambuf {
        POpenStreamBuf(std::string command,
                       bool captureStdErr = false,
                       size_t bufferSize = 1024);

        std::streambuf::int_type underflow() override;

        ~POpenStreamBuf() override;

        //        static constexpr size_t size = 1024;
        //        std::array<char, size> buffer;
        std::vector<char> buffer;

        FILE *pfile;
        int returnCode = 0;
    };

public:
    POpenStream(std::string command,
                bool captureStdErr = false,
                size_t bufferSize = 1024);

    POpenStreamBuf buffer;

    //! Returns non-zero result if there was a error during the execution of
    //! command
    int returnCode();
};
