/*
 * File name: mirror_jni.c
 * 
 * Description: jni for mirror process
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
#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mirror.h"
#include <android/log.h>

#define MIRROR_DBG 0
#define LOG_TAG "libmirror"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void output(const int *data, const int width, const int size)
{
    int i = 0;
    char *str = (char*) malloc(4 * size + size);
    memset(str, 0, 4 * size + size);
    
    while(i < size) {
        char str1[256];
        
        sprintf(str1, "%d", data[i++]);
        if (i % width == 0)
        {
            strcat(str1, "\n");
        }
        else
        {
            strcat(str1, "\t");
        }
        strcat(str, str1);
    }
    
    LOGI("%s", str);
    free(str);
}

/* return current time in milliseconds */
double now_ms(void)
{
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0*res.tv_sec + (double)res.tv_nsec/1e6;
}

void Java_com_freeme_camera_PhotoModule_mirrorBitmap(JNIEnv *env, jobject object,
    jintArray data, jint width, jint height, jint rotate)
{
    const int SIZE = width * height;
    LOGI("call mirror..., image width = %d, height = %d, rotate = %d", width, height, rotate);
    double t0, t_c;

    if (SIZE <=0 || data == NULL)
    {
        LOGE("call mirror, parameters illegal!");
        return;
    }

    int *pSource = (*env)->GetIntArrayElements(env, data, NULL);
    t0 = now_ms();
    mirror_bmp(pSource, width, height, rotate);
    t_c = now_ms() - t0;
    LOGI("spent %gms! process on c.", t_c);

    (*env)->ReleaseIntArrayElements(env, data, pSource, 0);
}

void Java_com_freeme_camera_PhotoModule_mirrorJpg(JNIEnv *env, jobject object,
    jbyteArray data, jbyteArray dest, jlong size)
{
    LOGI("Java_com_freeme_camera_PhotoModule_mirrorJpg..., image size = %d", size);
    double t0, t_c;

    if (size <=0 || data == NULL || dest == NULL)
    {
        LOGE("call mirror, parameters illegal!");
        return;
    }

    const unsigned char *pSrc = (unsigned char*) (*env)->GetByteArrayElements(env, data, NULL);
    unsigned char *pDest = (unsigned char*) (*env)->GetByteArrayElements(env, dest, NULL);
    t0 = now_ms();
    mirror_jpg(pSrc, pDest, size);
    t_c = now_ms() - t0;
    LOGI("mirror jpg data spent %gms!", t_c);

    (*env)->ReleaseByteArrayElements(env, data, (jbyte*) pSrc, 0);
    (*env)->ReleaseByteArrayElements(env, dest, (jbyte*) pDest, 0);
}