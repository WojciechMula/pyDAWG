from distutils.core import setup, Extension

module1 = Extension(
	'pydawg',
	sources = ['pydawg.c'],
	define_macros = [
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
	name = 'DAWG',
	ext_modules = [module1]
)
