/*
 Prelink (C) 2010 by bitflix.  All rights reserved.
*/

/**	\file	prelink.cxx
*	Library declarations.
*	Contains the basic routines to declare the DLL as a loadable
*	library for FiLMBOX.
*/

#include "bitflix/bitflix_debug.h"

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare( prelink )
{
	FBLibraryRegister( Prelink		);
	FBLibraryRegister( PrelinkLayout	);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ debug_begin(); return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ debug_end(); return true; }

/**
*	\mainpage	Previzion Link
*	\section	intro	Introduction
*	Sends model translation, rotation and scaling changes to
*   Previzion, either after GUI transactions, or as a continuous
*   stream (for running animation).
*	\section	walk	Walkthrough
*	\subsection	func1	FBCreate()
*	<ol>
*		<li>Create input nodes (FiLMBOX side);
*		<li>Create output nodes (Hardware side);
*		<li>Create model templates;
*		<li>Bind the model templates to the output nodes;
*		<li>Assign the icon name.
*	</ol>
*	\subsection	func2	AnimationNodeNotify()
*	<ol>
*		<li>Read the data off the input nodes;
*		<li>Write the data onto the output nodes.
*	</ol>
*	\subsection	func3	DeviceEvaluationNotify()
*	<ol>
*		<li>If stop/play;
*		<li>Read the data from the output nodes;
*		<li>Write the data to the hardware abstraction;
*		<li>Acknowledge the receipt of a sample.
*	</ol>
*	\subsection	func4	DeviceIONotify()
*	<ol>
*		<li>Send the data from the hardware abstraction to the hardware itself.
*	</ol>
*/
