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
//  File        : nx_ipc_command.c
//  Description :
//
//  Author      : SeongO Park (ray@nexell.co.kr)
//  History     : 2017-01-20 : first implementatioon (by SeongO Park)
//
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "nx_ipc_cmd_receiver.h"
#include "SockUtils.h"

#define IPC_SERVER_FILE "/tmp/user/svoice_server"

static pthread_t gstThreadHnd;
static void *gstPrivateData = NULL;
static void (*gstIPCCallback)(void*, char*) = NULL;
static int32_t gstExitLoop = 0;
static int32_t gstIsCreatedThread = 0;
static pthread_mutex_t gstCtrlMutex = PTHREAD_MUTEX_INITIALIZER;


static int32_t WaitClient( int32_t hSocket )
{
	//	client socket
	int32_t clientSocket;
	struct sockaddr_un clntAddr;
	int32_t clntAddrSize;

	if( -1 == listen(hSocket, 5) )
	{
		printf( "Error : listen (err = %d)\n", errno );
		return -1;
	}

	// int32_t hPoll;
	// struct pollfd	pollEvent;

	// do{
	// 	//	Wait Event form UART
	// 	pollEvent.fd		= m_hSocket;
	// 	pollEvent.events	= POLLIN | POLLERR;
	// 	pollEvent.revents	= 0;
	// 	hPoll = poll( (struct pollfd*)&pollEvent, 1, 3000 );

	// 	if( hPoll < 0 ) {
	// 		return -1;
	// 	}
	// 	else if( hPoll > 0 ) {
	// 		break;
	// 	}
	// }while( m_ExitLoop );

	clntAddrSize = sizeof( clntAddr );
	clientSocket = accept( hSocket, (struct sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize );

	if ( -1 == clientSocket )
	{
		printf( "Error : accept (err = %d)\n", errno );
		return -1;
	}
	return clientSocket;
}

static void *_IPCReceiverThread( void *obj )
{
	int32_t svrSocket = LS_Open( IPC_SERVER_FILE );
	int32_t clientSock;
	int32_t readSize;
	char pBuf[MAX_CMD_LENGTH+1];
	(void)obj;

	while( !gstExitLoop )
	{
		clientSock = WaitClient( svrSocket );
		if( clientSock < 0 ){
			break;
		}
		readSize = read(clientSock, pBuf, MAX_CMD_LENGTH);

		if( readSize < 0 )
			break;

		if( gstIPCCallback )
		{
			gstIPCCallback(gstPrivateData, pBuf);
		}
		pthread_mutex_lock( &gstCtrlMutex );
		if( gstExitLoop )
		{
			pthread_mutex_unlock( &gstCtrlMutex );
			break;
		}
		pthread_mutex_unlock( &gstCtrlMutex );

		close( clientSock );
	}

	close( svrSocket );

	return (void*)0xdeadface;
}


//
//	Client Side
//

void SendCommand( void *cmd, int32_t size )
{
	int32_t fd = LS_Connect( IPC_SERVER_FILE );
	if( fd > 0 )
	{
		write( fd, cmd, size );
		close( fd );
	}
}


//
//	Server Side
//
void StartCommandProc()
{
	if( gstIsCreatedThread )
		return;

	gstExitLoop = 0;
	if( 0 == pthread_create( &gstThreadHnd, NULL, _IPCReceiverThread, NULL ) )
	{
		gstIsCreatedThread = 1;
	}
}

void StopCommandProc()
{
	if( gstIsCreatedThread )
	{
		gstExitLoop = 1;
		SendCommand((void*)"Exit", 4);	//	Write Dummy Data for Read HangUp.
		pthread_join( gstThreadHnd, NULL );
		gstIsCreatedThread = 0;
	}
}

void RegCommandCallback( void *pPrivate, void (*callback)(void *, char *) )
{
	gstPrivateData = pPrivate;
	gstIPCCallback = callback;
}
