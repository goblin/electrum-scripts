all: pbkdf2 electrumize-seed

pbkdf2: pbkdf2.c
	gcc -o pbkdf2 -Wall -Wextra -O2 pbkdf2.c -lcrypto

electrumize-seed: electrumize-seed.c
	gcc -o electrumize-seed -Wall -Wextra -O2 electrumize-seed.c -lcrypto

install: pbkdf2 electrumize-seed ele_seed21st
	install -d $(DESTDIR)/usr/bin
	install pbkdf2 electrumize-seed ele_seed21st $(DESTDIR)/usr/bin
	install -d $(DESTDIR)/usr/share/doc/electrum-scripts
	install README.md $(DESTDIR)/usr/share/doc/electrum-scripts

