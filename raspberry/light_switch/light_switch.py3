#!/usr/bin/python
# -*- coding: utf-8 -*-

# pip install requests

# for debugging:
#for pin in BUTTONS_PINS:
#  GPIO.input(pin)

import logging
import time
import requests
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)

OPENHAB_HOST = "192.168.2.2"
OPENHAB_PORT =  8080
OPENHAB_URL  = "http://%s:%s/" % (OPENHAB_HOST, OPENHAB_PORT)

#BUTTONS_PINS     = [ 4, 17, 27, 18 ] # 22, 23
BUTTONS = {4: 'Light_1', 17: 'Light_2', 27: 'Light_3', 18: 'Light_4'} # { GPIO pin : openhab_item_name }
SMART_SWITCH_PIN  = 24
SMART_SWITCH_NAME = 'Smart_Switch_1'

BOUNCETIME        = 1500

def item_name_by(pin_number):
  #print "channel = %s" % pin_number
  return BUTTONS[pin_number] if pin_number != SMART_SWITCH_PIN else SMART_SWITCH_NAME

def send_http_request_to_openhab(pin_number):
  #print "mock -- request pin %s" % pin_number
  url = OPENHAB_URL + "CMD?%s=TOGGLE" % item_name_by(pin_number)
  r = requests.get(url)
  if r.status_code != requests.codes.ok:
    logging.error( 'Request to %s failed with status %s' %  (url, r.status_code ) )
  else:
    logging.info( 'Successful request to %s' %  url )

def setup_pin(pin_number):
  GPIO.setup(pin_number, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def add_callback(pin_number, method=send_http_request_to_openhab, event=GPIO.FALLING):
  GPIO.add_event_detect(pin_number, event, callback=method, bouncetime=BOUNCETIME)

for pin_number in (BUTTONS.keys() + [SMART_SWITCH_PIN]):
  setup_pin(pin_number)

for pin_number in BUTTONS.keys():
  add_callback(pin_number)

add_callback(SMART_SWITCH_PIN, event=GPIO.BOTH)

FILENAME = '/var/log/light_switch.log'
FORMAT   = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
logging.basicConfig(filename=FILENAME, format=FORMAT, level=logging.INFO)

try:
  while True:
    pass
except KeyboardInterrupt:
  print('Program terminated.')
finally:
  GPIO.cleanup()
  logging.shutdown()