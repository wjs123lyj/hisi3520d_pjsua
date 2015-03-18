#objs=main.c pjsua_app.c ./hisi3520d/hisi3520_dev.c ./hisi3520d/hisi3520_dev.h 

#ABDIR=`pwd`
#PWD=${ABDIR}
PWD=/root/hisi3520d_pjsua
CC=arm-hisiv100nptl-linux-gcc
STRIP=arm-hisiv100nptl-linux-strip

#FREETYPE_CFLAGS= -I${PWD}/hisi3520d/include -L$PWD/hisi3520d/lib -lfreetype

PJSIP_CFLAGS= -L${PWD}/pjsip/lib -lpjsua-arm-hisiv100nptl-linux-gnu -lpjsip-ua-arm-hisiv100nptl-linux-gnu -lpjsip-simple-arm-hisiv100nptl-linux-gnu -lpjsip-arm-hisiv100nptl-linux-gnu -lpjmedia-codec-arm-hisiv100nptl-linux-gnu -lpjmedia-videodev-arm-hisiv100nptl-linux-gnu -lpjmedia-arm-hisiv100nptl-linux-gnu -lpjmedia-audiodev-arm-hisiv100nptl-linux-gnu -lpjnath-arm-hisiv100nptl-linux-gnu -lpjlib-util-arm-hisiv100nptl-linux-gnu -lresample-arm-hisiv100nptl-linux-gnu -lmilenage-arm-hisiv100nptl-linux-gnu -lsrtp-arm-hisiv100nptl-linux-gnu -lgsmcodec-arm-hisiv100nptl-linux-gnu -lspeex-arm-hisiv100nptl-linux-gnu -lilbccodec-arm-hisiv100nptl-linux-gnu -lg7221codec-arm-hisiv100nptl-linux-gnu -lportaudio-arm-hisiv100nptl-linux-gnu  -lpj-arm-hisiv100nptl-linux-gnu -lm -lnsl -lrt -lpthread  -ldl
#PJSIP_CFLAGS= -I${PWD}/hisi3520_pjsip/include -L$PWD/hisi3520_pjsip/lib -lpjsua-arm-hisiv100nptl-linux-gnu -lpjsip-ua-arm-hisiv100nptl-linux-gnu -lpjsip-simple-arm-hisiv100nptl-linux-gnu -lpjsip-arm-hisiv100nptl-linux-gnu -lpjmedia-codec-arm-hisiv100nptl-linux-gnu -lpjmedia-videodev-arm-hisiv100nptl-linux-gnu -lpjmedia-arm-hisiv100nptl-linux-gnu -lpjmedia-audiodev-arm-hisiv100nptl-linux-gnu -lpjnath-arm-hisiv100nptl-linux-gnu -lpjlib-util-arm-hisiv100nptl-linux-gnu -lresample-arm-hisiv100nptl-linux-gnu -lmilenage-arm-hisiv100nptl-linux-gnu -lsrtp-arm-hisiv100nptl-linux-gnu -lgsmcodec-arm-hisiv100nptl-linux-gnu -lspeex-arm-hisiv100nptl-linux-gnu -lilbccodec-arm-hisiv100nptl-linux-gnu -lg7221codec-arm-hisiv100nptl-linux-gnu -lportaudio-arm-hisiv100nptl-linux-gnu  -lpj-arm-hisiv100nptl-linux-gnu -lm -lnsl -lrt -lpthread  -ldl -O2 -DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1
Cflags= -I${PWD}/pjsip/include -O2 -DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1

SDK_CFLAGS= -I${PWD}/hisi3520d/include  -I${PWD}/hisi3520d/common -I${PWD}/hisi3520d/extdrv/tw2865 -I${PWD}/hisi3520d/extdrv/tw2960 -I${PWD}/hisi3520d/extdrv/tlv320aic31 -I${PWD}/hisi3520d/extdrv/cx26828 -Dhi3520D -DHICHIP=0x3520D100 -DHI_DEBUG -DHI_XXXX -DDEMO -lpthread  -lm ${PWD}/hisi3520d/lib/libmpi.a ${PWD}/hisi3520d/lib/libhdmi.a ${PWD}/hisi3520d/lib/libVoiceEngine.a ${PWD}/hisi3520d/lib/libaec.a ${PWD}/hisi3520d/lib/libresampler.a  ${PWD}/hisi3520d/lib/libanr.a ${PWD}/hisi3520d/lib/libjpeg.a


FILE=${PWD}/*.c ${PWD}/hisi3520d/*.c ${PWD}/hisi3520d/*.h ${PWD}/hisi3520d/common/*.c ${PWD}/hisi3520d/common/*.h ${PWD}/hisi3520d/application/*.c ${PWD}/hisi3520d/application/*.h


#$CC $FILE $ULIBC $PJSIP_CFLAGS $PJSIP_CFLAGS $SDK_CFLAGS $FREETYPE_CFLAGS  -o pjsua_arm_hisi3520d -Wwrite-strings -fpermissive
#$CC  $PJSIP_CFLAGS $SDK_CFLAGS $FREETYPE_CFLAGS $FILE -Wall -g -o pjsua_arm_hisi3520d -Wwrite-strings -fpermissive
#$CC  $PJSIP_CFLAGS $SDK_CFLAGS  $FILE -Wall -g -o pjsua_arm_hisi3520d -Wwrite-strings -fpermissive

pjsua_hisi3520d:$(FILE)
	$(CC) -Wall -g -o $@ $(FILE) $(PJSIP_CFLAGS) $(Cflags) $(SDK_CFLAGS) -Wwrite-strings
#	$(CC) -o $@ $(objs) $(PJSIP_CFLAGS) $(Cflags) $(SDK_CFLAGS) -Wall -g -Wwrite-strings -shared
#	${STRIP} pjsua_hisi3520d


