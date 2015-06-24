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
            std::cout << "Read domain size as: (" << this->domainParameters.size[0] <<
                                        ", " << this->domainParameters.size[1] <<
                                        ", " << this->domainParameters.size[2] << 
                                        ") " << std::endl;
        }
        // parse point locations
        else if (tok == "POINTS")
        {
            double point[3];
            lineStream >> this->domainParameters.numPoints;
            std::cout << "Read numPoints as: " << this->domainParameters.numPoints << std::endl;

            getline(file, line, '\n'); lineNumber++;
            while (line.empty() || line.find_first_of("0123456789") == std::string::npos)
            {
                getline(file, line, '\n');
                std::cout << "Skipping line " << lineNumber << std::endl;
                lineNumber++;
            }
            for (int i = 0; i < this->domainParameters.numPoints; i++)
            {
                std::stringstream pointStream;
                pointStream << line;
                pointStream >> point[0] >> point[1] >> point[2];
                std::cout << lineNumber << ": " << point[0] << " " << point[1] << " " << point[2] << "\n";
                this->pointsField.push_back(std::vector<double>(point, point + sizeof(point) / sizeof(point[0])));
                getline(file, line, '\n');
                lineNumber++;
            }
            continue;
        }
        // parse point data field
        else if (tok == "POINT_DATA")
        {

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
