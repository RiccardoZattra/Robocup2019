import logging as log

#from Libraries import log

logFileline = 'LOGfile/file-log-linea.log'
logFileroom = 'LOGfile/file-log-stanza.log'

def lineLOG(typemsg,text):
    log.basicConfig(filename=logFileline,format='%(levelname)s %(asctime)s: %(message)s',level=log.DEBUG)
    if typemsg == 'warning' or typemsg == 'w':
        log.warning(text)
    elif typemsg == 'info' or typemsg == 'i':
        log.info(text)
    elif typemsg == 'error' or typemsg == 'e':
        log.error(text)
    elif typemsg == 'critical' or typemsg == 'c':
        log.critical(text)

def stanzaLOG(typemsg,text):
    log.basicConfig(filename=logFileroom,format='%(levelname)s %(asctime)s: %(message)s',level=log.DEBUG)
    if typemsg == 'warning' or typemsg == 'w':
        log.warning(text)
    elif typemsg == 'info' or typemsg == 'i':
        log.info(text)
    elif typemsg == 'error' or typemsg == 'e':
        log.error(text)
    elif typemsg == 'critical' or typemsg == 'c':
        log.critical(text)

#EXAMPLES:   
#lineLOG('c','ciao')
#stanzaLOG('w','ok')
