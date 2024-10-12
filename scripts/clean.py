import shutil
import os

if os.path.exists('.gradle'):
	shutil.rmtree('.gradle')

if os.path.exists('app/.cxx'):
	shutil.rmtree('app/.cxx')

if os.path.exists('app/build'):
	shutil.rmtree('app/build')

print('cleaned project')