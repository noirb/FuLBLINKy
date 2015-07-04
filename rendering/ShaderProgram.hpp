#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

// the code below was adapted from: http://r3dux.org/2013/08/a-simple-c-opengl-shader-loader/

#include <iostream>
#include <map>

#include "../loadShaders.hpp"

using std::map;
using std::string;
using std::cout;
using std::endl;


class ShaderProgram
{
	private:
		GLuint programId;   // The unique ID / handle for the shader program
		GLuint shaderCount; // How many shaders are attached to the shader program

		// Map of attributes and their binding locations
		map<string,int> attributeLocList;

		// Map of uniforms and their binding locations
		map<string,int> uniformLocList;

	public:
		// Constructor
        ShaderProgram()
        {
            programId = 0;

            // load nothing by default; total shaders is 0
            shaderCount = 0;
        }

		ShaderProgram(const char* vertex_file_path, const char* fragment_file_path)
		{
            // load the given shaders
			programId = LoadShaders(vertex_file_path, fragment_file_path);

			// If LoadShaders was successful, we now have two shaders linked in this program
            if (programId != 0)
    			shaderCount = 2;
		}

		// Destructor
		~ShaderProgram()
		{
			// Delete the shader program from the graphics card memory to
			// free all the resources it's been using
			glDeleteProgram(programId);
		}

        void loadAndLink(const char* vertex_file_path, const char* fragment_file_path)
        {
            // delete old shader program if we have one
            if (shaderCount > 0 && programId > 0)
            {
                glDeleteProgram(programId);
            }

            // load new shaders
            programId = LoadShaders(vertex_file_path, fragment_file_path);

            // if we succeeded, we now have two shaders
            if (programId != 0)
                shaderCount = 2;
        }

		// Method to enable the shader program
		void enable()
		{
			glUseProgram(programId);
		}


		// Method to disable the shader program
		void disable()
		{
			glUseProgram(0);
		}


		// Returns the bound location of a named attribute
		GLuint getAttribute(const string &attribute)
		{
			// You could do this function with the single line:
			//
			//		return attributeLocList[attribute];
			//
			// BUT, if you did, and you asked it for a named attribute
			// which didn't exist, like, attributeLocList["ThisAttribDoesn'tExist!"]
			// then the method would return an invalid value which will likely cause
			// the program to segfault. So we're making sure the attribute asked
			// for exists, and if it doesn't we can alert the user and stop rather than bombing out later.

			// Create an iterator to look through our attribute map and try to find the named attribute
			map<string, int>::iterator it = attributeLocList.find(attribute);

			// Found it? Great -return the bound location! Didn't find it? Alert user and halt.
			if ( it != attributeLocList.end() )
			{
				return attributeLocList[attribute];
			}
			else
			{
				cout << "Could not find attribute in shader program: " << attribute << endl;
				exit(-1);
			}
		}


		// Method to returns the bound location of a named uniform
		GLuint getUniform(const string &uniform)
		{
			// Note: You could do this method with the single line:
			//
			// 		return uniformLocList[uniform];
			//
			// But we're not doing that. Explanation in the attribute() method above.

			// Create an iterator to look through our uniform map and try to find the named uniform
			static map<string, int>::iterator it = uniformLocList.find(uniform);

			// Found it? Great - pass it back! Didn't find it? Alert user and halt.
			if ( it != uniformLocList.end() )
			{
				return uniformLocList[uniform];
			}
			else
			{
				cout << "Could not find uniform in shader program: " << uniform << endl;
				exit(-1);
			}
		}


		// Method to add an attrbute to the shader and return the bound location
		int addAttribute(const string &attributeName)
		{
			attributeLocList[attributeName] = glGetAttribLocation( programId, attributeName.c_str() );

			// Check to ensure that the shader contains an attribute with this name
			if (attributeLocList[attributeName] == -1)
			{
				cout << "Could not add attribute: " << attributeName << " - location returned -1!" << endl;
				exit(-1);
			}
			else
			{
				cout << "Attribute " << attributeName << " bound to location: " << attributeLocList[attributeName] << endl;
			}

			return attributeLocList[attributeName];
		}


		// Method to add a uniform to the shader and return the bound location
		int addUniform(const string &uniformName)
		{
			uniformLocList[uniformName] = glGetUniformLocation( programId, uniformName.c_str() );

			// Check to ensure that the shader contains a uniform with this name
			if (uniformLocList[uniformName] == -1)
			{
				cout << "Could not add uniform: " << uniformName << " - location returned -1!" << endl;
				exit(-1);
			}
			else
			{
				cout << "Uniform " << uniformName << " bound to location: " << uniformLocList[uniformName] << endl;
			}

			return uniformLocList[uniformName];
		}

};

#endif // SHADER_PROGRAM_HPP
