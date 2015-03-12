from BeautifulSoup import BeautifulSoup as soup
import requests
import re
from random import randint

def mod_hn(_from, _line):
    if _line == '.hn':
        return top_ten()
    else:
        if _line.startswith('.hn '):
            try:
                return get_th(int(re.findall(r'\d+', _line)[0]))
            except:
                return get_th(randint(1, 30))
        else:
            return 'lol'

def is_command():
    return True

def top_ten():
    httpreq = requests.get('https://news.ycombinator.com')
    dom = soup(httpreq.text)

    outer = dom.find("table")
    inner = outer.findAll("table")[1]

    rowlist = inner.findAll("tr")
    list = []
    for row in range(0,len(rowlist)-3,3):
        rowmain = rowlist[row].findAll("td")
        rowsub = rowlist[row+1].findAll("td")

        listitem = {"link": rowmain[2].find("a")["href"], "title": rowmain[2].find("a").string, "domain": rowmain[2].findAll("span")[1].string}
        try:
            listitem["poster"] = rowsub[1].findAll('a')[0].string
            listitem["posted"] = rowsub[1].findAll('a')[1].string
            listitem["comment"] = re.findall(r'\d+', rowsub[1].findAll('a')[2].string)[0]
        except:
            continue

        list.append(listitem)



    response = "HackerNews Top 10\n"
    for i in range(0,10):
        response += '['+str(i+1)+'] ' + list[i]['title'] + list[i]['domain'] + ' ('+list[i]["posted"]+' | ' +list[i]["comment"] +' comment)' + '\n'

    return response

def get_th(_num):
    num = 1
    if _num - 1 in range(0, 29):
        num = _num - 1
    else:
        num = randint(0,29)

    httpreq = requests.get('https://news.ycombinator.com')
    dom = soup(httpreq.text)

    outer = dom.find("table")
    inner = outer.findAll("table")[1]

    row = inner.findAll("tr")

    rowmain = row[(num*3)-3].findAll("td")
    rowsub = row[(num*3)-2].findAll("td")
    returnitem = {"link": rowmain[2].find("a")["href"], "title": rowmain[2].find("a").string, "domain": rowmain[2].findAll("span")[1].string}
    try:
        returnitem["poster"] = rowsub[1].findAll('a')[0].string
        returnitem["posted"] = rowsub[1].findAll('a')[1].string
        returnitem["comment"] = re.findall(r'\d+', rowsub[1].findAll('a')[2].string)[0]
        returnitem["posttype"] = 's'
    except:
        returnitem["posttype"] = 'j'

    response = '[->] ' + returnitem['title'] + returnitem['domain']
    if returnitem["posttype"] == 'j':
        response += ' (Job) \n'
    else:
        response += ' ('+returnitem["posted"]+' | ' +returnitem["comment"] +' comment)' + '\n'

    response += returnitem["link"]
    return response

def main():
  print mod_hn("#python", ".hn 7")

if __name__ == '__main__':
    main()
