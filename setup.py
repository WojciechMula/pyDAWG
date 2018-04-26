# -*- coding: utf-8 -*-
from distutils.core import setup, Extension

def get_readme():
    with open('README.rst', 'rt') as f:
        body = f.read()

        marker1 = '.. image'
        marker2 = '.. contents'
        
        s = body.index(marker1)
        e = body.index(marker2)

        body = body[:s] + body[e:]

        return body


module = Extension(
	'pydawg',
	sources = ['pydawg.c'],
	define_macros = [
		('DEBUG', 1),	# define debug mode
		('DAWG_PERFECT_HASHING', ''),	# enable perfect hashing
		('DAWG_UNICODE', ''),			# use unicode
	],
	depends = [
		'DAWG_class.c', 'DAWG_class.h',
		'DAWGIterator_class.c', 'DAWGIterator_class.h',
		'dawg.c', 'dawg.h', 'dawg_pickle.c', 'dawg_mph.c',
		'dawgnode.c', 'dawgcode.h',
		'slist.h', 'slist.c',
		'utils.c',
	]
)

setup(
	name                = 'pyDAWG',
    version             = '1.0.1',
	ext_modules         = [module],

    description         = "Directed Acyclic Word Graph (DAWG) allows to store huge strings set in compacted form",
    author              = "Wojciech Muła",
    author_email        = "wojciech_mula@poczta.onet.pl",
    maintainer          = "Wojciech Muła",
    maintainer_email    = "wojciech_mula@poczta.onet.pl",
    url                 = "http://github.com/WojciechMula/pyDAWG",
    platforms           = ["Linux", "Windows"],
    license             = "BSD (3 clauses)",
    long_description    = get_readme(),
    keywords            = [
        "dawg",
        "graph",
        "dictionary",
    ],
    classifiers      = [
        "Development Status :: 5 - Production/Stable",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: C",
        "Topic :: Software Development :: Libraries",
        "Topic :: Text Editors :: Text Processing",
    ],
)
