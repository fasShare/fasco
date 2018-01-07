FAS_CFLAGS = [
	'-Wall',
    '-std=c++11',
	'-Wno-old-style-cast',
    '-g',
    '-O2',
    '-export-dynamic',
    '-D_GNU_SOURCE',
    '-D_REENTRANT'
    ]

INCLUDE_PATH = ['.', 'mco', 'base', 'tools']
#INCLUDE_PATH = ['.', 'mco', 'base', 'tools', '/usr/include/i386-linux-gnu/c++/6']

mco_source = Glob("./mco/*.cpp") + Glob("./mco/*.S") + Glob("./base/*.cpp")
StaticLibrary('./lib/mco',
        mco_source,
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        CCFLAGS = FAS_CFLAGS
        )

Program('./bin/Main', './test/Main.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['mco', 'glog', 'pthread', 'dl'],
        CCFLAGS = FAS_CFLAGS
       )
Program('./bin/Mco', './test/mco.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['mco', 'glog', 'pthread', 'dl'],
        CCFLAGS = FAS_CFLAGS
       )
Program('./bin/McoInThread', './test/McoInThread.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['mco', 'glog', 'pthread', 'dl'],
        CCFLAGS = FAS_CFLAGS
       )
Program('./bin/HookApi', './test/HookApi.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['mco', 'glog', 'pthread', 'dl'],
        CCFLAGS = FAS_CFLAGS
       )
Program('./bin/Dlsym', './test/Dlsym.cpp',
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        LIBS = ['mco', 'glog', 'pthread', 'dl'],
        CCFLAGS = FAS_CFLAGS
       )
