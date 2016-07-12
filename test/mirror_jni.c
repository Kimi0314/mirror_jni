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

jintArray Java_com_freeme_test_TestMirror_mirror(JNIEnv *env, jobject object,
    jintArray source, jint width, jint height)
{
    const int SIZE = width * height;
    LOGI("call mirror neon..., image array size = %d", SIZE);
    double t0, t_c, t_neon;

    if (SIZE <=0 || source == NULL)
    {
        LOGE("call mirror, parameters illegal!");
        return NULL;
    }

    int *pSource = (*env)->GetIntArrayElements(env, source, NULL);
    int *result = (int*) malloc(4 * SIZE);
    #if 0
    // c version
    memset(result, 0, 4 * SIZE);
    t0 = now_ms();
    mirror0(pSource, result, width, height);
    t_c = now_ms() - t0;
    LOGI("spent %gms! for c version.", t_c);
    #endif
    // neon version
    memset(result, 0, 4 * SIZE);
    t0 = now_ms();
    mirror0_neon(pSource, result, width, height);
    t_neon = now_ms() - t0;
    LOGI("spent %gms! for neon version.", t_neon);

    t0 = now_ms();
    jintArray ret = (*env)->NewIntArray(env, SIZE);
    (*env)->SetIntArrayRegion(env, ret, 0, SIZE, result);
    LOGI("spent %gms! for copy c array to java.", now_ms() - t0);

    if (MIRROR_DBG) {
        /* output the source and mirror result */
        LOGI("source = ");
        output(pSource, width, SIZE);
        LOGI("result = ");
        output(result, width, SIZE);
    }

    (*env)->ReleaseIntArrayElements(env, source, pSource, 0);
    free(result);
    
    return ret;
}

void Java_com_freeme_test_TestMirror_mirror2(JNIEnv *env, jobject object,
    jintArray data, jint width, jint height, jint rotate)
{
    const int SIZE = width * height;
    LOGI("call mirror..., image array size = %d, rotate = %d", SIZE, rotate);
    double t0, t_c;

    if (SIZE <=0 || data == NULL)
    {
        LOGE("call mirror, parameters illegal!");
        return;
    }

    int *pSource = (*env)->GetIntArrayElements(env, data, NULL);
    t0 = now_ms();
    mirror(pSource, width, height, rotate);
    t_c = now_ms() - t0;
    LOGI("spent %gms! process on c.", t_c);

    (*env)->ReleaseIntArrayElements(env, data, pSource, 0);
}