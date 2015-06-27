#ifndef _DATA_PROVIDER_H
#define _DATA_PROVIDER_H

#include <vector>

class DataProvider
{
    public:
//        virtual ~DataProvider();
        virtual int GetField(std::string, std::vector<std::vector<double> >**) = 0;
        virtual std::vector<std::string> GetFieldNames() = 0;
        virtual int GetFieldDimension(std::string) = 0;
        virtual void NextTimeStep() = 0;
        virtual void PrevTimeStep() = 0;
        virtual int GetTimeStep()
        {
            return this->timestep;
        }
        virtual int GetMaxTimeStep() = 0;

    private:
        int timestep;
};

#endif
