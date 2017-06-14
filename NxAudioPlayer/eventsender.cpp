#include "eventsender.h"

EventSender::EventSender(QObject *parent) :
	QObject(parent)
{
}


void EventSender::UpdateWindow( int32_t id )
{
	emit UpdateWindowEvent( id );
}

void EventSender::RunExtCmd( char *cmd )
{
	emit ExtCmdProcedure( cmd );
}
