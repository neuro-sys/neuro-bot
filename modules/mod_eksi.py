#!/usr/bin/env python2
# -*- coding: utf8 -*-

# provided by decaf

from urlgrabber import urlread
from bs4 import BeautifulSoup


# sonuç boyutu [byte]
LIMIT = 509

def mod_eksi(_from, _line):
    return daModule(_line.replace('.eksi ',''))

def daModule(line):
    url = buildUrl(line)
    try:
        st = urlread(url)
    except:
        return '[' +line+ '] yok böyle bi şii'
    return parsit(st) 

def buildUrl(line):
    line = " ".join(line.split(' '))
    return "http://eksisozluk.com/" + line

def parsit(st):
    def entry_isle(entry):
        for br in entry.find_all('br'):
            br.insert_before(' ')
        return entry.text.replace('*','')
    
    def icindenkes(sondata):
        lst = sondata.split(' | ')
        lst.pop(-2)
        return ' | '.join(lst)

    soup = BeautifulSoup(st)
    tepelink = soup.find(id='title').a
    baslik = '['+tepelink.text+'] |'
    href = "http://eksisozluk.com"+tepelink.get('href')
    entries = soup.find_all("div", attrs={"class": "content"})
    dt = [entry_isle(entry) for entry in entries]
    netice = " | ".join(dt)
    netice = (baslik +' '+ netice).encode('utf-8') +' | '+ href
    while(len(netice) > LIMIT):
        netice = icindenkes(netice)
    return netice    
