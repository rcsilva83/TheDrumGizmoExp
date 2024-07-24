noinst_LTLIBRARIES = libdg.la libzr.la libpugi.la libnls.la

# shared nls component

libnls_la_CPPFLAGS =
libnls_la_LIBADD =
libnls_la_SOURCES = \
	translation.cc

# libzita-resampler

libzr_la_CPPFLAGS = \
	-ffast-math -D_USE_MATH_DEFINES $(PTHREAD_CFLAGS) \
	-I$(top_srcdir)/zita-resampler/libs

libzr_la_LIBADD = \
	$(PTHREAD_LIBS)

libzr_la_SOURCES = \
	zrwrapper.cc \
	$(top_srcdir)/zita-resampler/libs/resampler.cc \
	$(top_srcdir)/zita-resampler/libs/resampler-table.cc

# pugixml

libpugi_la_CPPFLAGS =
libpugi_la_LIBADD =
libpugi_la_SOURCES = \
	$(top_srcdir)/pugixml/src/pugixml.cpp

# libdg
libdg_la_CPPFLAGS = \
	$(DEBUG_FLAGS) \
	-I$(top_srcdir)/hugin -I$(top_srcdir)/pugixml/src \
	$(SSEFLAGS) -I$(top_srcdir)/zita-resampler/libs \
	$(SNDFILE_CFLAGS) $(PTHREAD_CFLAGS) \
	-Wno-deprecated-declarations

libdg_la_LIBADD = \
	$(SNDFILE_LIBS) $(PTHREAD_LIBS) libzr.la libpugi.la

libdg_la_SOURCES = \
	$(top_srcdir)/src/audiocachefile.cc \
	$(top_srcdir)/src/audiocache.cc \
	$(top_srcdir)/src/audiocacheeventhandler.cc \
	$(top_srcdir)/src/audiocacheidmanager.cc \
	$(top_srcdir)/src/audioinputenginemidi.cc \
	$(top_srcdir)/src/audiofile.cc \
	$(top_srcdir)/src/bytesizeparser.cc \
	$(top_srcdir)/src/channel.cc \
	$(top_srcdir)/src/channelmixer.cc \
	$(top_srcdir)/src/configfile.cc \
	$(top_srcdir)/src/configparser.cc \
	$(top_srcdir)/src/directory.cc \
	$(top_srcdir)/src/domloader.cc \
	$(top_srcdir)/src/dgxmlparser.cc \
	$(top_srcdir)/src/drumgizmo.cc \
	$(top_srcdir)/src/drumgizmoconf.cc \
	$(top_srcdir)/src/drumkit.cc \
	$(top_srcdir)/src/drumkitloader.cc \
	$(top_srcdir)/src/events.cc \
	$(top_srcdir)/src/events_ds.cc \
	$(top_srcdir)/src/inputprocessor.cc \
	$(top_srcdir)/src/instrument.cc \
	$(top_srcdir)/src/latencyfilter.cc \
	$(top_srcdir)/src/midimapparser.cc \
	$(top_srcdir)/src/midimapper.cc \
	$(top_srcdir)/src/path.cc \
	$(top_srcdir)/src/powerlist.cc \
	$(top_srcdir)/src/powermap.cc \
	$(top_srcdir)/src/powermapfilter.cc \
	$(top_srcdir)/src/random.cc \
	$(top_srcdir)/src/sample.cc \
	$(top_srcdir)/src/sample_selection.cc \
	$(top_srcdir)/src/sem.cc \
	$(top_srcdir)/src/staminafilter.cc \
	$(top_srcdir)/src/thread.cc \
	$(top_srcdir)/src/velocityfilter.cc \
	$(top_srcdir)/src/positionfilter.cc \
	$(top_srcdir)/src/position_power.cc \
	$(top_srcdir)/src/versionstr.cc

EXTRA_DIST = \
	$(libzr_la_SOURCES) \
	$(libpugi_la_SOURCES) \
	$(libdg_la_SOURCES) \
	DGDOM.h \
	atomic.h \
	audio.h \
	audiocache.h \
	audiocacheeventhandler.h \
	audiocachefile.h \
	audiocacheidmanager.h \
	audiofile.h \
	audioinputengine.h \
	audioinputenginemidi.h \
	audiooutputengine.h \
	audiotypes.h \
	bytesizeparser.h \
	channel.h \
	channelmixer.h \
	configfile.h \
	configparser.h \
	directory.h \
	cpp11fix.h \
	dgxmlparser.h \
	domloader.h \
	drumgizmo.h \
	drumgizmoconf.h \
	drumkit.h \
	drumkitloader.h \
	engineevent.h \
	events.h \
	events_ds.h \
	grid.h \
	id.h \
	inputfilter.h \
	inputprocessor.h \
	instrument.h \
	latencyfilter.h \
	logger.h \
	memory_heap.h \
	midimapparser.h \
	midimapper.h \
	nolocale.h \
	notifier.h \
	owner.h \
	path.h \
	platform.h \
	powerlist.h \
	powermap.h \
	powermapfilter.h \
	random.h \
	range.h \
	rangemap.h \
	sample.h \
	sample_selection.h \
	sem.h \
	settings.h \
	staminafilter.h \
	syncedsettings.h \
	thread.h \
	translation.h \
	velocityfilter.h \
	positionfilter.h \
	versionstr.h \
	zrwrapper.h

%.lint: %.cc
	clang-tidy --quiet $< -- $(libdg_la_CPPFLAGS) $(CXXFLAGS)

LINT_FILES = $(libdg_la_SOURCES:.cc=.lint)

lint: $(LINT_FILES)
