#include "thread_omp_util.h"
#include <windows.h>
#include <stdio.h>

#if defined(WIN32) && defined(_DLL)
# define CSG_SOLD_EXPORT extern "C" __declspec( dllexport )
#else
# define CSG_SOLD_EXPORT /* emptyr */
#endif

CSG_SOLD_EXPORT int getCPUNum()
{
	SYSTEM_INFO inf;
	GetSystemInfo(&inf);
	printf("%d\n", inf.dwNumberOfProcessors);
	return inf.dwNumberOfProcessors;
}

CSG_SOLD_EXPORT int getCPUCoreNum()
{
	const int MAX_INFOBUFFER= 32;
	DWORD cpuinfo[MAX_INFOBUFFER];

	SYSTEM_LOGICAL_PROCESSOR_INFORMATION infobuffer[MAX_INFOBUFFER];

	DWORD retlen= 0;
	GetLogicalProcessorInformation( NULL, &retlen );
	if( retlen > sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)*MAX_INFOBUFFER ){
		return 0; // error
	}

	if( !GetLogicalProcessorInformation( infobuffer, &retlen ) ){
		return 0; // error
	}

	SYSTEM_LOGICAL_PROCESSOR_INFORMATION *infop= infobuffer;
	int coreCount= 0;
	int cpuCount= 0;
	for( DWORD size= 0 ; size < retlen ;
		size+= sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION), infop++ )
	{

		switch( infop->Relationship )
		{
			case RelationProcessorCore:
			{
				int htCount= 0;
				for( int i= 0 ; i< 32 ; i++ )
				{
					if( infop->ProcessorMask & (1<<i) )
					{
						cpuinfo[i]= ( htCount++ << 8) + ( coreCount );
						cpuCount++;
					}
				}
				coreCount++;
			}
			break;
			default:
			break;
		}
	}
	printf("cpu %d core %d\n", cpuCount, coreCount);
	return coreCount;
}
