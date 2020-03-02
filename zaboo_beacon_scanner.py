import paho.mqtt.client as mqtt
import time
import zaboo_beacon_config as conf 


def on_connect(client, userdata, flags, rc):
    print("Connected with result code" + str(rc))
    suscription=str(conf.zaboo_config['id'])+"/beacon"
    print(suscription)
    client.subscribe(suscription)

def on_log(client, userdata, level, buf):
    print("log: ",buf)

def on_message(client, userdata, message):
    print("message received " ,str(message.payload))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)

def on_publish(client, obj, mid):
    print("mid: " + str(mid))

def on_subscribe(client, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

broker_address= "m16.cloudmqtt.com"  #Broker address
port = 11782                         #Broker port
user = "dover"                    #Connection username
password = "1020785214"            #Connection password

client_device = conf.zaboo_config['id']
#Conectar Cliente con el id que pertenece a Zaboo
# Este es el cliente MQTT
mqttc = mqtt.Client(client_device)

mqttc.on_connect = on_connect
mqttc.on_message=on_message
mqttc.on_log=on_log 
mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe


mqttc.username_pw_set(user, password)
mqttc.connect(broker_address, port)
mqttc.loop_start()
#mqttc.publish("house/bulbs/bulb1","ON")
while True:
	time.sleep(0.1)

#mqttc.loop_stop() #stop the loop