/*
 * Copyright (C) 2009 Hanback Electronics Inc.
 *
 */
#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <android/log.h>

jint Java_ac_kr_kgu_esproject_ArrayAdderActivity_calculate( JNIEnv* env, jobject thiz, jintArray ia, jint value){
	int i,sum = 0;

	jsize len = (*env)->GetArrayLength(env,ia);
	jint* array = (*env)->GetIntArrayElements(env, ia, 0);
	for(i=0; i<len; i++){
		sum += array[i];
	}
	(*env)->ReleaseIntArrayElements(env, ia, array, 0);

	return sum;
}

jint
Java_ac_kr_kgu_esproject_ArrayAdderActivity_buzzerControl( JNIEnv* env,
                                                  jobject thiz, jint value)
{
	int fd,ret;
	int data = value;

	fd = open("/dev/buzzer",O_WRONLY);
 
	if(fd < 0) return -errno;
 
	ret = write(fd, &data, 1);
	close(fd);
  
	if(ret == 1) return 0;
  	
	return -1;
}

jint Java_ac_kr_kgu_esproject_ArrayAdderActivity_dotMatrixControl( JNIEnv* env, jobject thiz, jstring data)
{
	jboolean iscopy;
	char* buf;
	int dev,ret,len;
	char str[100];

	buf = (*env)->GetStringUTFChars(env, data, &iscopy);
	len = (*env)->GetStringLength(env,data);

	dev = open("/dev/dotmatrix", O_RDWR | O_SYNC);

	if(dev != -1){
		ret = write(dev, buf, len);
		close(dev);
	} else {
		//__android_log_print(ANDROID_LOG_ERROR, "ArrayAdderActivity", "dot Open Error!\n");
		exit(1);
	}

	return 0;
}

jint Java_ac_kr_kgu_esproject_ArrayAdderActivity_segmentControl( JNIEnv* env, jobject thiz, jstring data)
{
	jboolean iscopy;
	char* buf;
	int dev,ret,len;
	char str[100];

	buf = (*env)->GetStringUTFChars(env, data, &iscopy);
	len = (*env)->GetStringLength(env,data);

	dev = open("/dev/segment", O_RDWR | O_SYNC);

	if(dev != -1){
		ret = write(dev, buf, len);
		close(dev);
	} else {
		//__android_log_print(ANDROID_LOG_ERROR, "ArrayAdderActivity", "dot Open Error!\n");
		exit(1);
	}

	return 0;
}
