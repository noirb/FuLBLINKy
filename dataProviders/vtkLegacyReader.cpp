#include "vtkLegacyReader.hpp"

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
        }
        // parse point locations
        else if (tok == "POINTS")
        {
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

                getline(file, line, '\n');
                lineNumber++;
            }
            continue;
        }
        // parse point data field
        else if (tok == "SCALARS" || tok == "VECTORS")
        {
            std::stringstream ptDataStream;
            std::string fieldName;
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
                ptDataStream << line;

                while (ptDataStream >> tmp)
                {
                    tmpv.push_back(tmp);
                }
                this->domainFields[fieldName].push_back(tmpv);
            }

        }
        // parse cell data field
        else if (tok == "CELL_DATA")
        {

        }

        lineNumber++;
    }
}

// returns a struct containing all the details about the domain
void vtkLegacyReader::getDomainParameters(DomainParameters* parameters)
{
}

// writes the data corresponding to the given field to the array passed in data
template<typename T> void vtkLegacyReader::getField(std::string field, T* data)
{

}

// tells the reader to load data from the next timestep
void vtkLegacyReader::NextTimeStep()
{
    if (this->timestep < this->maxTimesteps)
    {
        this->timestep += 1;
    }
}

// tells the reader to load data from the previous timestep
void vtkLegacyReader::PrevTimeStep()
{
    if (this->timestep > 0)
    {
        this->timestep -= 1;
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
