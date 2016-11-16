/**
 * @file        paramsdefinition.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing definition of class for parameters loading.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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

ParamsDefinition::Flag::Flag(std::string name)
{
    this->name = name;
}

ParamsDefinition::Flag::Flag(std::string name, ParamsDefinition::Flag::Params params)
{
    this->name = name;
    this->params = params;
}

void ParamsDefinition::Flag::defineParam(ParamsDefinition::Type type)
{
    params.defineParam(type);
}

void ParamsDefinition::Flag::setParam(unsigned int index, void *value)
{
    params.setParam(index, value);
}

ParamsDefinition::Flag::Params ParamsDefinition::Flag::getParams() const
{
    return params;
}

void ParamsDefinition::Flag::setParams(const ParamsDefinition::Flag::Params &params)
{
    this->params = params;
}

bool ParamsDefinition::Flag::getEnabled() const
{
    return enabled;
}

void ParamsDefinition::Flag::setEnabled(bool value)
{
    enabled = value;
}

std::string ParamsDefinition::Flag::getName() const
{
    return name;
}

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

ParamsDefinition::Type ParamsDefinition::Flag::Params::getParamType(unsigned int index) const
{
    return types[index];
}

size_t ParamsDefinition::Flag::Params::getCount() const
{
    return types.size();
}

void ParamsDefinition::defineParamsFlag(std::string name, ParamsDefinition::Flag::Params params)
{
    Flag flag(name, params);
    flags.insert(FlagsPair(name, flag));
}

void ParamsDefinition::defineParamsFlag(std::string name)
{
    Flag flag(name);
    flags.insert(FlagsPair(name, flag));
}

ParamsDefinition::Flags ParamsDefinition::getFlags() const
{
    return flags;
}

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
                    throw std::invalid_argument(flag->getName());
                    break;
                    // Throw an exception
                }
                size_t size;
                switch (type) {
                    case ParamsDefinition::INT: {
                        int number = static_cast<int>(std::stoi(argv[i], &size));
                        // Throw an exception
                        if (strlen(argv[i]) != size)
                            throw std::invalid_argument(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::LONGLONG: {
                        long long number = static_cast<long long>(std::stoll(argv[i], &size));
                        // Throw an exception
                        if (strlen(argv[i]) != size)
                            throw std::invalid_argument(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::UINT: {
                        unsigned int number = static_cast<unsigned int>(std::stoul(argv[i], &size));
                        // Throw an exception
                        if (strlen(argv[i]) != size)
                            throw std::invalid_argument(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::ULONGLONG: {
                        unsigned long long number = static_cast<unsigned long long>(std::stoull(argv[i], &size));
                        // Throw an exception
                        if (strlen(argv[i]) != size)
                            throw std::invalid_argument(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::FLOAT: {
                        float number = static_cast<float>(std::stof(argv[i], &size));
                        // Throw an exception
                        if (strlen(argv[i]) != size)
                            throw std::invalid_argument(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::DOUBLE: {
                        double number = static_cast<double>(std::stod(argv[i], &size));
                        // Throw an exception
                        if (strlen(argv[i]) != size)
                            throw std::invalid_argument(argv[i]);
                        flag->setParam(j, static_cast<void *>(&number));
                        break;
                    }
                    case ParamsDefinition::LONGDOUBLE: {
                        long double number = static_cast<long double>(std::stold(argv[i], &size));
                        // Throw an exception
                        if (strlen(argv[i]) != size)
                            throw std::invalid_argument(argv[i]);
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

std::string ParamsDefinition::getHelp() const
{
    return help;
}

void ParamsDefinition::setHelp(const std::string &value)
{
    help = value;
}
