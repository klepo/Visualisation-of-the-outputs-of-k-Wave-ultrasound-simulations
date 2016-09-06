#ifndef HELPER_H
#define HELPER_H

#include <hdf5helper.h>

namespace Helper
{

void printDebugTitle(std::string msg);
void printDebugMsg(std::string msg);
void printErrorMsg(std::string msg);
hsize_t round(double number);

}

#endif // HELPER_H
