from distutils.core import setup, Extension

module1 = Extension(
	'pydawg',
	sources = ['pydawg.c'],
	depends = [
		'DAWG_class.c', 'DAWG_class.h',
		'DAWGIterator_class.c', 'DAWGIterator_class.h',
		'dawg.c', 'dawg.h',
		'dawgnode.c', 'dawgcode.h',
		'slist.h', 'slist.c',
		'utils.c',
	]
)

setup(
	name = 'DAWG',
	ext_modules = [module1]
)
