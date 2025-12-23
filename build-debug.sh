# NOTE: This is used to pass in the DEBUG flag to enable logging.
# This is undocumented in the pebble sdk. May break at some point.
# See the following:
# https://github.com/coredevices/pebble-tool
# commit: eb955728f029be9690fb38aa2e33326150a7559a
# pebble-tool/pebble_tool/commands/sdk/project/build.py:38
CFLAGS="-DDEBUG" pebble build --debug
