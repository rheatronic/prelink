#include "xml_parser.h"

const char* WHITESPACE = " \t\n";
const char* WHITESPACE_OR_TAGEND = "/> \t\n";
const char* TAGEND = "/>";
const char* TAGSTART = "<";

std::string::size_type skipOver(const std::string pSrc, const char* pDelimiters, std::string::size_type pOffset=0)
{
	std::string::size_type i = pSrc.find_first_not_of(pDelimiters, pOffset);
	if( i == std::string::npos )
		throw BadSyntax();
	return i;
}

std::string::size_type findDelimiter(const std::string pSrc, const char* pDelimiters, std::string::size_type pOffset=0)
{
	// XXX Does not handle passing over quoted delimiters
	std::string::size_type i = pSrc.find_first_of(pDelimiters, pOffset);
	if( i == std::string::npos )
		throw BadSyntax();
	return i;
}

// Parse just one tag. Return index of following text. Indicate if its a single, start or end element.
std::string::size_type parseTagInternal( const std::string pSrc, std::string::size_type pOffset, std::string &pName, TagType &pTagType, std::string &pAttrs )
{
	std::string::size_type lElementLeft, lElementRight, lNameLeft, lNameEnd;

	//
	lElementLeft = findDelimiter(pSrc, "<", pOffset);
	
	// string ends with <, must be cut short
	if( lElementLeft+1 >= pSrc.length() )
		throw IncompleteParse();

	// tag begins with /, is an end tag
	if(pSrc[lElementLeft+1] == '/')
	{
		pTagType = TAGTYPE_END;
		lNameLeft = lElementLeft + 2;
	} else {
		pTagType = TAGTYPE_START;	// possible, but might also be TAGTYPE_SINGLE (see below)
		lNameLeft = lElementLeft + 1;
	}

	// string ends with < and one other character that doesn't close the tag
	if( lNameLeft >= pSrc.length() )
		throw IncompleteParse();

	//
	lNameEnd = findDelimiter(pSrc, WHITESPACE_OR_TAGEND, lNameLeft);
	pName.assign(pSrc, lNameLeft, lNameEnd-lNameLeft);

	// name ended with >
	if( pSrc[lNameEnd] == '>' )
	{
		// tag type set previously based on whether there was a / before name
		pAttrs = "";
		return lNameEnd+1;
	}

	// name ended with />
	if( pSrc[lNameEnd] == '/' && pSrc[lNameEnd+1] == '>')
	{
		pTagType = TAGTYPE_SINGLE;
		pAttrs = "";
		return lNameEnd+2;
	}

	//
	lElementRight = findDelimiter(pSrc, TAGEND, lNameEnd+1);
	if( pSrc[lElementRight] == '>' )
	{
		// tag type set previously based on whether there was a / before name
		pAttrs.assign(pSrc, lNameEnd+1, lElementRight-(lNameEnd+1));
		return lElementRight+1;
	}

	if( pSrc[lElementRight] == '/' && pSrc[lElementRight+1] == '>' )
	{
		pTagType = TAGTYPE_END;
		pAttrs.assign(pSrc, lNameEnd+1, lElementRight-(lNameEnd+1));
		return lElementRight+1;
	}

	throw IncompleteParse();
}

void parseAttrs(const std::string pAttrString, AttrVec &pAttrs)
{
	pAttrs.clear();
	
	std::string::size_type i = 0;
	while(i<pAttrString.length())
	{
		std::string::size_type j;

		// split out the name
		j = pAttrString.find_first_of("=", i);
		if( j == std::string::npos )
			throw BadSyntax();
		std::string name(pAttrString, i, j-i);

		i = j + 1;

		// step over the double quote
		if( pAttrString[i] != '\"' )
			throw BadSyntax();
		i++;

		// find end double quote
		j = pAttrString.find_first_of("\"", i);
		if( j == std::string::npos )
			throw BadSyntax();
		std::string value(pAttrString, i, j-i);

		i = j + 1;

		// consume following space (if present)
		if( pAttrString[i] == ' ' )
			i++;

		pAttrs.push_back(std::pair<std::string, std::string>(name, value));
	}
}

// parse out the content of the next element, grabbing attrs if present
std::string::size_type parseTag(const std::string pSrc, std::string::size_type pOffset, std::string &pName, TagType &pTagType, AttrVec &pAttrs)
{
	std::string lAttrString;
	std::string::size_type i = parseTagInternal(pSrc, pOffset, pName, pTagType, lAttrString);
	parseAttrs(lAttrString, pAttrs);
	return i;
}

std::string::size_type parseContent( const std::string pSrc, std::string::size_type pOffset, std::string &pContent )
{
	std::string::size_type i = pSrc.find_first_of(TAGSTART, pOffset);
	pContent.assign(pSrc, pOffset, i-pOffset);
	return i;
}
