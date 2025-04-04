import os
import cv2
import numpy as np
#from picamera.array import PiRGBArray
#from picamera import PiCamera
import tensorflow as tf
import argparse
import sys
#import serial
import struct

#import RPi.GPIO as GPIO
#from LOGfile import log
from Libraries import LED
import time

# Set up camera constants
IM_WIDTH = 640
IM_HEIGHT = 320

lmax = 300
hmax = 260

sys.path.append('..')

# Import utilites
from utils import label_map_util
from utils import visualization_utils as vis_util

'''############################NERA#########################'''
#fine_ar => funziona solo con le argento
#fine_ar-ne => funziona con entrambe ma non bene (le argento male le nere meglio)
MODEL_NAME_BLACK = 'fine30k'
# Grab path to current working directory
CWD_PATH_BLACK = '/home/pi/Desktop/ARGENTO_NERA/'#os.getcwd()
# Path to frozen detection graph .pb file, which contains the model that is used for object detection.
PATH_TO_CKPT_BLACK = os.path.join(CWD_PATH_BLACK,MODEL_NAME_BLACK,'frozen_inference_graph.pb')
# Path to label map file
PATH_TO_LABELS_BLACK = os.path.join(CWD_PATH_BLACK,'data','label_map_black.pbtxt')
# Number of classes the object detector can identify
NUM_CLASSES_BLACK = 1
## Load the label map.
# Label maps map indices to category names, so that when the convolution
# network predicts `5`, we know that this corresponds to `airplane`.
# Here we use internal utility functions, but anything that returns a
# dictionary mapping integers to appropriate string labels would be fine
label_map_BLACK = label_map_util.load_labelmap(PATH_TO_LABELS_BLACK)
categories_BLACK = label_map_util.convert_label_map_to_categories(label_map_BLACK, max_num_classes=NUM_CLASSES_BLACK, use_display_name=True)
category_index_BLACK = label_map_util.create_category_index(categories_BLACK)
# Load the Tensorflow model into memory.
detection_graph_BLACK = tf.Graph()
with detection_graph_BLACK.as_default():
    od_graph_def_BLACK = tf.GraphDef()
    with tf.gfile.GFile(PATH_TO_CKPT_BLACK, 'rb') as fid:
        serialized_graph_BLACK = fid.read()
        od_graph_def_BLACK.ParseFromString(serialized_graph_BLACK)
        tf.import_graph_def(od_graph_def_BLACK, name='')

    sess_BLACK = tf.Session(graph=detection_graph_BLACK)
    
# Define input and output tensors (i.e. data) for the object detection classifier
# Input tensor is the image
image_tensor_BLACK = detection_graph_BLACK.get_tensor_by_name('image_tensor:0')
# Output tensors are the detection boxes, scores, and classes
detection_boxes_BLACK = detection_graph_BLACK.get_tensor_by_name('detection_boxes:0')
# Each score represents level of confidence for each of the objects.
# The score is shown on the result image, together with the class label.
detection_scores_BLACK = detection_graph_BLACK.get_tensor_by_name('detection_scores:0')
detection_classes_BLACK = detection_graph_BLACK.get_tensor_by_name('detection_classes:0')
# Number of objects detected
num_detections_BLACK = detection_graph_BLACK.get_tensor_by_name('num_detections:0')

'''############################ARGENTO#########################'''
#fine_ar => funziona solo con le argento
#fine_ar-ne => funziona con entrambe ma non bene (le argento male le nere meglio)
MODEL_NAME = 'fine_ar'
# Grab path to current working directory
CWD_PATH = '/home/pi/Desktop/ARGENTO_NERA/'#os.getcwd()
# Path to frozen detection graph .pb file, which contains the model that is used for object detection.
PATH_TO_CKPT = os.path.join(CWD_PATH,MODEL_NAME,'frozen_inference_graph.pb')
# Path to label map file
PATH_TO_LABELS = os.path.join(CWD_PATH,'data','label_map.pbtxt')
# Number of classes the object detector can identify
NUM_CLASSES = 2
## Load the label map.
# Label maps map indices to category names, so that when the convolution
# network predicts `5`, we know that this corresponds to `airplane`.
# Here we use internal utility functions, but anything that returns a
# dictionary mapping integers to appropriate string labels would be fine
label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)
# Load the Tensorflow model into memory.
detection_graph = tf.Graph()
with detection_graph.as_default():
    od_graph_def = tf.GraphDef()
    with tf.gfile.GFile(PATH_TO_CKPT, 'rb') as fid:
        serialized_graph = fid.read()
        od_graph_def.ParseFromString(serialized_graph)
        tf.import_graph_def(od_graph_def, name='')

    sess = tf.Session(graph=detection_graph)
    
# Define input and output tensors (i.e. data) for the object detection classifier
# Input tensor is the image
image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')
# Output tensors are the detection boxes, scores, and classes
detection_boxes = detection_graph.get_tensor_by_name('detection_boxes:0')
# Each score represents level of confidence for each of the objects.
# The score is shown on the result image, together with the class label.
detection_scores = detection_graph.get_tensor_by_name('detection_scores:0')
detection_classes = detection_graph.get_tensor_by_name('detection_classes:0')
# Number of objects detected
num_detections = detection_graph.get_tensor_by_name('num_detections:0')
'''#####################FINE####################'''

def find_black(frame_expanded,frame,vis):#,boxes,classes,scores,category_index,min_score_thresh):
    #print('ok frame')
    min_score_thresh = 0.3
    (boxes, scores, classes, num) = sess_BLACK.run(
                [detection_boxes_BLACK, detection_scores_BLACK, detection_classes_BLACK, num_detections_BLACK],
                feed_dict={image_tensor_BLACK: frame_expanded})
    #print('ok analisi')
    angolo = 255
    distance = 255
    max_number_to_find = 20
    misure=[]
    coordinate=[]
    
    boxes = np.squeeze(boxes)
    #classes = np.squeeze(classes).astype(np.int32)
    scores=np.squeeze(scores)
    for i in range (0,max_number_to_find):
        #print(scores[i])
        if scores[i] >= min_score_thresh or scores is None:
            text = str(round(scores[i]*100,2))+'%'
            print('                                                       precisione=  ',text)
            ymin=(int)(boxes[i][0]*IM_HEIGHT)
            xmin=(int)(boxes[i][1]*IM_WIDTH)
            ymax=(int)(boxes[i][2]*IM_HEIGHT)
            xmax=(int)(boxes[i][3]*IM_WIDTH)
            cv2.rectangle(frame,((int)(xmin),(int)(ymax)),((int)(xmax),(int)(ymin)),(0,0,255),3)
            cv2.putText(frame, text, (350, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 255),5, lineType=cv2.LINE_AA)
            misurax=xmax-xmin
            misuray=ymax-ymin
            if misurax>lmax and misuray>hmax or misurax>lmax:#aggiunto
                print('x= ',misurax,'y= ',misuray)
                print('scartata')
                break
            rap = misurax/misuray
            if rap>=2 or rap<=0.5:
                print('out rapporto')
                break
            misure.append(misurax)
            coordinate.append([ymin,xmin,xmax])
    if len(misure) is not 0:
        nearest = max(misure)
        pos_coordinate=misure.index(nearest)
        ymin,xmin,xmax=coordinate[pos_coordinate]
        distance = (2220/nearest)
        distance_angolo= (2220/nearest) + 9.5
        pixel = 5/(xmax - xmin)    #5 cm diametro pallina
        pix_lato = -320 + (xmax+xmin)/2
        dist_lato = pixel * pix_lato    #distanza della pallina in cm dal centro della camera
        angolo = np.arctan(dist_lato/distance_angolo)
        distance = dist_lato/(np.sin(angolo))
        ipo = (9.5/(np.cos(angolo))) #ipotenusa robot
        distance = distance - ipo
        angolo=np.degrees(angolo)
        if angolo == 0:
            print('angolo 0')
            distance = distance_angolo #quando viene un angolo esattamente 0 il seno=0 e non calcola la distanza
        print('distanza=   ',distance,'angolo=   ',angolo)#,'pix   ',dist_lato)
    #print(angolo,distance)
    #print('fine lib')
    if angolo !=0 and distance !=0:
        if distance >= int(vis):
            LED.LED('red',True)
            return frame,255,255
        else:
            LED.LED('blue',True)
            return frame,angolo, distance
    elif angolo ==0 and distance !=0:
        LED.LED('blue',True)
        return frame,angolo, distance
    else:
        LED.LED('red',True)
        return frame,255,255

def find_silver(frame_expanded,frame,vis):#,boxes,classes,scores,category_index,min_score_thresh):
    #print('ok frame')
    min_score_thresh = 0.3
    (boxes, scores, classes, num) = sess.run(
                [detection_boxes, detection_scores, detection_classes, num_detections],
                feed_dict={image_tensor: frame_expanded})
    #print('ok analisi')
    angolo = 255
    distance = 255
    max_number_to_find = 20
    misure=[]
    coordinate=[]
    a_n_an = 'argento'
    boxes = np.squeeze(boxes)
    classes = np.squeeze(classes).astype(np.int32)
    scores=np.squeeze(scores)
    for i in range (0,max_number_to_find):
        #print(scores[i])
        if scores[i] >= min_score_thresh or scores is None:
            class_name = category_index[classes[i]]['name']
            if str(class_name) == a_n_an:
                text = str(round(scores[i]*100,2))+'%'
                print('                                                       precisione=  ',text)
                ymin=(int)(boxes[i][0]*IM_HEIGHT)
                xmin=(int)(boxes[i][1]*IM_WIDTH)
                ymax=(int)(boxes[i][2]*IM_HEIGHT)
                xmax=(int)(boxes[i][3]*IM_WIDTH)
                cv2.rectangle(frame,((int)(xmin),(int)(ymax)),((int)(xmax),(int)(ymin)),(255,255,255),3)
                cv2.putText(frame, text, (350, 50), cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 255),5, lineType=cv2.LINE_AA)
                misurax=xmax-xmin
                misuray=ymax-ymin
                if misurax>lmax and misuray>hmax or misurax>lmax:#aggiunto
                    print('x= ',misurax,'y= ',misuray)
                    print('scartata')
                    break
                rap = misurax/misuray
                if rap>=2 or rap<=0.5:
                    print('out rapporto')
                    break
                misure.append(misurax)
                coordinate.append([ymin,xmin,xmax])
    if len(misure) is not 0:
        nearest = max(misure)
        pos_coordinate=misure.index(nearest)
        ymin,xmin,xmax=coordinate[pos_coordinate]
        distance = (2220/nearest)
        distance_angolo= (2220/nearest) + 9.5
        pixel = 5/(xmax - xmin)    #5 cm diametro pallina
        pix_lato = -320 + (xmax+xmin)/2
        dist_lato = pixel * pix_lato    #distanza della pallina in cm dal centro della camera
        angolo = np.arctan(dist_lato/distance_angolo)
        distance = dist_lato/(np.sin(angolo))
        ipo = (9.5/(np.cos(angolo))) #ipotenusa robot
        distance = distance - ipo
        angolo=np.degrees(angolo)
        if angolo == 0:
            print('angolo 0')
            distance = distance_angolo #quando viene un angolo esattamente 0 il seno=0 e non calcola la distanza
        print('distanza=   ',distance,'angolo=   ',angolo)#,'pix   ',dist_lato)
    #print(angolo,distance)
    #print('fine lib')
    if angolo !=0 and distance !=0:
        if distance >= int(vis):
            LED.LED('red',True)
            return frame,255,255
        else:
            LED.LED('green',True)
            return frame,angolo, distance
    elif angolo ==0 and distance !=0:
        LED.LED('green',True)
        return frame,angolo, distance
    else:
        LED.LED('red',True)
        return frame,255,255
print('avviata lib')
