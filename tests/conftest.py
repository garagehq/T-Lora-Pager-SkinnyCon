"""Auto-generated conftest.py -- stubs for PlatformIO project testing."""
import sys
import os
from unittest.mock import MagicMock

# Add repo root to path so test files can import source modules
sys.path.insert(0, '')

# Mock serial and other common PlatformIO Python helper dependencies
for mod_name in ['serial', 'serial.tools', 'serial.tools.list_ports',
                 'platformio', 'platformio.project']:
    sys.modules[mod_name] = MagicMock()
