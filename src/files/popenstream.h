//! Copyright Mattias Larsson Sköld 2020

#pragma once

#include <array>
#include <cstdio>
#include <cstring>
#include <iostream>

class POpenStream : public std::iostream {
private:
    struct POpenStreamBuf : public std::streambuf {
        POpenStreamBuf(std::string command, bool captureStdErr = false);

        std::streambuf::int_type underflow() override {
            if (!pfile) {
                return std::char_traits<char>::eof();
            }

            if (fgets(buffer.data(), size, pfile)) {
                auto len = strlen(buffer.data());
                setg(buffer.data(), buffer.data(), buffer.data() + len);
                return std::char_traits<char>::to_int_type(*this->gptr());
            }
            else {
                pfile = nullptr;
                return std::char_traits<char>::eof();
            }
        }

        ~POpenStreamBuf() override;

        static constexpr size_t size = 1024;
        std::array<char, size> buffer;

        FILE *pfile;
    };

public:
    POpenStream(std::string command, bool captureStdErr = false);

    POpenStreamBuf buffer;
};
