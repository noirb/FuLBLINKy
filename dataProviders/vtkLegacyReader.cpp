#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <strsafe.h>
#endif

#include "vtkLegacyReader.hpp"
#include <algorithm>
#include <iterator>

vtkLegacyReader::vtkLegacyReader()
{
    _timestep = 0;
}

vtkLegacyReader::vtkLegacyReader(std::string filename)
{
    _timestep = 0;
    init(filename);
    _isReady = true;
}

vtkLegacyReader::vtkLegacyReader(std::string filename, std::function< void(DataProvider*) > callback)
{
    _backgroundWorkers.push_back(std::thread([this, filename, callback]()
    {
        std::unique_lock<std::mutex> guard(_mutex);
        _timestep = 0;
        init(filename);

        _isReady = true;

        guard.unlock();

        callback(this);
    }));
}

vtkLegacyReader::~vtkLegacyReader()
{
    WaitForWorkers();
}

// initializes the reader to pull data from the given file
void vtkLegacyReader::init(std::string filename)
{

    _filename = filename;
    std::cout << "Parsing file: " << filename << std::endl;

    // get timestep from filename
    unsigned int len = filename.length();
    std::string timestep = "";
    bool hitLastDot = false;
    for (unsigned int i = len; i > 0; --i)
    {
        if (filename[i] == '.' && !hitLastDot)
        {
            hitLastDot = true;
            continue;
        }
        else if (filename[i] == '.' && hitLastDot)
        {
            break;
        }
        else if (hitLastDot)
        {
            timestep = filename[i] + timestep;
        }
    }

    // try to parse timestep; if we didn't get a number, set it to -1 to indicate we don't know the current timestep
    try
    {
        _timestep = std::stoi(timestep);
    }
    catch(const std::invalid_argument&)
    {
        _timestep = -1;
    }

    // if we found a timestep number, try to find other files from different timesteps
    if (_timestep >= 0)
        _maxTimesteps = this->GetTimeStepsInDir(filename.substr(0, filename.find_last_of(PATH_SEP)), this->GetBaseFilename());
    if (_maxTimesteps > 1)
        _timestep = std::find(_timestepFilePaths.begin(), _timestepFilePaths.end(), this->GetFilePath()) - _timestepFilePaths.begin();

    // open file & read in domain parameters
    std::ifstream file(filename.c_str());
    std::string line;
    std::string tok;
    int lineNumber = 1;

    while( !file.eof() )
    {
        std::stringstream lineStream;
        getline(file, line, '\n');
        lineStream << line; // = std::stringstream(line);

        // skip blank lines & comment lines
        if (line.empty())
        {
            lineNumber++;
            continue;
        }
        if (line[0] == '#')
        {
            lineNumber++;
            continue;
        }

        // check first word on line
        lineStream >> tok;
        // fill in domain size
        if (tok == "DIMENSIONS")
        {
            lineStream >> _domainParameters.size[0] >> 
                          _domainParameters.size[1] >>
                          _domainParameters.size[2];
            std::cout << "\tRead domain size as: (" << _domainParameters.size[0] <<
                                               ", " << _domainParameters.size[1] <<
                                               ", " << _domainParameters.size[2] << 
                                               ") " << std::endl;
        }
        // parse point locations
        else if (tok == "POINTS")
        {
            double newExtents[6] = {std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest(),  // minX, maxX
                                    std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest(),  // minY, maxY
                                    std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()}; // minZ, maxZ
            _fieldNames.push_back("points");
            _domainFields["points"] = std::vector<std::vector<double> >();

            lineStream >> _domainParameters.numPoints;
            std::cout << "\tRead numPoints as: " << _domainParameters.numPoints << std::endl;

            getline(file, line, '\n'); lineNumber++;
            while (line.empty() || line.find_first_of("0123456789") == std::string::npos)
            {
                getline(file, line, '\n');
                std::cout << "\tSkipping line " << lineNumber << std::endl;
                lineNumber++;
            }
            for (int i = 0; i < _domainParameters.numPoints; i++)      /// TODO: This could probably be its own function
            {
                double tmp;
                std::vector<double> tmpv;
                std::stringstream pointStream;
                pointStream << line;

                while(pointStream >> tmp)
                {
                    tmpv.push_back(tmp);
                }
                _domainFields["points"].push_back(tmpv);
                // check new point to see if we need to adjust our extents
                newExtents[0] = tmpv[0] < newExtents[0] ? tmpv[0] : newExtents[0]; // -X
                newExtents[1] = tmpv[0] > newExtents[1] ? tmpv[0] : newExtents[1]; // +X
                newExtents[2] = tmpv[1] < newExtents[2] ? tmpv[1] : newExtents[2]; // -Y
                newExtents[3] = tmpv[1] > newExtents[3] ? tmpv[1] : newExtents[3]; // +Y
                newExtents[4] = tmpv[2] < newExtents[4] ? tmpv[2] : newExtents[4]; // -Z
                newExtents[5] = tmpv[2] > newExtents[5] ? tmpv[2] : newExtents[5]; // +Z
                getline(file, line, '\n');
                lineNumber++;
            }
            std::copy(std::begin(newExtents), std::end(newExtents), _extents);
            std::cout << "New extents: " << newExtents[0] << " -- " << newExtents[1] << ", " << newExtents[2] << " -- " << newExtents[3] << ", " << newExtents[4] << " -- " << newExtents[5] << std::endl;
            continue;
        }
        // parse point data field
        else if (tok == "SCALARS" || tok == "VECTORS")
        {
            std::stringstream ptDataStream;
            std::string fieldName;
            double minScalarVal = std::numeric_limits<double>::max();
            double maxScalarVal = std::numeric_limits<double>::lowest();

            lineStream >> fieldName; // get next word on line for fieldName

            _fieldNames.push_back(fieldName);
            _domainFields[fieldName] = std::vector<std::vector<double> >();            

            std::cout << "\tFound field: '" << fieldName << "'" << std::endl;

            // get to first line containing a number
            ptDataStream.str(std::string());
            getline(file, line, '\n'); lineNumber++;
            while (line.empty() || line.find_first_of("0123456789") == std::string::npos)
            {
                getline(file, line, '\n'); lineNumber++;
                std::cout << "\tSkipping line " << lineNumber << std::endl;
            }
            // read data
            for (int i = 0; i < _domainParameters.numPoints; i++)
            {
                double tmp;
                std::vector<double> tmpv;
                double tmpMag = 0;
                std::stringstream dataStream;
                dataStream << line;
                while (dataStream >> tmp)
                {
                    tmpv.push_back(tmp);
                    tmpMag += tmp*tmp;
                }
                _domainFields[fieldName].push_back(tmpv);

                // check to see if the magnitude of the data at this point is larger/smaller than our current max/min
                tmpMag = sqrt(tmpMag);
                minScalarVal = tmpMag < minScalarVal ? tmpMag : minScalarVal;
                maxScalarVal = tmpMag > maxScalarVal ? tmpMag : maxScalarVal;

                getline(file, line, '\n');
                lineNumber++;
            }
            _minFieldValues[fieldName] = minScalarVal;
            _maxFieldValues[fieldName] = maxScalarVal;
            continue;
        }
        // parse cell data field
        else if (tok == "CELL_DATA")
        {

        }

        lineNumber++;
    }
    std::cout << "\tDone." << std::endl;
}

std::string vtkLegacyReader::GetBaseFilename()
{
    // filter directory
    std::string result = _filename.substr(_filename.find_last_of(PATH_SEP)+1);
    // filter extension
    result = result.substr(0, result.find_last_of("."));
    // filter last .timestep
    result = result.substr(0, result.find_last_of("."));

    return result;
}

std::string vtkLegacyReader::GetFileDir()
{
    return _filename.substr(0, _filename.find_last_of(PATH_SEP)+1);
}

unsigned int vtkLegacyReader::GetTimeStepsInDir(std::string directoryName, std::string baseFileName)
{
#ifndef _WIN32 // the following region requires dirent.h, which is unix-only
    struct dirent** filenames;
    int res = scandir(directoryName.c_str(),
                      &filenames,
                      0,
                      alphasort);
    if (res < 0)
    {
        std::cout << "Error scanning directory: " << directoryName << std::endl;
    }
    else
    {
        _timestepFilePaths.clear();
        while (res--)
        {
            std::string entry = std::string(filenames[res]->d_name);
            if (entry.find(baseFileName) != std::string::npos &&
                entry.substr(entry.find_last_of(".")+1) == "vtk")
            {
                _timestepFilePaths.push_back(directoryName + PATH_SEP + std::string(filenames[res]->d_name));
            }
            free(filenames[res]);
        }
        free(filenames);
    }

#else
    WIN32_FIND_DATA ffd;
    TCHAR szDir[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    StringCchCopy(szDir, MAX_PATH, directoryName.c_str()); // directory to search
    StringCchCat(szDir, MAX_PATH, TEXT("\\"));
    StringCchCat(szDir, MAX_PATH, baseFileName.c_str());   // append base filename
    StringCchCat(szDir, MAX_PATH, TEXT(".*.vtk"));         // timestep iterations should be of the form: baseFileName.<step>.vtk

    // Find the first file in the directory.
    hFind = FindFirstFile(szDir, &ffd);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        std::cout << "Error: FindFirstFile failed to open handle in: " << directoryName << std::endl;
        return 0;
    }

    // clear any existing timesteps
    _timestepFilePaths.clear();

    // Collect all the files in the directory that we found
    do
    {
        _timestepFilePaths.push_back(directoryName + PATH_SEP + ffd.cFileName);
    } while (FindNextFile(hFind, &ffd) != 0);

    // done, check for errors
    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        std::cout << "Error from FindNextFile: " << dwError << std::endl;
    }
    // release FindFile handle
    FindClose(hFind);
#endif

    // sort the filenames we found
    std::sort(_timestepFilePaths.begin(), _timestepFilePaths.end(),
        [](std::string const& a, std::string const & b)
        {
            // compare timestep values, not filenames
            std::string a_trimmed = a.substr(a.find_last_of(PATH_SEP) + 1);
            a_trimmed = a_trimmed.substr(0, a_trimmed.find_last_of("."));
            a_trimmed = a_trimmed.substr(a_trimmed.find_last_of(".") + 1);
            std::string b_trimmed = b.substr(b.find_last_of(PATH_SEP) + 1);
            b_trimmed = b_trimmed.substr(0, b_trimmed.find_last_of("."));
            b_trimmed = b_trimmed.substr(b_trimmed.find_last_of(".") + 1);
            return std::stoi(a_trimmed) < std::stoi(b_trimmed);
        }
    );
    return _timestepFilePaths.size();
}

// returns a struct containing all the details about the domain
void vtkLegacyReader::getDomainParameters(DomainParameters* parameters)
{
    std::lock_guard<std::mutex> guard(_mutex);
    *parameters = _domainParameters;
}

// writes the data corresponding to the given field to the array passed in data
template<typename T> void vtkLegacyReader::getField(std::string field, T* data)
{

}

// tells the reader to load data from the next timestep
void vtkLegacyReader::NextTimeStep()
{
    if (_timestep < 0) { return; } // do nothing if our current timestep is invalid

    if (_timestep + 1 < (int)_maxTimesteps)
    {
        _timestep += 1;
        init(_timestepFilePaths[_timestep]); //this->GetFileDir() + this->GetBaseFilename() + "." + std::to_string(this->timestep) + ".vtk");
    }
}

// tells the reader to load data from the previous timestep
void vtkLegacyReader::PrevTimeStep()
{
    if (_timestep < 0) { return; } // do nothing if our current timestep is invalid
    if (_timestep > 0 && _timestepFilePaths.size() >= (unsigned int)(unsigned int)(unsigned int)(unsigned int)(unsigned int)(unsigned int)(unsigned int)(unsigned int)(unsigned int)_timestep)
    {
        _timestep -= 1;
        this->init(_timestepFilePaths[_timestep]);
    }
}

// tells the reader to load data from the specified timestep
void vtkLegacyReader::SetTimeStep(int step)
{
    std::lock_guard<std::mutex> guard(_mutex);
    _timestep = step;
}

// gets the current timestep
int vtkLegacyReader::GetTimeStep()
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
unsigned int vtkLegacyReader::GetMaxTimeStep()
{
    if (_mutex.try_lock())
    {
        unsigned int t = _maxTimesteps;
        _mutex.unlock();
        return t;
    }

    throw ProviderBusy("Could not safely retrieve max timestep");
}

double vtkLegacyReader::GetMinValueFromField(std::string fieldName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _minFieldValues[fieldName];
}

double vtkLegacyReader::GetMaxValueFromField(std::string fieldName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _maxFieldValues[fieldName];
}

// gets the filename of the file we're currently looking at
std::string vtkLegacyReader::GetFileName()
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
std::string vtkLegacyReader::GetFilePath()
{
    return _filename;
}

// retrieves data for the given field
// Return value is 0 on success, another value for failure (unknown field, etc)
int vtkLegacyReader::GetField(std::string fieldName, std::vector<std::vector<double> >** fieldData)
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

double* vtkLegacyReader::GetExtents()
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _extents;
}

std::vector<std::string> vtkLegacyReader::GetFieldNames()
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _fieldNames;
}

int vtkLegacyReader::GetFieldDimension(std::string fieldName)
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _fieldDimensions[fieldName];
}
