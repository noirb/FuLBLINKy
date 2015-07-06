#include "vtkLegacyReader.hpp"
#include <algorithm>
#include <iterator>

vtkLegacyReader::vtkLegacyReader()
{
    this->timestep = 0;
}

vtkLegacyReader::vtkLegacyReader(std::string filename)
{
    this->timestep = 0;
    init(filename);
}

// initializes the reader to pull data from the given file
void vtkLegacyReader::init(std::string filename)
{

    this->filename = filename;
    std::cout << "Parsing file: " << filename << std::endl;

    // get timestep from filename
    int len = filename.length();
    std::string timestep = "";
    bool hitLastDot = false;
    for (int i = len-1; i >= 0; i--)
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
        this->timestep = std::stoi(timestep);
    }
    catch(const std::invalid_argument&)
    {
        this->timestep = -1;
    }

    // if we found a timestep number, try to find other files from different timesteps
    if (this->timestep >= 0)
        this->maxTimesteps = this->GetTimeStepsInDir(filename.substr(0, filename.find_last_of("/")), this->GetBaseFilename());
    if (this->maxTimesteps > 1)
        this->timestep = std::find(this->timestepFilePaths.begin(), this->timestepFilePaths.end(), this->GetFilePath()) - this->timestepFilePaths.begin();

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
            lineStream >> this->domainParameters.size[0] >> 
                          this->domainParameters.size[1] >>
                          this->domainParameters.size[2];
            std::cout << "\tRead domain size as: (" << this->domainParameters.size[0] <<
                                        ", " << this->domainParameters.size[1] <<
                                        ", " << this->domainParameters.size[2] << 
                                        ") " << std::endl;
	    this->fieldNames.push_back("dimensions");
	    this->domainFields["dimensions"] = std::vector<std::vector<double> >();
	    std::vector<double> temp;
	    temp.push_back(this->domainParameters.size[0]);
	    temp.push_back(this->domainParameters.size[1]);
	    temp.push_back(this->domainParameters.size[2]);	    
	    this->domainFields["dimensions"].push_back(temp);
        }
        // parse point locations
        else if (tok == "POINTS")
        {
            double newExtents[6] = {std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest(),  // minX, maxX
                                    std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest(),  // minY, maxY
                                    std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()}; // minZ, maxZ
            this->fieldNames.push_back("points");
            this->domainFields["points"] = std::vector<std::vector<double> >();

            lineStream >> this->domainParameters.numPoints;
            std::cout << "\tRead numPoints as: " << this->domainParameters.numPoints << std::endl;

            getline(file, line, '\n'); lineNumber++;
            while (line.empty() || line.find_first_of("0123456789") == std::string::npos)
            {
                getline(file, line, '\n');
                std::cout << "\tSkipping line " << lineNumber << std::endl;
                lineNumber++;
            }
            for (int i = 0; i < this->domainParameters.numPoints; i++)      /// TODO: This could probably be its own function
            {
                double tmp;
                std::vector<double> tmpv;
                std::stringstream pointStream;
                pointStream << line;

                while(pointStream >> tmp)
                {
                    tmpv.push_back(tmp);
                }
                this->domainFields["points"].push_back(tmpv);
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
            std::copy(std::begin(newExtents), std::end(newExtents), this->extents);
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

            this->fieldNames.push_back(fieldName);
            this->domainFields[fieldName] = std::vector<std::vector<double> >();            

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
            for (int i = 0; i < this->domainParameters.numPoints; i++)
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
                this->domainFields[fieldName].push_back(tmpv);

                // check to see if the magnitude of the data at this point is larger/smaller than our current max/min
                tmpMag = sqrt(tmpMag);
                minScalarVal = tmpMag < minScalarVal ? tmpMag : minScalarVal;
                maxScalarVal = tmpMag > maxScalarVal ? tmpMag : maxScalarVal;

                getline(file, line, '\n');
                lineNumber++;
            }
            this->minFieldValues[fieldName] = minScalarVal;
            this->maxFieldValues[fieldName] = maxScalarVal;
            continue;
        }
        // parse cell data field
        else if (tok == "CELL_DATA")
        {

        }

        lineNumber++;
    }
}

std::string vtkLegacyReader::GetBaseFilename()
{
    // filter directory
    std::string result = this->filename.substr(this->filename.find_last_of("/")+1);
    // filter extension
    result = result.substr(0, result.find_last_of("."));
    // filter last .timestep
    result = result.substr(0, result.find_last_of("."));

    return result;
}

std::string vtkLegacyReader::GetFileDir()
{
    return this->filename.substr(0, this->filename.find_last_of("/")+1);
}

int vtkLegacyReader::GetTimeStepsInDir(std::string directoryName, std::string baseFileName)
{
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
        this->timestepFilePaths.clear();
        while (res--)
        {
            std::string entry = std::string(filenames[res]->d_name);
            if (entry.find(baseFileName) != std::string::npos &&
                entry.substr(entry.find_last_of(".")+1) == "vtk")
            {
                this->timestepFilePaths.push_back(directoryName + "/" + std::string(filenames[res]->d_name));
            }
            free(filenames[res]);
        }
        free(filenames);
    }
    std::sort(this->timestepFilePaths.begin(), this->timestepFilePaths.end(),
        [](std::string const& a, std::string const & b)
        {
            // compare timestep values, not filenames
            std::string a_trimmed = a.substr(a.find_last_of("/")+1);
            a_trimmed = a_trimmed.substr(0, a_trimmed.find_last_of("."));
            a_trimmed = a_trimmed.substr(a_trimmed.find_last_of(".")+1);
            std::string b_trimmed = b.substr(b.find_last_of("/")+1);
            b_trimmed = b_trimmed.substr(0, b_trimmed.find_last_of("."));
            b_trimmed = b_trimmed.substr(b_trimmed.find_last_of(".")+1);
            return std::stoi(a_trimmed) < std::stoi(b_trimmed);
        }
    );

    return this->timestepFilePaths.size();
}

// returns a struct containing all the details about the domain
void vtkLegacyReader::getDomainParameters(DomainParameters* parameters)
{
    parameters = &(this->domainParameters);
}

// writes the data corresponding to the given field to the array passed in data
template<typename T> void vtkLegacyReader::getField(std::string field, T* data)
{

}

// tells the reader to load data from the next timestep
void vtkLegacyReader::NextTimeStep()
{
    if (this->timestep < 0) { return; } // do nothing if our current timestep is invalid

    if (this->timestep < this->maxTimesteps-1)
    {
        this->timestep += 1;
        this->init(this->timestepFilePaths[this->timestep]); //this->GetFileDir() + this->GetBaseFilename() + "." + std::to_string(this->timestep) + ".vtk");
    }
}

// tells the reader to load data from the previous timestep
void vtkLegacyReader::PrevTimeStep()
{
    if (this->timestep < 0) { return; } // do nothing if our current timestep is invalid
    if (this->timestep > 0)
    {
        this->timestep -= 1;
        this->init(this->timestepFilePaths[this->timestep]); //this->GetFileDir() + this->GetBaseFilename() + "." + std::to_string(this->timestep) + ".vtk");
    }
}

// tells the reader to load data from the specified timestep
void vtkLegacyReader::SetTimeStep(int step)
{
    this->timestep = step;
}

// gets the current timestep
int vtkLegacyReader::GetTimeStep()
{
    return this->timestep;
}

// gets maximal timestep
int vtkLegacyReader::GetMaxTimeStep()
{
    return this->maxTimesteps;
}

double vtkLegacyReader::GetMinValueFromField(std::string fieldName)
{
    return this->minFieldValues[fieldName];
}

double vtkLegacyReader::GetMaxValueFromField(std::string fieldName)
{
    return this->maxFieldValues[fieldName];
}

// gets the filename of the file we're currently looking at
std::string vtkLegacyReader::GetFileName()
{
    return this->filename.substr(this->filename.find_last_of("/")+1);
}

// gets the full file path of the file we're currently looking at
std::string vtkLegacyReader::GetFilePath()
{
    return this->filename;
}

// retrieves data for the given field
// Return value is 0 on success, another value for failure (unknown field, etc)
int vtkLegacyReader::GetField(std::string fieldName, std::vector<std::vector<double> >** fieldData)
{
    // return -1 if the given fieldName does not exist
    if (this->domainFields.count(fieldName) == 0)
    {
        return -1;
    }
    else
    {
        *fieldData = &(this->domainFields[fieldName]);
    }

    return 0;
}

double* vtkLegacyReader::GetExtents()
{
    return this->extents;
}

std::vector<std::string> vtkLegacyReader::GetFieldNames()
{
    return this->fieldNames;
}

int vtkLegacyReader::GetFieldDimension(std::string fieldName)
{
    return this->fieldDimensions[fieldName];
}
