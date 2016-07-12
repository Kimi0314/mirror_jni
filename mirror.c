/*
 * File name: mirror.c
 * 
 * Description: mirror process for image
 *
 * Author: Kimi Wu, contact with wuqizhi@droi.com
 * 
 * Date: 2016/06/29
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
#include <string.h>
#include "jpeglib.h"
#include <arm_neon.h>
#include <android/log.h>

#define LOG_TAG "libmirror"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern void my_jpeg_mem_src (j_decompress_ptr cinfo, const unsigned char *data, unsigned long size);
extern double now_ms(void);

/* mirror bitmap buffer with c, rotate degree is 0 */
void mirror0_bmp(const int *src_p, int *dst_p, int width, int height)
{
    int i, j;
    
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            *(dst_p + i * width + j) = *(src_p + i * width + width - 1 - j);
        }
    }
}

/* mirror bitmap buffer with neon, rotate degree is 0 */
void mirror0_neon_bmp(const int *src_p, int *dst_p, int width, int height)
{
    int i, j;
    
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width / 4; j++)
        {
            int32x4_t src_vec = vld1q_s32(src_p + i * width + width - 1 - j * 4);
            vst1q_s32(dst_p + i * width + j * 4, src_vec);
        }

        if(width & 3)
        {
            for(j = width - (width & 3); j < width; j++)
            {
                *(dst_p + i * width + j) = *(src_p + i * width + width - 1 - j);
            }
        }
    }
}

/* mirror bitmap buffer that rotate degree */
void mirror_bmp(int *data, int width, int height, int rotate)
{
    int i, j;

    if (rotate % 180 == 0)
    {
        // vertical
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width / 2; j++)
            {
                int index1 = i * width + j;
                int index2 = index1 + width - 1 - 2 * j;//int index2 = i * width + width - 1 - j;
                /* swap */
                int temp = *(data + index1);
                *(data + index1) = *(data + index2);
                *(data + index2) = temp;
            }
        }
    }
    else
    {
        // horizontal
        const int offset = (height - 1) * width;
        #if 0
        // scanning by col
        for (i = 0; i < width; i++)
        {
            for (j = 0; j < height / 2; j++)
            {
                int index1 = j * width + i;
                //int index2 = (height - 1 - j) * width + i;
                int index2 = offset - index1 + 2 * i;
                /* swap */
                int temp = *(data + index1);
                *(data + index1) = *(data + index2);
                *(data + index2) = temp;
            }
        }
        #endif
        for (i = 0; i < height / 2; i++)
        {
            for (j = 0; j < width; j++)
            {
                int index1 = i * width + j;
                int index2 = offset - index1 + 2 * j ;
                /* swap */
                int temp = *(data + index1);
                *(data + index1) = *(data + index2);
                *(data + index2) = temp;
            }
        }
    }
}

/* mirror jpeg data buffer */
int mirror_jpg(const unsigned char *data, unsigned char *dest, unsigned long size)
{
    struct jpeg_decompress_struct dinfo;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr1, jerr2;
    //double t0, time;
    const int LINES = 1;

    dinfo.err = jpeg_std_error(&jerr1);
    cinfo.err = jpeg_std_error(&jerr2);
    jpeg_create_decompress(&dinfo);
    jpeg_create_compress(&cinfo);

    my_jpeg_mem_src(&dinfo, data, size);
    jpeg_mem_dest(&cinfo, &dest, &size);

    jpeg_read_header(&dinfo, TRUE);
    //LOGI("mirror_jpg: out_color_space = %d, width = %d, height = %d", dinfo.out_color_space, dinfo.image_width, dinfo.image_height);

    cinfo.image_width = dinfo.image_width;                  /* image width and height, in pixels */
    cinfo.image_height = dinfo.image_height;
    cinfo.input_components = dinfo.num_components;  /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;                             /* colorspace of input image */
    jpeg_set_defaults(&cinfo);

    jpeg_start_decompress(&dinfo);
    jpeg_start_compress(&cinfo, TRUE);
    int row_stride = dinfo.output_width * dinfo.output_components;
    JSAMPARRAY buffer = (*dinfo.mem->alloc_sarray) ((j_common_ptr) &dinfo, JPOOL_IMAGE, row_stride, LINES);

    //t0 = now_ms();
    JDIMENSION i;
    JSAMPLE temp[dinfo.output_components];
    while (dinfo.output_scanline < dinfo.output_height)
    {
        /*JDIMENSION lines = */jpeg_read_scanlines(&dinfo, buffer, LINES);
        //LOGI("lines = %d", lines);
        
        //mirror
        for (i = 0; i < dinfo.output_width / 2; i++)
        {
            int idx1 = i * dinfo.output_components;
            int idx2 = row_stride - dinfo.output_components - idx1;

            if (*(*buffer + idx1) == 0xFF && *(*buffer + idx1 + 1) == JPEG_EOI)
            {
                break;
            }
            
            memcpy((void*) temp, (void*) (*buffer + idx1), dinfo.output_components);
            memcpy((void*) (*buffer + idx1), (void*) (*buffer + idx2), dinfo.output_components);
            memcpy((void*) (*buffer + idx2), (void*) temp, dinfo.output_components);
        }
        
        jpeg_write_scanlines(&cinfo, buffer, LINES);
    }
    //time = now_ms() - t0;
    //LOGI("jpeg_read_scanlines and jpeg_write_scanlines spent total %gms!", time);

    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return 0;
}
