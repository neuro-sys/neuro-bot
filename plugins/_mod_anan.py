import random

def mod_anan(_from, _line):
    if "http" in _line:
        return
    if len(_line)<=3:
        return
    ananlar=[]
    kelimeler = _line.split(" ")
    if len(kelimeler)>=4:
        nlpstr = " ,".join(random.sample(kelimeler,3))
        ananlar += ["Senin icin googleda %s kelimeleriyle arama yaptim, gelen sonuc: anan bilir" % nlpstr, "hmm, %s.... yanlis hatirlamiyorsam yakin zamanlarda anan da buna benzer birseyler sormustu"% nlpstr,"loglara baktim senin icin, %s diye aradim, %s'in anasi bilir diye not dusulmus... malesef." % (nlpstr, _from)," ya %s gibisinden soru mu olur allasen anan bile boyle sormuyordu "% nlpstr]
    else:
        ananlar += ["bu soruyu anana sorsana","anan bilir","anandan iyi kim bilecek","ben bilmem anana sor","en son anan cevaplamisti","anan da zamaninda oyle sorardi","sen en iyisi anana sor qanqa","bundan 20 yil once bir garsoniyerde anan da o soruyu sormustu","fazla soru sorma anan da cok sorardi","bilen bilir bilen bilir, en iyisi anan bilir","yedi cihana sorduk herkes "+_from+"in anasi bilir dedi","CEVAP: (an)x2+a sor", "pazarda anana da oyle sormuslardi"]
    return "%s: %s" % (_from,random.choice(ananlar))

def is_command():
    return True
def main():
    print "zaa xd"

if __name__ == '__main__':
    main()
