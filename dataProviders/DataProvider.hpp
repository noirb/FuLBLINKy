#ifndef _DATA_PROVIDER_H
#define _DATA_PROVIDER_H

#include <thread>
#include <vector>
#include "DomainParameters.h"

class DataProvider
{
    public:
        virtual ~DataProvider() {};
        virtual int GetField(std::string, std::vector<std::vector<double> >**) = 0;
        virtual std::vector<std::string> GetFieldNames() = 0;
        virtual double GetMaxValueFromField(std::string) = 0;
        virtual double GetMinValueFromField(std::string) = 0;
        virtual void getDomainParameters(DomainParameters*) = 0;
        virtual int GetFieldDimension(std::string) = 0;
        virtual double* GetExtents() = 0;

        virtual void NextTimeStep() = 0;
        virtual void NextTimeStepAsync()
        {
            _isReady = false;
            std::thread t([&]()
            {
                this->NextTimeStep();
                _isReady = true;
            });
            t.detach();
        }

        virtual void PrevTimeStep() = 0;
        virtual void PrevTimeStepAsync()
        {
            _isReady = false;
            std::thread t([&]()
            {
                this->PrevTimeStep();
                _isReady = true;
            });
            t.detach();
        }

        virtual int GetTimeStep()
        {
            return this->timestep;
        }
        virtual int GetMaxTimeStep() = 0;

        virtual bool isReady()
        {
            return this->_isReady;
        }

    private:
        unsigned int timestep;

    protected:
        bool _isReady = false; // True if data is ready to be picked up, False otherwise
        DomainParameters domainParameters;
        double extents[6];  // size of domain: [-X, +X, -Y, +Y, -Z, +Z]
};
#endif
