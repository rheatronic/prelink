// The client agent runs in a thread below the application,
// listening for frame send requests from the application,
// and status messages from the server.
// Ownership of the connection is with this thread.

// Duties
// - Open connection to server
// - Report connection status changes (open/closed)
// - Respond to shutdown request
// - Forward animation to server
// - Request status stream from server
// - Forward status to client

// Note: it is legal to receive in one thread and send in another
// on the same socket

#pragma once

#include "prelink_client.h"
#include <agents.h>

// From client (application) to server
class PrelinkClientAgent : public Concurrency::agent 
{
public:
	explicit PrelinkClientAgent(
		Concurrency::ISource<FramedState*>& source,
		Concurrency::ITarget<PrevizionUpdate*>& target)
		:sender(source), receiver(target) {}

	void run() 
	{
		sender.run();
		receiver.run();
		while(true) {
			auto selector = Concurrency::make_selector(sender, receiver);
			switch (selector) {
			case 1:
				break;
			}
		}
	}
private:
	PrelinkClientSender sender;
	PrelinkClientReceiver receiver;
};

class PrelinkClientSender : public Concurrency::agent
{
public:
	explicit PrelinkClientSender(Concurrency::ISource<FramedState*>& source)
      : _source(source)
	{
	}

protected:
	void run()
    {
	    plc.Open();
	    while(true)
	    {
		    FramedState *fs = receive(_source);
		    plc.Send(fs);
			delete fs;
	    }
	    plc.Close();

		done();
    }

private:
	PrelinkClient plc;
	Concurrency::ISource<FramedState*>& _source;
};

class PrelinkClientReceiver : public Concurrency::agent
{
public:
	PrelinkClientReceiver(Concurrency::ISource<PrevizionUpdate*>& target)
		: _target(target)
	{
	}

protected:
	void run()
	{
	}

private:
	Concurrency::ITarget<PrevizionUpdate*>& _target;
};
