# Makefile for the Vim message translations for mingw32
#
# Eduardo F. Amatria <eferna1@platea.pntic.mec.es>
#
# Read the README_ming.txt file before using it.
#
# Use at your own risk but with care, it could even kill your canary.
#
# Previous to all you must have the environment variable LANGUAGE set to your
# language (xx) and add it to the next three lines.
#

LANGUAGES = af cs de es fr it ja ko pl tr zh_TW
MOFILES = af.mo cs.mo de.mo es.mo fr.mo it.mo ja.mo ko.mo pl.mo tr.mo zh_TW.mo
POFILES = af.po cs.po de.po es.po fr.po it.po ja.po ko.po pl.po tr.po zh_TW.po

PACKAGE = vim

# Uncomment one of the lines below or modify it to put the path to your
# gettex binaries; I use the first
#GETTEXT_PATH = C:/gettext.win32/bin/
#GETTEXT_PATH = C:/gettext-0.10.35-w32/win32/Release/
#GETTEXT_PATH = C:/cygwin/bin/

MSGFMT = $(GETTEXT_PATH)msgfmt
XGETTEXT = $(GETTEXT_PATH)xgettext
MSGMERGE = $(GETTEXT_PATH)msgmerge

MV = move
CP = copy
RM = del
MKD = mkdir

.SUFFIXES:
.SUFFIXES: .po .mo .pot
.PHONY: first_time all install clean $(LANGUAGES)

.po.mo:
	$(MSGFMT) -o $@ $<

all: $(MOFILES)

first_time:
	$(XGETTEXT) --default-domain=$(LANGUAGE) \
		--add-comments --keyword=_ --keyword=N_ $(wildcard ../*.c) $(wildcard ../globals.h)

$(LANGUAGES):
	$(XGETTEXT) --default-domain=$(PACKAGE) \
		--add-comments --keyword=_ --keyword=N_ $(wildcard ../*.c) $(wildcard ../globals.h)
	$(MV) $(PACKAGE).po $(PACKAGE).pot
	$(CP) $@.po $@.po.orig
	$(MV) $@.po $@.po.old
	$(MSGMERGE) $@.po.old $(PACKAGE).pot -o $@.po
	$(RM) $@.po.old

install:
	$(MKD) $(VIMRUNTIME)\lang\$(LANGUAGE)
	$(MKD) $(VIMRUNTIME)\lang\$(LANGUAGE)\LC_MESSAGES
	$(CP) $(LANGUAGE).mo $(VIMRUNTIME)\lang\$(LANGUAGE)\LC_MESSAGES\$(PACKAGE).mo

clean:
	$(RM) *.mo
	$(RM) *.pot

