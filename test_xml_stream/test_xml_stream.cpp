#include <iostream>
#include "xml_stream.h"

// Test the IXML stream
// The one thing it doesn't handle is a buffer with partial elements, 
// those will fail as npos appears in find calls

#define TEST(N,M,C) std::cout << ((C) ? "PASS:" : "FAIL:") << N << " " << M << std::endl;

void test_xml_stream()
{
	std::cout << "test_xml_stream" << std::endl;

	std::string buffer;
	IXMLStream ix;
	XMLElement* e;
	XMLBegin* xb;
	XMLText* xt;
	XMLEnd* xe;
	XMLSingleton* xs;

	buffer = "lkajsdf";
	e = ix.Read(buffer);
	TEST("No element","returns NULL",!e);
	TEST("No element","leaves buffer unchanged",buffer == "lkajsdf");
	
	buffer = "lakjsdf><foo";
	e = ix.Read(buffer);
	TEST("Text then partial element","returns XMLText",dynamic_cast<XMLText*>(e));
	TEST("Text then partial element","leaves <foo in buffer",buffer == "<foo");

	buffer = "<prelink foo?\"bar\">";
	try {
		e = ix.Read(buffer);
		TEST("Bad attr syntax","threw exception",false);
	}
	catch(XMLErrorSyntax*) {
		TEST("Bad attr syntax","threw exception",true);
	}

	//

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion>";
	e = ix.Read(buffer);
	xb = dynamic_cast<XMLBegin*>(e);
	TEST("Begin alone","return type",xb);
	TEST("Begin alone","name of element",xb->name == "previzion");
	TEST("Begin alone","no attrs in element",!xb->attrs.size());
	TEST("Begin alone","buffer empty after state",buffer.empty());
	TEST("Begin alone","container pushed",ix.containers.size() == 1);

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion>a;lskdjf;</previzion>";

	e = ix.Read(buffer);
	xb = dynamic_cast<XMLBegin*>(e);
	TEST("Begin Text End","begin return type",xb);
	TEST("Begin Text End","begin name of element",xb->name == "previzion");
	TEST("Begin Text End","begin no attrs in element",!xb->attrs.size());
	TEST("Begin Text End","begin buffer after text",buffer == "a;lskdjf;</previzion>");
	TEST("Begin Text End","begin container pushed",ix.containers.size() == 1);

	e = ix.Read(buffer);
	xt = dynamic_cast<XMLText*>(e);
	TEST("Begin Text End","text return type",xt);
	TEST("Begin Text End","text content",xt->text == "a;lskdjf;");
	TEST("Begin Text End","text no attrs in element",!xt->attrs.size());
	TEST("Begin Text End","text buffer after text",buffer == "</previzion>");
	TEST("Begin Text End","text container pushed",ix.containers.size() == 1);

	e = ix.Read(buffer);
	xe = dynamic_cast<XMLEnd*>(e);
	TEST("Begin Text End","end return type",xe);
	TEST("Begin Text End","end name of element",xe->name == "previzion");
	TEST("Begin Text End","end no attrs in element",!xe->attrs.size());
	TEST("Begin Text End","end buffer after text",buffer.empty());
	TEST("Begin Text End","end container pushed",ix.containers.size() == 0);

	//

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion />";
	e = ix.Read(buffer);
	xs = dynamic_cast<XMLSingleton*>(e);
	TEST("Singleton alone","return type",xs);
	TEST("Singleton alone","name of element",xs->name == "previzion");
	TEST("Singleton alone","no attrs in element",!xs->attrs.size());
	TEST("Singleton alone","buffer after text",buffer.empty());
	TEST("Singleton alone","containers empty",ix.containers.empty());

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion />a;lskdjf;</previzion>";
	e = ix.Read(buffer);
	xs = dynamic_cast<XMLSingleton*>(e);
	TEST("Singleton then text","return type",xs);
	TEST("Singleton then text","name of element",xs->name == "previzion");
	TEST("Singleton then text","no attrs in element",!xs->attrs.size());
	TEST("Singleton then text","buffer after text",buffer == "a;lskdjf;</previzion>");
	TEST("Singleton then text","containers empty",ix.containers.empty());

	//

	while( !ix.containers.empty() ) ix.containers.pop();
	ix.containers.push(new XMLBegin("previzion"));
	buffer = "</previzion>";
	e = ix.Read(buffer);
	xe = dynamic_cast<XMLEnd*>(e);
	TEST("End alone","return type",xe);
	TEST("End alone","name of element",xe->name == "previzion");
	TEST("End alone","no attrs in element",!xe->attrs.size());
	TEST("End alone","buffer empty after state",buffer.empty());
	TEST("End alone","container popped",ix.containers.empty());

	while( !ix.containers.empty() ) ix.containers.pop();
	ix.containers.push(new XMLBegin("previzion"));
	buffer = "</previzion>a;lskdjf;</previzion>";
	e = ix.Read(buffer);
	xe = dynamic_cast<XMLEnd*>(e);
	TEST("End then text","return type",xe);
	TEST("End then text","name of element",xe->name == "previzion");
	TEST("End then text","no attrs in element",!xe->attrs.size());
	TEST("End then text","buffer after text",buffer == "a;lskdjf;</previzion>");
	TEST("End then text","container popped",ix.containers.empty());

	//

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "</previzion>";
	try {
		e = ix.Read(buffer);
		TEST("End stack empty","threw exception",false);
	}
	catch(XMLErrorEnd*) {
		TEST("End stack empty","threw exception",true);
	}

	while( !ix.containers.empty() ) ix.containers.pop();
	ix.containers.push(new XMLBegin("foobar"));
	buffer = "</previzion>a;lskdjf;</foobar>";
	try {
		e = ix.Read(buffer);
		TEST("End stack wrong name","threw exception",false);
	}
	catch(XMLErrorEnd*) {
		TEST("End stack wrong name","threw exception",true);
	}

	// attrs, one or more on each different type

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion a=\"foo\">";
	e = ix.Read(buffer);
	xb = dynamic_cast<XMLBegin*>(e);
	TEST("Begin w/1 attr","return type",xb);
	TEST("Begin w/1 attr","name of element",xb->name == "previzion");
	TEST("Begin w/1 attr","attrs in element",xb->attrs.size() == 1 && xb->attrs["a"] == "foo");
	TEST("Begin w/1 attr","buffer empty after state",buffer.empty());
	TEST("Begin w/1 attr","container pushed",ix.containers.size() == 1);

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion a=\"foo\" b=\"bar\">";
	e = ix.Read(buffer);
	xb = dynamic_cast<XMLBegin*>(e);
	TEST("Begin w/2 attrs","return type",xb);
	TEST("Begin w/2 attrs","name of element",xb->name == "previzion");
	TEST("Begin w/2 attrs","attrs in element",xb->attrs.size() == 2 && xb->attrs["a"] == "foo" && xb->attrs["b"] == "bar");
	TEST("Begin w/2 attrs","buffer empty after state",buffer.empty());
	TEST("Begin w/2 attrs","container pushed",ix.containers.size() == 1);


	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion a=\"foo\" />";
	e = ix.Read(buffer);
	xs = dynamic_cast<XMLSingleton*>(e);
	TEST("Singleton w/1 attr","return type",xs);
	TEST("Singleton w/1 attr","name of element",xs->name == "previzion");
	TEST("Singleton w/1 attr","attrs in element",xs->attrs.size() == 1 && xs->attrs["a"] == "foo");
	TEST("Singleton w/1 attr","buffer empty after state",buffer.empty());
	TEST("Singleton w/1 attr","container empty",ix.containers.empty());

	while( !ix.containers.empty() ) ix.containers.pop();
	buffer = "<previzion a=\"foo\" b=\"bar\" />";
	e = ix.Read(buffer);
	xs = dynamic_cast<XMLSingleton*>(e);
	TEST("Singleton w/2 attrs","return type",xs);
	TEST("Singleton w/2 attrs","name of element",xs->name == "previzion");
	TEST("Singleton w/2 attrs","attrs in element",xs->attrs.size() == 2 && xs->attrs["a"] == "foo" && xs->attrs["b"] == "bar");
	TEST("Singleton w/2 attrs","buffer empty after state",buffer.empty());
	TEST("Singleton w/2 attrs","container empty",ix.containers.empty());

	// Doubt end will have attrs (may not be legal)

	std::cout << std::endl;
}
