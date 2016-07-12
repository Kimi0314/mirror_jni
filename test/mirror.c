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
#include <arm_neon.h>
#include "mirror.h"

/* mirror image buffer with c, rotate degree is 0 */
void mirror0(const int *src_p, int *dst_p, int width, int height)
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

/* mirror image buffer with neon, rotate degree is 0 */
void mirror0_neon(const int *src_p, int *dst_p, int width, int height)
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

/* mirror image buffer that rotate degree */
void mirror(int *data, int width, int height, int rotate)
{
    int i, j;

    if (rotate % 180 == 0)
    {
        // horizontal
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
        // vertical
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
