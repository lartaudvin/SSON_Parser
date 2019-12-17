#ifndef  SSON_PARSER_H
#define SSON_PARSER_H

#include <map>
#include <unordered_map>
#include <string>
#include <optional>

// Forward declaration for SSONDocument and SSONArray declaration.
class SSONContent;

//! A SSON document contains elements referred by their names, and ordered by the reader order.
typedef std::unordered_map<std::string, SSONContent> SSONDocument;
//! A SSON array contains string elements referred by their ids and ordered by id.
typedef std::map<int32_t, SSONContent> SSONArray;

/**
* \brief This class manages the storing of generic elements (used by SSON documents).
*/
class SSONContent
{
public:
	typedef enum {
		int_t,
		string_t,
		document_t,
		array_t
	} SSONContentType;

	SSONContent() = delete;

	template <typename Integer,
		std::enable_if_t<std::is_same<int32_t, Integer>::value, int> = 0
	>
		SSONContent(Integer i) : m_type{ int_t }, m_intContent{ i } {}
	template <typename String,
		std::enable_if_t<std::is_same<std::string, String>::value, int> = 0
	>
		SSONContent(String s) : m_type{ string_t }, m_strContent{ s } {}
	template <typename Document,
		std::enable_if_t<std::is_same<SSONDocument, Document>::value, int> = 0
	>
		SSONContent(Document d) : m_type{ document_t }, m_docContent{ d } {}
	template <typename Array,
		std::enable_if_t<std::is_same<SSONArray, Array>::value, int> = 0
	>
		SSONContent(Array a) : m_type{ array_t }, m_arrContent{ a } {}

	~SSONContent() = default;

	std::string toString() const;
private:
	SSONContentType m_type;
	int32_t m_intContent;
	std::string m_strContent;
	SSONDocument m_docContent;
	SSONArray m_arrContent;
};

/**
* \brief This class will parse and store the SSON result.
*/
class SSONParser
{
public:

	/**
	* \brief This method reads a SSON document and extract its data.
	* \param importPath The path of the document.
	* \return The document datas, nullopt if an error occured.
	*/
	static std::optional<SSONDocument> importSSON(const char* importPath);

private:
	/**
	* \brief This method reads an int32_t from the SSON buffer.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the int32_t from.
	* \return The string to extract, nullopt if something failed.
	*/
	static std::optional<int32_t> readInt32(const char* const ssonContent, size_t ssonLength, size_t& position);

	/**
	* \brief This method reads a string, which size is not coded from the SSON buffer.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \return The string to extract, nullopt if something failed.
	*/
	static std::optional<std::string> readCString(const char* const ssonContent, size_t ssonLength, size_t& position);

	/**
	* \brief This method reads a string, which size is coded from the SSON buffer.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \return The string to extract, nullopt if something failed.
	*/
	static std::optional<std::string> readString(const char* const ssonContent, size_t ssonLength, size_t& position);
	
	/**
	* \brief This method extracts a string information from the SSON buffer and stores it in a SSONDocument map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the string in, nullopt if something failed.
	*/
	static bool extractSSONString(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument);

	/**
	* \brief This method extracts a string information from the SSON buffer and stores it in a SSONArray map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the string in, nullopt if something failed.
	*/
	static bool extractSSONString(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument);

	/**
	* \brief This method extracts an embbed SSONDocument information from the SSON buffer and stores it in a SSONDocument map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the SSONDocument in, nullopt if something failed.
	*/
	static bool extractSSONEmbbed(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument);

	/**
	* \brief This method extracts an embbed SSONDocument information from the SSON buffer and stores it in a SSONArray map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the SSONDocument in, nullopt if something failed.
	*/
	static bool extractSSONEmbbed(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument);

	/**
	* \brief This method extracts a SSONArray information from the SSON buffer and stores it in a SSONDocument map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the SSONArray in, nullopt if something failed.
	*/
	static bool extractSSONArray(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument);

	/**
	* \brief This method extracts a SSONArray information from the SSON buffer and stores it in a SSONArray map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the SSONArray in, nullopt if something failed.
	*/
	static bool extractSSONArray(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument);

	/**
	* \brief This method extracts a int32_t information from the SSON buffer and stores it in a SSONDocument map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the int32_t in, nullopt if something failed.
	*/
	static bool extractSSONInteger(const char* const ssonContent, size_t ssonLength, size_t& position, SSONDocument& outputDocument);

	/**
	* \brief This method extracts a int32_t information from the SSON buffer and stores it in a SSONArray map.
	* \param ssonContent The buffer of the SSON content.
	* \param ssonLength The total length of the SSON content.
	* \param position The position of the currently pointed character where to read the string from.
	* \param outputDocument[in, out] The map where to store the int32_t in, nullopt if something failed.
	*/
	static bool extractSSONInteger(const char* const ssonContent, size_t ssonLength, size_t& position, SSONArray& outputDocument);

	template <typename T,
		typename = std::enable_if<std::is_same<SSONDocument, T>::value || std::is_same<SSONArray, T>::value>
	>
	/**
	* \brief Reads the content of a buffer and extract all the informations of a SSONDocument or a SSONArray
	* \param ssonContent The content read from a file streamer.
	* \param ssonLength The length of the SSON document or array.
	* \return The document or array, nullopt if something failed.s
	*/
	static std::optional<T> extractSSONMap(const char* const ssonContent, size_t ssonLength)
	{
		T result{};
		size_t position{ 0 };

		while (position < ssonLength - 1)
		{
			//first we try to know which kind of element we get
			const auto elementType{ ssonContent[position] };
			position++;

			// Then we call a function, depending on the element type and the kind of map to extract.
			switch (elementType)
			{
			case STRING:
			{
				if(!extractSSONString(ssonContent, ssonLength, position, result))
					return std::nullopt;
				break;
			}
			case EMBEDDED:
			{
				if(!extractSSONEmbbed(ssonContent, ssonLength, position, result))
					return std::nullopt;
				break;
			}
			case ARRAY:
			{
				if (!extractSSONArray(ssonContent, ssonLength, position, result))
					return std::nullopt;
				break;
			}
			case INTEGER:
			{
				if(!extractSSONInteger(ssonContent, ssonLength, position, result))
					return std::nullopt;
				break;
			}
			case ENDCHAR: // End of document reached before the end -> error
			default:
				return std::nullopt;
			}
		}
		if (ssonContent[position] != ENDCHAR)
			return std::nullopt;

		return result;
	};

	enum SSONTypes
	{
		ENDCHAR = 0,
		STRING = 2,
		EMBEDDED = 3,
		ARRAY = 4,
		INTEGER = 16 //x10
	};
};
#endif // ! SSON_PARSER_H
