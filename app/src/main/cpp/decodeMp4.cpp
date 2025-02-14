#include <jni.h>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>

}

// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("ffmpeg");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("ffmpeg")
//      }
//    }
extern "C"
JNIEXPORT void JNICALL
Java_com_android_ffmpeg_MainActivity_decodeMp4ToYuv(JNIEnv *env, jobject thiz, jstring source_name,
                                                    jstring yuv_name) {


}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_android_ffmpeg_MainActivity_getFFMpegVersion(JNIEnv *env, jobject thiz) {
    return env->NewStringUTF(av_version_info());
}