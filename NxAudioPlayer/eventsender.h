#ifndef EVENTSENDER_H
#define EVENTSENDER_H
#include <QObject>

class EventSender: public QObject
{
	Q_OBJECT
public:
	explicit EventSender(QObject *parent = 0);
	virtual ~EventSender(){}
public:
	void UpdateWindow( int id );

signals:
	void UpdateWindowEvent( int hide );
};

#endif // EVENTSENDER_H
