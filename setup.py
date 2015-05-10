from distutils.core import setup, Extension

module1 = Extension('spam', sources=['spammodule.c'])

setup(name='SpamModule',
	version='1.0',
	description='This is a spam module',
	ext_modules=[module1])

