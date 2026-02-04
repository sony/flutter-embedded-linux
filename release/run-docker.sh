docker run -ti --rm \
	--user "$(id -u):$(id -g)" \
	-v "$PWD":/flutter \
	-w /flutter \
	--env HOME=/flutter/home \
	flutter
