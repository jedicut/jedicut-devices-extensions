/*  Copyright 2012 Martin

    This file is part of jedicutplugin.

    Foobar is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "stdafx.h"

#include <stdio.h>

FILE* fp=NULL;

extern void closeCOMPort ();
extern int openComPort();
extern int PortNumber;;

/******************************************************************************************/
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
int ret=0;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:	

#if DEBUG_OUTPUT || DEBUG_OUTPUT_CMD

		if(fp == NULL)
		   fp = fopen("logging.txt","a");
#endif
		ret = openComPort();
#ifdef DEBUG_OUTPUT
		fprintf(fp,"COM Port COM%d opened with return status %d\n",PortNumber,ret);
#endif
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		closeCOMPort();
#if DEBUG_OUTPUT || DEBUG_OUTPUT_CMD
		fclose(fp);
		fp = NULL;
#endif
		break;
	}
	return TRUE;
}

