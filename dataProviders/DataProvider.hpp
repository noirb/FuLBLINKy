#ifndef _DATA_PROVIDER_H
#define _DATA_PROVIDER_H

#include <vector>

class DataProvider
{
    public:
        virtual int GetField(std::string, std::vector<std::vector<double> >**) = 0;
};

#endif
