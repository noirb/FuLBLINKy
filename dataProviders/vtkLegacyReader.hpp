#ifndef _VTK_LEGACY_READER_H
#define _VTK_LEGACY_READER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "DomainParameters.h"

class vtkLegacyReader
{
    public:
        vtkLegacyReader();
        vtkLegacyReader(std::string filename);

        // initializes the reader to pull data from the given file
        void init(std::string filename);

        // returns a struct containing all the details about the domain
        void getDomainParameters(DomainParameters* parameters);

        // writes the data corresponding to the given field to the array passed in data
        template<typename T> void getField(std::string field, T* data);

        // tells the reader to load data from the next timestep
        void NextTimeStep();

        // tells the reader to load data from the previous timestep
        void PrevTimeStep();

        // tells the reader to load data from the specified timestep
        void SetTimeStep(int step);

        // gets the current timestep from the reader
        int GetTimeStep();

        /// FIXME: This should be PRIVATE
        std::vector<std::vector<double> > pointsField;
    private:
        DomainParameters domainParameters;
        std::string filename;
        int timestep;
        int maxTimesteps;
};

#endif
