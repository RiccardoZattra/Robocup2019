import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)    
GPIO.setwarnings(False)
GPIO.setup(22,GPIO.OUT)
GPIO.setup(23,GPIO.OUT)
GPIO.setup(27,GPIO.OUT)
GPIO.output(22,GPIO.LOW)
GPIO.output(23,GPIO.LOW)
GPIO.output(27,GPIO.LOW)

def LED(color,level):
    if color == 'red' or color == 'rosso':
        GPIO.output(22,level)
        GPIO.output(23,0)
        GPIO.output(27,0)
    elif color == 'green' or color == 'verde':
        GPIO.output(22,0)
        GPIO.output(23,level)
        GPIO.output(27,0)
    elif color == 'blue' or color == 'blu':
        GPIO.output(22,0)
        GPIO.output(23,0)
        GPIO.output(27,level)
    elif color == 'off':
        GPIO.output(22,0)
        GPIO.output(23,0)
        GPIO.output(27,0)




'''
r=GPIO.PWM(22,100)
v=GPIO.PWM(23,100)
b=GPIO.PWM(27,100)
for k in range(0,100):
    #v.start(k)
    r.start(k)
    b.start(k)
    time.sleep(0.1)
'''
'''
while True:
    LED('red',True)
    time.sleep(0.5)
'''
'''
while True:
    print ('LED on')
    LED(0,1,0) 
    time.sleep(1)             
    print ('LED off')
    LED(0,0,0)
    time.sleep(1)
'''
'''
while True:
    print ('LED on')
    GPIO.output(23,1) 
    time.sleep(2)             
    print ('LED off')
    GPIO.output(23,0)
    time.sleep(2)
'''



