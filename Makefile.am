AUTOMAKE_OPTIONS = gnu
SUBDIRS = tools include src plugin drumgizmo man test
DISTDIRS = plugingui tools include src plugin drumgizmo man test

if DISABLE_GUI
SUBDIRS += plugingui
endif

EXTRA_DIST = \
	version.h \
	hugin/hugin.h \
	hugin/hugin.hpp \
	hugin/hugin.c \
	hugin/hugin_util.h \
	hugin/hugin_syslog.h \
	hugin/hugin_syslog.c \
	hugin/hugin_filter.h \
	hugin/hugin_filter.c
