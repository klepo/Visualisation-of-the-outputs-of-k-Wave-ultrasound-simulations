#include "helper.h"

namespace Helper
{

void printDebugTitle(std::string msg)
{
    std::cout << std::endl << std::endl << "---- " << msg << "----" << std::endl << std::endl << std::endl;
}

void printDebugMsg(std::string msg)
{
    std::cout << msg << std::endl;
}

void printErrorMsg(std::string msg)
{
    std::cerr << msg << std::endl;
}

hsize_t round(double number)
{
    return static_cast<hsize_t>(floor(number + 0.5));
}

}
