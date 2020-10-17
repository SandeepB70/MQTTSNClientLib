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
 *******************************************************************************/

#ifndef MQTTSNCONNECT_H_
#define MQTTSNCONNECT_H_

#include <stdio.h>
#include <stdint.h>

typedef struct
{
	/** The eyecatcher for this structure.  must be MQSC. */
	char struct_id[4];
	/** The version number of this structure.  Must be 0.
	  */
	int struct_version;
	MQTTSNString clientID;
	unsigned short duration;
	unsigned char cleansession;
	unsigned char willFlag;
} MQTTSNPacket_connectData;

#define MQTTSNPacket_connectData_initializer { {'M', 'Q', 'S', 'C'}, 0, {NULL, {0, NULL}}, 10, 1, 0 }

int MQTTSNSerialize_connect(unsigned char* buf, size_t buflen, MQTTSNPacket_connectData* options);

int MQTTSNDeserialize_connect(MQTTSNPacket_connectData* data, unsigned char* buf, size_t len);

int MQTTSNSerialize_connack(unsigned char* buf, size_t buflen, int connack_rc);
int MQTTSNDeserialize_connack(int* connack_rc, unsigned char* buf, size_t buflen);

int MQTTSNSerialize_disconnect(unsigned char* buf, size_t buflen, int duration);
int MQTTSNDeserialize_disconnect(int* duration, unsigned char* buf, size_t buflen);

int MQTTSNSerialize_pingreq(unsigned char* buf, size_t buflen, MQTTSNString clientid);
int MQTTSNDeserialize_pingreq(MQTTSNString* clientID, unsigned char* buf, size_t len);

int MQTTSNSerialize_pingresp(unsigned char* buf, size_t buflen);
int MQTTSNDeserialize_pingresp(unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willmsg(unsigned char* buf, size_t buflen, MQTTSNString willMsg);
int MQTTSNDeserialize_willmsg(MQTTSNString* willMsg, unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willmsgreq(unsigned char* buf, size_t buflen);
int MQTTSNDeserialize_willmsgreq(unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willmsgupd(unsigned char* buf, size_t buflen, MQTTSNString willMsg);
int MQTTSNDeserialize_willmsgupd(MQTTSNString* willMsg, unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willmsgresp(unsigned char* buf, size_t buflen, int resp_rc);
int MQTTSNDeserialize_willmsgresp(int* resp_rc, unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willtopic(unsigned char* buf, size_t buflen, int willQoS, unsigned char willRetain, MQTTSNString willTopic);
int MQTTSNDeserialize_willtopic(int* willQoS, unsigned char* willRetain, MQTTSNString* willTopic, unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willtopicreq(unsigned char* buf, size_t buflen);
int MQTTSNDeserialize_willtopicreq(unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willtopicupd(unsigned char* buf, size_t buflen, int willQoS, unsigned char willRetain, MQTTSNString willTopic);
int MQTTSNDeserialize_willtopicupd(int *willQoS, unsigned char *willRetain, MQTTSNString* willTopic, unsigned char* buf, size_t buflen);

int MQTTSNSerialize_willtopicresp(unsigned char* buf, size_t buflen, int resp_rc);
int MQTTSNDeserialize_willtopicresp(int* resp_rc, unsigned char* buf, size_t buflen);

#endif /* MQTTSNCONNECT_H_ */
