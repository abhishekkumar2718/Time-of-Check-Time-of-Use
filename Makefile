all: insecure-file-access

insecure-file-access: build-insecure-file-access
	touch temporary-file
	touch privileged-file
	sudo chown sachin privileged-file
	sudo chmod g-r privileged-file

build-insecure-file-access:
	gcc insecure-file-access.c -o insecure-file-access

demonstrate-insecure-file-access: insecure-file-access
	sudo ./insecure-file-access

switch-temporary-file:
	rm temporary-file
	ln -s privileged-file temporary-file

clean-insecure-file-access:
	rm insecure-file-access temporary-file privileged-file

clean: clean-insecure-file-access
