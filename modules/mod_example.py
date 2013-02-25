def mod_example(_from, _line):
  mod_example.counter += 1
  return "%d" % mod_example.counter
mod_example.counter = 0

def main():
  mod_example("#python", ".foo 42")

if __name__ == '__main__':
   main()


