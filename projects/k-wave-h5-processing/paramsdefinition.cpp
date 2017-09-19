/**
 * @file        paramsdefinition.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              19 September 2017 (updated)
 *
 * @brief       The implementation file containing ParamsDefinition class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "paramsdefinition.h"

const char *ParamsDefinition::typeStrings[] = {
    "INT",
    "LONGLONG",
    "UINT",
    "ULONGLONG",
    "FLOAT",
    "DOUBLE",
    "LONGDOUBLE",
    "STRING",
    "STRINGS_SEPARATED",
};

/**
 * @brief Defines parameter with type
 * @param[in] type Parameter type
 */
void ParamsDefinition::Flag::defineParam(ParamsDefinition::Type type)
{
    params.defineParam(type);
}

/**
 * @brief Sets parameter value with given index
 * @param[in] index Parameter index
 * @param[in] value Parameter value
 */
void ParamsDefinition::Flag::setParam(unsigned int index, void *value)
{
    params.setParam(index, value);
}

/**
 * @brief Reads parameter value
 * @param[in] index Parameter index
 * @param[out] value Parameter value
 */
void ParamsDefinition::Flag::Params::readParam(unsigned int index, void *value)
{
    Type type = types[index];
    size_t localIndex = indices[index];

    switch (type) {
        case ParamsDefinition::INT:
            static_cast<int *>(value)[0] = valuesInt[localIndex];
            break;
        case ParamsDefinition::LONGLONG:
            static_cast<long long *>(value)[0] = valuesLongLong[localIndex];
            break;
        case ParamsDefinition::UINT:
            static_cast<unsigned int *>(value)[0] = valuesUInt[localIndex];
            break;
        case ParamsDefinition::ULONGLONG:
            static_cast<unsigned long long *>(value)[0] = valuesULongLong[localIndex];
            break;
        case ParamsDefinition::FLOAT:
            static_cast<float *>(value)[0] = valuesFloat[localIndex];
            break;
        case ParamsDefinition::DOUBLE:
            static_cast<double *>(value)[0] = valuesDouble[localIndex];
            break;
        case ParamsDefinition::LONGDOUBLE:
            static_cast<long double *>(value)[0] = valuesLongDouble[localIndex];
            break;
        case ParamsDefinition::STRING:
            static_cast<std::string *>(value)[0] = valuesString[localIndex];
            break;
        case ParamsDefinition::STRINGS_SEPARATED:
            static_cast<ListOfStrings *>(value)[0] = valuesStringSeparated[localIndex];
            break;
    }
}

/**
 * @brief Returns parameter string
 * @param[in] index Parameter index
 * @return Parameter as string
 */
std::string ParamsDefinition::Flag::Params::getParamString(unsigned int index)
{
    Type type = types[index];
    size_t localIndex = indices[index];
    std::string str;

    switch (type) {
        case ParamsDefinition::INT:
            str = std::to_string(valuesInt[localIndex]);
            break;
        case ParamsDefinition::LONGLONG:
            str = std::to_string(valuesLongLong[localIndex]);
            break;
        case ParamsDefinition::UINT:
            str = std::to_string(valuesUInt[localIndex]);
            break;
        case ParamsDefinition::ULONGLONG:
            str = std::to_string(valuesULongLong[localIndex]);
            break;
        case ParamsDefinition::FLOAT:
            str = std::to_string(valuesFloat[localIndex]);
            break;
        case ParamsDefinition::DOUBLE:
            str = std::to_string(valuesDouble[localIndex]);
            break;
        case ParamsDefinition::LONGDOUBLE:
            str = std::to_string(valuesLongDouble[localIndex]);
            break;
        case ParamsDefinition::STRING:
            str = valuesString[localIndex];
            break;
        case ParamsDefinition::STRINGS_SEPARATED: {
            ParamsDefinition::ListOfStrings::const_iterator ci;
            for (ci = valuesStringSeparated[localIndex].begin(); ci != valuesStringSeparated[localIndex].end(); ++ci)
                 str.append(*ci + " ");
            break;
        }
    }
    return str;
}

/**
 * @brief Returns parameter type
 * @param[in] index Parameter index
 * @return Parameter type
 */
ParamsDefinition::Type ParamsDefinition::Flag::Params::getParamType(unsigned int index) const
{
    return types[index];
}

/**
 * @brief Returns number of parameters
 * @return Number of parameters
 */
size_t ParamsDefinition::Flag::Params::getCount() const
{
    return types.size();
}

/**
 * @brief Creates Flag object
 * @param[in] name Flag name
 */
ParamsDefinition::Flag::Flag(std::string name)
{
    this->name = name;
}

/**
 * @brief Creates Flag object with params
 * @param[in] name Flag name
 * @param[in] params Flag params
 */
ParamsDefinition::Flag::Flag(std::string name, ParamsDefinition::Flag::Params params)
{
    this->name = name;
    this->params = params;
}

/**
 * @brief Creates Params object
 */
ParamsDefinition::Flag::Params::Params()
{

}

/**
 * @brief Creates Params object with first param
 * @param[in] type Firts param type
 */
ParamsDefinition::Flag::Params::Params(ParamsDefinition::Type type)
{
    defineParam(type);
}

/**
 * @brief Defines parameter with given type
 * @param[in] type Parameter type
 */
void ParamsDefinition::Flag::Params::defineParam(ParamsDefinition::Type type)
{
    switch (type) {
        case ParamsDefinition::INT:
            valuesInt.push_back(0);
            types.push_back(ParamsDefinition::INT);
            indices.push_back(valuesInt.size() - 1);
            break;
        case ParamsDefinition::LONGLONG:
            valuesLongLong.push_back(0);
            types.push_back(ParamsDefinition::LONGLONG);
            indices.push_back(valuesLongLong.size() - 1);
            break;
        case ParamsDefinition::UINT:
            valuesUInt.push_back(0);
            types.push_back(ParamsDefinition::UINT);
            indices.push_back(valuesUInt.size() - 1);
            break;
        case ParamsDefinition::ULONGLONG:
            valuesULongLong.push_back(0);
            types.push_back(ParamsDefinition::ULONGLONG);
            indices.push_back(valuesULongLong.size() - 1);
            break;
        case ParamsDefinition::FLOAT:
            valuesFloat.push_back(0);
            types.push_back(ParamsDefinition::FLOAT);
            indices.push_back(valuesFloat.size() - 1);
            break;
        case ParamsDefinition::DOUBLE:
            valuesDouble.push_back(0);
            types.push_back(ParamsDefinition::DOUBLE);
            indices.push_back(valuesDouble.size() - 1);
            break;
        case ParamsDefinition::LONGDOUBLE:
            valuesLongDouble.push_back(0);
            types.push_back(ParamsDefinition::LONGDOUBLE);
            indices.push_back(valuesLongDouble.size() - 1);
            break;
        case ParamsDefinition::STRING:
            valuesString.push_back("");
            types.push_back(ParamsDefinition::STRING);
            indices.push_back(valuesString.size() - 1);
            break;
        case ParamsDefinition::STRINGS_SEPARATED:
            ListOfStrings emptyVector;
            valuesStringSeparated.push_back(emptyVector);
            types.push_back(ParamsDefinition::STRINGS_SEPARATED);
            indices.push_back(valuesStringSeparated.size() - 1);
            break;
    }
}

/**
 * @brief Sets parameter value with given index
 * @param[in] index Parameter index
 * @param[in] value Parameter value
 */
void ParamsDefinition::Flag::Params::setParam(unsigned int index, void *value)
{
    Type type = types[index];
    size_t localIndex = indices[index];

    switch (type) {
        case ParamsDefinition::INT:
            valuesInt[localIndex] = *static_cast<int *>(value);
            break;
        case ParamsDefinition::LONGLONG:
            valuesLongLong[localIndex] = *static_cast<long long *>(value);
            break;
        case ParamsDefinition::UINT:
            valuesUInt[localIndex] = *static_cast<unsigned int *>(value);
            break;
        case ParamsDefinition::ULONGLONG:
            valuesULongLong[localIndex] = *static_cast<unsigned long long *>(value);
            break;
        case ParamsDefinition::FLOAT:
            valuesFloat[localIndex] = *static_cast<float *>(value);
            break;
        case ParamsDefinition::DOUBLE:
            valuesDouble[localIndex] = *static_cast<double *>(value);
            break;
        case ParamsDefinition::LONGDOUBLE:
            valuesLongDouble[localIndex] = *static_cast<long double *>(value);
            break;
        case ParamsDefinition::STRING:
            valuesString[localIndex] = *static_cast<std::string *>(value);
            break;
        case ParamsDefinition::STRINGS_SEPARATED:
            valuesStringSeparated[localIndex] = *static_cast<ListOfStrings *>(value);
            break;
    }
}

/**
 * @brief Returns parameters
 * @return Parameters
 */
ParamsDefinition::Flag::Params ParamsDefinition::Flag::getParams() const
{
    return params;
}

/**
 * @brief Sets parameters
 * @param[in] params Parameters
 */
void ParamsDefinition::Flag::setParams(const ParamsDefinition::Flag::Params &params)
{
    this->params = params;
}

/**
 * @brief Is flag enabled?
 * @return True/False
 */
bool ParamsDefinition::Flag::getEnabled() const
{
    return enabled;
}

/**
 * @brief Sets enabled flag
 * @param[in] value Flag
 */
void ParamsDefinition::Flag::setEnabled(bool value)
{
    enabled = value;
}

/**
 * @brief Returns flag name
 * @return Flag name
 */
std::string ParamsDefinition::Flag::getName() const
{
    return name;
}

/**
 * @brief Defines parameters flag
 * @param[in] name Flag name
 * @param[in] params Parameters
 */
void ParamsDefinition::defineParamsFlag(std::string name, ParamsDefinition::Flag::Params params)
{
    Flag flag(name, params);
    flags.insert(FlagsPair(name, flag));
}

void ParamsDefinition::defineParamsFlag(std::string name, ParamsDefinition::Type paramsDefinition)
{
    ParamsDefinition::Flag::Params params(paramsDefinition);
    defineParamsFlag(name, params);
}

/**
 * @brief Defines parameters flag
 * @param[in] name Flag name
 */
void ParamsDefinition::defineParamsFlag(std::string name)
{
    Flag flag(name);
    flags.insert(FlagsPair(name, flag));
}

/**
 * @brief Returns flags
 * @return Flags
 */
ParamsDefinition::Flags ParamsDefinition::getFlags() const
{
    return flags;
}

int ParamsDefinition::toInt(const char *value)
{
    size_t size;
    int number = static_cast<int>(std::stoi(value, &size));
    // Throw an exception
    if (strlen(value) != size)
        throw std::invalid_argument(value);
    return number;
}

long long ParamsDefinition::toLongLong(const char *value)
{
    size_t size;
    long long number = static_cast<long long>(std::stoll(value, &size));
    // Throw an exception
    if (strlen(value) != size)
        throw std::invalid_argument(value);
    return number;
}

unsigned int ParamsDefinition::toUnsignedInt(const char *value)
{
    size_t size;
    unsigned int number = static_cast<unsigned int>(std::stoul(value, &size));
    // Throw an exception
    if (strlen(value) != size)
        throw std::invalid_argument(value);
    return number;
}

unsigned long long ParamsDefinition::toUnsignedLongLong(const char *value)
{
    size_t size;
    unsigned long long number = static_cast<unsigned long long>(std::stoull(value, &size));
    // Throw an exception
    if (strlen(value) != size)
        throw std::invalid_argument(value);
    return number;
}

float ParamsDefinition::toFloat(const char *value)
{
    size_t size;
    float number = static_cast<float>(std::stof(value, &size));
    // Throw an exception
    if (strlen(value) != size)
        throw std::invalid_argument(value);
    return number;
}

double ParamsDefinition::toDouble(const char *value)
{
    size_t size;
    double number = static_cast<double>(std::stod(value, &size));
    // Throw an exception
    if (strlen(value) != size)
        throw std::invalid_argument(value);
    return number;
}

long double ParamsDefinition::toLongDouble(const char *value)
{
    size_t size;
    long double number = static_cast<long double>(std::stold(value, &size));
    // Throw an exception
    if (strlen(value) != size)
        throw std::invalid_argument(value);
    return number;
}

std::string ParamsDefinition::toString(const char *value)
{
    return std::string(value);
}

/**
 * @brief Parses command line arguments
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 */
void ParamsDefinition::commandLineParse(int argc, char **argv)
{
    // Params parsing
    for (int i = 1; i < argc; i++) {
        std::string flagString = argv[i];
        // Remove '-'
        flagString.erase(0, 1);

        // Compare flag
        ParamsDefinition::Flags::iterator search = flags.find(flagString);
        if (search == flags.end()) {
            // Not found
            std::cout << help << std::endl;
            throw std::invalid_argument(argv[i]);
            // Throw an exception
        } else {
            // Found
            //std::string name = search->first;
            ParamsDefinition::Flag *flag = &search->second;
            flag->setEnabled(true);
            size_t c = flag->getParams().getCount();
            for (unsigned int j = 0; j < c; j++) {
                ParamsDefinition::Type type = flag->getParams().getParamType(j);
                i++;
                if (argc <= i) {
                    std::cout << help << std::endl;
                    // Throw an exception
                    throw std::invalid_argument(flag->getName());
                }
                switch (type) {
                    case ParamsDefinition::INT: {
                        int number = ParamsDefinition::toInt(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::LONGLONG: {
                        long long number = ParamsDefinition::toLongLong(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::UINT: {
                        unsigned int number = ParamsDefinition::toUnsignedInt(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::ULONGLONG: {
                        unsigned long long number = ParamsDefinition::toUnsignedLongLong(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::FLOAT: {
                        float number = ParamsDefinition::toFloat(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::DOUBLE: {
                        double number = ParamsDefinition::toDouble(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::LONGDOUBLE: {
                        long double number = ParamsDefinition::toLongDouble(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::STRING: {
                        std::string string = argv[i];
                        flag->setParam(j, static_cast<void *>(&string));
                        break;
                    }
                    case ParamsDefinition::STRINGS_SEPARATED: {
                        std::string myText(argv[i]);
                        std::istringstream iss(myText);
                        std::string token;
                        ParamsDefinition::ListOfStrings nMS;
                        while (getline(iss, token, ';')) {
                            nMS.insert(nMS.begin(), token);
                        }
                        if (nMS.size() > 0) {
                            flag->setParam(j, static_cast<void *>(&nMS));
                        }
                        break;
                    }
                }
            }
        }
    }
}

/**
 * @brief Returns help message
 * @return Help message
 */
std::string ParamsDefinition::getHelp() const
{
    return help;
}

/**
 * @brief Sets help message
 * @param[in] value Help message
 */
void ParamsDefinition::setHelp(const std::string &value)
{
    help = value;
}
