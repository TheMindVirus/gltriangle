all:
	gcc main.c -o app -I /opt/vc/include/ -L /opt/vc/lib/ -lm -lbcm_host -lbrcmEGL -lbrcmGLESv2
clean:
	rm -rf app
