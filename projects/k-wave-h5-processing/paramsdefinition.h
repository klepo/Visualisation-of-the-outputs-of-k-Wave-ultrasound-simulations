/**
 * @file        paramsdefinition.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              19 September 2017 (updated)
 *
 * @brief       The header file with ParamsDefinition class declaration.
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

#ifndef PARAMSDEFINITION_H
#define PARAMSDEFINITION_H

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <cstring>

/**
 * @brief The ParamsDefinition class represents wrapper for parameters definitions
 */
class ParamsDefinition
{
public:
    /// Parameters data types
    enum Type
    {
        INT,
        LONGLONG,
        UINT,
        ULONGLONG,
        ULONGLONG_SEPARATED,
        FLOAT,
        DOUBLE,
        LONGDOUBLE,
        STRING,
        STRINGS_SEPARATED,
    };
    /// Data types strings
    static const char *typeStrings[];

    /// List of strings data type
    typedef std::list<std::string> ListOfStrings;
    typedef std::vector<unsigned long long> VectorOfULongLongs;

    /**
     * @brief The Flag class represents wrapper for processing flag
     */
    class Flag
    {
    public:
        /**
         * @brief The Params class represents wrapper for processing params
         */
        class Params
        {
        public:
            Params();
            Params(Type type);
            void defineParam(Type type);
            void setParam(unsigned int index, void* value);
            void readParam(unsigned int index, void* value);
            std::string getParamString(unsigned int index);
            Type getParamType(unsigned int index) const;
            size_t getCount() const;

            /**
             * @brief Operator <<
             * @param[in] os std::ostream
             * @param[in] paramsC Params
             * @return std::ostream
             */
            friend std::ostream &operator<<(std::ostream &os, Params const &paramsC) {
                std::string str;
                Params params = paramsC;
                str.append("count: " + std::to_string(params.getCount()));
                str.append(", values:");
                for (unsigned int i = 0; i < params.getCount(); i++) {
                    str.append(" ");
                    str.append(params.getParamString(i) + "(");
                    str.append(typeStrings[params.getParamType(i)]);
                    str.append(")");
                }
                return os << str;
            }

        private:
            std::vector<Type> types;
            std::vector<size_t> indices;
            std::vector<int> valuesInt;
            std::vector<long> valuesLong;
            std::vector<long long> valuesLongLong;
            std::vector<unsigned int> valuesUInt;
            std::vector<unsigned long> valuesUlong;
            std::vector<unsigned long long> valuesULongLong;
            std::vector<VectorOfULongLongs> valuesULongLongSeparated;
            std::vector<float> valuesFloat;
            std::vector<double> valuesDouble;
            std::vector<long double> valuesLongDouble;
            std::vector<std::string> valuesString;
            std::vector<ListOfStrings> valuesStringSeparated;
        };

        Flag(std::string name);
        Flag(std::string name, Params params);

        void defineParam(Type type);
        void setParam(unsigned int index, void* value);

        Params getParams() const;
        void setParams(const Params &params);

        bool getEnabled() const;
        void setEnabled(bool value);

        std::string getName() const;

        /**
         * @brief Operator <<
         * @param[in] os std::ostream
         * @param[in] flag Flag
         * @return std::ostream
         */
        friend std::ostream &operator<<(std::ostream &os, Flag const &flag) {
            return os << flag.getName();
        }

    private:
        std::string name;
        bool enabled = false;
        Params params;
    };

    /// Flags data type
    typedef std::map<std::string, Flag> Flags;
    /// Flags pair data type
    typedef std::pair<std::string, Flag> FlagsPair;

    void defineParamsFlag(std::string name, Flag::Params params);
    void defineParamsFlag(std::string name, Type paramsDefinition);
    void defineParamsFlag(std::string name);

    Flags getFlags() const;

    static int toInt(const char *value);
    static long long toLongLong(const char *value);
    static unsigned int toUnsignedInt(const char *value);
    static unsigned long long toUnsignedLongLong(const char *value);
    static float toFloat(const char *value);
    static double toDouble(const char *value);
    static long double toLongDouble(const char *value);
    static std::string toString(const char *value);

    void commandLineParse(int argc, char **argv);

    std::string getHelp() const;
    void setHelp(const std::string &value);

private:
    Flags flags;
    std::string help;

};

#endif // PARAMSDEFINITION_H
