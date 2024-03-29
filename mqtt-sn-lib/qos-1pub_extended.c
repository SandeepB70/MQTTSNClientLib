/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - clarifications and/or documentation extension
 *
 * Description:
 * Extension to the specs in which a node can send a normal (long) topic name inside the
 * payload area to avoid the registration process and the usage of short/predefined types
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MQTTSNPacket.h"
#include "transport.h"


int main(int argc, char** argv)
{
	int rc = 0;
	int mysock;
	unsigned char buf[200];
	size_t buflen = sizeof(buf);
	MQTTSN_topicid topic;
	unsigned char* payload = (unsigned char*)"mypayload";
	int payloadlen = strlen((char*)payload);
	int len = 0;
	int dup = 0;
	int qos = 3;
	int retained = 0;
	short packetid = 0;
	char *topicname = "a long topic name";
	char *host = "127.0.0.1";
	int port = 1883;

	mysock = transport_open();
	if(mysock < 0)
		return mysock;

	if (argc > 1)
		host = argv[1];

	if (argc > 2)
		port = atoi(argv[2]);

	printf("Sending to hostname %s port %d\n", host, port);

	topic.type = MQTTSN_TOPIC_TYPE_NORMAL;
	topic.data.long_.name = topicname;
	topic.data.long_.len = strlen(topicname);

	len = MQTTSNSerialize_publish(buf, buflen, dup, qos, retained, packetid,
			topic, payload, payloadlen);

	rc = transport_sendPacketBuffer(host, port, buf, len);

	transport_close();

	return rc;
}
