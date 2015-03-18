#include "hisi3520_dev.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include "application/sample_audio.h"

#define THIS_FILE 	"hisi3520_dev.c"

static pjmedia_aud_dev_factory_op hisi3520_factory_op =
{
    &hisi3520_factory_init,
    &hisi3520_factory_destroy,
    &hisi3520_factory_get_dev_count,
    &hisi3520_factory_get_dev_info,
    &hisi3520_factory_default_param,
    &hisi3520_factory_create_stream,
    &hisi3520_factory_refresh
};

static pjmedia_aud_stream_op hisi3520_stream_op =
{
    &hisi3520_stream_get_param,
    &hisi3520_stream_get_cap,
    &hisi3520_stream_set_cap,
    &hisi3520_stream_start,
    &hisi3520_stream_stop,
    &hisi3520_stream_destroy
};



struct hisi3520_factory
{
    pjmedia_aud_dev_factory	 base;
    pj_pool_factory		*pf;
    pj_pool_t			*pool;
    pj_pool_t			*base_pool;
    unsigned			 dev_cnt;
    pjmedia_aud_dev_info	 devs[1];
};

struct hisi3520_stream
{
    pjmedia_aud_stream	 base;

    /* Common */
    pj_pool_t		*pool;
    struct hisi3520_factory *af;
    void		*user_data;
    pjmedia_aud_param	 param;		/* Running parameter 		*/
    int                  rec_id;      	/* Capture device id		*/
    int                  quit;

    /* Playback */
    unsigned int pb_ctrl_audio_manager_handle;

    unsigned int pb_audio_manager_handle;
    unsigned long        pb_frames; 	/* samples_per_frame		*/
    pjmedia_aud_play_cb  pb_cb;
    unsigned             pb_buf_size;
    char		*pb_buf;
    pj_thread_t		*pb_thread;

    /* Capture */

    unsigned int ca_audio_manager_handle;
    unsigned long        ca_frames; 	/* samples_per_frame		*/
    pjmedia_aud_rec_cb   ca_cb;
    unsigned             ca_buf_size;
    char		*ca_buf;
    pj_thread_t		*ca_thread;
};
/*
 * hisi3520 - tests loads the audio units and sets up the driver structure
 */
static pj_status_t hisi3520_add_dev (struct hisi3520_factory *af)
{
    pjmedia_aud_dev_info *adi;
    int pb_result, ca_result;
    unsigned int handle;

    if (af->dev_cnt >= PJ_ARRAY_SIZE(af->devs))
        return PJ_ETOOMANY;

    adi = &af->devs[af->dev_cnt];

    TRACE_((THIS_FILE, "hisi3520_add_dev Enter"));


    /* Reset device info */
    pj_bzero(adi, sizeof(*adi));

    /* Set device name */
    strcpy(adi->name, "preferred");

    /* Check the number of playback channels */
    adi->output_count = (pb_result >= 0) ? 1 : 0;

    /* Check the number of capture channels */
    adi->input_count = (ca_result >= 0) ? 1 : 0;

    /* Set the default sample rate */
    adi->default_samples_per_sec = 8000;

    /* Driver name */
    strcpy(adi->driver, "hisi3520");

    ++af->dev_cnt;

    PJ_LOG (4,(THIS_FILE, "Added sound device %s", adi->name));

    return PJ_SUCCESS;
}

/* Create hisi3520 audio driver. */
pjmedia_aud_dev_factory* pjmedia_hisi3520_factory(pj_pool_factory *pf)
{
    struct hisi3520_factory *af;
    pj_pool_t *pool;

    pool = pj_pool_create(pf, "hisi3520_aud_base", 256, 256, NULL);
    af = PJ_POOL_ZALLOC_T(pool, struct hisi3520_factory);
    af->pf = pf;
    af->base_pool = pool;
    af->base.op = &hisi3520_factory_op;

    return &af->base;
}


/* API: init factory */
static pj_status_t hisi3520_factory_init(pjmedia_aud_dev_factory *f)
{
    pj_status_t status;
    status = hisi3520_factory_refresh(f);
    if (status != PJ_SUCCESS)
        return status;

    PJ_LOG(4,(THIS_FILE, "hisi3520 initialized"));
    return PJ_SUCCESS;
}


/* API: destroy factory */
static pj_status_t hisi3520_factory_destroy(pjmedia_aud_dev_factory *f)
{
    struct hisi3520_factory *af = (struct hisi3520_factory*)f;

    if (af->pool) {
        TRACE_((THIS_FILE, "hisi3520_factory_destroy() - 1"));
        pj_pool_release(af->pool);
    }

    if (af->base_pool) {
        pj_pool_t *pool = af->base_pool;
        af->base_pool = NULL;
        TRACE_((THIS_FILE, "hisi3520_factory_destroy() - 2"));
        pj_pool_release(pool);
    }

    return PJ_SUCCESS;
}


/* API: refresh the device list */
static pj_status_t hisi3520_factory_refresh(pjmedia_aud_dev_factory *f)
{
    struct hisi3520_factory *af = (struct hisi3520_factory*)f;
    int err;

    TRACE_((THIS_FILE, "hisi3520_factory_refresh()"));

    if (af->pool != NULL) {
        pj_pool_release(af->pool);
        af->pool = NULL;
    }

    af->pool = pj_pool_create(af->pf, "hisi3520_aud", 256, 256, NULL);
    af->dev_cnt = 0;

    err = hisi3520_add_dev(af);

    PJ_LOG(4,(THIS_FILE, "hisi3520 driver found %d devices", af->dev_cnt));

    return err;
}


/* API: get device count */
static unsigned  hisi3520_factory_get_dev_count(pjmedia_aud_dev_factory *f)
{
    struct hisi3520_factory *af = (struct hisi3520_factory*)f;
    return af->dev_cnt;
}


/* API: get device info */
static pj_status_t hisi3520_factory_get_dev_info(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_dev_info *info)
{
    struct hisi3520_factory *af = (struct hisi3520_factory*)f;

    PJ_ASSERT_RETURN(index>=0 && index<af->dev_cnt, PJ_EINVAL);

    pj_memcpy(info, &af->devs[index], sizeof(*info));
    info->caps = PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY |
                 PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY;

    return PJ_SUCCESS;
}

/* API: create default parameter */
static pj_status_t hisi3520_factory_default_param(pjmedia_aud_dev_factory *f,
                                              unsigned index,
                                              pjmedia_aud_param *param)
{
    struct hisi3520_factory *af = (struct hisi3520_factory*)f;
    pjmedia_aud_dev_info *adi;

    PJ_ASSERT_RETURN(index>=0 && index<af->dev_cnt, PJ_EINVAL);

    adi = &af->devs[index];

    pj_bzero(param, sizeof(*param));
    if (adi->input_count && adi->output_count) {
        param->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
        param->rec_id = index;
        param->play_id = index;
    } else if (adi->input_count) {
        param->dir = PJMEDIA_DIR_CAPTURE;
        param->rec_id = index;
        param->play_id = PJMEDIA_AUD_INVALID_DEV;
    } else if (adi->output_count) {
        param->dir = PJMEDIA_DIR_PLAYBACK;
        param->play_id = index;
        param->rec_id = PJMEDIA_AUD_INVALID_DEV;
    } else {
        return PJMEDIA_EAUD_INVDEV;
    }

    param->clock_rate = adi->default_samples_per_sec;
    param->channel_count = 1;
    param->samples_per_frame = adi->default_samples_per_sec * 20 / 1000;
    param->bits_per_sample = 16;
    param->flags = adi->caps;
    param->input_latency_ms = PJMEDIA_SND_DEFAULT_REC_LATENCY;
    param->output_latency_ms = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;

    TRACE_((THIS_FILE, "hisi3520_factory_default_param clock = %d flags = %d"
                       " spf = %d", param->clock_rate, param->flags,
                       param->samples_per_frame));

    return PJ_SUCCESS;
}


static void close_play_pcm(struct hisi3520_stream *stream)
{

}

static void flush_play(struct hisi3520_stream *stream)
{

}

static void close_capture_pcm(struct hisi3520_stream *stream)
{

}

static void flush_capture(struct hisi3520_stream *stream)
{

}


/**
 * Play audio received from PJMEDIA
 */
static int pb_thread_func (void *arg)
{
    struct hisi3520_stream* stream = (struct hisi3520_stream *) arg;
    /* Handle from hisi3520_open_playback */
    /* Will be 640 */
    int size                   	= stream->pb_buf_size;
    /* 160 frames for 20ms */
    unsigned long nframes	= stream->pb_frames;
    void *user_data            	= stream->user_data;
    char *buf 		       	= stream->pb_buf;
    pj_timestamp tstamp;
    int result = 0;

    pj_bzero (buf, size);
    tstamp.u64 = 0;

    TRACE_((THIS_FILE, "pb_thread_func: size = %d ", size));

    /* Do the final initialization now the thread has started. */


    while (!stream->quit) {
        pjmedia_frame frame;

        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        /* pointer to buffer filled by PJMEDIA */
        frame.buf = buf;
        frame.size = size;
        frame.timestamp.u64 = tstamp.u64;
        frame.bit_info = 0;

        /* Read the audio from pjmedia */
        result = stream->pb_cb (user_data, &frame);
        if (result != PJ_SUCCESS || stream->quit)
            break;

        if (frame.type != PJMEDIA_FRAME_TYPE_AUDIO)
            pj_bzero (buf, size);
          //hikversion_sendaud_data(buf,size);
        /* Write 640 to play unit */


	tstamp.u64 += nframes;
    }

    flush_play(stream);
    close_play_pcm(stream);
    TRACE_((THIS_FILE, "pb_thread_func: Stopped"));

    return PJ_SUCCESS;
}



static int ca_thread_func (void *arg)
{
    struct hisi3520_stream* stream = (struct hisi3520_stream *) arg;
    int size                   = stream->ca_buf_size;
    unsigned long nframes      = stream->ca_frames;
    void *user_data            = stream->user_data;
    /* Buffer to fill for PJMEDIA */
    char *buf 		       = stream->ca_buf;
    pj_timestamp tstamp;
    int result;
    struct sched_param param;
    pthread_t *thid;

    TRACE_((THIS_FILE, "ca_thread_func: size = %d ", size));

    thid = (pthread_t*) pj_thread_get_os_handle (pj_thread_this());
    param.sched_priority = sched_get_priority_max (SCHED_RR);

    result = pthread_setschedparam (*thid, SCHED_RR, &param);
    if (result) {
        if (result == EPERM) {
            PJ_LOG (4,(THIS_FILE, "Unable to increase thread priority, "
                                  "root access needed."));
        } else {
            PJ_LOG (4,(THIS_FILE, "Unable to increase thread priority, "
                                  "error: %d", result));
        }
    }

    pj_bzero (buf, size);
    tstamp.u64 = 0;

    /* Final init now the thread has started */


    while (!stream->quit) {
        pjmedia_frame frame;

        pj_bzero (buf, size);

        /* read the input device */


        if (stream->quit)
            break;

//         int mp3_fd = -1;
//          const char *mp3_path = "/home/audiodata_recv.G711";
//          if ((mp3_fd = open(mp3_path, O_CREAT | O_TRUNC | O_WRONLY, 00666)) < 0)
//           {
//             printf("<recv_audio_data_ext> open %s failed!\n", mp3_path);
//             return -1;
//           }
//          write(mp3_fd, buf, size);

        /* Write the capture audio data to PJMEDIA */
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        frame.buf = (void *) buf;
        frame.size = size;
        frame.timestamp.u64 = tstamp.u64;
        frame.bit_info = 0;
        // audio_get_frame(buf,size);
        result = stream->ca_cb (user_data, &frame);
        if (result != PJ_SUCCESS || stream->quit)
            break;

        tstamp.u64 += nframes;
    }

    flush_capture(stream);
    close_capture_pcm(stream);
    TRACE_((THIS_FILE, "ca_thread_func: Stopped"));

    return PJ_SUCCESS;
}

/* Audio routing, speaker/headset */
static pj_status_t hisi3520_initialize_playback_ctrl(struct hisi3520_stream *stream,
                                                 bool speaker)
{
    /* Although the play and capture have audio manager handles, audio routing
     * requires a separate handle
     */
    int ret = PJ_SUCCESS;


    /* Set for either speaker or earpiece */

    return PJ_SUCCESS;
}

static pj_status_t hisi3520_open_playback (struct hisi3520_stream *stream,
                                       const pjmedia_aud_param *param)
{
    int ret = 0;


    unsigned int rate;
    unsigned long tmp_buf_size;

    if (param->play_id < 0 || param->play_id >= stream->af->dev_cnt) {
        return PJMEDIA_EAUD_INVDEV;
    }

    /* Use the hisi3520 audio manager API to open as opposed to QNX core audio
     * Echo cancellation built in
     */

    rate = param->clock_rate;
    /* Set the sound device buffer size and latency */
    if (param->flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY) {
        tmp_buf_size = (rate / 1000) * param->output_latency_ms;
    } else {
	tmp_buf_size = (rate / 1000) * PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
    }
    /* Set period size to samples_per_frame frames. */
    stream->pb_frames = param->samples_per_frame;
    stream->param.output_latency_ms = tmp_buf_size / (rate / 1000);

    /* Set our buffer */
    stream->pb_buf_size = stream->pb_frames * param->channel_count *
                          (param->bits_per_sample/8);
    stream->pb_buf = (char *) pj_pool_alloc(stream->pool, stream->pb_buf_size);

    TRACE_((THIS_FILE, "hisi3520_open_playback: pb_frames = %d clock = %d",
                       stream->pb_frames, param->clock_rate));
    
    return PJ_SUCCESS;
}

static pj_status_t hisi3520_open_capture (struct hisi3520_stream *stream,
                                      const pjmedia_aud_param *param)
{
    int ret = 0;
    unsigned int rate;
    unsigned long tmp_buf_size;
    int frame_size;

    /* Set clock rate */
    rate = param->clock_rate;
    stream->ca_frames = (unsigned long) param->samples_per_frame /
			param->channel_count;

    /* Set the sound device buffer size and latency */
    if (param->flags & PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY) {
        tmp_buf_size = (rate / 1000) * param->input_latency_ms;
    } else {
        tmp_buf_size = (rate / 1000) * PJMEDIA_SND_DEFAULT_REC_LATENCY;
    }

    stream->param.input_latency_ms = tmp_buf_size / (rate / 1000);

    /* Set our buffer */
    stream->ca_buf_size = stream->ca_frames * param->channel_count *
			  (param->bits_per_sample/8);
    stream->ca_buf = (char *)pj_pool_alloc (stream->pool, stream->ca_buf_size);

    TRACE_((THIS_FILE, "hisi3520_open_capture: ca_frames = %d clock = %d",
                       stream->ca_frames, param->clock_rate));

    return PJ_SUCCESS;
}


/* API: create stream */
static pj_status_t hisi3520_factory_create_stream(pjmedia_aud_dev_factory *f,
                                              const pjmedia_aud_param *param,
                                              pjmedia_aud_rec_cb rec_cb,
                                              pjmedia_aud_play_cb play_cb,
                                              void *user_data,
                                              pjmedia_aud_stream **p_strm)
{
    struct hisi3520_factory *af = (struct hisi3520_factory*)f;
    pj_status_t status;
    pj_pool_t* pool;
    struct hisi3520_stream* stream;

    pool = pj_pool_create (af->pf, "hisi3520%p", 1024, 1024, NULL);
    if (!pool)
        return PJ_ENOMEM;

    /* Allocate and initialize comon stream data */
    stream = PJ_POOL_ZALLOC_T (pool, struct hisi3520_stream);
    stream->base.op   = &hisi3520_stream_op;
    stream->pool      = pool;
    stream->af 	      = af;
    stream->user_data = user_data;
    stream->pb_cb     = play_cb;
    stream->ca_cb     = rec_cb;
    stream->quit      = 0;
    pj_memcpy(&stream->param, param, sizeof(*param));

    /* Init playback */
    if (param->dir & PJMEDIA_DIR_PLAYBACK) {
        //audio_playback_start();

//        status = pj_thread_create (stream->pool,
//                   "hik_audio_playback_start",
//                   audio_playback_start,
//                   stream,
//                   0,
//                   0,
//                   &stream->pb_thread);
//        if (status != PJ_SUCCESS)
//            return status;


        status = hisi3520_open_playback (stream, param);
        if (status != PJ_SUCCESS) {
            pj_pool_release (pool);
            return status;
        }
    }

    /* Init capture */
    if (param->dir & PJMEDIA_DIR_CAPTURE) {

        audio_cfg_init();
        status = hisi3520_open_capture (stream, param);

        if (status != PJ_SUCCESS) {
            if (param->dir & PJMEDIA_DIR_PLAYBACK) {
                close_play_pcm(stream);
            }
            pj_pool_release (pool);
            return status;
        }
    }

    /* Part of the play functionality but the RIM/Truphone loopback sample
     * initialializes after the play and capture
     * "false" is default/earpiece for output
     */
    status = hisi3520_initialize_playback_ctrl(stream,false);
    if (status != PJ_SUCCESS) {
    	return PJMEDIA_EAUD_SYSERR;
    }

    *p_strm = &stream->base;
    return PJ_SUCCESS;
}


/* 
 * API: get running parameter
 * based on ALSA template
 */
static pj_status_t hisi3520_stream_get_param(pjmedia_aud_stream *s,
                                         pjmedia_aud_param *pi)
{
    struct hisi3520_stream *stream = (struct hisi3520_stream*)s;

    PJ_ASSERT_RETURN(s && pi, PJ_EINVAL);

    pj_memcpy(pi, &stream->param, sizeof(*pi));

    return PJ_SUCCESS;
}


/*
 * API: get capability
 * based on ALSA template 
*/
static pj_status_t hisi3520_stream_get_cap(pjmedia_aud_stream *s,
                                       pjmedia_aud_dev_cap cap,
                                       void *pval)
{
    struct hisi3520_stream *stream = (struct hisi3520_stream*)s;

    PJ_ASSERT_RETURN(s && pval, PJ_EINVAL);

    if (cap==PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY &&
        (stream->param.dir & PJMEDIA_DIR_CAPTURE))
    {
        /* Recording latency */
        *(unsigned*)pval = stream->param.input_latency_ms;
        return PJ_SUCCESS;

    } else if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY &&
               (stream->param.dir & PJMEDIA_DIR_PLAYBACK))
    {
        /* Playback latency */
        *(unsigned*)pval = stream->param.output_latency_ms;
        return PJ_SUCCESS;

    } else {
        return PJMEDIA_EAUD_INVCAP;
    }
}


/*
 * API: set capability
 * Currently just supporting toggle between speaker and earpiece
 */
static pj_status_t hisi3520_stream_set_cap(pjmedia_aud_stream *strm,
                                       pjmedia_aud_dev_cap cap,
                                       const void *value)
{
    pj_status_t ret = PJ_SUCCESS;
    struct hisi3520_stream *stream = (struct hisi3520_stream*)strm;

    if (cap != PJMEDIA_AUD_DEV_CAP_OUTPUT_ROUTE || value == NULL) {
        TRACE_((THIS_FILE,"hisi3520_stream_set_cap() = PJMEDIA_EAUD_INVCAP"));
        return PJMEDIA_EAUD_INVCAP; 

    } else {
    	pjmedia_aud_dev_route route = *((pjmedia_aud_dev_route*)value);
        /* Use the initialization function which lazy-inits the
         * handle for routing
         */
    	if (route == PJMEDIA_AUD_DEV_ROUTE_LOUDSPEAKER) {
            ret = hisi3520_initialize_playback_ctrl(stream,true);
        } else {
            ret = hisi3520_initialize_playback_ctrl(stream,false);
        }
    }

    if (ret != PJ_SUCCESS) {
        TRACE_((THIS_FILE,"hisi3520_stream_set_cap() = %d",ret));
    }
    return ret;
}


/* API: start stream */
static pj_status_t hisi3520_stream_start (pjmedia_aud_stream *s)
{
    struct hisi3520_stream *stream = (struct hisi3520_stream*)s;
    pj_status_t status = PJ_SUCCESS;

    stream->quit = 0;
    if (stream->param.dir & PJMEDIA_DIR_PLAYBACK) {
        status = pj_thread_create (stream->pool,
                   "hisi3520sound_playback",
				   pb_thread_func,
				   stream,
				   0,
				   0,
				   &stream->pb_thread);
        if (status != PJ_SUCCESS)
            return status;
    }

    if (stream->param.dir & PJMEDIA_DIR_CAPTURE) {
        status = pj_thread_create (stream->pool,
                   "hisi3520sound_playback",
				   ca_thread_func,
				   stream,
				   0,
				   0,
				   &stream->ca_thread);
        if (status != PJ_SUCCESS) {
            stream->quit = PJ_TRUE;
            pj_thread_join(stream->pb_thread);
            pj_thread_destroy(stream->pb_thread);
            stream->pb_thread = NULL;
        }
    }

    return status;
}


/* API: stop stream */
static pj_status_t hisi3520_stream_stop (pjmedia_aud_stream *s)
{
    struct hisi3520_stream *stream = (struct hisi3520_stream*)s;
    audio_capture_stop();
    stream->quit = 1;
    TRACE_((THIS_FILE,"hisi3520_stream_stop()"));

    if (stream->pb_thread) {
        pj_thread_join (stream->pb_thread);
        pj_thread_destroy(stream->pb_thread);
        stream->pb_thread = NULL;
    }

    if (stream->ca_thread) {
        pj_thread_join (stream->ca_thread);
        pj_thread_destroy(stream->ca_thread);
        stream->ca_thread = NULL;
    }

    return PJ_SUCCESS;
}

static pj_status_t hisi3520_stream_destroy (pjmedia_aud_stream *s)
{
    struct hisi3520_stream *stream = (struct hisi3520_stream*)s;

    TRACE_((THIS_FILE,"hisi3520_stream_destroy()"));

    hisi3520_stream_stop (s);

    pj_pool_release (stream->pool);

    return PJ_SUCCESS;
}


