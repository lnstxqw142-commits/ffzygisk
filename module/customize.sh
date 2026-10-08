#!/system/bin/sh
SKIPUNZIP=0
ui_print "- FF Native Menu v4.0"
set_perm_recursive $MODPATH 0 0 0755 0644
set_perm $MODPATH/zygisk/arm64-v8a.so 0 0 0644
