import paho.mqtt.client as mqtt
import time
import zaboo_beacon_config as conf
import serial
import json
from time import sleep
import board
import digitalio
from PIL import Image, ImageDraw, ImageFont
import adafruit_ssd1306
import collections
import firebase_admin   
from firebase_admin import db
from firebase_admin import credentials,firestore



def Save_bb_change(device_id,devices,rssi,hora):
    mensaje = db.reference('zaboo_bb').push()
    mensaje.set({
             'ID':device_id,
             'devices':devices,
             'rssi':rssi,
             'time':hora
         })

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
    payload=message.payload.decode("utf8")
    print(payload)
    try:
        ser.write(payload.encode())
    except Exception as e:
        raise


def on_publish(client, obj, mid):
    print("mid: " + str(mid))

def on_subscribe(client, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

#En este archivo se encuentran las credenciales correspondientes
cred= credentials.Certificate('./ServiceAccountKey.json')
default_app=firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://bloggeekplatzi1.firebaseio.com'
})

#La base de datos para la cual se coloca como un cliente
cloud_db= firestore.client()

broker_address= "m16.cloudmqtt.com"  #Broker address
port = 11782                         #Broker port
user = "dover"                    #Connection username
password = "1020785214"            #Connection password
client_device = conf.zaboo_config['id']

mqttc = mqtt.Client(client_device)
mqttc.on_connect = on_connect
mqttc.on_message=on_message
mqttc.on_log=on_log 
mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe

# Define the Reset Pin
#oled_reset = digitalio.DigitalInOut(board.D4)
 
# Change these
# to the right size for your display!
WIDTH = 128
HEIGHT = 64     # Change to 64 if needed
BORDER = 5

# Use for I2C.
i2c = board.I2C()
oled = adafruit_ssd1306.SSD1306_I2C(WIDTH, HEIGHT, i2c, addr=0x3c)

# Clear display.
oled.fill(0)
oled.show()

# Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
image = Image.new('1', (oled.width, oled.height))
 
# Get drawing object to draw on image.
draw = ImageDraw.Draw(image)

#Inicializar los arreglos correspondientes a la trama previa y la trama nueva. 
current_devices_names_list= []
previous_devices_names_list= []
current_rssi_list=[]


#Conectar Cliente con el id que pertenece a Zaboo
# Este es el cliente MQTT

mqttc.username_pw_set(user, password)
mqttc.connect(broker_address, port)
mqttc.loop_start()
#mqttc.publish("house/bulbs/bulb1","ON")

ser = serial.Serial ("/dev/ttyS0", 115200)    #Open port with baud rate
font_aux=ImageFont.truetype('/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Light.ttf',10)
while True:

    received_data = ser.read()              #read serial port
    sleep(0.03)
    data_left = ser.inWaiting()             #check for remaining byte
    received_data += ser.read(data_left)
    #Datos recividos
    try:
        received_data=received_data.decode('utf8')
        if received_data[:2]=='S0':
            print('Initial STATE')
            ser.write(b'START')
    except:
        print('No es posible decodificar esta trama')


    try:
        #Limpiar el display
        oled.fill(0)
        oled.show()
        image = Image.new('1', (oled.width, oled.height))
        draw = ImageDraw.Draw(image)
        #Convertir datos a JSON
        json_data_incoming=json.loads(received_data)
        # Imprimir datos 
        print(json_data_incoming)
        nombre=json_data_incoming['devices'][0]['name']
        json_size=len(json_data_incoming['devices'])
        draw.text((0,0),nombre,font=font_aux, fill=255)
        draw.text((0,20),str(json_size),font=font_aux, fill=255)
        oled.image(image)
        oled.show()
        #sacar la lista de los devices actuales
        devices_names= json_data_incoming['devices']
        current_devices_names_list=[]
        current_rssi_list=[]

        # guardar en arreglo la nueva trama
        for i in range(len(devices_names)):
            current_devices_names_list.append(devices_names[i]['name'])
            current_rssi_list.append(devices_names[i]['rssi'])


        if collections.Counter(current_devices_names_list)==collections.Counter(previous_devices_names_list):
            print('Not changes')
        else:
            print('Devices_changes')
            current_time=round(time.time())
            Save_bb_change(client_device,current_devices_names_list,current_rssi_list,current_time)

        previous_devices_names_list=current_devices_names_list


    except ValueError as valerr:
        print(valerr)
#mqttc.loop_stop() #stop the loop