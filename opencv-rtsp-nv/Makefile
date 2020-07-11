CV4_PC_PATH=/mnt/hdd/dev/custom-rtsp-poc/deps/opencv-4.3.0-dist/lib/pkgconfig

NVACC_CFLAGS=\
	-std=c++11\
	-Wall\
	`PKG_CONFIG_PATH=$(CV4_PC_PATH) pkg-config --cflags opencv4`

NVACC_LFLAGS=\
	`PKG_CONFIG_PATH=$(CV4_PC_PATH) pkg-config --libs opencv4`

nvacc: nvacc.cpp
	g++ $(NVACC_CFLAGS) -o $@ $< $(NVACC_LFLAGS)

