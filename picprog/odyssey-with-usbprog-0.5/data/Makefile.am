EXTRA_DIST=pic.conf odyssey.conf

pkgdata_DATA = pic.conf
sysconf_DATA = odyssey.conf

PIC_FILES=pic12f6xx.m4 pic16f6xx.m4 pic16c679xx.m4 pic16f7x.m4 pic16f8x.m4 \
	pic16f8xx.m4 pic18fxxx.m4
# pic12c5xx.m4
pic.conf:
	@rm -rf pic.conf
	@for F in ${PIC_FILES}; do \
		m4 $$F >> pic.conf; \
	done
