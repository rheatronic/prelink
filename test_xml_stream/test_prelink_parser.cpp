#include <iostream>
#include "test_prelink_parser.h"
#include "prelink_parser.h"
#include "xml_stream.h"
#include "prelink_messages.h"

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

#define TEST(N,M,C) std::cout << ((C) ? "PASS:" : "FAIL:") << N << " " << M << std::endl;

void test_camera_request()
{
	std::cout << "test camera_request" << std::endl;

	{
		StateStack stack;
		PrelinkMessage* plm;

		{
			ParseState::initialize(stack);
			TEST("initialize", "stack depth", stack.size() == 1);
		}


		if( false ){
			XMLText* xt = new XMLText("foo");

			plm = stack.top()->next(xt, stack);
			TEST("leader text", "returned null", !plm);
			TEST("leader text", "stack depth", stack.size() == 1);
		}

		{
			XMLBegin* pb = new XMLBegin("previzion");

			plm = stack.top()->next(pb, stack);
			TEST("previzion begin", "returned null", !plm);
			TEST("previzion begin", "stack depth", stack.size() == 2);
		}

		{
			XMLBegin* sb = new XMLBegin("status_request");
			sb->attrs["value"] = "true";

			plm = stack.top()->next(sb, stack);
			TEST("status_request begin", "returned null", !plm);
			TEST("status_request begin", "stack depth", stack.size() == 3);
		}

		{
			XMLBegin* cb = new XMLBegin("camera");

			plm = stack.top()->next(cb, stack);
			TEST("camera begin", "returned null", !plm);
			TEST("camera begin", "stack depth", stack.size() == 4);
		}

		{
			XMLEnd* ce = new XMLEnd("camera");

			plm = stack.top()->next(ce, stack);
			TEST("camera end", "returned null", !plm);
			TEST("camera end", "stack depth", stack.size() == 3);
		}

		{
			XMLEnd* se = new XMLEnd("status_request");

			plm = stack.top()->next(se, stack);
			PrelinkStatusRequest* psr = dynamic_cast<PrelinkStatusRequest*>(plm);
			TEST("status_request end", "returned message type", psr);
			TEST("status_request end", "returned message content", psr->status && psr->items.size() == 1 && psr->items[0] == "camera");
			TEST("status_request end", "stack depth", stack.size() == 2);
		}

		{
			XMLEnd* pe = new XMLEnd("previzion");

			plm = stack.top()->next(pe, stack);
			TEST("previzion end", "returned null", !plm);
			TEST("previzion end", "stack depth", stack.size() == 1);
		}
	}

	std::cout << std::endl;
}

void test_frame()
{
	std::cout << "test frame" << std::endl;

	{
		StateStack stack;
		PrelinkMessage* plm;
		int expected_depth = 0;

		{
			ParseState::initialize(stack);
			expected_depth++;

			TEST("initialize", "stack depth", stack.size() == expected_depth);
		}

		if( true ){
			XMLText* xt = new XMLText("foo");

			plm = stack.top()->next(xt, stack);

			TEST("leader text", "returned null", !plm);
			TEST("leader text", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLBegin* pb = new XMLBegin("previzion");

			plm = stack.top()->next(pb, stack);
			expected_depth++;

			TEST("previzion begin", "returned null", !plm);
			TEST("previzion begin", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLBegin* sb = new XMLBegin("animation");

			plm = stack.top()->next(sb, stack);
			expected_depth++;

			TEST("animation begin", "returned null", !plm);
			TEST("animation begin", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLBegin* e = new XMLBegin("frame");
			e->attrs["timecode"] = "01:02:03:04";
			e->attrs["rate"] = "23.976";

			plm = stack.top()->next(e, stack);
			expected_depth++;

			TEST("frame begin", "returned null", !plm);
			TEST("frame begin", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLBegin* e = new XMLBegin("node");
			e->attrs["id"] = "Null";

			plm = stack.top()->next(e, stack);
			expected_depth++;

			TEST("node begin", "returned null", !plm);
			TEST("node begin", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLBegin* e = new XMLBegin("property");
			e->attrs["id"] = "visible";

			plm = stack.top()->next(e, stack);
			expected_depth++;

			TEST("property begin", "returned null", !plm);
			TEST("property begin", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLText* e = new XMLText("true");

			plm = stack.top()->next(e, stack);
			
			TEST("property text", "returned null", !plm);
			TEST("property text", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLEnd* pe = new XMLEnd("property");

			plm = stack.top()->next(pe, stack);
			expected_depth--;

			TEST("property end", "returned null", !plm);
			TEST("property end", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLBegin* e = new XMLBegin("matrix");

			plm = stack.top()->next(e, stack);
			expected_depth++;

			TEST("matrix end", "returned null", !plm);
			TEST("matrix end", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLText* e = new XMLText("1.0 0.0 0.0 0.0  0.0 1.0 0.0 0.0  0.0 0.0 1.0 0.0  0.0 0.0 0.0 1.0");

			plm = stack.top()->next(e, stack);

			TEST("matrix text", "returned null", !plm);
			TEST("matrix text", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLEnd* se = new XMLEnd("matrix");

			plm = stack.top()->next(se, stack);
			expected_depth--;

			TEST("matrix end", "returned null", !plm);
			TEST("matrix end", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLEnd* se = new XMLEnd("node");

			plm = stack.top()->next(se, stack);
			expected_depth--;

			TEST("node end", "returned null", !plm);
			TEST("node end", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLEnd* se = new XMLEnd("frame");

			plm = stack.top()->next(se, stack);
			expected_depth--;

			PrelinkFrame* pf = dynamic_cast<PrelinkFrame*>(plm);
			TEST("frame end", "returned type", pf);
			TEST("frame end", "returned timecode", pf->timecode == Timecode("01:02:03:04", 23.976f));
			TEST("frame end", "returned # nodes", pf->nodes.size() == 1);
			TEST("frame end", "returned node name", pf->nodes[0]->name == "Null");
			TEST("frame end", "returned matrix", pf->nodes[0]->matrix.matrix[0][0] == 1.0);
			TEST("frame end", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLEnd* pe = new XMLEnd("animation");

			plm = stack.top()->next(pe, stack);
			expected_depth--;

			TEST("animation end", "returned null", !plm);
			TEST("animation end", "stack depth", stack.size() == expected_depth);
		}

		{
			XMLEnd* pe = new XMLEnd("previzion");

			plm = stack.top()->next(pe, stack);
			expected_depth--;

			TEST("previzion end", "returned null", !plm);
			TEST("previzion end", "stack depth", stack.size() == expected_depth);
		}
	}

	std::cout << std::endl;
}

void test_prelink_parser()
{
	std::cout << "test prelink parser" << std::endl;
	test_camera_request();
	test_frame();
	std::cout << std::endl;
}