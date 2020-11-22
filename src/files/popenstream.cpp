
#include "files/popenstream.h"
#include <iostream>

POpenStream::POpenStreamBuf::POpenStreamBuf(std::string command,
                                            bool captureStdErr,
                                            size_t bufferSize)
    : pfile(popen((command + (captureStdErr ? " 2>&1" : "")).c_str(), "r")) {
    buffer.resize(bufferSize);
    if (!pfile) {
        throw std::runtime_error("could not run command " + command);
    }
}

std::istream::int_type POpenStream::POpenStreamBuf::underflow() {
    if (!pfile) {
        return std::char_traits<char>::eof();
    }

    if (fgets(buffer.data(), buffer.size(), pfile)) {
        auto len = strlen(buffer.data());
        setg(buffer.data(), buffer.data(), buffer.data() + len);
        return std::char_traits<char>::to_int_type(*this->gptr());
    }
    else {
        returnCode = WEXITSTATUS(pclose(pfile));
        pfile = nullptr;
        return std::char_traits<char>::eof();
    }
}

POpenStream::POpenStreamBuf::~POpenStreamBuf() {
    if (pfile) {
        pclose(pfile);
    }
}

POpenStream::POpenStream(std::string command,
                         bool captureStdErr,
                         size_t bufferSize)
    : buffer(command, captureStdErr, bufferSize) {
    rdbuf(&buffer);
}

int POpenStream::returnCode() {
    return buffer.returnCode;
}
