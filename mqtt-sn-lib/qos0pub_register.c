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
 * Normal topic name used to show registration process
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "MQTTSNPacket.h"
#include "transport.h"


int main(int argc, char** argv)
{
	int rc = 0;
	int mysock;
	unsigned char buf[500];
	size_t buflen = sizeof(buf);
	MQTTSN_topicid topic;
	MQTTSNString topicstr;
	size_t len = 0;
	uint8_t retained = 0;
	char *topicname = "status/dev";
	char *host = "50.255.7.18";
	int port = 60885;
	MQTTSNPacket_connectData options = MQTTSNPacket_connectData_initializer;
	unsigned short topicid;

	mysock = transport_open();
	if(mysock < 0)
		return mysock;

	if (argc > 1)
		host = argv[1];

	if (argc > 2)
		port = atoi(argv[2]);

	printf("Sending to hostname %s port %d\n", host, port);

	options.clientID.cstring = "d:quickstart:udptest:9002f7f1ad23";
	rc = MQTTSNSerialize_connect(buf, buflen, &options);
	if (rc > 0) {
		len = (size_t) rc;
	} else {
		goto exit;
	}
	transport_sendPacketBuffer(host, port, buf, len);

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
	{
		printf("could not connect to gateway\n");
		goto exit;
	}

	/* register topic name */
	printf("Registering\n");
	uint8_t packetid = 1;
	topicstr.cstring = topicname;
	topicstr.lenstring.len = strlen(topicname);
	rc = MQTTSNSerialize_register(buf, buflen, 0, packetid, &topicstr);
	if (rc > 0) {
		len = (size_t) rc;
	} else {
		goto exit;
	}
	// ! should not cast. need to fix it later
	rc = (int) transport_sendPacketBuffer(host, port, buf, len);

	if (MQTTSNPacket_read(buf, buflen, transport_getdata) == MQTTSN_REGACK) 	/* wait for regack */
	{
		unsigned short submsgid;
		unsigned char returncode;

		rc = MQTTSNDeserialize_regack(&topicid, &submsgid, &returncode, buf, buflen);
		if (returncode != 0)
		{
			printf("return code %d\n", returncode);
			goto exit;
		}
		else
			printf("regack topic id %d\n", topicid);
	}
	else
		goto exit;

    while (1)
    {
        if (1)
        {               
			/* publish with obtained id */
			printf("Publishing\n");
			topic.type = MQTTSN_TOPIC_TYPE_NORMAL;
			topic.data.id = topicid;
			static const char* joypos[] = {"SN-LEFT", "SN-RIGHT", "SN-CENTRE", "SN-UP", "SN-DOWN"};

		   	unsigned char payload[250];
			// ! fix this
    		size_t payloadlen = (size_t) sprintf((char*)payload,
     "{\"d\":{\"myName\":\"IoT mbed\",\"accelX\":%0.4f,\"accelY\":%0.4f,\"accelZ\":%0.4f,\"temp\":%0.4f,\"joystick\":\"%s\",\"potentiometer1\":%0.4f,\"potentiometer2\":%0.4f}}",
			(rand() % 10) * 2.0, (rand() % 10) * 2.0, (rand() % 10) * 2.0, (rand() % 10) + 18.0, joypos[rand() % 5], (rand() % 10) * 30.0, (rand() % 10) * 30.0); 
			// ! should check for error, and don't cast!
			len = (size_t) MQTTSNSerialize_publish(buf, buflen, 0, 0, retained, 0, topic, payload, payloadlen);
			// ! should check for error, and don't cast!
			rc = (int) transport_sendPacketBuffer(host, port, buf, len);

			printf("rc %d from send packet for publish length %lu\n", rc, len);
        }
        sleep(1);  // Publish a message every second
	}
exit:
	transport_close();

	return 0;
}
