/**
 * @file        settings.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        19 July      2017 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with Settings class declaration.
 *
 * @license     This file is part of the h5-rename tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <list>
#include <iomanip>

#include "../k-wave-h5-processing/paramsdefinition.h"

/**
 * @brief The Settings class represents wrapper for processing settings
 */
class Settings
{
public:
    Settings();
    Settings(int argc, const char **argv);
    void loadParams(int argc, const char **argv);

    // Simulation data files
    std::string getFilename() const;
    void setFilename(const std::string &value);

    // Sizes
    std::string getName() const;
    void setName(const std::string &value);
    std::string getValue() const;
    void setValue(const std::string &value);

    // Application modes
    bool getFlagRenameObject() const;
    void setFlagRenameObject(bool value);
    bool getFlagRenameAttribute() const;
    void setFlagRenameAttribute(bool value);
    bool getFlagChangeAttributeValue() const;
    void setFlagChangeAttributeValue(bool value);

    ParamsDefinition getParamsDefinition() const;

private:
    // Filenames
    std::string filename = "";

    // Vars
    std::string name = "";
    std::string value = "";

    // Application modes
    bool flagRenameObject = false;
    bool flagRenameAttribute = false;
    bool flagChangeAttributeValue = false;

    // Params definition
    ParamsDefinition paramsDefinition;
};

#endif // SETTINGS_H
