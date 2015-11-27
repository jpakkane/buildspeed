project('infodumper', 'cpp')

qt5m = import('qt5')

add_global_arguments('-std=c++11', '-DQT_NO_KEYWORDS', language : 'cpp')

qt5 = dependency('qt5', modules : ['DBus'])
glib = dependency('glib-2.0')
gio = dependency('gio-2.0')

hdrs = qt5m.preprocess(
moc_headers : ['eventprinter.h',
'urfkillroot.h', 'urfkillswitch.h', 
'nmroot.h', 'nmsettings.h', 'nmconnsettings.h', 'nmactiveconnection.h', 'nmwirelessdevice.h',
'nmaccesspoint.h', 'nmdevice.h', 'nmmodem.h',
'ofonoroot.h', 'ofonomodemmodem.h', 'ofonomodemsimmanager.h', 'ofonomodemnetworkregistration.h',
'ofononetworkoperator.h', 'ofonomodemconnectionmanager.h',
'numberadder.h', 'systemstate.h', 'indicatoradaptor.h']
)

executable('i-n-extractor', 'i-n-extractor.cpp', 'eventprinter.cpp',
'urfkillroot.cpp', 'urfkillswitch.cpp',
'nmroot.cpp', 'nmsettings.cpp', 'nmconnsettings.cpp', 'nmactiveconnection.cpp',
'nmwirelessdevice.cpp', 'nmaccesspoint.cpp', 'nmdevice.cpp', 'nmmodem.cpp',
'ofonoroot.cpp', 'ofonomodemmodem.cpp', 'ofonomodemsimmanager.cpp', 'ofonomodemnetworkregistration.cpp',
'ofononetworkoperator.cpp', 'ofonomodemconnectionmanager.cpp',
'numberadder.cpp', 'systemstate.cpp', 'indicatoradaptor.cpp',
'indicatorexporter.cpp',
hdrs,
dependencies : [qt5, gio],
cpp_pch : 'pch/dumper-pch.h')

executable('actiontest', 'actiontest.cpp',
dependencies : [gio, glib])

executable('in-testserver', 'in-testserver.cpp', 'actionprinter.cpp', 'indicatoradaptor.cpp',
qt5m.preprocess(moc_headers: ['actionprinter.h', 'indicatoradaptor.h']),
dependencies : qt5)

executable('menuxporter', 'exporter.cpp', 'menuexporter.cpp',
dependencies : gio)
