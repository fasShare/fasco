FAS_CFLAGS = [
	'-Wall',
    '-static',
    '-std=c++11',
	'-Wno-old-style-cast',
    '-g'
    ]

INCLUDE_PATH = ['.', 'mco']

mco_source = Glob("./mco/*.cpp") + Glob("./mco/*.S")
StaticLibrary('./lib/mco',
        mco_source,
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        CCFLAGS = FAS_CFLAGS
        )

Program('./bin/Mco', './test/Main.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['mco', 'glog', 'pthread'],
        CCFLAGS = FAS_CFLAGS
       )
Program('./bin/McoInThread', './test/McoInThread.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['mco', 'glog', 'pthread'],
        CCFLAGS = FAS_CFLAGS
       )
