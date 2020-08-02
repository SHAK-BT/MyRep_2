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

void RangedRandDemo( int range_min, int range_max, int n , void* bgn)  
{  
  
   int i;
   for ( i = 0; i < n; i++ )  
   {  
      int u = (double)rand() / (RAND_MAX + 1) * (range_max - range_min)  
            + range_min;  
      
     if(range_max == max_LOC_range)       
     { 	
     	*( ((unsigned short*)bgn) + i ) = (unsigned short)u;
   	 }
   	 else if(range_max == max_TEMP_range)
   	 {
   	 	*( (char*)bgn + i*sizeof(char) ) = (char)u;
     }	
   } 


}  
  


A1_tmp TEMP;

A2_loc LOCATION;

A3_stt STT;


HANDLE hComm;



DWORD WINAPI ThreadProc1(CONST LPVOID lpParam) {
  CONST HANDLE hMutex = (CONST HANDLE)lpParam;
  DWORD i;
  
   

	 while(1)
	 {	
	  	  WaitForSingleObject(hMutex, INFINITE);
	    	
	    	
	    				DWORD dNoOFBytestoWrite;         
						DWORD dNoOfBytesWritten = 0;     
						
						RangedRandDemo( 0, max_LOC_range, 32 ,&LOCATION.loc[0]);
				    	LOCATION.crc = crcSlow_data((u8*)&LOCATION, sizeof(u16)*32 + 1);


						dNoOFBytestoWrite = sizeof(A2_loc);
					

						bool Status = WriteFile(hComm,        
						                   &LOCATION,     
						                   dNoOFBytestoWrite,  
						                   &dNoOfBytesWritten, 
						                   NULL);

						ReleaseMutex(hMutex);

						 if(!Status)
					  		printf("THREAD 1 LOCATION Send ERROR!\n");

	      
	      Sleep(42);			// 24 times per sec ~= 0,042 sec
	  }
	  ExitThread(0);
}

DWORD WINAPI ThreadProc2(CONST LPVOID lpParam) {
  CONST HANDLE hMutex = (CONST HANDLE)lpParam;
  DWORD i;
  
	 while(1)
	 {	
	  	  RangedRandDemo( min_TEMP_range, max_TEMP_range, 1 ,&TEMP.tmp);
	  	  WaitForSingleObject(hMutex, INFINITE);
	    	
	    	
	  	  	DWORD dNoOFBytestoWrite;        
			DWORD dNoOfBytesWritten = 0;     
						
			
			TEMP.crc = TEMP.hdr ^ TEMP.tmp;	    	


						dNoOFBytestoWrite = sizeof(A1_tmp);
						

						bool Status = WriteFile(hComm,       
						                   &TEMP,     
						                   dNoOFBytestoWrite,  
						                   &dNoOfBytesWritten, 
						                   NULL);

						ReleaseMutex(hMutex);	

						 if(!Status)
					  		printf("THREAD 2 TEMP Send ERROR!\n");

	      
	      
	      Sleep(1000);     
	  }
	  ExitThread(0);
}

DWORD WINAPI ThreadProc3(CONST LPVOID lpParam) {
  CONST HANDLE hMutex = (CONST HANDLE)lpParam;
  DWORD i;
  
 
	  u32 cnt_change_status = 0;

	 while(1)
	 {	
	  	  WaitForSingleObject(hMutex, INFINITE);
	    	
	    		
	    	DWORD dNoOFBytestoWrite;         
			DWORD dNoOfBytesWritten = 0;     
						
			STT.hdr = 0xA3;
			STT.crc = STT.hdr ^ STT.stt;	    	


						dNoOFBytestoWrite = sizeof(A3_stt);
						

						bool Status = WriteFile(hComm,        
						                   &STT,     
						                   dNoOFBytestoWrite,  
						                   &dNoOfBytesWritten, 
						                   NULL);

						ReleaseMutex(hMutex);

			     		 if(!Status)
					  		printf("THREAD 3 STATUS Send ERROR!\n");

		 	
	      
	      cnt_change_status++;
		  if(cnt_change_status == 200)		// Change status ~= every 5 sec
		  {	
		  		STT.stt ^= (1<<0);
		  		cnt_change_status = 0;
		  }	

	      Sleep(25);				// 40 times per sec ~= 0,025 sec

	  }
	  ExitThread(0);
}



void main()
{
    setlocale(LC_ALL, "RUS");

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);



    printf("#####______      MK 1      ______#####\n");
    

    TEMP.hdr = TEMP_hdr;
    LOCATION.hdr = LOC_hdr;
    STT.hdr = STT_hdr;


    HANDLE hThreads[3];
    CONST HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
  	if(NULL == hMutex) {
    	 printf("Failed to create mutex.\r\n");
  	}

  	hComm = CreateFile("\\\\.\\COM22",                
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
		
		 timeouts.WriteTotalTimeoutConstant   = 50; 
		timeouts.WriteTotalTimeoutMultiplier = 10; 



  	

  	 Status = SetCommTimeouts(hComm, &timeouts);

  	  if(Status)
	  	printf("Set Timeouts NORM!\n");
	  else
	  	printf("Set Timeouts ERROR!\n");




	 
   		hThreads[0] = CreateThread(NULL, 0, &ThreadProc1, hMutex, 0, NULL);
    	if(NULL == hThreads[0]) {
      		printf("Failed to create thread 1.\r\n");
    	}
  		
  		hThreads[1] = CreateThread(NULL, 0, &ThreadProc2, hMutex, 0, NULL);
    	if(NULL == hThreads[1]) {
      		printf("Failed to create thread 2 .\r\n");
    	}

    	hThreads[2] = CreateThread(NULL, 0, &ThreadProc3, hMutex, 0, NULL);
    	if(NULL == hThreads[2]) {
      		printf("Failed to create thread 3.\r\n");
    	}




    getch(); // wait for any key to be pressed
}