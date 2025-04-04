#1- sudo raspi-config
#2- Advanced Options -> Enter
#3- Audio -> Enter
#4- "Force 3.5mm ('headphone') jack"
'''
import pyglet

music = pyglet.resource.media("Users\Matteo\Music\Imagine.Dragons.Thunder.mp3")
music.play()

pyglet.app.run()
'''
import os

file = "file.mp3"
os.system("mpg123 " + file)

