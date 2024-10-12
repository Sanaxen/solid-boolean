/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include "Vertex.h"
#include "VertexSet.h"
#include "ColorSet.h"

#include <stdio.h>

#include <math.h>

#include <Windows.h>
#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif



#ifndef VERTEX_LINK_USE
//static int Vertex_mark_call_stack_max = 0;
static int Vertex_mark_call_stack = 0;

void Vertex_mark_metod_call_stack_init()
{
	Vertex_mark_call_stack = 0;
}

void __fastcall Vertex::mark(const int stat)
{

	Vertex_mark_call_stack++;

	this->status = status;

	//if ( Vertex_mark_call_stack_max < Vertex_mark_call_stack )
	//{
	//	Vertex_mark_call_stack_max = Vertex_mark_call_stack;
	//	//printf("call stack depth %d\n", Vertex_mark_call_stack_max);
	//}

	if ( Vertex_mark_call_stack > 3000 )
	{
		//printf("stack over ‰ñ”ð\n");
		Vertex_mark_call_stack--;
		return;
	}
	//return;

	__try
	{
		VertexPointerSet::VerticesPtrSet::const_iterator it = adjacentVertices.Get().begin();
		VertexPointerSet::VerticesPtrSet::const_iterator end = adjacentVertices.Get().end();
		for(; it != end; ++it)
		{
			Vertex * pVertexI = *it;

			if(pVertexI->status==Vertex_UNKNOWN)
			{
				pVertexI->mark(status);
			}
		}
	}__except(EXCEPTION_EXECUTE_HANDLER)
	{
		printf("Exception handler %lX\n", _exception_code());
		throw "Exception stack overflow";
	}

	Vertex_mark_call_stack--;
}
#endif
