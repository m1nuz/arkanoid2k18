#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import urllib.request
from zipfile import ZipFile

assets_directory = './assets'
url = 'https://www.dropbox.com/sh/exjr53qaz7q0c7q/AACzxZMA8oRpDpX0q2u0PfAma?dl=1'

if not os.path.exists(assets_directory):
    file_name = 'assets.zip'
    urllib.request.urlretrieve(url, file_name)
    with ZipFile(file_name, 'r') as zip:
        zip.printdir()

        print('Extracting all the files now...')
        zip.extractall('./assets')
        print('Done!')
