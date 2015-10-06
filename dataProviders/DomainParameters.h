#ifndef _DOMAIN_PARAMETERS_H
#define _DOMAIN_PARAMETERS_H

    // needs to be a regular struct so C programs can load it up
    typedef struct DomainParameters
    {
        int size[3];    // x, y, z size of domain
        int numPoints;  // total number of data points in the domain
        char** fields; // list of fields available in the dataset (density, velocity, etc.)
    } DomainParameters;

#endif
