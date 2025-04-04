import os
import cv2
import numpy as np
from picamera.array import PiRGBArray
from picamera import PiCamera
#import tensorflow as tf
import argparse
import sys
import serial
import struct

import time

#import RPi.GPIO as GPIO
from Libraries import log
from Libraries import LED
from Libraries import lib

# Set up camera constants
IM_WIDTH = 640
IM_HEIGHT = 480

camera_type = 'picamera'

sys.path.append('..')

# Import utilites
from utils import label_map_util
from utils import visualization_utils as vis_util

def send_to_arduino(data):
    #while answer_ardu != 'A' :
        #answer_ardu=''
    #print('data=',data)
    #print(data)
    low=data & 0x00ff
    high=(data & 0xff00)>>8
    #print('high=',high<<8,'low=',low,'d=',data)
    ser.write(struct.pack('>B',low))
    ser.write(struct.pack('>B',high))
    #ser.flushInput()
    '''
        while answer_ardu=='':
            #print('W=',ser.inWaiting())
            #print('2')
            if(ser.inWaiting()>=1):
                answer_ardu=(ser.read(1)).decode('utf-8')
                #print(answer_ardu)
                #print('3')
                ser.flushInput()
    '''
ser=serial.Serial(
        port='/dev/ttyAMA0',#ttyS0
        baudrate=500000,#il tempo impiegato per trasmettere un singolo bit è di 2 us
        parity=serial.PARITY_EVEN,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,

        timeout=0
        )

def find_ball(silver,black,frame_expanded,frame,long):
    #silver=True cerca argento
    #black=True cerca nere
    if silver:
        frameA,an,di = lib.find_silver(frame_expanded,frame,long)
        cv2.imshow('Object silver',frameA)
    if black:
        frameN,ang,dis = lib.find_black(frame_expanded,frame,long)
        cv2.imshow('Object black', frameN)

def black_silver(cosa_cercare,frame_expanded1,frame1):
    dati = False
    if cosa_cercare=='a':
        cosa_cercare='argento'
    elif cosa_cercare=='n':
        cosa_cercare='nera'

    elif cosa_cercare=='b':
        cosa_cercare='argentoL'
    elif cosa_cercare=='o':
        cosa_cercare='neraL'
        
    if cosa_cercare=='argento':
        print('cerco argento')
        frame2,angolo,distanza = lib.find_silver(frame_expanded1,frame1,dimin)#0.5#0.4
        cv2.putText(frame1, 'A', (600, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 165, 255),5, lineType=cv2.LINE_AA)
        if distanza!=255 and angolo!=255:
            log.stanzaLOG('i','argento:  dist= '+str(round(distanza,2))+' ang= '+str(round(angolo,2))+'  k= '+str(k))
        dati = True
    elif cosa_cercare=='nera':
        print('cerco nera')
        frame2,angolo,distanza = lib.find_black(frame_expanded1,frame1,dimin)
        cv2.putText(frame1, 'N', (600, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 165, 255),5, lineType=cv2.LINE_AA)
        if distanza!=255 and angolo!=255:
            log.stanzaLOG('i','nera:  dist= '+str(round(distanza,2))+' ang= '+str(round(angolo,2))+'  k= '+str(k))
        dati = True
    elif cosa_cercare=='argentoL':
        print('cerco argento a 40cm')
        frame2,angolo,distanza = lib.find_silver(frame_expanded1,frame1,dimax)
        cv2.putText(frame1, 'A40', (550, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 165, 255),5, lineType=cv2.LINE_AA)
        if distanza!=255 and angolo!=255:
            log.stanzaLOG('i','argento a 40:  dist= '+str(round(distanza,2))+' ang= '+str(round(angolo,2))+'  k= '+str(k))
        dati = True
    elif cosa_cercare=='neraL':
        print('cerco nera a 40cm')
        frame2,angolo,distanza = lib.find_black(frame_expanded1,frame1,dimax)
        cv2.putText(frame1, 'N40', (550, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 165, 255),5, lineType=cv2.LINE_AA)
        if distanza!=255 and angolo!=255:
            log.stanzaLOG('i','nera a 40:  dist= '+str(round(distanza,2))+' ang= '+str(round(angolo,2))+'  k= '+str(k))
        dati = True
    else:
        print('dato_sbagliato')
        dati = False
    frame2=frame1
    if dati:
        ser.flushInput()
        stop = False
        print('invio angolo')
        send_to_arduino(int(round(angolo,0)))
        msg = (ser.read(1)).decode('utf-8','ignore')
        while msg !='d':
            print('aspetto d angolo      ',msg)
            if msg =='e':
                send_to_arduino(int(round(angolo,0)))
                print('reinvio angolo')
                ser.flushInput()
            elif msg=='m' or msg=='j':
                stop = True
                break
            msg = (ser.read(1)).decode('utf-8','ignore')
        ser.flushInput()
        print('invio dist')
        send_to_arduino(int(round(distanza,0)))
        msg = (ser.read(1)).decode('utf-8','ignore')
        while msg !='m' and not stop:
            print('aspetto d dist      ',msg)
            if msg =='e':
                send_to_arduino(int(round(distanza,0)))
                print('reinvio dist')
                ser.flushInput()
            elif msg=='j':
                stop = True
                break
            msg = (ser.read(1)).decode('utf-8','ignore')
        if stop:
            log.stanzaLOG('e','invio non riuscito')
            print('invio non riuscito')
        else:
            #log.stanzaLOG('i','invio dati riuscito')
            print('inviato')
        cv2.putText(frame2, str(round(distanza,0)), (20, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (255, 0, 0),5, lineType=cv2.LINE_AA)
        cv2.putText(frame2, str(round(angolo,0)), (150, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0),5, lineType=cv2.LINE_AA)
        
        #cv2.imshow('Object',frame2)
    
    ser.flushInput()
    return frame2

file1 = open("/home/pi/Desktop/LOGfile/file-log-stanza.log","w")
file1.write("")
file1.close()
#print('led off')

ser.flushInput()
k = 0
dimax = 40
dimin = 35
print('ok Object Detection')
log.stanzaLOG('w','ok Object Detection')
if camera_type == 'picamera':
    camera = PiCamera()
    camera.resolution = (IM_WIDTH,IM_HEIGHT)
    camera.rotation = 180
    camera.framerate = 10
    rawCapture = PiRGBArray(camera, size=(IM_WIDTH,IM_HEIGHT))
    #rawCapture.truncate(0)#prima presente
    time.sleep(1)#0.5
    first = 1
    IM_HEIGHT = 320
    IM_WIDTH = 640
    set_camera = 1
    print('tensorflow avviato')
    log.stanzaLOG('w','tensorflow avviato')
    for frame1 in camera.capture_continuous(rawCapture, format="bgr",use_video_port=True):
        k += 1
        frame = np.copy(frame1.array)
        frame = frame[160:480,0:640]
        frame.setflags(write=1)
        frame_expanded = np.expand_dims(frame, axis=0)
        #set_camera=0
        if set_camera:
            frameA,angle,dist = lib.find_silver(frame_expanded,frame,dimin)
            frameN,ang,di = lib.find_black(frame_expanded,frame,dimin)
            #cv2.imshow('Object detector', frame)
            set_camera = 0
            print('camera avviata')
            log.stanzaLOG('w','camera avviata')
            
        image = frame
        if(ser.inWaiting()>=1):
            cosa_cercare=(ser.read(1)).decode('utf-8','ignore')
            if cosa_cercare == 'l':
                cv2.destroyAllWindows()
                send_to_arduino(0)
                second = 1
                k = 0
                LED.LED('off',False)
                file = open("/home/pi/Desktop/GlobalVariables.txt","w")
                file.write("0")
                file.close()
                print('scritto x linea')
                log.stanzaLOG('w','scritto per linea')
                ser.flushInput()
                while second:
                    #print('linea')
                    key=cv2.waitKey(10)& 0xff
                    file = open("/home/pi/Desktop/GlobalVariables.txt","r").read()
                    if file == '1':
                        second = 0
                        print('si reinizio stanza')
                        log.stanzaLOG('w','reinizio stanza')
                        LED.LED('blue',True)
                        ser.flushInput()
                        send_to_arduino(20000)
                        t=0
                        while t<3:
                            LED.LED('green',True)
                            time.sleep(0.1)
                            LED.LED('green',False)
                            time.sleep(0.1)
                            t+=1
                        break
            angolo = distanza = 255
            ser.flushInput()
            print(cosa_cercare)
            image = black_silver(cosa_cercare,frame_expanded,frame)

        #black_silver('a')
        #find_ball(True,True,frame_expanded,frame,dimin)
        #find_ball(argento?(True/False),nera?(True/False),frame_expanded,frame)

        cv2.imshow('Object Detection',image)
        cv2.imwrite('/home/pi/Desktop/FILE/SCANDAGLIA/image%s.jpg' % k,image)
        
        while first:
            #print('linea')
            #if cv2.waitKey(1) == 13:
                #break
            key=cv2.waitKey(10)& 0xff
            file = open("/home/pi/Desktop/GlobalVariables.txt","r").read()
            if file == '1':
                LED.LED('off',True)
                first = 0
                print('si inizio stanza')
                log.stanzaLOG('w','inizio stanza')
                ser.flushInput()
                send_to_arduino(20000)
                t=0
                while t<3:
                    LED.LED('green',True)
                    time.sleep(0.1)
                    LED.LED('green',False)
                    time.sleep(0.1)
                    t+=1
                break
            
        key = cv2.waitKey(1)
        if key == ord('a'):#a=aspetto blocca il sw in un while
            while cv2.waitKey(1) != ord('q'):
                a=1
        elif key == 27:
            print(k)
            break
        elif key == ord('k'):
            print(k)
            k = 0
        LED.LED('off',True)
        ser.flushInput()
        while ser.inWaiting()==0 and first == 0:
            if cv2.waitKey(1) == 13:
                break
        
        rawCapture.truncate(0)
        #LED.LED('off',False)
    LED.LED('off',False)
    camera.close()
    exit(0)
