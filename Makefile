all: build-secure-file-access build-insecure-file-access setup-test-files

setup-test-files:
	touch temporary-file
	touch privileged-file
	whoami
	sudo chown sachin privileged-file
	sudo chmod g-r privileged-file
	sudo chmod o-r privileged-file
	ls -la privileged-file

check-read-permission:
	cat privileged-file || true

check-write-permission:
	echo "Hello, World" >> privileged-file || true

display-privileged-file:
	sudo cat privileged-file

build-secure-file-access:
	gcc secure-file-access.c -o secure-file-access

demonstrate-secure-file-access:
	sudo ./secure-file-access

build-insecure-file-access:
	gcc insecure-file-access.c -o insecure-file-access

demonstrate-insecure-file-access:
	sudo ./insecure-file-access

switch-temporary-file:
	rm temporary-file
	ln -s privileged-file temporary-file

clean:
	rm -f secure-file-access insecure-file-access temporary-file privileged-file
