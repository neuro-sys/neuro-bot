import re, sys, urllib, urllib2, nltk

def mod_eksi(_from, _line):
    entry = _line.split()[1]
    opener = urllib2.build_opener()
    opener.addheaders.append(('Cookie', 'zero-intro-skipped=1'))
    f = opener.open("http://www.eksisozluk.com/?q=" + entry)
    m = re.search('content">(.*?)<', f.read())
    return nltk.clean_html(m.group(1))


