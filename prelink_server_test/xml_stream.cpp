#include "xml_stream.h"

const char* _WHITESPACE = " \t\n";
const char* _TAGEND = "/>";
const char* _WHITESPACE_OR_TAGEND = "/> \t\n";
const std::string TE( _TAGEND );

std::string::size_type ParseName( const std::string buffer, std::string::size_type s, std::string &name )
{
	// Parse < name WHITESPACE_OR_TAGEND ... >
	// modify name
	// Return start position of WHITESPACE_OR_TAGEND
	std::string::size_type en = buffer.find_first_of( _WHITESPACE_OR_TAGEND, s );
	name.assign( buffer, s, en - s );
	return en;
}

std::string::size_type ParseAttrs( const std::string buffer, std::string::size_type s, XMLAttributes &attrs  )
{
	// Repeatedly parse WHITESPACE_OR_TAGEND name = " text "
	// Modify attrs
	// Return start position of WHITESPACE_OR_TAGEND

	// we may start with whitespace followed by tagend

	// skip leading whitespace
	s = buffer.find_first_not_of( _WHITESPACE, s );

	// while we haven't run into tag end
	while( TE.find( buffer[s] ) == std::string::npos )
	{
		std::string name, value;

		// skip to equal sign
		std::string::size_type es = buffer.find( '=', s );
		name.assign( buffer, s, es - s );

		s = es + 1;

		if( buffer[s] != '"' )
			throw new XMLErrorSyntax(buffer);

		s++;

		// skip to quote
		es = buffer.find( '"', s );

		if( es == std::string::npos )
			throw new XMLErrorSyntax(buffer);

		value.assign( buffer, s, es - s );

		attrs[name] = value;

		// skip over quote
		s = es + 1;

		// skip trailing whitespace (if any)
		s = buffer.find_first_not_of( _WHITESPACE, s );
	}

	// Skip singleton tag if present
	if( buffer[s] == '/' )
		s++;

	return s;
}

void ParseElement( std::string &buffer, std::string::size_type s, std::string::size_type e, XMLElement *x )
{
	s = ParseName( buffer, s, x->name );
	s = ParseAttrs( buffer, s, x->attrs );
	if( s != e )
		throw new XMLErrorSyntax( buffer );

	_ASSERT( e < buffer.size() );
	
	if( e + 1 == buffer.size() )
		buffer.erase();
	else
		buffer.erase( 0, e + 1 );
}

XMLBegin* ParseBegin( std::string &buffer, std::string::size_type e )
{
	XMLBegin* x = new XMLBegin();
	ParseElement( buffer, 1, e, x );
	return x;
}

XMLSingleton* ParseSingleton( std::string &buffer, std::string::size_type e )
{
	XMLSingleton* x = new XMLSingleton();
	ParseElement( buffer, 1, e, x );
	return x;
}

XMLEnd* ParseEnd( std::string &buffer, std::string::size_type e )
{
	XMLEnd* x = new XMLEnd();
	ParseElement( buffer, 2, e, x );
	return x;
}

XMLElement* IXMLStream::Parse( std::string &buffer, std::string::size_type e )
{
	// buffer e+ should not be affected
	// XXX should do this with a regular expression

	_ASSERT( buffer[0] == '<' && e > 0 && buffer[e] == '>' );

	if( buffer[ 1 ] == '/' ) {
		XMLEnd* xe = ParseEnd( buffer, e );
		if( containers.empty() || containers.top()->name != xe->name )
			throw new XMLErrorEnd(xe);
		containers.pop();
		return xe;
	}

	if( buffer[ e-1 ] == '/' )
		return ParseSingleton( buffer, e );

	XMLBegin* xb = ParseBegin( buffer, e );
	containers.push(xb);
	return xb;
}

XMLElement* IXMLStream::Seek( std::string &buffer, XMLBegin* exemplar )
{
	if( exemplar->attrs.size() )
		throw new XMLErrorExemplar( exemplar );

	std::string pattern = "<" + exemplar->name + ">";
	std::string::size_type i = buffer.find( pattern );
	if ( i == std::string::npos ) {
		buffer.clear();
		return NULL;
	} else {
		buffer.erase( 0, i );
		return Read( buffer );
	}
}

XMLElement* IXMLStream::Read( std::string &buffer )
{
	// Determine if buffer has either <element...> or a text block at the head
	// Consume the text and update state.

	{
		// find start character
		// XXX check whether quoted
		std::string::size_type s = buffer.find('<');
		if( s == std::string::npos ) {
			// no element start character, so keep buffer until one shows up later, 
			// head of buffer will become XMLText
			_incomplete = true;
			return NULL;
		}

		if( s > 0 ) {
			// text followed by a start character,
			// consume text block, leave buffer at start char
			// next pass we'll get the element
			_incomplete = false;
			XMLText* t = new XMLText(buffer.substr(0, s));
			buffer.erase( 0, s );
			return t;
		}

		// block allows s to go out of scope, as it's no longer valid
	}

	// buffer now begins with <

	// look for a closing bracket
	std::string::size_type e = buffer.find( '>' );
	if( e == std::string::npos ) {
		// no closing bracket visible (yet), leave buffer as is and return NULL (waiting for close in a future read)
		_incomplete = true;
		return NULL;
	}

	_incomplete = false;
	return Parse(buffer, e);
}

XMLElement* IXMLSocketStream::Seek( XMLBegin* exemplar )
{
	Get();
	if( _buffer.size() )
		return IXMLStream::Seek( _buffer, exemplar );
	else
		return NULL;
}

XMLElement* IXMLSocketStream::Read() 
{
	Get();
	if( _buffer.size() )
		return IXMLStream::Read( _buffer );
	else
		return NULL;
}

void IXMLSocketStream::Get()
{
	// XXX dangerous non-blocking and complete message assumption
	if( _buffer.size() ) return;

	const int max = 1024;	// May limit elements to 1k
	char buf[ max ];
	int num = 0;
	num = recv( _socket, buf, max, 0 );
	if( num > 0 )
		_buffer.append( buf, num );
}
