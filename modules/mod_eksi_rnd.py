#!/usr/bin/env python

import re, urllib2
import random

r_entry = re.compile(r'<li[^>]*>(.+?)<div')
uri = 'http://antik.eksisozluk.com/show.asp?t=%s'

def remove_html_tags(data):
    p = re.compile(r'<.*?>')
    return p.sub('', data)

def mod_eksi_rnd(phenny, input): 
    word = "+".join(input.split()[1:])
    o = urllib2.build_opener()
    o.addheaders.append(('Cookie', 'zero-intro-skipped=1'))
    f = o.open(uri % word)
    bytes = f.read()

    rawEntry = r_entry.findall(bytes)

    if not rawEntry:
        return 'yok oyle bir sey...'

    i = random.randint(0, len(rawEntry) - 1)
    enStr = str (i + 1) + '. ' + remove_html_tags(rawEntry[i])
    return enStr
