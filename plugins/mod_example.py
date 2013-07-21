
def mod_example(_from, _line):
  return "PRIVMSG " + _from + " :Hello!" 

def main():
  print mod_example("#python", ".foo 42")

if __name__ == '__main__':
   main()


