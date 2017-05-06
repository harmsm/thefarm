#!/usr/bin/env python3

import sys, os

# Try using setuptools first, if it's installed
from setuptools import setup, find_packages

# Need to add all dependencies to setup as we go!
setup(name='thefarm',
      packages=find_packages(),
      version='0.1',
      description='software for controlling a smart farm',
      author='Michael J. Harms',
      author_email='harmsm@gmail.com',
      url='https://github.com/harmsm/thefarm',
      download_url='https://XX',
      zip_safe=False,
      install_requires=[],
      classifiers=[])

