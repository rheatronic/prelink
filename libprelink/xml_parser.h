#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

// Currently used for client-side message parsing in precam

#include <string>
#include <vector>

typedef std::vector<std::pair<std::string, std::string>> AttrVec;

class IncompleteParse : public std::exception {};
class BadSyntax : public std::exception {};

enum TagType
{
	TAGTYPE_SINGLE,
	TAGTYPE_START,
	TAGTYPE_END
};

std::string::size_type parseTag( const std::string pSrc, std::string::size_type pOffset, std::string &pName, TagType &pTagType, AttrVec &pAttrs );

std::string::size_type parseContent( const std::string pSrc, std::string::size_type pOffset, std::string &pContent );

// ----

class Element
{
public:
	Element();	// generic, to be read
	Element(std::string pName); // expected to be read
	Element(std::string pName, AttrVec pAttrs);	// to be output

	std::string mName;
	AttrVec mAttrs;
	
	// get child / content stream
};

class XMLIElementStream
{
public:
	XMLIElementStream(std::string);
};

// IAttrStream

// IContentStream

#endif
