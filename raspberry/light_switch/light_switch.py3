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
#from IPython import embed
GPIO.setmode(GPIO.BCM)

OPENHAB_HOST = "192.168.2.2"
OPENHAB_PORT =  8080
OPENHAB_URL  = "http://%s:%s/" % (OPENHAB_HOST, OPENHAB_PORT)

#BUTTONS_PINS     = [ 4, 17, 27, 18 ] # 22, 23
BUTTONS =      {  4: 'Virtual_Switch_1',
                 17: 'Virtual_Switch_2',
                 18: 'Virtual_Switch_3',
                 27: 'Virtual_Switch_4' } # { GPIO pin : openhab_item_name }
SWITCHES     = { 24: 'Virtual_Switch_5' }
DOOR_SENSORS = { 22: 'Door_Main' }

VIRTUAL_SWITCH_PIN  = 24 # переделать VIRTUAL_SWITCH тоже в хэш как и кнопки и двери
VIRTUAL_SWITCH_NAME = 'Virtual_Switch_5'

BOUNCETIME = 1500

def all_keys():
  return BUTTONS.keys() + SWITCHES.keys() + DOOR_SENSORS.keys()

def item_name_by(pin_number):
  #print "channel = %s" % pin_number
  if pin_number in BUTTONS:
    name = BUTTONS[pin_number]
  if pin_number in SWITCHES:
    name = SWITCHES[pin_number]
  if pin_number in DOOR_SENSORS:
    name = DOOR_SENSORS[pin_number]
  return name

def send_http_toggle_to_openhab(pin_number):
  #print "mock -- request pin %s" % pin_number
  url = OPENHAB_URL + "CMD?%s=TOGGLE" % item_name_by(pin_number)
  r = requests.get(url)
  log_request(r.status_code, url)

def change_door_state(pin_number):
  #print "mock -- request pin %s" % pin_number
  url = OPENHAB_URL +"rest/items/%s/state" % item_name_by(pin_number)
  headers = {'content-type': 'text/plain'}
  value = GPIO.input(pin_number)
  state = 'OPEN' if value else 'CLOSED'
  r = requests.put(url, data=state)
  log_request(r.status_code, url)

def door_open(pin_number):
  change_door_state(pin_number, 'OPEN')

def door_closed(pin_number):
  change_door_state(pin_number, 'CLOSED')

def setup_pin(pin_number):
  GPIO.setup(pin_number, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def add_callback(pin_number, method=send_http_toggle_to_openhab, event=GPIO.FALLING, bounce=BOUNCETIME):
  GPIO.add_event_detect(pin_number, event, callback=method, bouncetime=bounce)

def log_request(status_code, url):
  if status_code != requests.codes.ok:
    logging.error( 'Request to %s failed with status %s' %  (url, status_code ) )
  else:
    logging.info( 'Successful request to %s' %  url )

for pin_number in all_keys():
  setup_pin(pin_number)

for pin_number in BUTTONS.keys():
  add_callback(pin_number)

for pin_number in DOOR_SENSORS.keys():
  add_callback(pin_number, method=change_door_state,   event=GPIO.BOTH, bounce=300)

for pin_number in SWITCHES.keys():
  add_callback(pin_number, event=GPIO.BOTH)

FILENAME = '/var/log/light_switch.log'
FORMAT   = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
logging.basicConfig(filename=FILENAME, format=FORMAT, level=logging.INFO)

try:
  while True:
    pass
    #embed()
except KeyboardInterrupt:
  print('Program terminated.')
finally:
  GPIO.cleanup()
  logging.shutdown()