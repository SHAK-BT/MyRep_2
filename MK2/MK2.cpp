#include <windows.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "MK.h"


u16 crcSlow_data (u8 *message, int nBytes)
{
	u16 remainder = 0xFFFF;
	int byte = 0;
	int i = 0;
	u8  bit;

	  for (byte = 0; byte < nBytes; ++byte)
	  {
		   
		    remainder ^= ( *(message+byte)  << (8 * (sizeof(u16) - 1)));
		    for (bit = 8; bit > 0; --bit)
		    {
			      if (remainder & (1 << (8 * sizeof(u16) - 1)))
			      {
			        	remainder = (remainder << 1) ^ 0x1021;
			      }
			      else
			      {
			        	remainder = (remainder << 1);
			      }
		    }
	  }
  
    return remainder;
}



A1_tmp TEMP;
A2_loc LOCATION;
A3_stt STT;


DWORD WINAPI ThreadProc0(CONST LPVOID lpParam) {

	CONST HANDLE hMutex = (CONST HANDLE)lpParam;
	DWORD i;
  	
  	while(1)
  	{
  		if(LOCATION.loc[1] != 0)
  			break;
  	}	
	  	

	  	while(1)
	  	{
	  		
	  			Sleep(100);
	  		
	  			printf("TEMP: %d \n",TEMP.tmp);
				printf("LOCATION: ");
					 		for(int j=0;j<32;j++)
							{
								if(j%8 == 0)
					 				printf("\n");							
									printf(" %5d ",LOCATION.loc[j]);
							}
				 			
				printf("\n");			
				printf("STT: %d \n",STT.stt);	
				printf("\n");		
	  	}	
    

 
  	 
  	ExitThread(0);
}





void main()
{
    setlocale(LC_ALL, "RUS");

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

    printf("#####______      MK 2      ______#####\n");

    


    HANDLE hComm;

  	hComm = CreateFile("\\\\.\\COM11",                
                      GENERIC_READ | GENERIC_WRITE, 
                      0,                            
                      NULL,                         
                      OPEN_EXISTING,
                      0,            
                      NULL);        

	  if (hComm == INVALID_HANDLE_VALUE)
	      printf("Error in opening serial port\n");
	  else
	      printf("opening serial port successful\n");


	  DCB dcbSerialParams = { 0 }; 
	  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	  bool Status = GetCommState(hComm, &dcbSerialParams);

	  if(Status)
	  	printf("Read Status NORM!\n");
	  else
	  	printf("Read Status ERROR!\n");


	  dcbSerialParams.BaudRate = CBR_19200;  
	  dcbSerialParams.ByteSize = 8;         
	  dcbSerialParams.StopBits = ONESTOPBIT;
	  dcbSerialParams.Parity   = ODDPARITY;  


	   Status = GetCommState(hComm, &dcbSerialParams);

	  if(Status)
	  	printf("Set Settings NORM!\n");
	  else
	  	printf("Set Settings ERROR!\n"); 

	  COMMTIMEOUTS timeouts = { 0 };
		timeouts.ReadIntervalTimeout         = 1; 
		timeouts.ReadTotalTimeoutConstant    = 2;
		timeouts.ReadTotalTimeoutMultiplier  = 1; 
	
  	 Status = SetCommTimeouts(hComm, &timeouts);

  	  if(Status)
	  	printf("Set Timeouts NORM!\n");
	  else
	  	printf("Set Timeouts ERROR!\n");


	  Status = SetCommMask(hComm, EV_RXCHAR);
	  if(Status)
	  	printf("Set SetCommMask NORM!\n");
	  else
	  	printf("Set SetCommMask ERROR!\n");



	  	HANDLE hThread;
	  	hThread = CreateThread(NULL, 0, &ThreadProc0, NULL, 0, NULL);
    	if(NULL == hThread) {
      		printf("Failed to create thread 1.\r\n");
    	}
	  
	  
	  while(1)
	  {
						
	  					u8 type;
	  					

						  PurgeComm(hComm,PURGE_TXABORT |
										   PURGE_RXABORT |
										   PURGE_TXCLEAR |
										   PURGE_RXCLEAR );
						  
						 DWORD dwEventMask; 	
						 	
						  Status = WaitCommEvent(hComm, &dwEventMask, NULL);
						  
						  if(!Status)
						  	printf("Set WaitCommEvent ERROR!\n");
						  
						  
						  	u8 TempChar; 
							u8 SerialBuffer[256];
							DWORD NoBytesRead;
							int i = 0;
							u8 rcv_cnt;
							
							do
						    {
							   ReadFile( hComm,           
							             &TempChar,      
							             1,     
							             &NoBytesRead,    
							             NULL);
							   
							   
							    if( i == 0 )
							    {	

							    	type = TempChar;
							   		if(TempChar == TEMP_hdr)
							   		{
							   			
							   			 rcv_cnt = TEMP_ln;

							   		}
							   		else if(TempChar == LOC_hdr)
							   		{
							   			rcv_cnt = LOC_ln;
							   		}
							   		else if(TempChar == STT_hdr)
							   		{
							   			rcv_cnt = STT_ln;
							   		}
							   		else
							   		{
							   		    
							   		    PurgeComm(hComm,PURGE_TXABORT |
														   PURGE_RXABORT |
														   PURGE_TXCLEAR |
														   PURGE_RXCLEAR );
							   			break;
							   		}	
							    }

							   SerialBuffer[i] = TempChar;
							   i++;

							   if(i == rcv_cnt)
							   { 
							   	 PurgeComm(hComm,PURGE_TXABORT |
												   PURGE_RXABORT |
												   PURGE_TXCLEAR |
												   PURGE_RXCLEAR );
							   	
							 

							   	 switch(type)
							   	 {
									   	 case LOC_hdr:
											   	 {	
												   	 u16 crc =  crcSlow_data( &SerialBuffer[0], rcv_cnt - 2);
												   	 if(crc == *( (u16*)(&SerialBuffer[rcv_cnt - 2])))
												   	 {		
												   	 		memcpy(LOCATION.loc,&SerialBuffer[1],LOC_ln-3);
												   	 	
												 	 }		
												   	 else
												   	 		printf("CRC LOCATION ERR!\n");	
												   	
												  
												   	 
												 }
												 break;
										 case TEMP_hdr:
										 case STT_hdr:   		 
												 {
												 		if(SerialBuffer[2] == SerialBuffer[0]^SerialBuffer[1])
												 		{
												 			if(type == TEMP_hdr)
												 			{
												 				
												 				memcpy(&TEMP.tmp,&SerialBuffer[1],1);
												 			}
												 			else if(type == STT_hdr)
												 			{	
												 				
												 				memcpy(&STT.stt,&SerialBuffer[1],1); 
												 			}
												 		}
												 		else
												 			printf("CRC 0x%2X ERR!\n",type);
												 }
												 break;	
										default:
											break;		 
								  }				

							   }


							  }

							while (NoBytesRead > 0);

						
							PurgeComm(hComm,PURGE_TXABORT |
												   PURGE_RXABORT |
												   PURGE_TXCLEAR |
												   PURGE_RXCLEAR );

						
		}



	  CloseHandle(hComm);
    getch();

 }