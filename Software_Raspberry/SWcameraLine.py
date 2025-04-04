import cv2
import time
import numpy as np
import serial
import struct
import os
import sys

import RPi.GPIO as GPIO

from Libraries import LED
from Libraries import log

LED.LED('green',True)

def send_to_arduino(data):
    #while answer_ardu != 'A' :
        #answer_ardu=''
    #print('data=',data)
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
                answer_ardu=(ser.read(1)).decode('utf-8','ignore')
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
#temp=time.time()
#while True:
    #print(time.time(),'   ',temp)
#os.chdir('/home/pi/Desktop/LOGfile/')
file1 = open("/home/pi/Desktop/LOGfile/file-log-linea.log","w")
file1.write("")
file1.close()

ind=0
'''
while True:
    if ind == 0:
        print('ind = 0')
    if ind >=100:
        print('continue')
        ind=0
        continue
    if ind <1000:
        print('on',ind)
    ind=ind+1
'''
#log.lineLOG('w','############NUOVO AVVIO############')
while True:
    try:
        const_argento= 15000
        const_acutadx= 1000
        const_acutasx= 1100
        const_gira_verde_dx=3000
        const_gira_verde_sx=3100
        const_stop=2000
        const_gyro_grades=4500
        const_doppio_verde=6500
        angolo=0
        f_curvadx=0
        f_curvasx=0
        resolution = [320,240]
        cap = cv2.VideoCapture(0)
        cap.set(3,resolution[0])
        cap.set(4,resolution[1])
        
        #cap.set(21,1)
        f_controllo_verde=1
        soglia = 100
        acuta = 0
        pixel_verdi_dx=0
        pixel_verdi_sx=0
        ultima_distanza_dx=0
        ultima_distanza_sx=0
        pxc = 80#cambiato da 80
        f_acuta=0
        f_verde=0
        f_start_gira_verde=0
        f_end_gira_verde=0
        f_no_verde=0
        f_controllo_buffer=0
        f_controllo_doppio_verde=1
        f_inizio_correzione_gap=0
        f_fine_correzione_gap=0

        f_argento=0
        f_argento_sicuro=False
        f_cont_argento=0

        primissimo=0
        f_correzione_verde=0
        primo_verde=0
        f_entrato_verde=0

        sent_angolo = 0 #variabile da salvare sul file log
        
        meta=0
        piccolo_dx=0
        piccolo_sx=0
        cont_dopo_linea=0
        lower=np.array([35,80,80],np.uint8)#55,110,100
        upper=np.array([80, 255, 255],np.uint8)#75,180,160
        ser.flushOutput()
        file = open("/home/pi/Desktop/GlobalVariables.txt","w")
        file.write("0")
        file.close()
        #LED.LED('blue',False)
        while True:
            msg = (ser.read(1)).decode('utf-8','ignore')
            
            #if msg != '':
            #print(msg)
                
            if msg == 's':
                file = open("/home/pi/Desktop/GlobalVariables.txt","w")
                file.write("1")
                file.close()
                first = 1
                print('scritto')
                log.lineLOG('w','scritto x stanza')
                ser.flushInput()
                LED.LED('off',False)
                #send_to_arduino(20000)
                cv2.destroyAllWindows()
                print('stanza')
                #######################
                while first:
                    #ret,image=cap.read()
                    #cv2.waitKey(0)
                    key=cv2.waitKey(10)& 0xff
                    file = open("/home/pi/Desktop/GlobalVariables.txt","r").read()
                    if file == '0':
                        first = 0
                        print('si')
                        log.lineLOG('w','reinizio linea')
                        ser.flushInput()
                        #send_to_arduino(20000)
                        break
                    
                print('ok linea aspetto')
                #time.sleep(1)
                send_to_arduino(2200)
                #time.sleep(1)
                temp = int(time.time()*10)
                #print(temp)
                LED.LED('red',True)
                while True:#(int(time.time())-temp)<2:
                    timeVAR = int(time.time()*10)-temp
                    if (timeVAR%2)!=0:
                        LED.LED('red',True)
                    else:
                        LED.LED('red',False)
                    if timeVAR>7:
                        break
                    ret,image = cap.read()
                    print('time delay  ',timeVAR)
                    #cv2.imshow('prima',image)
                    #send_to_arduino(2200)
                LED.LED('green',True)
                
                f_argento=0
                f_argento_sicuro=False
                f_cont_argento=0
                
                print('ok camera')
                log.lineLOG('w','ok camera linea')
                
                #cv2.imwrite('/home/pi/Desktop/image.jpg',image)
                #print(im.shape)
                #######################
            
            elif msg == 'i':#(ser.read(1))=='i':
                send_to_arduino(99)
                ser.flushInput()
                msg = (ser.read(1)).decode('utf-8','ignore')
                log.lineLOG('i','ostacolo')
                LED.LED('blue',True)
                while msg != 'f':
                    #ser.flushInput()
                    ret,image=cap.read()#aggiunto dopo
                    msg = (ser.read(1)).decode('utf-8','ignore')
                    if msg == 'j':
                        break
                    if cv2.waitKey(1) == 13:
                        break
                log.lineLOG('i','end ostacolo')
                LED.LED('blue',False)
                #print('end ostacolo')
                ser.flushInput()

            elif msg == 'l' or msg == 'j':
                f_argento=0
                f_argento_sicuro=False
            ###################INIZIO PROGRAMMA########################

            
            ret,image=cap.read()
            
            img=cv2.cvtColor(image,cv2.COLOR_BGR2GRAY)

            imghsv=cv2.cvtColor(image[30:35,0:320],cv2.COLOR_BGR2HSV)
            #imghsv=cv2.cvtColor(image,cv2.COLOR_BGR2HSV)
            #cv2.circle(image,(280,32),4,[0,0,0],-1)
            #print(imghsv[32,280])

            
            imggreen=cv2.inRange(imghsv,lower,upper)
            cv2.imshow('imggreen',imggreen)

            LED.LED('off',False)
            f_exception = 1
            
            if (f_verde=='v' or (cv2.countNonZero(imggreen[0:5,0:320])>70)) and f_no_verde==0 and f_controllo_buffer==0:
                cv2.line(image,(0,30),(320,30),[255,0,255], 2)
                if f_verde==0:
                    send_to_arduino(const_stop)
                if f_controllo_verde:
                    imggray=img[30:31,0:320]
                    ret,imgotsu=cv2.threshold(imggray,0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                    only_linea=cv2.bitwise_or(imgotsu,imggreen[0:1,0:320])
                    cv2.imshow('ol',only_linea)
                    pixel_verdi_dx=0
                    pixel_verdi_sx=0
                    cont_neri_max=0
                    if primo_verde=='dx':#caso solo a destra
                        i=319
                        while i>=0:#con il for controllo dov'è il punto iniziale e finale della linea e quindi anche il centrale da dx a sx
                            if only_linea[0:1,i]==0:
                                f_enough=1
                                cont_neri=1
                                while f_enough==1 and i-cont_neri>0:
                                    if only_linea[0:1,i-cont_neri]==0:
                                        cont_neri=cont_neri+1
                                    else:
                                        f_enough=0
                                if cont_neri>30:
                                    meta=(i+i-cont_neri)/2
                                    break
                                else:
                                    i=i-cont_neri
                            else:
                                i=i-1
                    elif primo_verde!=0:#caso doppio verde o sinistra
                        i=0
                        while i<320:#con il for controllo dov'è il punto iniziale e finale della linea e quindi anche il centrale da sx a dx
                            if only_linea[0:1,i]==0:
                                f_enough=1
                                cont_neri=1
                                while f_enough==1 and i+cont_neri<320:
                                    if only_linea[0:1,cont_neri+i]==0:
                                        cont_neri=cont_neri+1
                                    else:
                                        f_enough=0
                                if cont_neri>30:
                                    meta=(i+i+cont_neri)/2
                                    break
                                else:
                                    i=i+cont_neri
                            else:
                                i=i+1
                    else:#primo caso
                        i=0
                        while i<320:#con il for controllo dov'è il punto iniziale e finale della linea e quindi anche il centrale da sx a dx
                            if only_linea[0:1,i]==0:
                                f_enough=1
                                cont_neri=1
                                while f_enough==1 and i+cont_neri<320:
                                    if only_linea[0:1,cont_neri+i]==0:
                                        cont_neri=cont_neri+1
                                    else:
                                        f_enough=0
                                if cont_neri>30 and cont_neri>cont_neri_max:
                                    meta=(i+i+cont_neri)/2
                                    cont_neri_max=cont_neri
                                    i=i+int(meta)
                                else:
                                    i=i+cont_neri
                            else:
                                i=i+1
                    
                    if meta!=0:
                        pixel_verdi_dx=cv2.countNonZero(imggreen[0:5,int(meta):320])#conto i pixel di colore verde a dx della linea
                        pixel_verdi_sx=cv2.countNonZero(imggreen[0:5,0:int(meta)])#conto i pixel di colore verde a sx della linea
                        #print('\npv1=',primo_verde)
                        if f_entrato_verde==0:
                            print('\n\tSTART VERDE')
                            print('meta=',meta)
                            print('pdx=',pixel_verdi_dx)
                            print('psx=',pixel_verdi_sx)
                            #cv2.imwrite('imggreen.jpg',imggreen)
                            #cv2.imwrite('ol.jpg',only_linea)
                        #print('fe=',f_entrato_verde)
                        if pixel_verdi_dx>30 and pixel_verdi_sx<30 and f_entrato_verde==0:
                            primissimo=primo_verde='dx'
                            #cv2.imwrite('imageline.jpg',only_linea)
                            #cv2.imwrite('image2.jpg',image)
                        elif pixel_verdi_dx<30 and pixel_verdi_sx>30 and f_entrato_verde==0:
                            primissimo=primo_verde='sx'
                            #cv2.imwrite('imageline.jpg',only_linea)
                            #cv2.imwrite('image2.jpg',image)
                        elif pixel_verdi_dx>30 and pixel_verdi_sx>30:
                            primo_verde='dv'
                        f_entrato_verde=1
                        #if(primo_verde==0):
                        #   while True:
                        #      a=0
                        
                if f_verde==0 and primo_verde!=0:
                    '''controllo incrocio a X e a T'''
                    #print('pv1=',primo_verde)
                    if primo_verde=='sx':#primo verde a sx
                        punto_fin=int(meta)
                        meta=0
                        i=0
                        while i<punto_fin:#con il for controllo dov'è la metà orizzontalmente del verde
                            if imggreen[0:1,i]==255:
                                f_enough=1
                                cont_verdi=1
                                while f_enough==1 and i+cont_verdi<punto_fin:
                                    if imggreen[0:1,cont_verdi+i]==255:
                                        cont_verdi=cont_verdi+1
                                    else:
                                        f_enough=0
                                #print('cv=',cont_verdi)
                                if cont_verdi>5:
                                    meta=(i+i+cont_verdi)/2
                                    break
                                else:
                                    i=i+cont_verdi
                            else:
                                i=i+1
                    else:#primo verde a dx o doppio verde
                        punto_fin=int(meta)
                        meta=0
                        i=319
                        while i>punto_fin:#con il for controllo dov'è la metà orizzontalmente del verde
                            if imggreen[0:1,i]==255:
                                f_enough=1
                                cont_verdi=1
                                while f_enough==1 and i-cont_verdi>punto_fin:
                                    if imggreen[0:1,i-cont_verdi]==255:
                                        cont_verdi=cont_verdi+1
                                    else:
                                        f_enough=0
                                #print('cv=',cont_verdi)
                                if cont_verdi>5:
                                    meta=(i+i-cont_verdi)/2
                                    break
                                else:
                                    i=i-cont_verdi
                            else:
                                i=i-1
                    
                    
                    #print('meta=',meta)
                    cv2.circle(image,(int(meta),5),5,[255,0,0],-1)#segno la metà del verde
                    if int(meta)-2<0:#controllo con questi if che non avvengano errori nel programma durante il calcolo dei limiti per la conversione dell'immagine nella fascia desiderata
                        lim_min=0
                        lim_max=int(meta)+2
                    elif int(meta)+2>320:
                        lim_max=320
                        lim_min=int(meta)-2
                    else:
                        lim_min=int(meta)-2
                        lim_max=int(meta)+2
                    imghsv=cv2.cvtColor(image[0:240,lim_min:lim_max],cv2.COLOR_BGR2HSV)
                    imggreen=cv2.inRange(imghsv,lower,upper)
                    indice_last=0
                    for i in range(0,240):#con questo for cerco l'ultimo punto bianco (ovvero verde)
                        if imggreen[i,0:1]==255:
                            indice_last=i
                    if indice_last+20>200:#stabilisco i limiti massimi e minimi in altezza per poter fare una binarizzazione nella parte dell'immagine che ci serve
                        lim_min_i=200
                        lim_max_i=240
                    elif indice_last+60>240:
                        lim_min_i=indice_last+20
                        lim_max_i=240
                    else:
                        lim_min_i=indice_last+20
                        lim_max_i=indice_last+60
                    #print('pv=',primo_verde)
                    if primo_verde=='sx':#in base a dove era stato visto il verde impongo i limiti in  larghezza per applicare la binarizzazione
                        print('dopo linea sx')
                        lim_min=0
                        lim_max=5
                        lim_tagl_min=0
                        lim_tagl_max=1
                    else:
                        print('dopo linea dx')
                        lim_min=315
                        lim_max=320
                        lim_tagl_min=4
                        lim_tagl_max=5
                        
                    cv2.circle(image,(int(meta),indice_last),5,[255,0,0],-1)#metto un punto dove il verde finisce e nella sua metà
                    cv2.circle(image,(lim_max,indice_last),5,[0,0,255],-1)#metto un punto nel bordo a destra o sinistra del frame con altezza indice_last che corrisponde all'ordinata in cui finisce il verde
                    ret,imgotsu=cv2.threshold(img[lim_min_i:lim_max_i,lim_min:lim_max],127,255,cv2.THRESH_BINARY)
                    imghsv=cv2.cvtColor(image[lim_min_i:lim_max_i,lim_min:lim_max],cv2.COLOR_BGR2HSV)                    
                    imggreen=cv2.inRange(imghsv,lower,upper)
                    imgbin=cv2.bitwise_or(imgotsu,imggreen)
                    cv2.rectangle(image,(lim_min,lim_min_i),(lim_max,lim_max_i),(255,0,0))
                    cont_neri=0
                    for i in range(0,lim_max_i-lim_min_i):#valuto la presenza del nero per decidere finalmente se il verde e dopo la linea o meno
                        if imgbin[i,lim_tagl_min:lim_tagl_max]==0:
                            cont_neri=cont_neri+1
                    '''FINE controllo incrocio a X e a T'''
                    if cont_neri>20:#in base a quanto nero c'è prendo la decisione
                        pixel_verdi_sx=0
                        pixel_verdi_dx=0
                        cont_dopo_linea=cont_dopo_linea+1
                        #cv2.imwrite('POSim'+str(cont_dopo_linea)+'.jpg',image)
                        if cont_dopo_linea>=2:
                            f_no_verde=1
                            primo_verde=cont_dopo_linea=f_entrato_verde=0
                            send_to_arduino(0)
                    else:#altrimenti stampo a video dov'è il verde
                        cont_dopo_linea=0
                        f_verde='v'
                        #cv2.imwrite('NEGim.jpg',image)
                        if pixel_verdi_dx<30 or pixel_verdi_sx<30:
                            send_to_arduino(0)
                 
                if primo_verde=='dv' and f_verde=='v':#caso doppio verde
                    print('double green')
                    if primissimo=='sx':
                        send_to_arduino(const_doppio_verde+170)#mando comando all'arduino di girarsi di 180 gradi
                    elif primissimo=='dx':
                        send_to_arduino(const_doppio_verde-170)#mando comando all'arduino di girarsi di 180 gradi
                    else:
                        print('6680')
                        send_to_arduino(const_doppio_verde+180)
                    f_verde=0
                    f_controllo_buffer=1#abilito flag che mi fa passare sotto nell'elif e che controlla quando arriva il messaggio di fine dall'arduino
                    f_controllo_doppio_verde=1#Serve per riabilitare il flag del doppio verde nel caso in cui veda doppio verde ma è già andato sulla linea
                    ser.flushInput()#pulisco il buffer in ricezione
                    
                elif primo_verde=='sx' and f_verde=='v':#caso verde a sinistra
                    #print('verde sx')
                    if f_end_gira_verde==0:
                        if f_start_gira_verde==0:
                            img_gray_sx=img[0:100,5:6]#immagine laterale linea a sinistra in scala di grigi
                            ret,img_nero_sx=cv2.threshold(img_gray_sx,150,255,cv2.THRESH_BINARY)#immagine laterale linea a sinistra binarizzata
                            img_hsv_sx=cv2.cvtColor(image[0:100,5:8],cv2.COLOR_BGR2HSV)
                            img_green_sx=cv2.inRange(img_hsv_sx,lower,upper)
                            only_linea_sx=cv2.bitwise_or(img_nero_sx,img_green_sx[0:100,0:1])
                            num_black_pixel=100-(cv2.countNonZero(only_linea_sx))
                            #print(cv2.countNonZero(only_linea_sx),' ',num_black_pixel)
                            #cv2.imshow('sx',only_linea_sx)
                            if num_black_pixel>=25:
                                f_controllo_verde=0
                                f_start_gira_verde=1
                                f_correzione_verde=0
                                #print('verde sx')
                            #print('1')
                        else:
                            imghsv=cv2.cvtColor(image[5:235,0:10],cv2.COLOR_BGR2HSV)
                            imggreen=cv2.inRange(imghsv,lower,upper)
                            ret,imgotsu=cv2.threshold(img[5:235,0:10],150,255,cv2.THRESH_BINARY)
                            whereis_linea=cv2.bitwise_or(imgotsu,imggreen)
                            imgtemp=whereis_linea
                            meta=0
                            for i in range(0,225):#controllo il punto medio della linea distinguendola dal verde
                                if whereis_linea[i,0:1]==0:
                                    f_enough=1
                                    cont_neri=1
                                    while f_enough==1 and (i+cont_neri)<225:
                                        if whereis_linea[cont_neri+i,0]==0:
                                            cont_neri=cont_neri+1
                                        else:
                                            f_enough=0
                                    if cont_neri>50:
                                        meta=(i+i+cont_neri)/2
                                        cv2.circle(image,(5,int(meta)),3,[0,255,0],-1)
                                        break
                            if meta>60:#quando la metà della linea e sotto gli 80 pixel abilito la rotazione
                                send_to_arduino(-148-int(meta))#mando ad arduino la difficoltà della curva e gli dico da che parte girare
                                #print('>60  ',-148-int(meta))
                            elif meta<=60 and meta>0:
                                send_to_arduino(-148-int(meta))
                                #print('<60,>10  ',-148-int(meta))
                                f_correzione_verde=1
                            elif f_correzione_verde:
                                send_to_arduino(const_gira_verde_sx)
                                f_start_gira_verde=0
                                f_controllo_doppio_verde=1
                                f_end_gira_verde=1
                                #print('2')

                    else:
                        meta=0
                        ret,whereis_linea=cv2.threshold(img[0:5,0:320],150,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)#binarizzo con otsu per capire dov'e la linea il verde non c'è più e quindi nemmeno il filtro per distinguerlo dal verde
                        for i in range(0,320):#calcolo punto medio della linea
                            if whereis_linea[0:1,i]==0:
                                f_enough=1
                                cont_neri=1
                                while f_enough==1 and i+cont_neri<320:
                                    if whereis_linea[0:1,cont_neri+i]==0:
                                        cont_neri=cont_neri+1
                                    else:
                                        f_enough=0
                                if cont_neri>50:
                                    meta=(i+i+cont_neri)/2
                                    cv2.circle(image,(int(meta),5),3,[0,255,0],-1)
                                    break
                        if meta>150:#quando il punto medio della linea è nella parte alta del frame con ascissa superiore a 150 il verde è superato
                            send_to_arduino(0)
                            primissimo=f_entrato_verde=primo_verde=f_end_gira_verde=f_verde=0
                            print('\tEND VERDE sx')
                            f_controllo_verde=1
                            #print('3')
                        
                            
                elif primo_verde=='dx' and f_verde=='v':#caso verde  a destra
                    #print('verde dx')
                    if f_end_gira_verde==0:
                        if f_start_gira_verde==0:
                            img_gray_dx=img[0:100,315:316]#immagine laterale linea a sinistra in scala di grigi
                            ret,img_nero_dx=cv2.threshold(img_gray_dx,150,255,cv2.THRESH_BINARY)#immagine laterale linea a sinistra binarizzata
                            img_hsv_dx=cv2.cvtColor(image[0:100,315:318],cv2.COLOR_BGR2HSV)
                            img_green_dx=cv2.inRange(img_hsv_dx,lower,upper)
                            only_linea_dx=cv2.bitwise_or(img_nero_dx,img_green_dx[0:100,0:1])
                            num_black_pixel=100-(cv2.countNonZero(only_linea_dx))
                            #print(cv2.countNonZero(only_linea_sx),' ',num_black_pixel)
                            #cv2.imshow('sx',only_linea_sx)
                            if num_black_pixel>=25:
                                f_controllo_verde=0
                                f_start_gira_verde=1
                                f_correzione_verde=0
                                #print('verde dx')
                            #print('1')
                        else:
                            imghsv=cv2.cvtColor(image[5:235,310:320],cv2.COLOR_BGR2HSV)
                            imggreen=cv2.inRange(imghsv,lower,upper)
                            ret,imgotsu=cv2.threshold(img[5:235,310:320],150,255,cv2.THRESH_BINARY)
                            whereis_linea=cv2.bitwise_or(imgotsu,imggreen)
                            imgtemp=whereis_linea
                            meta=0
                            for i in range(0,225):#controllo il punto medio della linea distinguendola dal verde
                                if whereis_linea[i,0:1]==0:
                                    f_enough=1
                                    cont_neri=1
                                    while f_enough==1 and (i+cont_neri)<225:
                                        if whereis_linea[cont_neri+i,0]==0:
                                            cont_neri=cont_neri+1
                                        else:
                                            f_enough=0
                                    if cont_neri>50:
                                        meta=(i+i+cont_neri)/2
                                        cv2.circle(image,(315,int(meta)),3,[0,255,0],-1)
                                        break
                            if meta>60:#quando la metà della linea e sotto gli 80 pixel abilito la rotazione
                                send_to_arduino(148+int(meta))#mando ad arduino la difficoltà della curva e gli dico da che parte girare
                                #print('>60  ',148+int(meta))
                            elif meta<=60 and meta>0:
                                send_to_arduino(148+int(meta))
                                #print('<60,>10  ',148+int(meta))
                                f_correzione_verde=1
                            elif f_correzione_verde:
                                send_to_arduino(const_gira_verde_dx)
                                f_start_gira_verde=0
                                f_controllo_doppio_verde=1
                                f_end_gira_verde=1
                                #print('2')
                            
                    else:
                        meta=0
                        ret,whereis_linea=cv2.threshold(img[0:5,0:320],150,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                        for i in range(0,320):#calcolo punto medio della linea
                            if whereis_linea[0:1,i]==0:
                                f_enough=1
                                cont_neri=1
                                while f_enough==1 and i+cont_neri<320:
                                    if whereis_linea[0:1,cont_neri+i]==0:
                                        cont_neri=cont_neri+1
                                    else:
                                        f_enough=0
                                if cont_neri>50:
                                    meta=(i+i+cont_neri)/2
                                    cv2.circle(image,(int(meta),5),3,[0,255,0],-1)
                                    break
                        if meta<170:#quando il punto medio della linea è nella parte alta del frame con ascissa minore a 170 il verde è superato
                            send_to_arduino(0)
                            primissimo=primo_verde=f_entrato_verde=f_end_gira_verde=f_verde=0
                            print('\tEND VERDE dx')
                            f_controllo_verde=1
                            #print('6')
                            
            elif f_no_verde and (cv2.countNonZero(imggreen[0:5,0:320])<50):#Quando prendo verde dopo linea, disattivo la funzione del verde e la riabilito quando non lo vedo più
                f_no_verde=0
            elif f_controllo_buffer:#fase di controllo buffer in ricezione abilitata f_controllo_buffer che aspetta il messaggio di ritorno da arduino
                #print('aspetto_verde')
                ser.flushInput()
                msg = (ser.read(1)).decode('utf-8','ignore')
                log.lineLOG('i','doppio verde')
                LED.LED('green',True)
                while msg != 'F':#ser.inWaiting()==0:
                    #print('aspetto_verde')
                    ret,image=cap.read()
                    cv2.imshow('doppio verde',image)
                    msg = (ser.read(1)).decode('utf-8','ignore')
                    if msg == 'j':
                        break
                    if cv2.waitKey(1) == 13:
                        break
                log.lineLOG('i','end doppio verde')
                LED.LED('green',False)
                ser.flushInput()
                f_entrato_verde=f_controllo_buffer=primo_verde=primissimo=0
                send_to_arduino(0)
                #ret,image=cap.read()
                print('\tEND VERDE dv')
            else:
                tot=np.zeros((240,320),np.uint8)
                finale=np.zeros((240,320),np.uint8)
                finale[0:240,0:320]=255
                tot[0:240,0:320]=255
                tot[10:230,5]=img[10:230,5] #sx
                tot[10:230,316]=img[10:230,316] #dx
                tot[5,10:310]=img[5,10:310] #a
                tot[235,10:310]=img[235,10:310] #b
                tot[pxc,10:310]=img[pxc,10:310] #center
             
                minsx=min(tot[10:230,5].ravel())
                mindx=min(tot[10:230,316].ravel())
                mina=min(tot[5,10:310].ravel())
                minb=min(tot[235,10:310].ravel())
                minc=min(tot[pxc,10:310].ravel())
                maxsx=max(tot[10:230,5].ravel())
                maxdx=max(tot[10:230,316].ravel())
                maxa=max(tot[5,10:310].ravel())
                maxb=max(tot[235,10:310].ravel())
                maxc=max(tot[pxc,10:310].ravel())
                diffsx=maxsx-minsx
                diffdx=maxdx-mindx
                diffa=maxa-mina
                diffb=maxb-minb
                diffc=maxc-minc
                medsx=(int(minsx)+int(maxsx))*0.5
                meddx=(int(mindx)+int(maxdx))*0.5
                meda=(int(mina)+int(maxa))*0.5
                medb=(int(minb)+int(maxb))*0.5
                medc=(int(minc)+int(maxc))*0.5
            
                valsx='n' if diffsx>150 else 't' if medsx<80 else 'b'
                valdx='n' if diffdx>150 else 't' if meddx<80 else 'b'
                vala='n' if diffa>100 else 't' if meda<80 else 'b'#diffa>150
                valb='n' if diffb>150 else 't' if medb<90 else 'b'
                valc='n' if diffc>125 else 't' if medc<85 else 'b'

                #print('s=',medsx,' d=',meddx,' a=',meda,' b=',medb)
                #print('s=',valsx,' d=',valdx,' a=',vala,' b=',valb)
                
                if valsx=='n':
                    ret,finale[10:230,5:6]=cv2.threshold(tot[10:230,5],0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                        
                elif valsx=='t':
                    finale[10:230,5]=0
                else:
                    finale[10:230,5]=255
                if valdx=='n':
                    ret,finale[10:230,315:316]=cv2.threshold(tot[10:230,316],0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                elif valdx=='t':
                    finale[10:230,316]=0
                else:
                    finale[10:230,316]=255
                    
                if vala=='n':
                    ret,finale[5:6,10:310]=cv2.threshold(tot[5:6,10:310],0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                elif vala=='t':
                    finale[5,10:310]=0
                else:
                    finale[5,10:310]=255
                
                if valb=='n':
                   ret,finale[235:236,10:310]=cv2.threshold(tot[235:236,10:310],0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                elif valb=='t':
                    finale[235,10:310]=0
                else:
                    finale[235,10:310]=255

                if valc=='n':
                   ret,finale[pxc:121,10:310]=cv2.threshold(tot[pxc:121,10:310],0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
                elif valc=='t':
                    finale[pxc,10:310]=0
                else:
                    finale[pxc,10:310]=255


          
                corners = cv2.goodFeaturesToTrack(finale,22,0.04,5)#trova le coordinate dei punti dei segmenti

                sommaB = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                sommaA = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                sommaD = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                sommaS = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                sommaC = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                contB=0
                contA=0
                contD=0
                contS=0
                contC=0
                if corners is not None and f_inizio_correzione_gap==0:#aggiunto flag per essere sicuri di corregere
                    corners=np.int0(corners)
                    #for corner in corners:
                        #x,y = corner.ravel()
                        #print(x,y)
                        #cv2.circle(touch,(x,y),3,[200,200,200],-1)

                    corners = corners.ravel()
                    #print(corners)
            
                    for i in range(1,len(corners),2):
                       if corners[i] == (resolution[1]-5):#basso
                           sommaB[contB]=corners[i-1]
                           contB=contB+1       
                       elif corners[i] == 5:#alto
                           sommaA[contA]=corners[i-1]
                           contA=contA+1
                       elif corners[i] == pxc:#centrale
                           sommaC[contC]=corners[i-1]
                           contC=contC+1
                            
                    for i in range(0,len(corners),2):
                       if corners[i]==(resolution[0]-5):#dx
                           sommaD[contD]=corners[i+1]
                           contD=contD+1
                       elif corners[i]==5:#sx
                           sommaS[contS]=corners[i+1]
                           contS=contS+1






                    
                    '''INIZIO controllo argento
                    if (contA>7 or contC>4) and f_argento_sicuro==False:
                        print('argento sicuro  Alto: ',contA,'Centro: ',contC)
                        print(f_cont_argento)
                        f_cont_argento+=1 #Flag per disabilitare curve dx,sx quando vede argento
                        f_argento_sicuro=True
                        f_curvadx=0
                        f_curvasx=0
                        send_to_arduino(0)
                        #cv2.imwrite('/home/pi/Desktop/FILE/LINEA/finale%s.jpg' % ind,finale)
                        #cv2.imwrite('/home/pi/Desktop/FILE/LINEA/image%s.jpg' % ind,image)
                        #ind+=1
                    elif (contA>4 or contC>4) and f_argento_sicuro==False:
                        print(f_cont_argento)
                        print('Alto: ',contA,'Centro: ',contC)
                        f_cont_argento=f_cont_argento+1 #Flag per disabilitare curve dx,sx quando vede argento
                        f_curvadx=0
                        f_curvasx=0
                        send_to_arduino(0)
                    if f_argento_sicuro or f_cont_argento>=5:
                        f_argento=1#disabilita le curve
                        print('argento!!!',f_argento_sicuro,'contatore: ',f_cont_argento)
                        LED.LED('blue',True)
                        log.lineLOG('w','argento')
                    FINE controllo argento'''


                        



                    
                    '''controllo fughe'''
                    if contA==2 and f_argento==0:
                        fugamin=min(sommaA[0],sommaA[1])#controllo per le fughe alto (solo fuga)
                        fugamax=max(sommaA[0],sommaA[1])
                        if fugamin!=10 and fugamax!=310:
                           if fugamax-fugamin<50:
                                contA=0
                                #print('altof1')
                                #f_fuga=1

                    if contD==2 and f_argento==0:
                        fugamin=min(sommaD[0],sommaD[1])#controllo per le fughe laterali (solo fuga)
                        fugamax=max(sommaD[0],sommaD[1])
                        if fugamin!=10 and fugamax!=230:
                           if fugamax-fugamin<50:
                                contD=0
                                #print('destraf1')
                                #f_fuga=1

                    if contS==2 and f_argento==0:
                        fugamin=min(sommaS[0],sommaS[1])#controllo per le fughe laterali (solo fuga)
                        fugamax=max(sommaS[0],sommaS[1])
                        if fugamin!=10 and fugamax!=230:
                           if fugamax-fugamin<50:
                                contS=0
                                #print('sinistraf1')
                                #f_fuga=1
            
                    if contA==4 and f_argento==0:#controllo per fughe alto (linea+fuga) (fuga+fuga)
                        f_cont_argento=0
                        contA=2
                        dist1=max(sommaA[0],sommaA[1])-min(sommaA[0],sommaA[1])
                        dist2=max(sommaA[2],sommaA[3])-min(sommaA[2],sommaA[3])
                        if dist1<50 and dist2<50:
                            contA=0
                        elif dist1<dist2:
                            sommaA[0]=sommaA[2]
                            sommaA[1]=sommaA[3]
                            
                    if contD==4 and f_argento==0:#controllo per fughe e acute laterali (linea+fuga) (fuga+fuga)
                        contD=2
                        dist1=max(sommaD[0],sommaD[1])-min(sommaD[0],sommaD[1])
                        dist2=max(sommaD[2],sommaD[3])-min(sommaD[2],sommaD[3])
                        if dist1<50 and dist2<50:
                            contD=0
                        elif dist1<dist2:
                            sommaD[0]=sommaD[2]
                            sommaD[1]=sommaD[3]

                    if contS==4 and f_argento==0:#controllo per fughe e acute laterali (linea+fuga) (fuga+fuga)
                        contS=2
                        dist1=max(sommaS[0],sommaS[1])-min(sommaS[0],sommaS[1])
                        dist2=max(sommaS[2],sommaS[3])-min(sommaS[2],sommaS[3])
                        if dist1<50 and dist2<50:
                            contS=0
                        elif dist1<dist2:
                            sommaS[0]=sommaS[2]
                            sommaS[1]=sommaS[3]

                    '''FINE controllo fughe'''
                    
                    ''' INIZIO calcolo angolo linea'''
                    if contB==2 and contA==2 and contC==2 and f_argento==0:
                        f_cont_argento=0
                        imageLine = image
                        cv2.circle(finale,(int(sommaC[0]),pxc),3,[0,0,0],-1)
                        cv2.circle(finale,(int(sommaC[1]),pxc),3,[0,0,0],-1)
                        xcentro=int((sommaC[0]+sommaC[1])/2)
                        xbasso=int((sommaB[0]+sommaB[1])/2)
                        cv2.line(imageLine, (xcentro,pxc), (xbasso,235), [0,0,255], 1)
                        cv2.line(imageLine, (sommaC[0],pxc), (sommaC[1],pxc), [0,0,255], 1)
                        cv2.line(imageLine, (sommaB[0],235), (sommaB[1],235), [0,0,255], 1)
                        angolo=np.arctan((xcentro-xbasso)/(235-pxc))
                        angolo=np.degrees(angolo)
                        #cv2.imshow('immagine',imageLine)
                        #print('angle=',(int)(angolo),'centro',xcentro,'bas',xbasso)
                    ''' FINE calcolo angolo linea'''

                    '''INIZIO controllo acute'''
                    if contD==2 and contS==2 and contA==0 and f_argento==0:#controllo per acute
                        ultima_distanza_dx=0
                        ultima_distanza_sx=0
                        piccolo_dx=min(sommaD[0],sommaD[1])
                        piccolo_sx=min(sommaS[0],sommaS[1])
                        if piccolo_sx>piccolo_dx:
                            f_acuta='s'
                        else:
                            f_acuta='d'
                    '''FINE controllo acute'''
                    
                    '''INIZIO controllo lato veritiero'''
                    if contD==2 and contS==2 and f_curvadx==0 and f_curvasx==0 and f_argento==0:
                        misura_dx=max(sommaD[0],sommaD[1])-min(sommaD[0],sommaD[1])
                        misura_sx=max(sommaS[0],sommaS[1])-min(sommaS[0],sommaS[1])
                        if misura_sx>misura_dx:
                            contD=0
                        elif misura_sx<misura_dx:
                            contS=0
                        else:
                            contD=0
                            contS=0
                    '''FINE  controllo lato veritiero'''
                    
                    if (contA == 2 or acuta=='s' or acuta=='d') and f_argento==0:
                        #print('altof0')
                        f_curvadx=0
                        f_curvasx=0
                        f_acuta=0
                        f_inizio_correzione_gap=0
                        f_fine_correzione_gap=0
                        distanza = (sommaA[0]+sommaA[1])/2 - resolution[0]/2#differenza tra ascissa punto medio alto e 320/2
                        if distanza<=0 :#controllo per aggiornare distanze di destra e sinistra sul dritto
                            ultima_distanza_sx=distanza
                            ultima_distanza_dx=0
                        else:
                            ultima_distanza_dx=distanza
                            ultima_distanza_sx=0
                        ultima_distanza=distanza
                        if acuta==0:
                            send_to_arduino(int(distanza))
                        
                        if distanza!=-160 and contA==2:
                            alt = (int((sommaA[0]+sommaA[1])/2),5)
                            cv2.circle(finale,alt,3,[0,0,0],-1)
                        
                        if((distanza >=-100 and acuta=='s') or (distanza!=-160 and distanza<=100 and acuta=='d')) and contA==2:
                            acuta=0
                            send_to_arduino(0)
                        
                    elif contD == 2 and f_curvadx==0 and f_argento==0:
                        #print('destraf0')
                        f_curvasx=1
                        #f_fuga=0
                        dx = (resolution[0]-5,int((sommaD[0]+sommaD[1])/2))
                        distanza=((resolution[0]/2-5)+(sommaD[0]+sommaD[1])/2)
                        ultima_distanza_dx=distanza
                        send_to_arduino(int(distanza))
                        cv2.circle(finale,dx,3,[0,0,0],-1)
                        #print('d')
                    
                    elif contS == 2 and f_curvasx==0 and f_argento==0:
                        #print('sinistraf0')
                        f_curvadx=1
                        #f_fuga=0
                        sx = (5,int((sommaS[0]+sommaS[1])/2))
                        distanza=-int((resolution[0]/2-5)+(sommaS[0]+sommaS[1])/2)
                        ultima_distanza_sx=distanza
                        send_to_arduino(int(distanza))
                        cv2.circle(finale,sx,3,[0,0,0],-1)
                        #print('s')
                
                    elif ((ultima_distanza_sx<-265 and f_acuta==0) or f_acuta=='s') and contS==0 and f_argento==0:
                        send_to_arduino(const_acutasx)
                        acuta='s'
                        #print('s=',f_acuta,'usx=',ultima_distanza_sx,'udx=',ultima_distanza_dx)
                        #print('ps=',piccolo_sx,'pd=',piccolo_dx)
                        ultima_distanza_sx=0#azzero distanza per evitare problemi
                    
                    elif ((ultima_distanza_dx>265 and f_acuta==0) or f_acuta=='d') and contD==0 and f_argento==0:
                        send_to_arduino(const_acutadx)
                        acuta='d'
                        #print('d=',f_acuta,'udx=',ultima_distanza_dx,'usx=',ultima_distanza_sx)
                        #print('ps=',piccolo_sx,'pd=',piccolo_dx)
                        ultima_distanza_dx=0#azzero distanza per evitare problemi
                                     
                    else:
                        send_to_arduino(0)
                        #print('d=',ultima_distanza_dx,'s=',ultima_distanza_sx,'contB=',contB)
                elif acuta==0:
                   
                    #print('ini=',f_inizio_correzione_gap,'fin=',f_fine_correzione_gap)
                    if(ultima_distanza_sx>-148  or ultima_distanza_dx<148) and f_inizio_correzione_gap==0 and f_fine_correzione_gap==0:
                        #print('angolo=',int(angolo))
                        if angolo>=4 or angolo<=-4:#solo se angolo è sostanzioso
                            if ultima_distanza_dx>50:#caso inerzia grave
                                angolo=angolo-3
                            elif ultima_distanza_sx<-50:
                                angolo=angolo+3
                            send_to_arduino(const_gyro_grades+int(angolo))
                            
                            sent_angolo = int(angolo)#salvo su una variabile per stampare successivamente sul file log

                            ser.flushInput()
                            f_inizio_correzione_gap=1
                
                    elif f_inizio_correzione_gap:#aspetto qui una risposta (il flag serve inoltre per non entrare nell' "if Corners is not None")
                        '''
                        if ser.inWaiting()==1:
                            f_inizio_correzione_gap=0
                            f_fine_correzione_gap=1
                            #print('risposta')
                            send_to_arduino(0)
                        '''
                        #print('aspetto_gap')
                        ser.flushInput()
                        msg = (ser.read(1)).decode('utf-8','ignore')
                        log.lineLOG('i','gap   gradi= '+str(sent_angolo))
                        LED.LED('blue',True)
                        while msg !='F':
                            ret,image=cap.read()
                            msg = (ser.read(1)).decode('utf-8','ignore')
                            if msg == 'j':
                                break
                            if cv2.waitKey(1) == 13:
                                break
                        log.lineLOG('i','end gap')
                        LED.LED('blue',False)
                        ser.flushInput()
                        #f_controllo_buffer=0
                        #send_to_arduino(0)    
                        f_inizio_correzione_gap=0
                        f_fine_correzione_gap=1
                        send_to_arduino(0)
                        #print('end gap')

                    else:#ho ricevuto la risposta di fine (ho girato di angolo), resetto l'angolo che non torni dentro per qualche motivo, faccio andare avanti finchè non vedo la linea in "if Corners is not None"
                        send_to_arduino(0)
                        angolo=0
                
            cv2.imshow('image',image)
            cv2.imshow('finale',finale)
            #cv2.imwrite('/home/pi/Desktop/FILE/LINEA/finale%s.jpg' % o,finale)
            #cv2.imwrite('/home/pi/Desktop/FILE/LINEA/image%s.jpg' % o,image)
            #o+=1
            key=cv2.waitKey(10)& 0xff
            if key==27:
                LED.LED('off',False)
                cap.release()
                cv2.destroyAllWindows()
                exit(0)
                break
    except Exception as e:
        ty,val,traceback=sys.exc_info()
        LED.LED('red',True)
        #print(traceback.print_exc())
        if f_exception:
            f_exception = 0
            log.lineLOG('e',str(e))
            
        #print(e)
LED.LED('off',False)
cap.release()
cv2.destroyAllWindows()
exit(0)

