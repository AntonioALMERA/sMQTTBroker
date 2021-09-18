#include"sMQTTBroker.h"

bool sMQTTBroker::init(unsigned short port)
{
	_server = new TCPServer(port);
	if (_server == 0)
		return false;
	_server->begin();
	return true;
};
void sMQTTBroker::update()
{
#if defined(ESP8266) || defined(ESP32)
	WiFiClient client = _server->available();
	if (client)
	{
		ESP_LOGD(SMQTTTAG, "New Client");
		clients.push_back(new sMQTTClient(this, &client));
	}
#endif
	sMQTTClientList::iterator clit;
	for (clit = clients.begin(); clit != clients.end(); clit++)
	{
		sMQTTClient *c = *clit;
		if (c->isConnected())
			c->update();
		else
		{
			delete c;
			clit=clients.erase(clit);
			MSQTT_LOGD("Clients %d", clients.size());
			if (clit == clients.end())
				break;
		}
	}
};
bool sMQTTBroker::subscribe_topic(sMQTTClient *client, const char *topic)
{
	sMQTTTopic *newTopic = new sMQTTTopic(topic);

	if (isTopicValidName(newTopic->Name()) == false)
	{
		delete newTopic;
		return false;
	}

	sMQTTTopicList::iterator sub;
	for (sub = subscribes.begin(); sub != subscribes.end(); sub++)
	{
		if (strcmp((*sub)->Name(), newTopic->Name()) == 0)
		{
			delete newTopic;
			(*sub)->subscribe(client);
			findRetainTopic(*sub, client);
			return true;
		}
	}
	newTopic->subscribe(client);
	subscribes.push_back(newTopic);

	findRetainTopic(newTopic, client);
	return true;
};
void sMQTTBroker::unsubscribe_topic(sMQTTClient *client, const char *topic)
{
	sMQTTTopicList::iterator it;
	for (it = subscribes.begin(); it != subscribes.end(); it++)
	{
		if (strcmp((*it)->Name(), topic) == 0)
		{
			if ((*it)->unsubscribe(client) == true)
			{
				delete *it;
				subscribes.erase(it);
			}
			break;
		}
	}
};
void sMQTTBroker::publish(sMQTTTopic *topic, sMQTTMessage *msg)
{
	sMQTTTopicList::iterator sub;
	for (sub = subscribes.begin(); sub != subscribes.end(); sub++)
	{
		if ((*sub)->match(topic))
		{
			sMQTTClientList subList = (*sub)->getSubscribeList();
			for (auto cl : subList)
			{
				msg->sendTo(cl);
			}
		}
	}
};
bool sMQTTBroker::isTopicValidName(const char *filter)
{
	int length = strlen(filter);
	char *hashpos = strchr(filter, '#');	/* '#' wildcard can be only at the beginning or the end of a topic */

	if (hashpos != NULL)
	{
		char *second = strchr(hashpos + 1, '#');
		if ((hashpos != filter && hashpos != filter + (length - 1)) || second != NULL)
			return false;
	}
	/* '#' or '+' only next to a slash separator or end of name */
	for (const char *c = "#+"; *c != '\0'; ++c)
	{
		char *pos = strchr(filter, *c);
		while (pos != NULL)
		{
			if (pos > filter)
			{	/* check previous char is '/' */
				if (*(pos - 1) != '/')
					return false;
			}
			if (*(pos + 1) != '\0')
			{	/* check that subsequent char is '/' */
				if (*(pos + 1) != '/')
					return false;
			}
			pos = strchr(pos + 1, *c);
		}
	}
	return true;
};
void sMQTTBroker::updateRetainedTopic(sMQTTTopic *topic)
{
	ESP_LOGD(SMQTTTAG, "updateRetainedTopic %s", topic->Name());
	sMQTTTopicList::iterator it;
	for (it = retains.begin(); it != retains.end(); it++)
	{
		if (strcmp((*it)->Name(), topic->Name()) == 0)
			break;
	}
	if (it != retains.end())
	{
		(*it)->update(topic);
	}
	else
	{
		sMQTTTopic *newTopic = new sMQTTTopic(topic);
		retains.push_back(newTopic);
	}
};
void sMQTTBroker::findRetainTopic(sMQTTTopic *topic, sMQTTClient *client)
{
	ESP_LOGD(SMQTTTAG, "findRetainTopic %s %d", topic->Name(), retains.size());
	sMQTTTopicList::iterator it;
	for (it = retains.begin(); it != retains.end(); it++)
	{
		if (topic->match(*it))
		{
			ESP_LOGD(SMQTTTAG, "findRetainTopic %s", topic->Name());
			sMQTTMessage msg(sMQTTMessage::Type::Publish);
			msg.add((*it)->Name(), strlen((*it)->Name()));
			// msg_id
			if ((*it)->QoS()) {
				msg.add(0);
				msg.add(0);
			}
			msg.add((*it)->Payload(), strlen((*it)->Payload()), false);
			msg.sendTo(client);
			break;
		}
	}
};