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
import pyrebase
from picamera import PiCamera
from time import sleep

#Import Firebase config for photos
config={

    "apiKey": "AIzaSyDNUbWOOFxuR-1fNYJujWir2xRQU5QF8gg",
    "authDomain": "bloggeekplatzi1.firebaseapp.com",
    "databaseURL": "https://bloggeekplatzi1.firebaseio.com",
    "projectId": "bloggeekplatzi1",
    "storageBucket": "bloggeekplatzi1.appspot.com",
    "messagingSenderId": "763115962574",
    "appId": "1:763115962574:web:287110d9b1615d1c9a509f",
    "measurementId": "G-ZMPBZ14S63"    
}

def captureAndSend(nameImage,device_id):
    camera=PiCamera()
    path_on_cloud=device_id+'/'+nameImage
    path_local='/home/pi/Desktop/heyZaboo/'+nameImage
    camera.start_preview()
    sleep(2)
    camera.capture(path_local)
    camera.stop_preview()
    camera.close()
    storage.child(path_on_cloud).put(path_local)



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
    suscription1=str(conf.zaboo_config['id'])+"/beacon"
    suscription2=str(conf.zaboo_config['id'])+"/photo"
    print(suscription1)
    client.subscribe(suscription1)
    client.subscribe(suscription2)

def on_log(client, userdata, level, buf):
    print("log: ",buf)

def on_message(client, userdata, message):
    print("message received " ,str(message.payload))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
    payload=message.payload.decode("utf8")
    topic=message.topic
    #print(payload)

    if topic==str(conf.zaboo_config['id'])+"/photo":
        #en el payload viene el nombre que se debe poner al archivo
        photo_filename = payload 
        # responder que el mensaje fue recibido
        mqttc.publish("ZABOO_CLOUD","{\"device\":\""+str(conf.zaboo_config['id'])+"\",\"type\":\"conf\",\"filename\":\""+photo_filename+"\"}")
        print('take Picture')
        captureAndSend(photo_filename,str(conf.zaboo_config['id']))
        mqttc.publish("ZABOO_CLOUD","{\"device\":\""+str(conf.zaboo_config['id'])+"\",\"type\":\"upload\",\"filename\":\""+photo_filename+"\"}")
    elif topic==str(conf.zaboo_config['id'])+"/beacon":   
        try:
            ser.write(message.payload)
        except :
            print('not posible to write in serial port')


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

#pyrebase for photos
firebase = pyrebase.initialize_app(config)
storage=firebase.storage()

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



ser = serial.Serial ("/dev/ttyS0", 115200)    #Open port with baud rate
font= ImageFont.truetype('/usr/share/fonts/truetype/piboto/Piboto-Bold.ttf',20)
font_small= ImageFont.truetype('/usr/share/fonts/truetype/piboto/Piboto-Bold.ttf',14)
sub_title='ZABOO'
title='POLLY'
title_font= ImageFont.truetype('/usr/share/fonts/truetype/piboto/Piboto-Bold.ttf',26)

(font_width, font_height) = title_font.getsize(text)
draw.text((0,0),title,font=sub_title, fill=255) 
draw.text((oled.width//2 - font_width//2, oled.height//2 - font_height//2),title,font=title_font, fill=255) 
oled.image(image)
oled.show()
#Conectar Cliente con el id que pertenece a Zaboo
# Este es el cliente MQTT


mqttc.username_pw_set(user, password)
mqttc.connect(broker_address, port)
mqttc.loop_start()
#mqttc.publish("house/bulbs/bulb1","ON")


while True:
    nombres=[]
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
        json_size=len(json_data_incoming['devices'])

        print(json_data_incoming)

        try:
            for i in range(json_size):
                name_to_save=json_data_incoming['devices'][i]['name']
                name_to_save=name_to_save.split()
                name_to_save=name_to_save[0][0:4]
                nombres.append(name_to_save)
        except:
            nombre= 'No devices'
            draw.text((0,0),nombre,font=font, fill=255)

        for i in range(len(nombres)):
            if i== 0 or i== 2 or i== 4: 
                draw.text((0,i*10),nombres[i],font=font, fill=255)
            elif i== 1 or i== 3 or i== 5: 
                draw.text((60,(i-1)*10),nombres[i],font=font, fill=255)
            else: 
                pass

        draw.text((120,47),str(json_size),font=font_small, fill=255)
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