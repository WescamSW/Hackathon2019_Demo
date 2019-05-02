###########################################################################
## Useful functions for parsing strings like 'repo.git', 'mybranch!repo.git'
# Returns the .git repo after optional separator [mybranch!]repo.git
repo = $(lastword $(subst !, ,$1))
# Returns the branch in 'mybranch!repo.git' if present, otherwise master
branch = $(if $(word 2,$(subst !, ,$1)), $(firstword $(subst !, ,$1)),master)
###########################################################################

# Check for environment variables.
ifndef SHELL
SHELL = /usr/bin/sh
endif

ifndef ARCH
ARCH = $(shell uname -m)
endif

ifndef ARSDK3
ARSDK3=.
endif

PREFIX = /usr/local/
DISTDIR = $(CURDIR)/dist/
DEPDIR = $(CURDIR)/deps/
OUTPUT_DIRS = $(CURDIR)/deps $(DISTDIR)/$(PREFIX)
SYS_INC = -I/usr/local/include

ifndef DESTDIR
export DESTDIR =$(DISTDIR)
endif

CC = $(TOOL_PREFIX)g++
AR = $(TOOL_PREFIX)ar
LD = $(TOOL_PREFIX)ld

SWARM  = bebop2Swarm_$(ARCH)
WEBCAM = OpenCVWithWebcam_$(ARCH)

FLAGS += -std=c++14 -Wall -pedantic -O2 -Wno-deprecated-declarations
LOC_INC = -I$(DISTDIR)/$(PREFIX)/include -I$(ARSDK3)/include
LOC_LIB = -L$(DISTDIR)/$(PREFIX)/lib -L$(ARSDK3)/lib

SYS_INC = -I/usr/local/include
SYS_LIB = -L/usr/local/lib

# for Bebop2 SDK and wscDrone
SYS_LIB += -lwscDrone -larsal -larcommands -lardiscovery -larcontroller -larmedia -larnetwork -larnetworkal -larstream -larstream2
SYS_LIB += -lavcodec -lavformat -lswscale
#SYS_LIB += -lwscDrone
SYS_LIB += -lpthread -lrtsp -lsdp -lmux -lpomp -ljson-c -lulog -lfutils

# for opencv
SYS_INC += -I$(OPENCV)/include/opencv4/ -I$(OPENCV)/include/
SYS_LIB += -L$(OPENCV)/lib
SYS_LIB += -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_cvv -lopencv_dnn_objdetect -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_optflow -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_sfm -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_xfeatures2d -lopencv_shape -lopencv_video -lopencv_ml -lopencv_ximgproc -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core

COMMON_SRC = \
      src/VideoFrameOpenCV.cpp \
      src/OpenCVProcessing.cpp

SWARM_SRC = src/Bebop2Swarm.cpp \
      src/Missions.cpp

WEBCAM_SRC = src/OpenCVWithWebcam.cpp

all: swarm webcam
	
swarm: directories deps
	${CC} ${FLAGS} -o ${SWARM} ${SWARM_SRC} ${COMMON_SRC} ${SYS_INC} ${LOC_INC} ${LOC_LIB} ${SYS_LIB}

webcam: directories deps
	${CC} ${FLAGS} -o ${WEBCAM} ${WEBCAM_SRC} ${COMMON_SRC} ${SYS_INC} ${LOC_INC} ${LOC_LIB} ${SYS_LIB}
	
directories:
	mkdir -p $(OUTPUT_DIRS)

clean: mrproper

mrproper:
	-rm -rf $(DISTDIR)
	-rm -rf $(SWARM)
	-rm -rf $(WEBCAM)

deps: directories $(DEP_BUILD_LIST)
%.git:
	@if [ -d $(DEPDIR)/$(call repo,$@) ]; then \
	echo "DEPENDENCIES: $(DEPDIR)/$(call repo,$@) exists...skipping clone"; \
	else \
	echo "DEPENDENCIES: $(call branch,$@) @ $(call repo,$@) " \
	&& git clone -b $(call branch,$@) --single-branch ssh://git@cesium:7999/$(call repo,$@) $(DEPDIR)/$(call repo,$@) \
	&& cd $(DEPDIR)/$(call repo,$@) \
	&& cd $(CURDIR) \
	|| ( rm -Rf $(DEPDIR)/$(call repo,$@); exit 1 ); \
	fi
	make -C $(DEPDIR)/$(call repo,$@) DESTDIR=$(DESTDIR) install || exit 1; \

printvar:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
