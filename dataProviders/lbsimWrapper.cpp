#include "lbsimWrapper.hpp"
#include <algorithm>
#include <iterator>
#include <glm/glm.hpp>

lbsimWrapper::lbsimWrapper()
{
    _timestep = 0;
    _maxTimesteps = 0;
}

lbsimWrapper::lbsimWrapper(std::string filename)
{
    _timestep = 0;
    _maxTimesteps = 0;
    init(filename);
    _isReady = true;
}

lbsimWrapper::lbsimWrapper(std::string filename, std::function< void(DataProvider*) > callback)
{
    _backgroundWorkers.push_back(std::thread([this, filename, callback]()
    {
        std::unique_lock<std::mutex> guard(_mutex);
        _timestep = 0;
        _maxTimesteps = 0;
        init(filename);
        _isReady = true;

        guard.unlock();

        callback(this);
    }));
}

lbsimWrapper::~lbsimWrapper()
{
    WaitForWorkers();
    free(_collideField);
    free(_streamField);
    free(_flagField);
}

// initializes the reader to pull data from the given file
void lbsimWrapper::init(std::string filename)
{
    int timesteps, timestepsPerPlotting, totalElements;
    char** c_filename;
    c_filename = (char**) calloc(2, sizeof(char*));
    c_filename[1] = (char*) calloc(filename.length() + 1, sizeof(char));
#ifndef _WIN32 // strcpy throws error C4996 in VS
    std::strcpy(c_filename[1], filename.c_str());
#else
    strncpy_s(c_filename[1], (filename.length() + 1) * sizeof(char), filename.c_str(), filename.length());
#endif

    _filename = filename;
    std::cout << "Loading lbsim input file: " << filename << std::endl;

    setlocale(LC_NUMERIC, "C"); // REQUIRED for sscanf used by readParameters to recognize decimal points on all systems

    // read domain parameters from input
    if (readParameters(&_xlength, &_ylength, &_zlength, &_tau, &timesteps, &timestepsPerPlotting, 2, c_filename) == -1)
    {
        std::cout << "ERROR: Could not read lbsim parameter file: " << filename << std::endl;
    }

    // store domain parameters
    _domainParameters.size[0] = _xlength;
    _domainParameters.size[1] = _ylength;
    _domainParameters.size[2] = _zlength;
    _domainParameters.numPoints = _xlength * _ylength * _zlength; // # of OUTPUT points, not including actual boundary cells

    totalElements = (_xlength+2) * (_ylength+2) * (_zlength+2);
    // allocate space for simulation
    _collideField = (double*) calloc(19 * totalElements, sizeof(double));
    _streamField  = (double*) calloc(19 * totalElements, sizeof(double));
    _flagField    = (flag_data*) calloc(totalElements, sizeof(flag_data));

    // exit if we fail to allocate enough space
    if (_collideField == NULL || _streamField == NULL || _flagField == NULL)
    {
        std::cout << "ERROR: Could not allocate space to run lbsim on " << totalElements << " elements!" << std::endl;
        exit(-1);
    }

    // initialize fields
    if (initialiseFields(_collideField, _streamField, _flagField, _xlength, _ylength, _zlength, c_filename[1]) == -1)
    {
        std::cout << "ERROR: Could not initialize domain for lbsim. Exiting..." << std::endl;
        exit(-1);
    }

    // initialize output fields
    _fieldNames.push_back("density");
    _fieldNames.push_back("velocity");
    _fieldNames.push_back("flags");
    _fieldNames.push_back("points");

    /// TODO: Store all distributions in a SINGLE field!
    for (int i = 0; i < 19; i++)
    {
        _fieldNames.push_back("distributions" + std::to_string(i));
    }

    _domainFields["points"]   = std::vector<std::vector<double> >();
    _domainFields["density"]  = std::vector<std::vector<double> >();
    _domainFields["velocity"] = std::vector<std::vector<double> >();
    _domainFields["flags"]    = std::vector<std::vector<double> >();

    /// TODO: Store all distributions in a SINGLE field!
    for (int i = 0; i < 19; i++)
    {
        _domainFields["probability" + std::to_string(i)] = std::vector<std::vector<double> >();
    }

    _extents[0] = 0;
    _extents[1] = _xlength;
    _extents[2] = 0;
    _extents[3] = _ylength;
    _extents[4] = 0;
    _extents[5] = _zlength;

    this->Update();

    free(c_filename);
}

// extracts field data so it's ready for a renderer to pick up
void lbsimWrapper::Update()
{
    double minDensity = 100;
    double maxDensity = -100;
    double minVelocity = 100;
    double maxVelocity = -100;

    _domainFields["points"].clear();
    _domainFields["density"].clear();
    _domainFields["velocity"].clear();
    _domainFields["flags"].clear();

    for (int l = 0; l < 19; l++)
    {
        _domainFields["probability" + std::to_string(l)].clear();
    }

    int count = 0;
    for (int i = 1; i <= _xlength; i++)
    {
        for (int j = 1; j <= _ylength; j++)
        {
            for (int k = 1; k <= _zlength; k++)
            {
                double density;
                double velocity[3];
                computeDensity(&(_collideField[indexOf(i, j, k, 0)]), &density);
                computeVelocity(&(_collideField[indexOf(i, j, k, 0)]), &density, velocity);

                std::vector<double> p_tmp, d_tmp, v_tmp, f_tmp;
                p_tmp.push_back( (double)i); p_tmp.push_back( (double)j ); p_tmp.push_back( (double)k );
                d_tmp.push_back( density );
                v_tmp.push_back( velocity[0] ); v_tmp.push_back( velocity[1] ); v_tmp.push_back( velocity[2] );
                f_tmp.push_back( _flagField[indexOf(i, j, k)].flag );

                _domainFields["points"].push_back( p_tmp );
                _domainFields["density"].push_back( d_tmp );
                _domainFields["velocity"].push_back( v_tmp );
                _domainFields["flags"].push_back( f_tmp );

                for (int l = 0; l < 19; l++)
                {
                    std::vector<double> prob;
                    prob.push_back(_collideField[indexOf(i, j, k, l)]);
                    _domainFields["probability" + std::to_string(l)].push_back( prob );
                }

                count++;

                minDensity = density < minDensity ? density : minDensity;
                maxDensity = density > maxDensity ? density : maxDensity;
                double velocityMag = glm::length(glm::vec3(velocity[0], velocity[1], velocity[2]));
                minVelocity = velocityMag < minVelocity ? velocityMag : minVelocity;
                maxVelocity = velocityMag > maxVelocity ? velocityMag : maxVelocity;
            }
        }
    }

    _minFieldValues["density"] = minDensity;
    _maxFieldValues["density"] = maxDensity;
    _minFieldValues["velocity"] = minVelocity;
    _maxFieldValues["velocity"] = maxVelocity;
}

std::string lbsimWrapper::GetBaseFilename()
{
    // filter directory
    std::string result = _filename.substr(_filename.find_last_of("/")+1);
    // filter extension
    result = result.substr(0, result.find_last_of("."));
    // filter last .timestep
    result = result.substr(0, result.find_last_of("."));

    return result;
}

std::string lbsimWrapper::GetFileDir()
{
    return _filename.substr(0, _filename.find_last_of("/")+1);
}

// returns a struct containing all the details about the domain
void lbsimWrapper::getDomainParameters(DomainParameters* parameters)
{
    *parameters = _domainParameters;
}

// writes the data corresponding to the given field to the array passed in data
template<typename T> void lbsimWrapper::getField(std::string field, T* data)
{

}

// tells the reader to load data from the next timestep
void lbsimWrapper::NextTimeStep()
{
    double* swap = NULL;
    doStreaming(_collideField, _streamField, _flagField, _xlength, _ylength, _zlength);
    swap = _collideField;
    _collideField = _streamField;
    _streamField = swap;

    doCollision(_collideField, _flagField, &_tau, _xlength, _ylength, _zlength);
    treatBoundary(_collideField, _flagField, _xlength, _ylength, _zlength);

    this->Update();

    _timestep++;
}

// tells the reader to load data from the previous timestep
void lbsimWrapper::PrevTimeStep()
{
    // do nothing: can't simulate backwards!
}

// tells the reader to load data from the specified timestep
void lbsimWrapper::SetTimeStep(int step)
{
    // do nothing: can't skip or go backwards!
}

// gets the current timestep
int lbsimWrapper::GetTimeStep()
{
    if (_mutex.try_lock())
    {
        unsigned int t = _timestep;
        _mutex.unlock();
        return t;
    }

    throw ProviderBusy("Could not safely retrieve current timestep");
}

// gets maximal timestep
unsigned int lbsimWrapper::GetMaxTimeStep()
{
    if (_mutex.try_lock())
    {
        unsigned int t = _maxTimesteps;
        _mutex.unlock();
        return t;
    }

    throw ProviderBusy("Could not safely retrieve max timestep");
}

double lbsimWrapper::GetMinValueFromField(std::string fieldName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _minFieldValues[fieldName];
}

double lbsimWrapper::GetMaxValueFromField(std::string fieldName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _maxFieldValues[fieldName];
}

// gets the filename of the file we're currently looking at
std::string lbsimWrapper::GetFileName()
{
    if (_mutex.try_lock())
    {
        std::string s = _filename.substr(_filename.find_last_of(PATH_SEP) + 1);
        _mutex.unlock();
        return s;
    }

    throw ProviderBusy("Could not safely retrieve file name");
}

// gets the full file path of the file we're currently looking at
std::string lbsimWrapper::GetFilePath()
{
    return _filename;
}

// retrieves data for the given field
// Return value is 0 on success, another value for failure (unknown field, etc)
int lbsimWrapper::GetField(std::string fieldName, std::vector<std::vector<double> >** fieldData)
{
    std::lock_guard<std::mutex> guard(_mutex);
    // return -1 if the given fieldName does not exist
    if (_domainFields.count(fieldName) == 0)
    {
        return -1;
    }
    else
    {
        *fieldData = &(_domainFields[fieldName]);
    }

    return 0;
}

double* lbsimWrapper::GetExtents()
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _extents;
}

std::vector<std::string> lbsimWrapper::GetFieldNames()
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _fieldNames;
}

int lbsimWrapper::GetFieldDimension(std::string fieldName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _fieldDimensions[fieldName];
}
