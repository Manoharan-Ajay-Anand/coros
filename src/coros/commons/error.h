#ifndef COMMONS_ERROR_H
#define COMMONS_ERROR_H

#include <string>

namespace coros::base {
    void throw_errno(int status, std::string prefix);
}


#endif
