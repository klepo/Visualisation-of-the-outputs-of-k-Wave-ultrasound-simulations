/**
 * @file        filescontext.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with FilesContext class declaration.
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

#ifndef FILESCONTEXT_H
#define FILESCONTEXT_H

#include <k-wave-h5-helper.h>

#include "helper.h"
#include "settings.h"

/**
 * @brief The FilesContext class represents wrapper for files context
 */
class FilesContext
{
public:
    FilesContext(const Settings *settings);
    ~FilesContext();

    H5Helper::File *getSimOutputFile() const;
    H5Helper::File *getSimInputFile() const;
    H5Helper::File *getPcsOutputFile();
    H5Helper::File *getPcsInputFile() const;

private:
    /// Disable copy contructor
    FilesContext(const FilesContext &);
    /// Disable assignment operator
    /// \return FilesContext
    FilesContext &operator=(const FilesContext &);

    H5Helper::File *loadSimulationFile(std::string filename);
    void resolveOutputFilename(const Settings *settings);
    H5Helper::File *createOrOpenOutputFile(std::string filename);

    /// Simulation output file
    H5Helper::File *simOutputFile = nullptr;
    /// Simulation input file
    H5Helper::File *simInputFile = nullptr;
    /// Processing output file
    H5Helper::File *pcsOutputFile = nullptr;
    /// Processing input file
    H5Helper::File *pcsInputFile = nullptr;

    /// New empty output file flag
    bool newEmptyOutputFileFlag = false;
    /// Output filename
    std::string outputFilename = "";
};

#endif // FILESCONTEXT_H
