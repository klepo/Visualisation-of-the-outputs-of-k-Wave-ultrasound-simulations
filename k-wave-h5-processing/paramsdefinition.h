/**
 * @file        paramsdefinition.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The header file with ParamsDefinition class declaration.
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
    typedef enum Type
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
    } Type;

    /// Data types strings
    static const std::map<Type, std::string> typeStrings;

    /// List of strings data type
    typedef std::list<std::string> ListOfStrings;
    /// Vector of unsigned long longs
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
            void setParam(unsigned int index, const void *value);
            void readParam(unsigned int index, void *value);
            std::string getParamString(unsigned int index);
            Type getParamType(unsigned int index) const;
            size_t getCount() const;

            /**
             * @brief Operator <<
             * @param[in] os std::ostream
             * @param[in] paramsC Params
             * @return std::ostream
             */
            friend std::ostream &operator<<(std::ostream &os, Params const &paramsC)
            {
                std::string str;
                Params params = paramsC;
                str.append("count: " + std::to_string(params.getCount()));
                str.append(", values:");
                for (unsigned int i = 0; i < params.getCount(); i++) {
                    str.append(" ");
                    str.append(params.getParamString(i) + "(");
                    str.append(typeStrings.at(params.getParamType(i)));
                    str.append(")");
                }
                return os << str;
            }

        private:
            /// Types
            std::vector<Type> types;
            /// Indices
            std::vector<size_t> indices;
            /// Integer values
            std::vector<int> valuesInt;
            /// Long values
            std::vector<long> valuesLong;
            /// Long long values
            std::vector<long long> valuesLongLong;
            /// Unsigned integer values
            std::vector<unsigned int> valuesUInt;
            /// Unsigned long values
            std::vector<unsigned long> valuesUlong;
            /// Unsigned long long values
            std::vector<unsigned long long> valuesULongLong;
            /// Unsigned long long separated values
            std::vector<VectorOfULongLongs> valuesULongLongSeparated;
            /// Float values
            std::vector<float> valuesFloat;
            /// Double values
            std::vector<double> valuesDouble;
            /// Long double values
            std::vector<long double> valuesLongDouble;
            /// String values
            std::vector<std::string> valuesString;
            /// String separated values
            std::vector<ListOfStrings> valuesStringSeparated;
        };

        Flag(std::string name);
        Flag(std::string name, Params params);

        void defineParam(Type type);
        void setParam(unsigned int index, const void *value);

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
        friend std::ostream &operator<<(std::ostream &os, Flag const &flag)
        {
            return os << flag.getName();
        }

    private:
        /// Name of flag
        std::string name;
        /// Enabled flag
        bool enabled = false;
        /// Flag params
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

    void commandLineParse(int argc, const char **argv);

    std::string getHelp() const;
    void setHelp(const std::string &value);

private:
    /// Flags
    Flags flags;
    /// Help string
    std::string help;
};

#endif // PARAMSDEFINITION_H
