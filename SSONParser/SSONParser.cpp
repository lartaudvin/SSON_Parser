#include "SSONParser.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

std::string SSONContent::toString() const
{
	switch (m_type)
	{
	case int_t:
		return "[int] " + std::to_string(m_intContent);
	case string_t:
		return "[string] " + m_strContent;
	case document_t:
	{
		std::string result{};
		for (auto&& doc : m_docContent)
		{
			result += "[document] name: " + doc.first + " value:\n" + doc.second.toString();
		}
		return result;
	}
	case array_t:
	{
		std::string result{};
		for (auto&& arr : m_arrContent)
		{
			result += "{\'" + std::to_string(arr.first) + "\': \'" + arr.second.toString() + "\'}";
		}
		return result;
	}
	default: // should not happen.
		return "";
	}
}

std::optional<SSONDocument> SSONParser::importSSON(const char* importPath)
{
	// Try reading the file if possible.
	std::ifstream reader{ importPath, std::ifstream::in };
	if (!reader.is_open())
		return std::nullopt;

	// Get the length of the file.
	char lenChars[4];
	reader.read(lenChars, 4);
	size_t pos{ 0 };
	const auto ssonLengthOpt{ readInt32(lenChars , 4, pos )};
	if (!ssonLengthOpt.has_value())
		return std::nullopt;

	const auto ssonLength{ ssonLengthOpt.value() - 4 };

	// Read the file content.
	char* ssonContent = new char[ssonLength];
	reader.read(ssonContent, ssonLength);

	const auto result{ extractSSONMap<SSONDocument>(ssonContent, ssonLength) };

	//Free the file content.
	delete[] ssonContent;

	return result;
}

std::optional<int32_t> SSONParser::readInt32(const char* const ssonContent, size_t ssonLength, size_t& position)
{
	if (ssonLength < position + 4)
		return std::nullopt;

	// Fuse bytes to get an integer.
	int32_t result = 0;
	for (int i = 0; i < 4; ++i)
	{
		result |= static_cast<unsigned char>(ssonContent[position]) << (i * 8);
		position++;
	}
	return result;
}

std::optional<std::string> SSONParser::readCString(const char* const ssonContent, size_t ssonLength, size_t& position)
{
	std::string result{};
	// We read the string until the ENDCHAR character.
	while (ssonContent[position] != ENDCHAR)
	{
		// If the total document size is reached.
		if (position >= ssonLength)
			return std::nullopt;

		result += ssonContent[position];
		position++;
	}
	position++;
	return result;
}

std::optional<std::string> SSONParser::readString(const char* const ssonContent, size_t ssonLength, size_t& position)
{
	// We get the string size from the first integer element.
	const auto strSize{ readInt32(ssonContent, ssonLength, position) };

	// If the total document size will be reached.
	if (!strSize.has_value() || position + strSize.value() > ssonLength)
		return std::nullopt;

	//knowing the size we get the char
	std::string result{};
	for (size_t i = 0; i < static_cast<size_t>(strSize.value()) - 1; ++i)
	{
		result += ssonContent[position];
		position++;
	}

	// The last character must be ENDCHAR.
	if (ssonContent[position] != ENDCHAR)
		return std::nullopt;
	position++;
	return result;
}

bool SSONParser::extractSSONString(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument)
{
	// A SSONDocument string has a name and a content.
	const auto name{ readCString(ssonContent, ssonLength, position) };
	if (!name.has_value())
		return false;

	const auto str{ readString(ssonContent, ssonLength, position) };
	if (!str.has_value())
		return false;

	outputDocument.insert({ name.value(), str.value() });
	return true;
}

bool SSONParser::extractSSONString(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument)
{
	// A SSONArray string has an integer id and a content.
	const auto idStr{ readCString(ssonContent, ssonLength, position) };
	if (!idStr.has_value())
		return false;

	// The id must be an unsigned integer.
	if (std::find_if(idStr.value().begin(),
		idStr.value().end(), [](char c) { return !std::isdigit(c); }) != idStr.value().end())
		return false;

	const auto str{ readString(ssonContent, ssonLength, position) };
	if (!str.has_value())
		return false;

	outputDocument.insert({ std::stoi(idStr.value()), str.value() });
	return true;
}

bool SSONParser::extractSSONEmbbed(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument)
{
	// We get the embed document name
	const auto name{ readCString(ssonContent, ssonLength, position) };
	if (!name.has_value())
		return false;

	// We get its size.
	const auto docSize{ readInt32(ssonContent, ssonLength, position) };
	if(!docSize.has_value())
		return false;

	// We call the parent method recurcivly.
	const auto doc{ extractSSONMap<SSONDocument>(&ssonContent[position], docSize.value() - 4) };
	position += (docSize.value() - 4);
	if (!doc.has_value())
		return false;

	outputDocument.insert({ name.value(), doc.value() });
	return true;
}

bool SSONParser::extractSSONEmbbed(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument)
{
	// A SSONArray string has an integer id and a content.
	const auto idStr{ readCString(ssonContent, ssonLength, position) };
	if (!idStr.has_value())
		return false;

	// The id must be an unsigned integer.
	if (std::find_if(idStr.value().begin(),
		idStr.value().end(), [](char c) { return !std::isdigit(c); }) != idStr.value().end())
		return false;

	// We get its size.
	const auto docSize{ readInt32(ssonContent, ssonLength, position) };
	if (!docSize.has_value())
		return false;

	// We call the parent method recurcivly.
	const auto doc{ extractSSONMap<SSONDocument>(&ssonContent[position], docSize.value() - 4) };
	position += (docSize.value() - 4);
	if (!doc.has_value())
		return false;

	outputDocument.insert({ std::stoi(idStr.value()),doc.value() });
	return true;
}

bool SSONParser::extractSSONArray(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument)
{
	// We get the array name.
	const auto name{ readCString(ssonContent, ssonLength, position) };
	if (!name.has_value())
		return false;

	// We get the array size.
	const auto docSize{ readInt32(ssonContent, ssonLength, position) };
	if (!docSize.has_value())
		return false;

	// We call a SSONArray adapted version of the parent method. 
	const auto doc{ extractSSONMap<SSONArray>(&ssonContent[position], docSize.value() - 4) };
	position += (docSize.value() - 4);
	if (!doc.has_value())
		return false;

	outputDocument.insert({ name.value(), doc.value() });
	return true;
}

bool SSONParser::extractSSONArray(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument)
{
	// A SSONArray string has an integer id and a content.
	const auto idStr{ readCString(ssonContent, ssonLength, position) };
	if (!idStr.has_value())
		return false;

	// The id must be an unsigned integer.
	if (std::find_if(idStr.value().begin(),
		idStr.value().end(), [](char c) { return !std::isdigit(c); }) != idStr.value().end())
		return false;

	// We get the array size.
	const auto docSize{ readInt32(ssonContent, ssonLength, position) };
	if (!docSize.has_value())
		return false;

	// We call a SSONArray adapted version of the parent method. 
	const auto doc{ extractSSONMap<SSONArray>(&ssonContent[position], docSize.value() - 4) };
	position += (docSize.value() - 4);
	if (!doc.has_value())
		return false;

	outputDocument.insert({ std::stoi(idStr.value()),doc.value() });
	return true;
}

bool SSONParser::extractSSONInteger(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument)
{
	const auto name{ readCString(ssonContent, ssonLength, position) };
	if (!name.has_value())
		return false;
	const auto i{ readInt32(ssonContent, ssonLength, position) };
	if (!i.has_value())
		return false;

	outputDocument.insert({ name.value(), i.value() });
	return true;
}

bool SSONParser::extractSSONInteger(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument)
{
	// A SSONArray string has an integer id and a content.
	const auto idStr{ readCString(ssonContent, ssonLength, position) };
	if (!idStr.has_value())
		return false;

	// The id must be an unsigned integer.
	if (std::find_if(idStr.value().begin(),
		idStr.value().end(), [](char c) { return !std::isdigit(c); }) != idStr.value().end())
		return false;

	const auto i{ readInt32(ssonContent, ssonLength, position) };
	if (!i.has_value())
		return false;

	outputDocument.insert({ std::stoi(idStr.value()),i.value() });
	return true;
}
