# Copyright (C) 2009 Hanback Electronics Inc.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := esterm
LOCAL_SRC_FILES := esterm.c

include $(BUILD_SHARED_LIBRARY)
