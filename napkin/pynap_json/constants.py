import platform
import sys

UNKNOWN_TYPE = '<<UNKNOWN>>'
PROP_COMPONENTS = 'Components'
PROP_CHILDREN = 'Children'

FILENAME_FILTER_EXE = 'Executable (*.exe)' if any(
    platform.win32_ver()) else 'Executable (*)'
