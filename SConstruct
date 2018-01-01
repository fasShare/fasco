FAS_CFLAGS = [
	'-Wall',
    '-fPIC',
    '-std=c++11',
	'-Wno-old-style-cast',
    '-g',
    '-D_GNU_SOURCE',
    '-D_REENTRANT',
    '-export-dynamic'
    ]

INCLUDE_PATH = ['.', 'mco']

mco_source = Glob("./mco/*.cpp") + Glob("./mco/*.S")
StaticLibrary('./lib/mco',
#SharedLibrary('./lib/mco',
        mco_source,
        LIBPATH = ['lib'],
        CPPPATH = INCLUDE_PATH,
        CCFLAGS = FAS_CFLAGS
        )

Program('./bin/Mco', './test/Main.cpp',
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
