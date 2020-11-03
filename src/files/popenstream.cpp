
#include "files/popenstream.h"
#include <iostream>

POpenStream::POpenStreamBuf::POpenStreamBuf(std::string command,
                                            bool captureStdErr)
    : pfile(popen((command + (captureStdErr ? " 2>&1" : "")).c_str(), "r")) {
    if (!pfile) {
        throw std::runtime_error("could not run command " + command);
    }
}

POpenStream::POpenStreamBuf::~POpenStreamBuf() {
    if (pfile) {
        pclose(pfile);
    }
}

POpenStream::POpenStream(std::string command, bool captureStdErr)
    : buffer(command, captureStdErr) {
    rdbuf(&buffer);
}
