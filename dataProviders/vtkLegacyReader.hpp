#ifndef _VTK_LEGACY_READER_H
#define _VTK_LEGACY_READER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstring>

#include "DataProvider.hpp"
#include "DomainParameters.h"

class vtkLegacyReader : public DataProvider
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

        // retrieves data for the given field and returns a pointer in fieldData. Returns 0 on success, -1 on failure.
        virtual int GetField(std::string fieldName, std::vector<std::vector<double> >** fieldData);

        // gets a list of all fields available
        virtual std::vector<std::string> GetFieldNames();

        virtual int GetFieldDimension(std::string);

    private:
        DomainParameters domainParameters;
        std::string filename;

        std::map<std::string, std::vector<std::vector<double> > > domainFields;
        std::map<std::string, int> fieldDimensions;
        std::vector<std::string> fieldNames;

        int timestep;
        int maxTimesteps;

};

#endif
