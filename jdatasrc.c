/*
 * File name: jdatasrc.c
 * 
 * Description: This file contains decompression data source routines for the case of
 * reading JPEG data from jpeg data buffer.
 *
 * Author: Kimi Wu, contact with wuqizhi@droi.com
 * 
 * Date: 2016/07/05
 * 
 * Copyright (C) 2016 Shanghai Droi Technology Co.,Ltd.
 * 
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
#include <android/log.h>

#define LOG_TAG "libmirror"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define UNUSED(x) (void)x

/* Expanded data source object for data buffer input */
typedef struct {
    struct jpeg_source_mgr pub;     /* public fields */
    const unsigned char *data;
    size_t buffer_size;
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

#define INPUT_BUF_SIZE  32768    /* choose an efficiently fread'able size */

void init_source (j_decompress_ptr cinfo)
{
    UNUSED(cinfo);
    /* no work necessary here */
}

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;
    size_t nbytes = INPUT_BUF_SIZE;

    if (src->pub.current_offset >= src->buffer_size)
    {
        nbytes = -1;
    }
    else if (src->pub.current_offset + INPUT_BUF_SIZE > src->buffer_size)
    {
        nbytes = src->buffer_size - src->pub.current_offset;
    }

    //LOGI("fill_input_buffer, nbytes = %d, src->pub.current_offset = %d", nbytes, src->pub.current_offset);
    if (nbytes <= 0)
    {
        return FALSE;
    }
    
    src->pub.next_input_byte = src->data + src->pub.current_offset;
    src->pub.bytes_in_buffer = nbytes;
    src->pub.current_offset += nbytes;

    return TRUE;
}

void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;

    if (num_bytes > 0)
    {
        while (num_bytes > (long) src->pub.bytes_in_buffer)
        {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            src->pub.fill_input_buffer(cinfo);
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

void term_source (j_decompress_ptr cinfo)
{
    UNUSED(cinfo);
    /* no work necessary here */
}

/* Prepare for input from data buffer. */
void my_jpeg_mem_src (j_decompress_ptr cinfo, const unsigned char *data, unsigned long size)
{
    my_src_ptr src;

    if (cinfo->src == NULL)
    {
        /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo,
                                    JPOOL_PERMANENT, SIZEOF(my_source_mgr));
    }

    src = (my_src_ptr) cinfo->src;
    src->data = data;
    src->buffer_size = size;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;    /* use default method */
    src->pub.term_source = term_source;
    src->pub.next_input_byte = NULL;                                /* until buffer loaded */
    src->pub.bytes_in_buffer = 0;                                       /* forces fill_input_buffer on first read */
    src->pub.current_offset = 0;
}
