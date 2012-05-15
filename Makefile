all:
	make -C src all

debug:
	make -C src debug

install: 
	make -C src install
	make -C man install

clean:
	make -C src clean
