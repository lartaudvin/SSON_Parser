// SSONParser.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include "SSONParser.h"

int main(int argc, char** argv)
{
	char* importPath{ nullptr };
	for (int i = 1; i < argc; ++i)
	{
		// The -ip option defines the import path
		if (strcmp(argv[i], "-ip") == 0)
		{
			i++;
			importPath = argv[i];
		}
	}

	if (importPath != nullptr)
	{
		const auto SSONDoc{ SSONParser::importSSON(importPath) };
		if (SSONDoc.has_value())
		{
			for (auto&& element : SSONDoc.value())
			{
				std::cout << "name: " << element.first << ", value: " << element.second.toString() << std::endl;
			}
		}
		else
		{
			std::cout << "an error occured" << std::endl;
		}
	}
}
