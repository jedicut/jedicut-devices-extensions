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
#include <process.h>

int PortNumber;
DWORD baudRate;
HANDLE hCOM;
DCB	dcbSav;
bool mcBusy = false;
COMMTIMEOUTS CptimeoutsSav;
#if DEBUG_OUTPUT || DEBUG_OUTPUT_CMD
  extern FILE *fp;    // File pointer for logging
#endif

void closeCOMPort ();


/******************************************************************************************/
void getComPort()
{
	char cdir[256];
	char* ptr;

	// get the directory of the jedicut executable
	// here the ini file must be located  comport.ini

	/* content of the ini file
	;------------------------------------------------
	; COM Settings (comport.ini)
	;------------------------------------------------

	; 
	[COMPORT]
	PORTNUMBER = 4  ; means COM4
	BAUDRATE   = 115200  ;
	*/

	GetModuleFileNameA(NULL,cdir,255);
	ptr = strrchr(cdir,'\\');
	if(ptr) ptr[0] = 0;


	sprintf(cdir,"%s\\comport.ini",cdir);

	// if the ini file cant be found, default is COM1
	PortNumber = GetPrivateProfileIntA("COMPORT", "PORTNUMBER", 1, cdir);
	baudRate = GetPrivateProfileIntA("COMPORT", "BAUDRATE", 115200, cdir);

}

/******************************************************************************************/
int openComPort()
{
char tmp[64];
DCB	dcbSet;
COMMTIMEOUTS Cptimeouts;

	getComPort();

	sprintf(tmp,"\\\\.\\COM%d",PortNumber);
	hCOM=CreateFileA(tmp,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if (hCOM==INVALID_HANDLE_VALUE) return(1);
	
		if (GetCommState( hCOM,	        // handle of communications device
						  &dcbSav		// address of device-control block structure
						) == FALSE)
		{
			CloseHandle (hCOM);
			return(1);
		}

    // eventually read the baudrate etc from the ini-file too TODO
	dcbSet = dcbSav;
	dcbSet.BaudRate = baudRate;
	dcbSet.fParity = FALSE;
	dcbSet.fOutxCtsFlow = FALSE;
	dcbSet.fOutxDsrFlow = FALSE;
	dcbSet.fDtrControl = DTR_CONTROL_DISABLE;
	dcbSet.fDsrSensitivity = FALSE;
	dcbSet.fOutX = FALSE;
	dcbSet.fInX = FALSE;
	dcbSet.fRtsControl = RTS_CONTROL_DISABLE;  
	dcbSet.ByteSize = 8;
	dcbSet.Parity = NOPARITY;
	dcbSet.StopBits = ONESTOPBIT;


	if (SetCommState(
		hCOM,	// handle of communications device
		&dcbSet	// address of device-control block structure
	   ) == FALSE)
	{
		CloseHandle (hCOM);
		return(1);
	}

  

  if(!GetCommTimeouts(hCOM,&CptimeoutsSav))
  {
	  closeCOMPort ();
      return(1);
  }

  // adjust the parameters to don't wait for receive byte
  Cptimeouts.ReadIntervalTimeout         = MAXDWORD;
  Cptimeouts.ReadTotalTimeoutMultiplier  = 0; 
  Cptimeouts.ReadTotalTimeoutConstant    = 0;   
  Cptimeouts.WriteTotalTimeoutMultiplier = 0;
  Cptimeouts.WriteTotalTimeoutConstant   = 0;

  if(!SetCommTimeouts(hCOM, &Cptimeouts))
  {
	  closeCOMPort ();
      return(1);
  }
 
  return 0;
	
}
/**************************************************************************/
void closeCOMPort ()
{
	if (hCOM!=INVALID_HANDLE_VALUE)
	{
		// restore com props
		SetCommState(
				hCOM,	    // handle of communications device
				&dcbSav		// address of device-control block structure
				);
		
		SetCommTimeouts(hCOM, &CptimeoutsSav);

		// close device
		CloseHandle (hCOM);
	}

	 
}

/**************************************************************************/
/**/
void checkInput()
{
int n;
char inBuf;

	ReadFile(hCOM, &inBuf, 1, (LPDWORD)((void *)&n), NULL);
	if(n=1)
	{
		switch(inBuf)
		{
		case 'S': // stop transmission
			mcBusy = true;
			break;
		case 'C': // continue transmission
			mcBusy = false;
			break;
		case 'E':  // Limit switch hit TODO
			break;
		case 'A':  // Stop the complete cut TODO
			break;
		}
	}
}

/**************************************************************************/
void writeCommand(char* cmd)
{
int n;
    // write 4 bytes for one command
	while(mcBusy) checkInput();
	WriteFile(hCOM,cmd,4,(LPDWORD)((void*)&n),NULL);
	checkInput();
#ifdef DEBUG_OUTPUT_CMD
  fprintf(fp,"cmd: %c 0x%02x\n",cmd[0],cmd[1]);
#endif

}