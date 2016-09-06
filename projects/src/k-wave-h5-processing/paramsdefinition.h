#ifndef PARAMSDEFINITION_H
#define PARAMSDEFINITION_H

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>

class ParamsDefinition
{
public:
    enum Type
    {
        INT,
        LONGLONG,
        UINT,
        ULONGLONG,
        FLOAT,
        DOUBLE,
        LONGDOUBLE,
        STRING,
        STRINGS_SEPARATED,
    };
    static const char *typeStrings[];

    typedef std::list<std::string> ListOfStrings;

    class Flag
    {
    public:
        class Params
        {
        public:
            void defineParam(Type type);
            void setParam(unsigned int index, void* value);
            void readParam(unsigned int index, void* value);
            std::string getParamString(unsigned int index);
            Type getParamType(unsigned int index) const;
            size_t getCount() const;

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

        friend std::ostream &operator<<(std::ostream &os, Flag const &flag) {
            return os << flag.getName();
        }

    private:
        std::string name;
        bool enabled = false;
        Params params;
    };

    typedef std::map<std::string, Flag> Flags;
    typedef std::pair<std::string, Flag> FlagsPair;

    void defineParamsFlag(std::string name, Flag::Params params);
    void defineParamsFlag(std::string name);

    Flags getFlags() const;

    void commandLineParse(int argc, char **argv);

    std::string getHelp() const;
    void setHelp(const std::string &value);

private:
    Flags flags;
    std::string help;

};

#endif // PARAMSDEFINITION_H
