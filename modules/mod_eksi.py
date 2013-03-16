import re, urllib2, nltk

def mod_eksi(_from, _line):
    e = "+".join(_line.split()[1:])
    o = urllib2.build_opener()
    o.addheaders.append(('Cookie', 'zero-intro-skipped=1'))
    f = o.open("http://www.eksisozluk.com/?q=" + e)
    m = re.search('content">(.*?)<', f.read())
    return nltk.clean_html(m.group(1))
    

