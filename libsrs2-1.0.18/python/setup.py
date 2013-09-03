#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name="libsrs2",
	version="1.0.11",
	description="Sender Rewriting Scheme",
	author="Shevek",
	author_email="srs@anarres.org",
	url="http://www.libsrs2.org/",
	ext_modules=[Extension("libsrs2", ["libsrs2.c"],
		library_dirs=["../libsrs2"],
		libraries=["srs2"])])
