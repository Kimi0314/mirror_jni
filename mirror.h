/*
 * File name: mirror.h
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
#ifndef __MIRROR_H__
#define __MIRROR_H__

/* mirror bitmap buffer with c, rotate degree is 0 */
extern void mirror0_bmp(const int *src_p, int *dst_p, int width, int height);

/* mirror bitmap buffer with neon, rotate degree is 0 */
extern void mirror0_neon_bmp(const int *src_p, int *dst_p, int width, int height);

/* mirror bitmap buffer that rotate degree */
extern void mirror_bmp(int *data, int width, int height, int rotate);

/* mirror jpeg data buffer */
extern int mirror_jpg(const unsigned char *data, unsigned char *dest, unsigned long size);

#endif