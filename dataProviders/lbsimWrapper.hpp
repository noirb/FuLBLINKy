#ifndef _LBSIM_WRAPPER_H
#define _LBSIM_WRAPPER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#ifndef _WIN32 // dirent.h does not exist under Windows
#include <dirent.h>
#endif

#include "DataProvider.hpp"
#include "DomainParameters.h"

extern "C" {
#include "../lbsim/LBDefinitions.h"
#include "../lbsim/initLB.h"
#include "../lbsim/collision.h"
#include "../lbsim/boundary.h"
#include "../lbsim/streaming.h"
};

class lbsimWrapper : public DataProvider
{
    public:
        lbsimWrapper();
        lbsimWrapper(std::string filename);

        virtual ~lbsimWrapper();

        // initializes the reader to pull data from the given file
        void init(std::string filename);

        // returns a struct containing all the details about the domain
        void getDomainParameters(DomainParameters* parameters);

        // writes the data corresponding to the given field to the array passed in data
        template<typename T> void getField(std::string field, T* data);

        // gets the bounds of the domain: [-X, +X, -Y, +Y, -Z, +Z]
        double* GetExtents();

        // tells the reader to load data from the next timestep
        void NextTimeStep();

        // tells the reader to load data from the previous timestep
        virtual void PrevTimeStep();

        // tells the provider to refresh all of its data fields
        void Update();

        // tells the reader to load data from the specified timestep
        virtual void SetTimeStep(int step);

        // gets the current timestep from the reader
        int GetTimeStep();

        // gets the highest known timestep
        virtual int GetMaxTimeStep();

        // retrieves data for the given field and returns a pointer in fieldData. Returns 0 on success, -1 on failure.
        virtual int GetField(std::string fieldName, std::vector<std::vector<double> >** fieldData);

        virtual double GetMinValueFromField(std::string);
        virtual double GetMaxValueFromField(std::string);

        // retrieves the file name the reader is currently handling
        std::string GetFileName();

        // retrives the full file path of the file the reader is currently handling
        std::string GetFilePath();

        // gets a list of all fields available
        virtual std::vector<std::string> GetFieldNames();

        virtual int GetFieldDimension(std::string);

    private:
        double*    collideField;
        double*    streamField;
        flag_data* flagField;
        int xlength, ylength, zlength;
        double tau;

        int indexOf(int x, int y, int z, int i) { return (19 * ((z) * (ylength+2) * (xlength+2) + (y) * (xlength+2) + (x)) + (i)); };
        int indexOf(int x, int y, int z)        { return (     ((z) * (ylength+2) * (xlength+2) + (y) * (xlength+2) + (x))      ); };

        DomainParameters domainParameters;
        std::string filename;
        std::vector<std::string> timestepFilePaths;

        std::map<std::string, std::vector<std::vector<double> > > domainFields;
        std::map<std::string, double> minFieldValues;
        std::map<std::string, double> maxFieldValues;
        std::map<std::string, int> fieldDimensions;
        std::vector<std::string> fieldNames;

        int timestep;
        int maxTimesteps;
        
        // for a file '/dir/problem.timestep.vtk', this returns 'problem'
        std::string GetBaseFilename();
        std::string GetFileDir();
        int GetTimeStepsInDir(std::string, std::string);

};

#endif
