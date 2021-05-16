#ifndef _XML_STREAM_H_
#define _XML_STREAM_H_

/// Inspired by XMLLite but without COM and less conservative read strategy (whole elements only)

#include <string>
#include <map>

typedef std::map<std::string, std::string> XMLAttributes;

class XMLElement {
public:
	XMLElement() {}
	XMLElement(const char* name) : name(name) {}
	virtual void dummy() {}	/// Make class polymorphic (there's a better way)
	std::string name;
	XMLAttributes attrs;
};

class XMLSingleton : public XMLElement {
public:
	XMLSingleton() : XMLElement() {}
	XMLSingleton(const char* name) : XMLElement(name) {}
};

class XMLBegin : public XMLElement {
public:
	XMLBegin() : XMLElement() {}
	XMLBegin(const char* name) : XMLElement(name) {}
};

class XMLText : public XMLElement {
public:
	XMLText(std::string text) : text(text) {}
	std::string text;
};

class XMLEnd : public XMLElement {
public:
	XMLEnd() : XMLElement() {}
	XMLEnd(const char* name) : XMLElement(name) {}
};

class XMLError {};

class XMLErrorExemplar : public XMLError {
public:
	XMLErrorExemplar(XMLElement* exemplar) {}
};

class XMLErrorSyntax : public XMLError {
public:
	XMLErrorSyntax(std::string buffer) : buffer(buffer) {}
	std::string buffer;
};

class XMLErrorEnd : public XMLError {
public:
	XMLErrorEnd(XMLElement* found) {}
};

#include <stack>

typedef std::stack <XMLBegin*> XMLStack;

// can this be implemented as a basic_istream?
class IXMLStream {
	/// Abstract stream of XML elements from an input source
	/// Maintains container state, e.g. start elements above us
public:
	IXMLStream() : _incomplete(false) {}
	virtual XMLElement* Seek( std::string &buffer, XMLBegin* exemplar );
		/// Find next exemplar element in buffer.
		/// Return NULL if not found (and clear buffer),
		/// or call Read on buffer to consume chars and return element.
		/// Exemplar limited to begin and cannot specify attributes.
	virtual XMLElement* Read( std::string &buffer );
		/// Read next element from chars in buffer.
		/// If it's a begin element, push onto containers stack.
		/// If it's an end element, find corresponding element on stack and pop it (or throw error).
		/// Does not block.
		/// Erases read chars from buffer.
		/// Returns runtime typed element or NULL.
	XMLStack containers;
		/// Stack of begin elements we've read from buffer.
		/// Corresponding ends pop these.
	bool incomplete() { return _incomplete; }
protected:
	XMLElement* Parse( std::string &buffer, std::string::size_type e );
	bool _incomplete;
};

#include <WinSock2.h>

class IXMLSocketStream : public IXMLStream {
	/// Connector from socket stream to IXMLStream
public:
	IXMLSocketStream(SOCKET s) : _socket(s), IXMLStream() {}
	virtual XMLElement* Seek(XMLBegin* exemplar);
	virtual XMLElement* Read();
	bool empty() { return _buffer.empty(); }
protected:
	void Get();
	void Put(const char* out, int len) { send(_socket, out, len, 0); }
private:
	SOCKET _socket;
	std::string _buffer;
};

#endif
