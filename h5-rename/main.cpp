/**
 * @file        main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        19 July      2017 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The main implementation file containing h5-rename application.
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

#include <k-wave-h5-helper.h>

#include "settings.h"

//#include "../k-wave-h5-processing/helper.h"

/**
 * @brief Main k-Wave processing function
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 * @return EXIT_SUCCESS
 */
int main(int argc, const char **argv)
{
    Settings *settings = new Settings(argc, argv);
    H5Helper::File *file = nullptr;

    Helper::printDebugTitle("Loading file");

    // Load simulation output file
    if (!settings->getFilename().empty()) {
        try {
            file = new H5Helper::File(settings->getFilename(), H5Helper::File::OPEN);
        } catch (std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else {
        Helper::printErrorMsg("Missing parameter -f (simulation output filename)");
        Helper::printMsg(settings->getParamsDefinition().getHelp());
        exit(EXIT_FAILURE);
    }

    // Rename object
    if (settings->getFlagRenameObject()) {
        if (settings->getName().empty()) {
            Helper::printErrorMsg("Missing parameter -n (object name)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        if (settings->getValue().empty()) {
            Helper::printErrorMsg("Missing parameter -v (new object name)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        if (file->objExistsByName(settings->getName())) {
            try {
                file->objRename(settings->getName(), settings->getValue());
                Helper::printDebugMsg("Rename object ... OK");
            } catch (std::exception &e) {
                Helper::printErrorMsg(e.what());
                Helper::printDebugMsg("Rename object ... error");
                std::exit(EXIT_FAILURE);
            }
        } else {
            Helper::printErrorMsg("Wrong parameter -n (object with given name does not exist)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }
    }
    // Rename attribute
    else if (settings->getFlagRenameAttribute()) {
        if (settings->getName().empty()) {
            Helper::printErrorMsg("Missing parameter -n (attribute name)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        if (settings->getValue().empty()) {
            Helper::printErrorMsg("Missing parameter -v (new attribute name)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        std::string delimiter = "/";

        std::string s = settings->getName();
        std::string srcAttributeName = s.substr(s.rfind(delimiter) + 1);
        std::string srcObjectName = s.substr(0, s.rfind(delimiter));
        if (srcObjectName == srcAttributeName || srcObjectName.empty())
            srcObjectName = "/";
        if (srcAttributeName.empty()) {
            Helper::printErrorMsg("Wrong parameter -n (given attribute does not exist)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        std::string d = settings->getValue();
        std::string dstAttributeName = d.substr(d.rfind(delimiter) + 1);
        std::string dstObjectName = d.substr(0, d.rfind(delimiter));
        if (dstObjectName == dstAttributeName|| dstObjectName.empty())
            dstObjectName = "/";
        if (dstAttributeName.empty()) {
            Helper::printErrorMsg("Wrong parameter -v (missing destination attribute)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        if (srcObjectName != dstObjectName) {
            Helper::printErrorMsg("Wrong parameter -n or -v (src object path != dst object path)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        try {
            file->renameAttribute(srcAttributeName, dstAttributeName, srcObjectName);
            Helper::printDebugMsg("Rename attribute ... OK");
        } catch (std::exception &e) {
            Helper::printErrorMsg(e.what());
            Helper::printDebugMsg("Rename attribute ... error");
            std::exit(EXIT_FAILURE);
        }
    }
    // Change attribute value
    else if (settings->getFlagChangeAttributeValue()) {
        if (settings->getName().empty()) {
            Helper::printErrorMsg("Missing parameter -n (attribute name)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        if (settings->getValue().empty()) {
            Helper::printErrorMsg("Missing parameter -v (new attribute value)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }


        std::string delimiter = "/";

        std::string s = settings->getName();
        std::string srcAttributeName = s.substr(s.rfind(delimiter) + 1);
        std::string srcObjectName = s.substr(0, s.rfind(delimiter));
        if (srcObjectName == srcAttributeName || srcObjectName.empty())
            srcObjectName = "/";
        if (srcAttributeName.empty()) {
            Helper::printErrorMsg("Wrong parameter -n (given attribute does not exist)");
            Helper::printMsg(settings->getParamsDefinition().getHelp());
            exit(EXIT_FAILURE);
        }

        try {
            H5Helper::Object *object = nullptr;
            if (file->getObjTypeByName(srcObjectName) == H5G_GROUP) {
                object = file->openGroup(srcObjectName);
            } else if (file->getObjTypeByName(srcObjectName) == H5G_DATASET) {
                object = file->openDataset(srcObjectName);
            } else {
                Helper::printErrorMsg("Wrong parameter -n (given dataset or group does not exist)");
                Helper::printMsg(settings->getParamsDefinition().getHelp());
                exit(EXIT_FAILURE);
            }

            H5Helper::Attribute *attribute = object->getAttribute(srcAttributeName);
            if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_INT)) {
                object->setAttribute(srcAttributeName, ParamsDefinition::toInt(settings->getValue().c_str()));
            } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_UINT)) {
                object->setAttribute(srcAttributeName, ParamsDefinition::toUnsignedInt(settings->getValue().c_str()));
            } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_INT64)) {
                object->setAttribute(srcAttributeName, ParamsDefinition::toLongLong(settings->getValue().c_str()));
            } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_UINT64)) {
                object->setAttribute(srcAttributeName, ParamsDefinition::toUnsignedLongLong(settings->getValue().c_str()));
            } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_FLOAT)) {
                object->setAttribute(srcAttributeName, ParamsDefinition::toFloat(settings->getValue().c_str()));
            } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_DOUBLE)) {
                object->setAttribute(srcAttributeName, ParamsDefinition::toDouble(settings->getValue().c_str()));
            } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_LDOUBLE)) {
                object->setAttribute(srcAttributeName, ParamsDefinition::toLongDouble(settings->getValue().c_str()));
            } else if (H5Tget_class(attribute->getDatatype()) == H5Tget_class(H5T_C_S1)) {
                object->setAttribute(srcAttributeName, settings->getValue());
            } else {
                Helper::printErrorMsg("Unsupported attribute datatype");
                Helper::printMsg(settings->getParamsDefinition().getHelp());
                exit(EXIT_FAILURE);
            }
            delete attribute;
            attribute = nullptr;

            file->closeObject(object);

            Helper::printDebugMsg("Change attribute value ... OK");
        } catch (std::exception &e) {
            Helper::printErrorMsg(e.what());
            Helper::printDebugMsg("Change attribute value ... error");
            std::exit(EXIT_FAILURE);
        }
    }

    delete file;
    file = nullptr;
    delete settings;
    settings = nullptr;

    std::exit(EXIT_SUCCESS);
}
