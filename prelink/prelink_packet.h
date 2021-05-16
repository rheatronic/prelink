#ifndef __PRELINK_PACKET_H__
#define __PRELINK_PACKET_H__

#include "bitflix/bitflix_debug.h"

#include "prelink_state.h"
#include "prelink_container.h"
#include <strstream>
#include <stack>

class Packet
{
public:
	Packet() {}
	Packet(const Packet &p) { this->buffer << p.buffer; }	// based on docs, etc this should work ;-)
	void begin(Container &container) { container.writeBegin(this->buffer); this->stack.push(&container); }
	void add(State &state) { state.write(this->buffer); }
	void add(Packet &packet) { this->buffer << packet.result(); }
	void end() { this->stack.top()->writeEnd(this->buffer); this->stack.pop(); }
	char* result() { this->buffer << std::ends; char *s = this->buffer.str(); this->buffer.freeze(false); return s; }
private:
	std::stack<Container*> stack;
	std::ostrstream buffer;
};

#define DEFAULT_PORT 4444

#endif
