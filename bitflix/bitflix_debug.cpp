#include "bitflix_debug.h"
#include <Windows.h>
#include <malloc.h>

void debug_begin()
{
#ifdef _LEAK_DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_MAP_ALLOC | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_CRT_DF );
#endif
}

void debug_end()
{
#ifdef _LEAK_DEBUG
	_CrtDumpMemoryLeaks();
#endif
}

// http://www.abstraction.net/content/articles/analyzing%20the%20heaps%20of%20a%20win32%20process.htm

void debug_dump_blocks_in_heaps()
{
#ifdef _LEAK_DEBUG
	//get all the heaps in the process
	HANDLE heaps [100];
	DWORD c = ::GetProcessHeaps (100, heaps);
	_RPT1(_CRT_WARN, "The process has %d heaps.\n", c);

	//get the default heap and the CRT heap (both are among 
	//those retrieved above)
	const HANDLE default_heap = ::GetProcessHeap ();
	const HANDLE crt_heap = (HANDLE) _get_heap_handle ();
	for (unsigned int i = 0; i < c; i++)
	{
		//query the heap attributes
		ULONG heap_info = 0;
		SIZE_T ret_size = 0;
		if (::HeapQueryInformation(heaps [i], 
                                   HeapCompatibilityInformation, 
                                   &heap_info, 
								   sizeof (heap_info), 
								   &ret_size))
        {
			//show the heap attributes
			switch (heap_info)
			{
			case 0:
				_RPT1(_CRT_WARN, "Heap %d is a regular heap.\n", (i + 1));
				break;
			case 1:
				_RPT1(_CRT_WARN, "Heap %d is a heap with look-asides (fast heap).\n", (i + 1));
				break;
			case 2:
				_RPT1(_CRT_WARN, "Heap %d is a LFH (low-fragmentation) heap.\n", (i + 1));
				break;
			default:
				_RPT1(_CRT_WARN, "Heap %d is of unknown type.\n", (i + 1));
				break;
			}

			if (heaps [i] == default_heap)
			{
				_RPT0(_CRT_WARN, " This the DEFAULT process heap.\n");
			}

			if (heaps [i] == crt_heap)
			{
				_RPT0(_CRT_WARN, " This the heap used by the CRT.\n");   
			}

			//walk the heap and show each allocated block inside it 
			//(the attributes of each entry will differ between 
			//DEBUG and RELEASE builds)
			PROCESS_HEAP_ENTRY entry;
			memset (&entry, 0, sizeof (entry));
			int count = 0;
			while (::HeapWalk (heaps [i], &entry))
			{
				if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
				{
						_RPT3(_CRT_WARN, " Allocated entry %d: size: %d, overhead: %d.\n", ++count, entry.cbData, entry.cbOverhead);
				}
			}
        }
	}
#endif
}

// Logger and display tool
// http://www.codeproject.com/KB/applications/leakfinder.aspx

/*
User Mode Debug Heap tool from MS
http://support.microsoft.com/?kbid=268343

Before You Use UMDH
 If you think that you are experiencing a memory leak, be aware that memory 
leaks may not be what they appear to be. You may discover that a memory leak
is not a true memory leak, but is a performance enhancement. For example, the
Microsoft Jet database engine can consume large amounts of memory (up to 128 MB
on a 256-MB computer) because it retrieves data and writes caches. The cache
permits the Jet engine to get fast read-ahead and write-ahead buffering.

 To determine whether or not a process is experiencing memory leaks, use
 Windows Performance Monitor (Perfmon.exe) and monitor Private Bytes under the
 Process category for your application. Private bytes is the total memory that
 the process has allocated, but is not sharing with other processes. Note that
 this is different from Virtual Bytes, which is also interesting to monitor.
 Virtual Bytes is the current size in bytes of the virtual address space that
 the process uses. An application can leak virtual memory, but may not see a
 difference in the private bytes that are allocated. If you do not see memory
 increase when you monitor private bytes, but you suspect that you are still
 running out of memory, monitor virtual bytes to see if you are using up
 virtual memory. For additional information about detecting memory leaks and
 the overview of Windows Performance Monitor (Perfmon.exe), visit the following
 Microsoft Web site:

 http://msdn.microsoft.com/en-us/library/ms404355.aspx

 To make sure that your application is leaking memory, put the suspect code in
 a loop with many iterations, and then monitor private and virtual bytes for
 any increases of memory. Watch to make sure that the number of private bytes
 and virtual bytes does not eventually stay the same and the number stops
 increasing. If there is a point at which the memory stops increasing, (for
 example, it does not continue to climb indefinitely) you do not see a memory
 leak but more likely, you see a cache that is growing to its maximum size.
*/