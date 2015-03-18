#ifndef __PJMEDIA_HIK_HISI3520_DEV_H__
#define __PJMEDIA_HIK_HISI3520_DEV_H__

#include "pjsua.h"

#include <pjmedia_audiodev.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>
#include <pj/pool.h>
#include <pjmedia/errno.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */



#ifndef PJ_BBSDK_VER
    /* Format: 0xMMNNRR:  MM: major, NN: minor, RR: revision */
#   define PJ_BBSDK_VER	0x100006
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <errno.h>




#define hisi3520_DEVICE_NAME 		"plughw:%d,%d"
/* Double these for 16khz sampling */
#define PREFERRED_FRAME_SIZE 320
#define VOIP_SAMPLE_RATE 8000

/* Set to 1 to enable tracing */
#if 1
#    define TRACE_(expr)		PJ_LOG(4,expr)
#else
#    define TRACE_(expr)
#endif

pjmedia_aud_dev_factory* pjmedia_hisi3520_factory(pj_pool_factory *pf);
/*
 * Factory prototypes
 */
static pj_status_t hisi3520_factory_init(pjmedia_aud_dev_factory *f);
static pj_status_t hisi3520_factory_destroy(pjmedia_aud_dev_factory *f);
static pj_status_t hisi3520_factory_refresh(pjmedia_aud_dev_factory *f);
static unsigned    hisi3520_factory_get_dev_count(pjmedia_aud_dev_factory *f);
static pj_status_t hisi3520_factory_get_dev_info(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_dev_info *info);
static pj_status_t hisi3520_factory_default_param(pjmedia_aud_dev_factory *f,
                                              unsigned index,
                                              pjmedia_aud_param *param);
static pj_status_t hisi3520_factory_create_stream(pjmedia_aud_dev_factory *f,
                                              const pjmedia_aud_param *param,
                                              pjmedia_aud_rec_cb rec_cb,
                                              pjmedia_aud_play_cb play_cb,
                                              void *user_data,
                                              pjmedia_aud_stream **p_strm);

/*
 * Stream prototypes
 */
static pj_status_t hisi3520_stream_get_param(pjmedia_aud_stream *strm,
                                         pjmedia_aud_param *param);
static pj_status_t hisi3520_stream_get_cap(pjmedia_aud_stream *strm,
                                       pjmedia_aud_dev_cap cap,
                                       void *value);
static pj_status_t hisi3520_stream_set_cap(pjmedia_aud_stream *strm,
                                       pjmedia_aud_dev_cap cap,
                                       const void *value);
static pj_status_t hisi3520_stream_start(pjmedia_aud_stream *strm);
static pj_status_t hisi3520_stream_stop(pjmedia_aud_stream *strm);
static pj_status_t hisi3520_stream_destroy(pjmedia_aud_stream *strm);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __PJMEDIA_hikversion_aud_DEV_H__ */
