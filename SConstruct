env = Environment()
env['CPPPATH'] = ['.']
env.ParseConfig("pkg-config glib-2.0 gio-2.0 jansson --cflags --libs")
env.ParseConfig("python-config --cflags")
env.ParseConfig("python-config --libs")
env.ParseConfig("curl-config --cflags --libs")
sources = Glob('*.c') + Glob('modules/*.c')
env.Program('ircclient', sources)
