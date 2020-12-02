All code was compiled using a Makefile and GCC version 7.5.0 and ran in Linux Ubuntu 18.04.5 LTS, 64 bit. The client files needed to be run are all in the "tests" folder. The MQTT-SN Gateway will need to be running before attempting to run the clients. There should already be an executable file called "MQTT-SNGateway" within the "paho.mqtt-sn.embedded-c" directory. Once the user is within the same directory as the "MQTT-SNGateway" executable, the following command should be used to run the gateway: 

./MQTT-SNGateway -f .../paho.mqtt-sn.embedded-c/MQTTSNGateway/gateway.conf

The "..." in the above command needs to be replace by the path to "paho.mqtt-sn.embedded-c" directory in the user's machine.

Once the gateway is running, the user will need to navigate to the "tests" directory where the MQTT-SN clients are located and run the Makefile, if the executables are not there. The user simply has to type "make" and the Makefile will handle the compiling. To get rid of the executables, the user can type "make clean".

The clients can be executed in any order by typing "./Name_Of_Executable_File" at the terminal. The order the clients were executed in for the final demo was: "MQTTSN_PublishV2", "MQTTSN_SubscribeV2", "MQTTSN_PubSubV2", and "MQTTSN_SleepV2". To stop each client and the gateway from running, simply use "control c".

Each client is hardcoded with correct IP address and port number the GW is using to communicate. There is a PAHO tester file within the "paho.mqtt-sn.embedded-c/MQTTSNGateway/GatewayTester/Build" directory called "MQTT-SNGatewayTester" that was used to obtain the configured port number and IP address the Gateway is using, but it should not need to be used since these values are hardcoded into the Gateway. If one desires to execute it, make sure the Gateway is running and then navigate to the directory mentioned above and run the "MQTT-SNGatewayTester" executable.

Below is the structure of the Makefile which should be copied exactly. Each of the areas where it says "..." should be replaced with the local path to that directory/file in the user's machine. All files should be kept within the same directories as they are in the github repository, otherwise it will lead to errors when attempting to compile with the Makefile.







TARGETS = MQTTSN_PublishV2 MQTTSN_SubscribeV2 MQTTSN_PubSubV2 MQTTSN_SleepV2


GCC_FLAGS = -Wextra -Wconversion -Werror -Wall
#GCC_FLAGS = -Werror -Wall  
all: $(TARGETS)

MQTTSN_PublishV2: *.c 
	$(CC) $(GCC_FLAGS) MQTTSN_PublishV2.c -I .../mqtt-sn-lib -I .../src .../src/Util.c .../src/Connect.c .../src/WillTopic.c .../src/WillMsg.c .../src/Register.c .../src/Disconnect.c .../src/RegAck.c .../src/PubRecRelComp.c .../src/Publish.c .../src/PubAck.c .../src/PingReq.c .../src/PingResp.c .../mqtt-sn-lib/transport.c .../mqtt-sn-lib/MQTTSNPacket.c .../mqtt-sn-lib/MQTTSNConnectClient.c .../mqtt-sn-lib/StackTrace.c  .../mqtt-sn-lib/MQTTSNDeserializePublish.c .../mqtt-sn-lib/MQTTSNSerializePublish.c .../mqtt-sn-lib/MQTTSNSubscribeClient.c .../mqtt-sn-lib/MQTTSNUnsubscribeClient.c -o MQTTSN_PublishV2 -Os -s

MQTTSN_SubscribeV2: *.c 
	$(CC) $(GCC_FLAGS) MQTTSN_SubscribeV2.c -I .../mqtt-sn-lib -I .../src .../src/Util.c .../src/Connect.c .../src/WillTopic.c .../src/WillMsg.c .../src/Register.c .../src/Disconnect.c .../src/RegAck.c .../src/PubRecRelComp.c .../src/Subscribe.c .../src/PubAck.c .../src/PingReq.c .../src/PingResp.c .../mqtt-sn-lib/transport.c .../mqtt-sn-lib/MQTTSNPacket.c .../mqtt-sn-lib/MQTTSNConnectClient.c .../mqtt-sn-lib/StackTrace.c  .../mqtt-sn-lib/MQTTSNDeserializePublish.c .../mqtt-sn-lib/MQTTSNSerializePublish.c .../mqtt-sn-lib/MQTTSNSubscribeClient.c .../mqtt-sn-lib/MQTTSNUnsubscribeClient.c -o MQTTSN_SubscribeV2 -Os -s

MQTTSN_PubSubV2: *.c 
	$(CC) $(GCC_FLAGS) MQTTSN_PubSubV2.c -I .../mqtt-sn-lib -I .../src .../src/Util.c .../src/Connect.c .../src/WillTopic.c .../src/WillMsg.c .../src/Register.c .../src/Disconnect.c .../src/RegAck.c .../src/PubRecRelComp.c .../src/Subscribe.c .../src/Publish.c .../src/PubAck.c .../src/PingReq.c .../src/PingResp.c .../mqtt-sn-lib/transport.c .../mqtt-sn-lib/MQTTSNPacket.c .../mqtt-sn-lib/MQTTSNConnectClient.c .../mqtt-sn-lib/StackTrace.c  .../mqtt-sn-lib/MQTTSNDeserializePublish.c .../mqtt-sn-lib/MQTTSNSerializePublish.c .../mqtt-sn-lib/MQTTSNSubscribeClient.c .../mqtt-sn-lib/MQTTSNUnsubscribeClient.c -o MQTTSN_PubSubV2 -Os -s

MQTTSN_SleepV2: *.c 
	$(CC) $(GCC_FLAGS) MQTTSN_SleepV2.c -I .../mqtt-sn-lib -I .../src .../src/Util.c .../src/Connect.c .../src/WillTopic.c .../src/WillMsg.c .../src/Register.c .../src/Disconnect.c .../src/RegAck.c .../src/PubRecRelComp.c .../src/Subscribe.c .../src/PubAck.c .../src/PingReq.c .../src/PingResp.c .../mqtt-sn-lib/transport.c .../mqtt-sn-lib/MQTTSNPacket.c .../mqtt-sn-lib/MQTTSNConnectClient.c .../mqtt-sn-lib/StackTrace.c  .../mqtt-sn-lib/MQTTSNDeserializePublish.c .../mqtt-sn-lib/MQTTSNSerializePublish.c .../mqtt-sn-lib/MQTTSNSubscribeClient.c .../mqtt-sn-lib/MQTTSNUnsubscribeClient.c -o MQTTSN_SleepV2 -Os -s

clean:
	rm -f $(TARGETS)

