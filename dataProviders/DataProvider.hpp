#ifndef _DATA_PROVIDER_H
#define _DATA_PROVIDER_H

#include <stdexcept>
#include <thread>
#include <mutex>
#include <vector>
#include "DomainParameters.h"

// there may be a better place for this
#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif


class DataProvider
{
    public:
        virtual ~DataProvider() 
        {
            WaitForWorkers();
            _backgroundWorkers.clear();
        }

        virtual int GetField(std::string, std::vector<std::vector<double> >**) = 0;
        virtual std::vector<std::string> GetFieldNames() = 0;
        virtual double GetMaxValueFromField(std::string) = 0;
        virtual double GetMinValueFromField(std::string) = 0;
        virtual void getDomainParameters(DomainParameters*) = 0;
        virtual int GetFieldDimension(std::string) = 0;
        virtual double* GetExtents() = 0;

        virtual void NextTimeStepAsync()
        {
            _isReady = false;
            _backgroundWorkers.push_back(std::thread([&]()
            {
                std::lock_guard<std::mutex> guard(_mutex);
                _isReady = false;
                this->NextTimeStep();
                _isReady = true;
            }));
        }

        virtual void PrevTimeStepAsync()
        {
            _isReady = false;
            _backgroundWorkers.push_back(std::thread([&]()
            {
                std::lock_guard<std::mutex> guard(_mutex);
                _isReady = false;
                this->PrevTimeStep();
                _isReady = true;
            }));
        }

        virtual int GetTimeStep()
        {
            return this->_timestep;
        }
        virtual unsigned int GetMaxTimeStep() = 0;

        virtual bool isReady()
        {
            return this->_isReady;
        }

    private:

    protected:
        std::mutex _mutex;
        std::mutex _workerMutex;
        std::vector<std::thread> _backgroundWorkers;

        bool _isReady = false; // True if data is ready to be picked up, False otherwise
        DomainParameters _domainParameters;
        double _extents[6];  // size of domain: [-X, +X, -Y, +Y, -Z, +Z]
        unsigned int _timestep;
        unsigned int _maxTimesteps;

        virtual void NextTimeStep() = 0;
        virtual void PrevTimeStep() = 0;

        void WaitForWorkers()
        {
            // ensure all attached threads complete
            for (unsigned int i = 0; i < _backgroundWorkers.size(); i++)
            {
                if (_backgroundWorkers[i].joinable())
                {
                    _backgroundWorkers[i].join();
                }
            }
        }
};

class ProviderBusy : public std::runtime_error 
{
    public:
        explicit ProviderBusy(const std::string& msg) : std::runtime_error(msg) {}
};

#endif
