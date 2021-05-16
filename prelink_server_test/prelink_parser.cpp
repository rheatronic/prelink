#include "prelink_parser.h"
#include "xml_stream.h"
#include "prelink_messages.h"
#include <iostream>
#include <sstream>
#include <string>

/* 

from client we receive either:

...status request

<previzion>
	<status_request value="true">
		<camera></camera>
	</status_request>
</previzion>

...or animation send (which can be huge; many nodes and possibly multiple frames)

<previzion>
	<animation>
		<frame timecode="00:00:04:08" rate="29.97">
			<node id="node0" >
				<matrix>1.0 0.0 0.0 0.0 0.0 1.0 0.0 40.633 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0</matrix>
			</node>
			<node id="node1" >
				<property id="visible" >true</property>
				<matrix>1.0 0.0 0.0 0.0 0.0 1.0 0.0 15.747 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0</matrix>
			</node>
		</frame>
	</animation>
</previzion>

*/

class BlockState : public ParseState {
public:
	BlockState(const char* name) : _name(name) {}

	virtual PrelinkMessage* first(XMLElement* element, StateStack& stack) {
		stack.push(this);
		return NULL;
	}

	bool ending(XMLElement* element, StateStack& stack) {
		XMLEnd* end = dynamic_cast<XMLEnd*>(element);
		if( end ) {
			if( end->name == _name ) {
				stack.pop();
				return true;
			} else {
				throw StateException(element, stack, "Unexpected end element");
			}
		}

		return false;
	}

protected:
	std::string _name;
};

class MatrixState : public BlockState {
public:
	MatrixState(PrelinkNode* pln) : _pln(pln), BlockState("matrix") {}

	// first is default

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		if( ending(element, stack) )
			return NULL;

		// next parses text element containing 16 floats
		XMLText* te = dynamic_cast<XMLText*>(element);
		if( te ) {
			std::istringstream iss(te->text, std::istringstream::in);
			try {
				for(int r=0; r<4; r++)
					for(int c=0; c<4; c++)
						iss >> _pln->matrix.matrix[r][c];
			}
			catch(std::ios_base::failure &) {
				throw new StateException(element, stack, "missing or bad floats in matrix element");
			}
			return NULL;
		}

		throw new StateException(element, stack, "expected /matrix or 16 floats");
	}

protected:
	PrelinkNode* _pln;
};

class PropertyState : public BlockState {
public:
	PropertyState(PrelinkNode* pln) : _pln(pln), BlockState("property") {}

	virtual PrelinkMessage* first(XMLElement* element, StateStack& stack) {
		// parse id="visible" attribute
		auto idAttr = element->attrs.find("id");
		if( idAttr == element->attrs.cend() )
			throw new StateException(element, stack, "'property' element missing 'id' attribute");

		if( idAttr->second != "visible" )
			throw new StateException(element, stack, "unknown property for node");

		return BlockState::first(element, stack);
	}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		// parse ending or text element with either true or false
		if( ending(element, stack) )
			return NULL;

		XMLText* te = dynamic_cast<XMLText*>(element);
		if( te ) {
			if( te->text == "true" )
				_pln->visibility = PrelinkNode::visible;
			else if( te->text == "false" )
				_pln->visibility = PrelinkNode::invisible;
			else
				throw new StateException(element, stack, "bad value for property visible, expected true or false");
			return NULL;
		}

		throw new StateException(element, stack, "unexpected element in property block");
	}

protected:
	PrelinkNode* _pln;
};

class NodeState : public BlockState {
public:
	NodeState(PrelinkNode* pln) : _pln(pln), BlockState("node") {}

	virtual PrelinkMessage* first(XMLElement* element, StateStack& stack) {
		// id
		auto idAttr = element->attrs.find("id");
		if( idAttr == element->attrs.cend() )
			throw new StateException(element, stack, "'node' element missing 'id' attribute");

		_pln->name = idAttr->second;
		_pln->visibility = PrelinkNode::nochange;

		return BlockState::first(element, stack);
	}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		if( ending(element, stack) )
			return NULL;

		// property
		// matrix
		XMLBegin* begin = dynamic_cast<XMLBegin*>(element);
		if( begin ) {
			if( begin->name == "property" ) {
				PropertyState* ps = new PropertyState(_pln);
				return ps->first(element, stack);
			}
			if( begin->name == "matrix" ) {
				MatrixState* ms = new MatrixState(_pln);
				return ms->first(element, stack);
			}
		}

		throw StateException(element, stack, "Expected /node, property or matrix element");
	}

protected:
	PrelinkNode *_pln;
};

class FrameState : public BlockState {
public:
	FrameState() : BlockState("frame") {}
	
	virtual PrelinkMessage* first(XMLElement* element, StateStack& stack) {
		// optional timecode attributes
		auto tcAttr = element->attrs.find("timecode");
		if(tcAttr != element->attrs.cend()) {
			auto rateAttr = element->attrs.find("rate");
			float rate;
			if(rateAttr == element->attrs.cend())
				//throw new StateException(element, stack, "'timecode' attribute in 'frame' block missing 'rate' attribute");
				rate = 0.0f;
			else
				rate = (float)atof(rateAttr->second.c_str());
			_plf.timecode = Timecode(tcAttr->second.c_str(), rate);
		}

		return BlockState::first(element, stack);
	}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		if( ending(element, stack) )
			return new PrelinkFrame(_plf);

		XMLBegin* begin = dynamic_cast<XMLBegin*>(element);
		if( begin ) {
			if( begin->name == "node" ) {
				PrelinkNode* pln = new PrelinkNode();
				_plf.nodes.push_back(pln);
				NodeState* ns = new NodeState(pln);
				return ns->first(element, stack);
			}
		}

		throw StateException(element, stack, "Expected /frame or node element");
	}
protected:
	PrelinkFrame _plf;
};

class AnimationState : public BlockState {
public:
	AnimationState() : BlockState("animation") {}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		if( ending(element, stack) )
			return NULL;

		// frame
		XMLBegin* begin = dynamic_cast<XMLBegin*>(element);
		if( begin ) {
			if( begin->name == "frame" ) {
				FrameState* state = new FrameState();
				return state->first(element, stack);
			}
		}

		throw StateException(element, stack, "Expected /animation or frame element");
	}
};

class CameraRequestState : public BlockState {
public:
	CameraRequestState() : BlockState("camera") {}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		if( ending(element, stack) )
			return NULL;
		throw StateException(element, stack, "Unexpected element");
	}
};

class StatusRequestState : public BlockState {
public:
	StatusRequestState() : BlockState("status_request") {}

	virtual PrelinkMessage* first(XMLElement* element, StateStack& stack) {
		auto attr = element->attrs.find("value");
		if(attr == element->attrs.cend())
			throw new StateException(element, stack, "Unknown attribute in status request, expected value=true or false");

		_plsr.status = attr->second == "true" ? true : false;

		return BlockState::first(element, stack);
	}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		if( ending(element, stack) )
			return  new PrelinkStatusRequest(_plsr);

		XMLBegin* begin = dynamic_cast<XMLBegin*>(element);
		if( begin ) {
			if( begin->name == "camera" ) {
				_plsr.items.push_back("camera");

				CameraRequestState* crs = new CameraRequestState();
				return crs->first(element, stack);
			}
		}

		throw StateException(element, stack, "Expected /previzion, status or animation element");
	}

protected:
	PrelinkStatusRequest _plsr;
};

class PrevizionState : public BlockState {
public:
	PrevizionState() : BlockState("previzion") {}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		if( ending(element, stack) )
			return NULL;

		XMLBegin* begin = dynamic_cast<XMLBegin*>(element);
		if( begin ) {
			if( begin->name == "status_request" ) {
				StatusRequestState* ss = new StatusRequestState();
				return ss->first(element, stack);
			} else if( begin->name == "animation" ) {
				AnimationState* as = new AnimationState();
				return as->first(element, stack);
			}
		}

		throw StateException(element, stack, "Expected /previzion, status or animation");
	}
};

class UnknownState : public ParseState {
public:
	UnknownState() {}

	virtual PrelinkMessage* first(XMLElement* element, StateStack& stack) {
		stack.push(this);
		return NULL;
	}

	virtual PrelinkMessage* next(XMLElement* element, StateStack& stack) {
		XMLBegin* begin = dynamic_cast<XMLBegin*>(element);
		if( begin && begin->name == "previzion" ) {
			PrevizionState* ps = new PrevizionState();
			return ps->first(element, stack);
		}

		// ignore anything else
		return NULL;
	}
};

void ParseState::initialize(StateStack& stack) {
	while( !stack.empty() ) stack.pop();
	ParseState* ps = new UnknownState();
	ps->first(NULL, stack);
}
