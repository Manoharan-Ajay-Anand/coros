#include "error.h"

#include <stdexcept>
#include <string>
#include <cstring>
#include <cerrno>

void coros::base::throw_errno(int status, std::string prefix) {
    if (status == -1) {
        throw std::runtime_error(prefix.append(strerror(errno)));
    }
}
