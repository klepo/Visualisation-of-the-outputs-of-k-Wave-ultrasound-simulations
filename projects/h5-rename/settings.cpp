/**
 * @file        settings.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        19 July      2017 (created) \n
 *              19 September 2017 (updated)
 *
 * @brief       The implementation file containing Settings class definition.
 *
 * @license     This file is part of the h5-rename tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "settings.h"

/**
 * @brief Creates Settings object
 */
Settings::Settings()
{

}

Settings::Settings(int argc, char **argv)
{
    loadParams(argc, argv);
}

/**
 * @brief Loads params
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 */
void Settings::loadParams(int argc, char **argv)
{
    // Define expected params
    // Modes
    paramsDefinition.defineParamsFlag("help");
    paramsDefinition.defineParamsFlag("renameObject");
    paramsDefinition.defineParamsFlag("renameAttribute");
    paramsDefinition.defineParamsFlag("changeAttributeValue");

    // Name
    paramsDefinition.defineParamsFlag("n", ParamsDefinition::STRING);

    // Value
    paramsDefinition.defineParamsFlag("v", ParamsDefinition::STRING);

    // HDF5 simulation output filename
    paramsDefinition.defineParamsFlag("f", ParamsDefinition::STRING);

    // Help message
    paramsDefinition.setHelp("\n"
                             "Usage: h5-rename [options]\n"
                             "where options include:\n\n"
                             "  -f filename ........................... Required parameter. HDF5 file.\n"
                             "\n"
                             "  -renameObject ......................... Optional parameter. Renames object.\n"
                             "\n"
                             "  -renameAttribute ...................... Optional parameter. Renames object attribute.\n"
                             "\n"
                             "  -changeAttributeValue.................. Optional parameter. Changes attribute value.\n"
                             "\n"
                             "  -n name ............................... Required parameter. Name of an object \n"
                             "                                          or attribute.\n"
                             "\n"
                             "  -v name .............. ................ Required parameter. New name of an object, \n"
                             "                                          attribute or attribute value.\n"
                             "\n"
                             "  -help ................................. Prints this help message.\n"
                             "\n");

    // Parse params from command line
    try {
        paramsDefinition.commandLineParse(argc, argv);
    } catch (std::exception &e) {
        std::cerr << "\n  Wrong parameter " << e.what() << std::endl << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Set flags according to params
    ParamsDefinition::Flags flags = paramsDefinition.getFlags();

    if (flags.at("help").getEnabled()) {
        std::cout << paramsDefinition.getHelp() << std::endl;
        exit(EXIT_SUCCESS);
    }

    setFlagRenameObject(flags.at("renameObject").getEnabled());
    setFlagRenameAttribute(flags.at("renameAttribute").getEnabled());
    setFlagChangeAttributeValue(flags.at("changeAttributeValue").getEnabled());

    if (flags.at("f").getEnabled()) {
        std::string filename;
        flags.at("f").getParams().readParam(0, &filename);
        setFilename(filename);
    }

    if (flags.at("n").getEnabled()) {
        std::string name;
        flags.at("n").getParams().readParam(0, &name);
        setName(name);
    }

    if (flags.at("v").getEnabled()) {
        std::string value;
        flags.at("v").getParams().readParam(0, &value);
        setValue(value);
    }
}

/**
 * @brief Returns filename
 * @return Filename
 */
std::string Settings::getFilename() const
{
    return filename;
}

/**
 * @brief Sets filename
 * @param[in] value Filename
 */
void Settings::setFilename(const std::string &value)
{
    filename = value;
    std::cout << "\n  Filename:\n    " << filename << std::endl;
}

/**
 * @brief Returns name
 * @return Name
 */
std::string Settings::getName() const
{
    return name;
}

/**
 * @brief Sets name
 * @param[in] value Name
 */
void Settings::setName(const std::string &value)
{
    name = value;
    std::cout << "\n  Name:\n    " << name << std::endl;
}

/**
 * @brief Returns value
 * @return Value
 */
std::string Settings::getValue() const
{
    return value;
}

/**
 * @brief Sets value
 * @param[in] value Value
 */
void Settings::setValue(const std::string &value)
{
    this->value = value;
    std::cout << "\n  Value:\n    " << this->value << std::endl;
}


/**
 * @brief Returns rename object mode flag
 * @return Rename object mode flag
 */
bool Settings::getFlagRenameObject() const
{
    return flagRenameObject;
}

/**
 * @brief Sets rename object mode flag
 * @param[in] value Rename object mode flag
 */
void Settings::setFlagRenameObject(bool value)
{
    flagRenameObject = value;
    if (value)
        std::cout << "\n  Rename object mode: ON\n" << std::endl;
    else
        std::cout << "\n  Rename object mode: OFF\n" << std::endl;
}

/**
 * @brief Returns rename attribute mode flag
 * @return Rename attribute mode flag
 */
bool Settings::getFlagRenameAttribute() const
{
    return flagRenameAttribute;
}

/**
 * @brief Sets rename attribute mode flag
 * @param[in] value Rename attribute mode flag
 */
void Settings::setFlagRenameAttribute(bool value)
{
    flagRenameAttribute = value;
    if (value)
        std::cout << "\n  Rename attribute mode: ON\n" << std::endl;
    else
        std::cout << "\n  Rename attribute mode: OFF\n" << std::endl;
}

/**
 * @brief Returns change attribute value mode flag
 * @return Change attribute value mode flag
 */
bool Settings::getFlagChangeAttributeValue() const
{
    return flagChangeAttributeValue;
}

/**
 * @brief Sets change attribute value mode flag
 * @param[in] value Change attribute value mode flag
 */
void Settings::setFlagChangeAttributeValue(bool value)
{
    flagChangeAttributeValue = value;
    if (value)
        std::cout << "\n  Change attribute value mode: ON\n" << std::endl;
    else
        std::cout << "\n  Change attribute value mode: OFF\n" << std::endl;
}

/**
 * @brief Returns params definition
 * @return Params definition
 */
ParamsDefinition Settings::getParamsDefinition() const
{
    return paramsDefinition;
}
