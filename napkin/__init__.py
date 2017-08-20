import sys
import os

# add nap python module library path
# (we're not installing to user's default python path for now)
pynap_lib_path = '%s/lib' % os.path.dirname(__file__)
if not pynap_lib_path in sys.path:
    sys.path.append(pynap_lib_path)