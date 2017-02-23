//------------------------------------------------------------------------------
//
//  Copyright (C) 2016 Nexell Co. All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      : Command send/receive module via IPC(pipe)
//  File        : nx_ipc_command.h
//  Description :
//				  
//  Author      : SeongO Park (ray@nexell.co.kr)
//  History     : 2017-01-20 : first implementatioon (by SeongO Park)
//
//------------------------------------------------------------------------------

#ifndef __NX_IPC_COMMAND_H__
#define __NX_IPC_COMMAND_H__

#include <stdint.h>	//	*int**_t

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_CMD_LENGTH		(2048)

//	Client Side
void SendCommand( void *cmd, int32_t size );

//	Server Side
void StartCommandProc();
void StopCommandProc();
void RegCommandCallback( void *, void (*callback)(void *, char *) );


#ifdef __cplusplus
}
#endif

#endif // __NX_IPC_COMMAND_H__
