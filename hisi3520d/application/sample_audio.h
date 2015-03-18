#ifndef __PJMEDIA_SAMPLE_AUDIO_DEV_H__
#define __PJMEDIA_SAMPLE_AUDIO_DEV_H__

#include "pjsua.h"
#include "sample_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

void audio_cfg_init();
void *audio_get_frame(char *buf, int size);

//void audio_capture_start();
void audio_capture_stop();


    
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __PJMEDIA_HISI_DEV_H__ */
