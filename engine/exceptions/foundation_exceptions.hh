#pragma once
#include <stdexcept>


namespace Tufan::Exceptions
{
    class ContainerException : public std::runtime_error
    {
        public:
            explicit ContainerException(const std::string &msg): std::runtime_error("ETA: Container" + msg){}
    };
}
