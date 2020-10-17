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
 * Short topic name used to avoid registration process
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
	size_t payloadlen = strlen((char*)payload);
	size_t len = 0;
	uint8_t dup = 0;
	uint8_t qos = 1;
	uint8_t retained = 0;
	uint16_t packetid = 1;
	char *host = "50.255.7.18";
	int port = 60885;
	MQTTSNPacket_connectData options = MQTTSNPacket_connectData_initializer;

	mysock = transport_open();
	if(mysock < 0)
		return mysock;

	if (argc > 1)
		host = argv[1];

	if (argc > 2)
		port = atoi(argv[2]);

	printf("Sending to hostname %s port %d\n", host, port);

	options.clientID.cstring = "myclientid";
	rc = MQTTSNSerialize_connect(buf, buflen, &options);
	if (rc <= 0) {
		rc = -1;
		goto exit;
	}
	len = (size_t) rc;
	rc = (int) transport_sendPacketBuffer(host, port, buf, len);

	/* wait for connack */
	if (MQTTSNPacket_read(buf, buflen, transport_getdata) == MQTTSN_CONNACK)
	{
		int connack_rc = -1;

		if (MQTTSNDeserialize_connack(&connack_rc, buf, buflen) != 1 || connack_rc != 0)
		{
			printf("Unable to connect, return code %d\n", connack_rc);
			goto exit;
		}
		else 
			printf("connected rc %d\n", connack_rc);
	}
	else
		goto exit;

	/* publish with short name */
	topic.type = MQTTSN_TOPIC_TYPE_SHORT;
	memcpy(topic.data.short_name, "tt", 2);
	rc = MQTTSNSerialize_publish(buf, buflen - len, dup, qos, retained, packetid,
			topic, payload, payloadlen);
	if (rc <= 0) {
		rc = -1;
		goto exit;
	}
	len = (size_t) rc;
	rc = (int) transport_sendPacketBuffer(host, port, buf, len);

	/* wait for puback */
	if (MQTTSNPacket_read(buf, buflen, transport_getdata) == MQTTSN_PUBACK)
	{
		unsigned short packet_id, topic_id;
		unsigned char returncode;

		if (MQTTSNDeserialize_puback(&topic_id, &packet_id, &returncode, buf, buflen) != 1 || returncode != MQTTSN_RC_ACCEPTED)
			printf("Unable to publish, return code %d\n", returncode);
		else 
			printf("puback received, id %d\n", packet_id);
	}
	else
		goto exit;

	rc = MQTTSNSerialize_disconnect(buf, buflen, 0);
	if (rc <= 0) {
		rc = -1;
		goto exit;
	}
	len = (size_t) rc;
	rc = (int) transport_sendPacketBuffer(host, port, buf, len);

exit:
	transport_close();

	return rc;
}
