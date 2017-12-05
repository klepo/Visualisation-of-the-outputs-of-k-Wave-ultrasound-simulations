/**
 * @file        filescontext.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with FilesContext class declaration.
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

#ifndef FILESCONTEXT_H
#define FILESCONTEXT_H

#include <k-wave-h5-helper.h>
#include <helper.h>
#include <settings.h>

/**
 * @brief The FilesContext class represents wrapper for files context
 */
class FilesContext
{
public:
    FilesContext(Settings *settings);
    ~FilesContext();

    H5Helper::File *getSimOutputFile() const;
    H5Helper::File *getSimInputFile() const;
    H5Helper::File *getPcsOutputFile();
    H5Helper::File *getPcsInputFile() const;

private:
    FilesContext(const FilesContext &);
    FilesContext &operator=(const FilesContext &);
    H5Helper::File *loadSimulationFile(std::string filename);
    void resolveOutputFilename(Settings *settings);
    H5Helper::File *createOrOpenOutputFile(std::string filename);
    H5Helper::File *simOutputFile = 0;
    H5Helper::File *simInputFile = 0;
    H5Helper::File *pcsOutputFile = 0;
    H5Helper::File *pcsInputFile = 0;

    bool newEmptyOutputFileFlag = false;
    std::string outputFilename = "";
};

#endif // FILESCONTEXT_H
