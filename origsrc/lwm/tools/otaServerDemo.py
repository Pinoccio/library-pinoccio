#!/usr/bin/python

import optparse
import serial
import time
import sys
import os

OTA_MAX_BLOCK_SIZE          = 90

UART_COMMAND_COMM_CHECK     = 0x01
UART_COMMAND_START_REQUEST  = 0x02
UART_COMMAND_BLOCK_REQUEST  = 0x03
UART_COMMAND_NOTIFICATION   = 0x04

d_command = {
  0x01: 'UART_COMMAND_COMM_CHECK',
  0x02: 'UART_COMMAND_START_REQUEST',
  0x03: 'UART_COMMAND_BLOCK_REQUEST',
  0x04: 'UART_COMMAND_NOTIFICATION',
}

d_status = {
  0x00: 'OTA_SUCCESS_STATUS',
  0x01: 'OTA_CLIENT_READY_STATUS',
  0x02: 'OTA_NETWORK_ERROR_STATUS',
  0x03: 'OTA_CRC_ERROR_STATUS',
  0x04: 'OTA_NO_RESPONSE_STATUS',
  0x05: 'OTA_SESSION_TIMEOUT_STATUS',
  0x06: 'OTA_UPGRADE_STARTED_STATUS',
  0x07: 'OTA_UPGRADE_COMPLETED_STATUS',
  0x10: 'OTA_NO_SPACE_STATUS',
  0x11: 'OTA_HW_FAIL_STATUS',
  0x80: 'APP_UART_STATUS_CONFIRMATION',
  0x81: 'APP_UART_STATUS_PASSED',
  0x82: 'APP_UART_STATUS_UPGRADE_IN_PROGRESS',
  0x83: 'APP_UART_STATUS_NO_UPGRADE_IN_PROGRESS',
  0x84: 'APP_UART_STATUS_UNKNOWN_COMMAND',
  0x85: 'APP_UART_STATUS_MALFORMED_REQUEST',
  0x86: 'APP_UART_STATUS_MALFORMED_COMMAND',
  0x87: 'APP_UART_STATUS_SESSION_TIMEOUT',
}

#
#
#
def error(msg):
  print 'Error: %s' % msg
  sys.exit(1)

#
#
#
def uint16(num):
  return [(num >> 0) & 0xff, (num >> 8) & 0xff]

#
#
#
def uint32(num):
  return [(num >> 0) & 0xff, (num >> 8) & 0xff, (num >> 16) & 0xff, (num >> 24) & 0xff]

#
#
#
def send(port, data, debug):
  cs = (0x1234 + sum(data)) & 0xffff;
  frame = [0x55, len(data)] + data + [(cs >> 0) & 0xff, (cs >> 8) & 0xff, 0xaa]
  port.write(''.join([chr(x) for x in frame]))
  if debug:
    print '-> %s' % d_command[data[0]]

#
#
#
def recv(port, debug, expect = None):
  data = []
  timeout = 0
  while len(data) < 7:
    byte = port.read(1)
    if byte:
      data += [byte]
      timeout = 0
    else:
      timeout += 1
    if timeout > 10:
      error('response timeout')

  data = [ord(d) for d in data]

  if data[0] == 0x55 and data[1] == 2 and data[2] == UART_COMMAND_NOTIFICATION and data[6] == 0xaa:
    status = data[3]
    if expect is not None:
      if status in d_status:
        if d_status[status] != expect:
          error('unexpected status (%s)' % d_status[status])
      else:
        error('unexpected status (UNKNOWN_0x%02x)' % status)
    if debug:
      print '<- %s' % d_status[status]
    return status
  else:
    error('invalid response')

#
#
#
def commCheck(options, port):
  data = [UART_COMMAND_COMM_CHECK]
  send(port, data, options.debug)
  recv(port, options.debug, expect = 'APP_UART_STATUS_CONFIRMATION')
  recv(port, options.debug, expect = 'APP_UART_STATUS_PASSED')
  if options.debug:
    print ' * Comm check passed'

#
#
#
def startUpgrade(options, port):
  data = [UART_COMMAND_START_REQUEST] + uint16(options.addr) + uint16(options.panid) + \
         [options.channel] + uint16(options.client) + uint32(options.size)
  send(port, data, options.debug)
  recv(port, options.debug, expect = 'APP_UART_STATUS_CONFIRMATION')
  recv(port, options.debug, expect = 'OTA_CLIENT_READY_STATUS')
  if options.debug:
    print ' * Upgrade started'

  f = open(options.name, 'rb')

  remaining = options.size
  while remaining > 0:
    if remaining > OTA_MAX_BLOCK_SIZE:
      size = OTA_MAX_BLOCK_SIZE
    else:
      size = remaining

    remaining -= size

    block = f.read(size)

    data = [UART_COMMAND_BLOCK_REQUEST] + [ord(b) for b in block]
    send(port, data, options.debug)
    recv(port, options.debug, expect = 'APP_UART_STATUS_CONFIRMATION')
    if remaining > 0:
      recv(port, options.debug, expect = 'OTA_CLIENT_READY_STATUS')
    else:
      recv(port, options.debug, expect = 'OTA_UPGRADE_COMPLETED_STATUS')

    if options.debug:
      print ' * Block sent'
    else:
      sys.stdout.write('.')
      sys.stdout.flush()

  if options.debug:
    print ' * File sent'

#
#
#
def checkOptions(options):
  def evalOption(opt, name):
    try:
      return eval(opt)
    except:
      error('invalid %s (%s)' % (name, options.baud))

  options.baud = evalOption(options.baud, 'baudrate')
  options.panid = evalOption(options.panid, 'PAN ID')
  options.channel = evalOption(options.channel, 'channel')
  options.addr = evalOption(options.addr, 'server address')
  options.client = evalOption(options.client, 'client address')

#
#
#
def main():
  parser = optparse.OptionParser(usage = 'usage: %prog [options] file')
  parser.add_option('-d', '--debug',   dest = 'debug',   help = 'enable debug output', default = False, action = 'store_true')
  parser.add_option('-p', '--port',    dest = 'port',    help = 'communication port [default COM1]', default = 'COM1', metavar = 'PORT')
  parser.add_option('-b', '--baud',    dest = 'baud',    help = 'communication baudrate [default 38400]', default = '38400', metavar = 'BAUD')
  parser.add_option('-m', '--comm',    dest = 'comm',    help = 'perform only server communication check', default = False, action = 'store_true')
  parser.add_option('-i', '--panid',   dest = 'panid',   help = 'PAN ID [default 0x1234]', default = '0x1234', metavar = 'PANID')
  parser.add_option('-c', '--channel', dest = 'channel', help = 'channel [default 15]', default = '15', metavar = 'CHANNEL')
  parser.add_option('-a', '--addr',    dest = 'addr',    help = 'server network address [default 0x8555]', default = '0x8555', metavar = 'ADDR')
  parser.add_option('-t', '--client',  dest = 'client',  help = 'client network address [default 0x0000]', default = '0x0000', metavar = 'CLIENT')
  (options, args) = parser.parse_args()

  if len(args) != 1 and options.comm == False:
    error('invalid image file name')

  options.name = args[0]
  try:
    options.size = os.path.getsize(options.name)
  except OSError, msg:
    error(msg)

  checkOptions(options)

  if options.debug:
    print 'Port         : %s' % options.port
    print 'Baudrate     : %d' % options.baud
    print 'Server       : 0x%04x' % options.addr
    print 'Client       : 0x%04x' % options.client
    print 'PAN ID       : 0x%04x' % options.panid
    print 'Channel      : %d (0x%02x)' % (options.channel, options.channel)
    print 'Firmware     : %s' % options.name
    print 'Firmware size: %d bytes' % options.size

  try:
    port = serial.Serial(options.port, options.baud, timeout = 1)
  except serial.serialutil.SerialException, msg:
    error(msg)

  commCheck(options, port)

  if options.comm == False:
    startUpgrade(options, port)

  port.close()

#
#
#
main()
