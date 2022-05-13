from typing import List, Tuple, Dict


def split(s, category):
    _list = []
    lines = [line for line in s.splitlines() if line]
    for line in lines:
        xs = line.split()
        _list.append((category, xs[0], xs[1:]))
    return _list


class Module:
    def __init__(self, name: str, mode: Tuple[int, int, int, int],
                 vers: Dict[str, int], elems: List[str]):
        self.name = name
        self.mode = mode
        self.vers = vers
        self.elems = elems

    def __repr__(self):
        return f"<{self.__class__.__name__} '{self.name}'>"

    def __str__(self):
        return " ".join([self.name] + self.elems)

    def as_tuple(self):
        m = self
        return (m.name,) + m.mode + tuple(m.vers.values())

# object name          Module definition           0=disabled, 1=static, 2=shared
posix                = Module('posix',             mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'posixmodule.c'])
errno                = Module('errno',             mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['errnomodule.c'])
pwd                  = Module('pwd',               mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['pwdmodule.c'])
_sre                 = Module('_sre',              mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_sre.c'])
_codecs              = Module('_codecs',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_codecsmodule.c'])
_weakref             = Module('_weakref',          mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_weakref.c'])
_functools           = Module('_functools',        mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_functoolsmodule.c'])
_operator            = Module('_operator',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_operator.c'])
_collections         = Module('_collections',      mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_collectionsmodule.c'])
_abc                 = Module('_abc',              mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_abc.c'])
itertools            = Module('itertools',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['itertoolsmodule.c'])
atexit               = Module('atexit',            mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['atexitmodule.c'])
_signal              = Module('_signal',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'signalmodule.c'])
_stat                = Module('_stat',             mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_stat.c'])
time                 = Module('time',              mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', 'timemodule.c'])
_thread              = Module('_thread',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '_threadmodule.c'])
_locale              = Module('_locale',           mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_localemodule.c'])
_io                  = Module('_io',               mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '-I$(srcdir)/Include/internal', '-I$(srcdir)/Modules/_io', '_io/_iomodule.c', '_io/iobase.c', '_io/fileio.c', '_io/bytesio.c', '_io/bufferedio.c', '_io/textio.c', '_io/stringio.c'])
faulthandler         = Module('faulthandler',      mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['faulthandler.c'])
zipimport            = Module('zipimport',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 0, '3.9': 0, '3.10': 0}, elems=['-DPy_BUILD_CORE', 'zipimport.c'])
_tracemalloc         = Module('_tracemalloc',      mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_tracemalloc.c'])
_peg_parser          = Module('_peg_parser',       mode=(1, 1, 1, 1), vers={'3.7': 0, '3.8': 0, '3.9': 1, '3.10': 1}, elems=['_peg_parser.c'])
_symtable            = Module('_symtable',         mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['symtablemodule.c'])
readline             = Module('readline',          mode=(1, 1, 1, 1), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['readline.c', '-lreadline', '-ltermcap'])
array                = Module('array',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', 'arraymodule.c'])
cmath                = Module('cmath',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cmathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE'])
math                 = Module('math',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['mathmodule.c', '_math.c', '-DPy_BUILD_CORE_MODULE'])
_contextvars         = Module('_contextvars',      mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_contextvarsmodule.c'])
_struct              = Module('_struct',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', '_struct.c'])
_weakref             = Module('_weakref',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_weakref.c'])
_testcapi            = Module('_testcapi',         mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_testcapimodule.c'])
_testinternalcapi    = Module('_testinternalcapi', mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_testinternalcapi.c', '-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_MODULE'])
_random              = Module('_random',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_randommodule.c', '-DPy_BUILD_CORE_MODULE'])
_elementtree         = Module('_elementtree',      mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '_elementtree.c'])
_pickle              = Module('_pickle',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_MODULE', '_pickle.c'])
_datetime            = Module('_datetime',         mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_datetimemodule.c'])
_zoneinfo            = Module('_zoneinfo',         mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 0, '3.9': 1, '3.10': 1}, elems=['_zoneinfo.c', '-DPy_BUILD_CORE_MODULE'])
_bisect              = Module('_bisect',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_bisectmodule.c'])
_heapq               = Module('_heapq',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_heapqmodule.c', '-DPy_BUILD_CORE_MODULE'])
_asyncio             = Module('_asyncio',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_asynciomodule.c'])
_json                = Module('_json',             mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-I$(srcdir)/Include/internal', '-DPy_BUILD_CORE_BUILTIN', '_json.c'])
_statistics          = Module('_statistics',       mode=(0, 0, 0, 0), vers={'3.7': 0, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_statisticsmodule.c'])
unicodedata          = Module('unicodedata',       mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['unicodedata.c', '-DPy_BUILD_CORE_BUILTIN'])
_uuid                = Module('_uuid',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_uuidmodule.c'])
_opcode              = Module('_opcode',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_opcode.c'])
_multiprocessing     = Module('_multiprocessing',  mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_multiprocessing/multiprocessing.c', '_multiprocessing/semaphore.c'])
_posixshmem          = Module('_posixshmem',       mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_multiprocessing/posixshmem.c'])
fcntl                = Module('fcntl',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['fcntlmodule.c'])
spwd                 = Module('spwd',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['spwdmodule.c'])
grp                  = Module('grp',               mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['grpmodule.c'])
select               = Module('select',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['selectmodule.c'])
mmap                 = Module('mmap',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['mmapmodule.c'])
_csv                 = Module('_csv',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_csv.c'])
_socket              = Module('_socket',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['socketmodule.c'])
_ssl_shared          = Module('_ssl',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto'])
_hashlib_shared      = Module('_hashlib',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-lssl', '-lcrypto'])
_ssl                 = Module('_ssl',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_ssl.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libssl.a', '-Wl,--exclude-libs,libssl.a', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a'])
_hashlib             = Module('_hashlib',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_hashopenssl.c.c', '-I$(OPENSSL)/include', '-L$(OPENSSL)/lib', '-l:libcrypto.a', '-Wl,--exclude-libs,libcrypto.a'])
_crypt               = Module('_crypt',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_cryptmodule.c', '-lcrypt'])
nis                  = Module('nis',               mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['nismodule.c', '-lnsl'])
termios              = Module('termios',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['termios.c'])
resource             = Module('resource',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['resource.c'])
_posixsubprocess     = Module('_posixsubprocess',  mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['-DPy_BUILD_CORE_BUILTIN', '_posixsubprocess.c'])
audioop              = Module('audioop',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['audioop.c'])
_md5                 = Module('_md5',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['md5module.c'])
_sha1                = Module('_sha1',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha1module.c'])
_sha256              = Module('_sha256',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha256module.c', '-DPy_BUILD_CORE_BUILTIN'])
_sha512              = Module('_sha512',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['sha512module.c', '-DPy_BUILD_CORE_BUILTIN'])
_sha3                = Module('_sha3',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_sha3/sha3module.c'])
_blake2              = Module('_blake2',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_blake2/blake2module.c', '_blake2/blake2b_impl.c', '_blake2/blake2s_impl.c'])
syslog               = Module('syslog',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['syslogmodule.c'])
_curses              = Module('_curses',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_cursesmodule.c', '-lcurses', '-ltermcap', '-DPy_BUILD_CORE_MODULE'])
_curses_panel        = Module('_curses_panel',     mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_curses_panel.c', '-lpanel', '-lncurses'])
_dbm                 = Module('_dbm',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_dbmmodule.c', '-lndbm'])
_gdbm                = Module('_gdbm',             mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_gdbmmodule.c', '-I/usr/local/include', '-L/usr/local/lib', '-lgdbm'])
binascii             = Module('binascii',          mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['binascii.c'])
parser               = Module('parser',            mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 0}, elems=['parser.c'])
_lsprof              = Module('_lsprof',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['_lsprof.o', 'rotatingtree.c'])
zlib                 = Module('zlib',              mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['zlibmodule.c', '-I$(prefix)/include', '-lz'])
pyexpat              = Module('pyexpat',           mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['expat/xmlparse.c', 'expat/xmlrole.c', 'expat/xmltok.c', 'pyexpat.c', '-I$(srcdir)/Modules/expat', '-DHAVE_EXPAT_CONFIG_H', '-DUSE_PYEXPAT_CAPI', '-DXML_DEV_URANDOM'])
_multibytecodec      = Module('_multibytecodec',   mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/multibytecodec.c'])
_codecs_cn           = Module('_codecs_cn',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_cn.c'])
_codecs_hk           = Module('_codecs_hk',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_hk.c'])
_codecs_iso2022      = Module('_codecs_iso2022',   mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_iso2022.c'])
_codecs_jp           = Module('_codecs_jp',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_jp.c'])
_codecs_kr           = Module('_codecs_kr',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_kr.c'])
_codecs_tw           = Module('_codecs_tw',        mode=(0, 0, 0, 0), vers={'3.7': 1, '3.8': 1, '3.9': 1, '3.10': 1}, elems=['cjkcodecs/_codecs_tw.c'])

core = [
    posix,
    errno,
    pwd,
    _sre,
    _codecs,
    _weakref,
    _functools,
    _operator,
    _collections,
    _abc,
    itertools,
    atexit,
    _signal,
    _stat,
    time,
    _thread,
    _locale,
    _io,
    faulthandler,
    zipimport,
    _tracemalloc,
    _peg_parser,
    _symtable,
]
for m in core:
    print(m)