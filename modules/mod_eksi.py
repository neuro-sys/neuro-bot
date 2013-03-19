import re, urllib2, nltk, sys

def mod_eksi(_from, _line):
    e = "+".join(_line.split()[1:])
    o = urllib2.build_opener()
    o.addheaders.append(('Cookie', 'zero-intro-skipped=1'))
    try: f = o.open("http://www.eksisozluk.com/?q=" + e) 
    except: return "Yok boyle bisi"
    m = re.search('content">(.*?)<', f.read().replace("\\n", ""))
    return nltk.clean_html(m.group(1))
    
if __name__ == "__main__":
    print(mod_eksi("", sys.argv[1]))

