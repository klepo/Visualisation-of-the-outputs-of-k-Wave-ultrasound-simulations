/**
 * @file        helper.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing Helper class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "helper.h"

namespace Helper
{

/// Enable debug messages flag
bool enableDebugMsgs = true;
/// Debug started flag
bool debugStarted = false;
std::stack<bool> debugFlag;

/**
 * @brief Prints debug title
 * @param[in] str Debugging string
 */
void printDebugTitle(std::string str)
{
    if (!enableDebugMsgs)
        return;
    if (debugStarted)
        printDebugMsgEnd("");
    printLine();
    std::cout << wrapMsg(std::string((63 - str.length()) / 2, ' ') + str, 0, 0);
    printLine();
}

/**
 * @brief Prints debug time
 * @param[in] ofWhat Time of what?
 * @param[in] t0 Start time
 * @param[in] t1 End time
 */
void printDebugTime(std::string ofWhat, double t0, double t1)
{
    printDebugMsgStart("Time of " + ofWhat);
    printDebugMsgEnd(std::to_string(int(t1 - t0)) + " ms");
}

/**
 * @brief Prints debug message
 * @param[in] str Debugging message
 */
void printDebugMsg(std::string str)
{
    if (!enableDebugMsgs)
        return;
    if (debugStarted)
        printDebugMsgEnd("");
    std::cout << wrapMsg(str);
}

/**
 * @brief Prints start debug message
 * @param[in] str Debugging string
 */
void printDebugMsgStart(std::string str)
{
    if (!enableDebugMsgs)
        return;
    if (debugStarted)
        printDebugMsgEnd("");
    if (51 - int(str.length()) > 0)
        std::cout << "| " << str << std::string(51 - str.length(), ' ');
    else
        std::cout << wrapMsg(str) << "| " << std::string(51, ' ');
    debugStarted = true;
}

/**
 * @brief Prints end debug message
 * @param[in] str Debugging string
 */
void printDebugMsgEnd(std::string str)
{
    if (!enableDebugMsgs)
        return;
    if (!debugStarted)
        std::cout << "| " << std::string(51, ' ');
    if (10 - int(str.length()) <= 0) {
        printDebugMsgEnd("");
        std::cout << wrapMsg(str, 0, 0, true);
    } else {
        std::cout << std::string(10 - str.length(), ' ') << str << " |" << std::endl;
    }
    debugStarted = false;
}

/**
 * @brief Prints end debug message
 * @param[in] str Debugging string
 */
void printMsgEnd(std::string str)
{
    std::cout << std::string(10 - str.length(), ' ') << str << " |" << std::endl;
    debugStarted = false;
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, std::string second, size_t indentation, size_t firstWidth,
                           size_t secondWidth, bool rightAlign)
{
    if (!enableDebugMsgs)
        return;
    if (debugStarted)
        printDebugMsgEnd("");
    size_t widthTmp1 = firstWidth;
    size_t widthTmp2 = secondWidth;
    if (widthTmp1 <= first.length())
        widthTmp1 = (size_t(first.length()) / 10) * 10 + 10;
    if (widthTmp2 <= second.length())
        widthTmp2 = (size_t(second.length()) / 10) * 10 + 10;
    std::stringstream ss;
    if (rightAlign)
        ss << std::right << std::setw(widthTmp1) << first << std::setw(widthTmp2) << second;
    else
        ss << std::left << std::setw(widthTmp1) << first << std::setw(widthTmp2) << second;
    std::cout << wrapMsg(ss.str(), indentation + 2, indentation, rightAlign);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, int second, size_t indentation, size_t firstWidth, size_t secondWidth,
                           bool rightAlign)
{
    printDebugTwoColumnsS(first, std::to_string(second), indentation, firstWidth, secondWidth, rightAlign);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, unsigned long long second, size_t indentation, size_t firstWidth,
                           size_t secondWidth, bool rightAlign)
{
    printDebugTwoColumnsS(first, std::to_string(second), indentation, firstWidth, secondWidth, rightAlign);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, float second, size_t indentation, size_t firstWidth, size_t secondWidth,
                           bool rightAlign)
{
    std::ostringstream out;
    out.precision(4);
    out << second;
    printDebugTwoColumnsS(first, out.str(), indentation, firstWidth, secondWidth, rightAlign);
}

/**
 * @brief Prints debug into two columns with 2 spaces
 * @param[in] first First string
 * @param[in] second Second string
 * @param[in] width Column width (optional)
 */
void printDebugTwoColumnsS(std::string first, double second, size_t indentation, size_t firstWidth, size_t secondWidth,
                           bool rightAlign)
{
    printDebugTwoColumnsS(first, std::to_string(second), indentation, firstWidth, secondWidth, rightAlign);
}

/**
 * @brief Prints message
 * @param[in] str String
 */
void printMsg(std::string str)
{
    if (debugStarted)
        printMsgEnd("");
    std::cout << wrapMsg(str);
}

/**
 * @brief Prints unformatted message
 * @param[in] str String
 */
void printUnformattedMsg(std::string str)
{
    std::cout << str << std::endl;
}

/**
 * @brief Prints error message
 * @param[in] str Debugging string
 */
void printErrorMsg(std::string str)
{
    if (debugStarted)
        printMsgEnd("");
    printLineErr();
    std::cerr << wrapMsg("Error: " + str);
    printLineErr();
}

void printDebugLine()
{
    if (!enableDebugMsgs)
        return;
    std::cout << "+" << std::string(63, '-') << "+" << std::endl;
}

void printLine()
{
    std::cout << "+" << std::string(63, '-') << "+" << std::endl;
}

void printLineErr()
{
    std::cerr << "+" << std::string(63, '-') << "+" << std::endl;
}

std::string wrapMsg(std::string str, size_t indentation, size_t firstIndentation, bool rightAlign)
{
    size_t s                  = 61;
    std::string strIn         = str.substr(0, (str.find_last_not_of(' ') + 1));
    std::string strOut        = "";
    size_t currentIndentation = firstIndentation;
    while (strIn.length() + currentIndentation > s) {
        size_t pos = strIn.substr(0, s - currentIndentation).find_last_of(" ");
        size_t c   = 0;
        if (pos == std::string::npos) {
            c   = 1;
            pos = strIn.substr(0, s - currentIndentation - c).find_last_of(",.-?:_\"!'/()=%;~^\\|");
            if (pos == std::string::npos) {
                pos = s - currentIndentation - c;
            }
        }
        std::string strCurrent = strIn.substr(0, pos + c);
        size_t first           = strCurrent.find_first_not_of(' ');
        size_t last            = strCurrent.find_last_not_of(' ');
        strCurrent             = strCurrent.substr(first, (last - first + 1));

        if (rightAlign)
            strOut += "| " + std::string(s - strCurrent.length() - currentIndentation, ' ') + strCurrent
                      + std::string(currentIndentation, ' ') + " |\n";
        else
            strOut += "| " + std::string(currentIndentation, ' ') + strCurrent
                      + std::string(s - strCurrent.length() - currentIndentation, ' ') + " |\n";
        strIn              = strIn.substr(pos + 1);
        currentIndentation = indentation;
    }
    // strOut += strIn + std::string(s - strIn.length() - currentIndentation, ' ') + " |\n";
    if (rightAlign)
        strOut += "| " + std::string(s - strIn.length() - currentIndentation, ' ') + strIn
                  + std::string(currentIndentation, ' ') + " |\n";
    else
        strOut += "| " + std::string(currentIndentation, ' ') + strIn
                  + std::string(s - strIn.length() - currentIndentation, ' ') + " |\n";
    return strOut;
}

/**
 * @brief Round
 * @param[in] number Float number
 * @param[in] n Number of decimal places
 * @return Rounded number
 */
float roundf(float number, int n)
{
    if (n != 0) {
        float c  = powf(10.0, float(n));
        float c1 = roundf(number * c);
        return c1 / c;
    } else {
        return (number >= 0.0f) ? floorf(number + 0.5f) : ceilf(number - 0.5f);
    }
}

/**
 * @brief Round
 * @param[in] number Float number
 * @return Rounded number
 */
unsigned long long round(float number)
{
    return static_cast<unsigned long long>(roundf(number));
}

/**
 * @brief Convert float to string with precision
 * @param[in] number Float number
 * @param[in] precision Precision
 * @return Formatted number
 */
std::string floatToString(float number, size_t precision)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << number;
    return stream.str();
}

/**
 * @brief Store last debug flag and set new debug flag
 * @param[in] flag Debug flag to set
 */
void setDebugFlagAndStoreLast(bool flag)
{
    debugFlag.push(enableDebugMsgs);
    enableDebugMsgs = flag;
}

/**
 * @brief Recover last debug flag
 */
void recoverLastDebugFlag()
{
    if (debugFlag.empty()) {
        printMsg("Empty debugFlag stack!!!");
        return;
    }
    enableDebugMsgs = debugFlag.top();
    debugFlag.pop();
}
} // namespace Helper
