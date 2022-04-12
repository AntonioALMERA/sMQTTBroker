#include"sMQTTEvent.h"

sMQTTNewClientEvent::sMQTTNewClientEvent(sMQTTClient *client,
	std::string &_login, std::string &_password) :sMQTTEvent(NewClient_sMQTTEventType),_client(client),
	login(_login),password(_password)
{
};
sMQTTClient *sMQTTNewClientEvent::Client()
{
	return _client;
};
sMQTTRemoveClientEvent::sMQTTRemoveClientEvent(sMQTTClient *client):sMQTTEvent(RemoveClient_sMQTTEventType),_client(client)
{
};
sMQTTClient *sMQTTRemoveClientEvent::Client()
{
	return _client;
};
sMQTTPublicEvent::sMQTTPublicEvent(sMQTTClient *client,
	std::string &_topic, std::string &_payload) :sMQTTEvent(Public_sMQTTEventType),_client(client),
	topic(_topic),payload(_payload)
{
};
sMQTTClient *sMQTTPublicEvent::Client()
{
	return _client;
};
sMQTTLostConnectionEvent::sMQTTLostConnectionEvent() :sMQTTEvent(LostConnect_sMQTTEventType)
{
};