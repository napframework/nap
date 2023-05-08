
auxdir                  = @ac_aux_dir@
AUX_DIST                = $(auxdir)/install-sh $(auxdir)/missing \
                        $(auxdir)/mkinstalldirs

SUBDIRS = artnet debian

EXTRA_DIST = libartnet.pc.in Doxyfile README.debian

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libartnet.pc

ACLOCAL                 = aclocal -I $(auxdir)
