#pragma once

#include <stack>
#include <string>
#include <exception>
class PrelinkMessage;
class XMLElement;
class ParseState;

typedef std::stack<ParseState*> StateStack;

class StateException : public std::exception {
public:
	StateException(XMLElement* element, StateStack& states, std::string msg)
		: _element(element), _states(&states), _msg(msg) {}
	StateStack* _states;
	XMLElement* _element;
	std::string _msg;
};

class ParseState {
public:
	static void initialize(StateStack& stack);
	virtual PrelinkMessage* first(XMLElement* element, StateStack& stack) = NULL;
	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) = NULL;
};
