#define LOG_NDEBUG 0
#define LOG_TAG "RILWrapThread"

#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <telephony/ril.h>
#include <utils/Log.h>
#include "rilthreads.h"
#include "multicast.h"

void RIL_startThreads(void *param1, void *param2)
{
	int ret;
	pthread_attr_t attr;
	pthread_t tid_android, tid_radio;

	pthread_attr_init (&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&tid_radio, &attr, RIL_radioThread, param1);
	if (ret < 0) {
		LOGE("Failed to create radio thread errno:%d", errno);
		return;
	}

	ret = pthread_create(&tid_android, &attr, RIL_androidThread, param2);
	if (ret < 0) {
		LOGE("Failed to create android thread errno:%d", errno);
		return;
	}
}

void *RIL_radioThread(void *param)
{
	RIL_RadioFunctions *functions = (RIL_RadioFunctions*) param;

	while(1) {
		/* TODO: Do something */
		sleep(1);
	}

	return NULL;
}

void *RIL_androidThread(void *param)
{
	struct RIL_Env *env = (struct RIL_Env*) param;

	while(1) {
		/* TODO: Do something */
		sleep(1);
	}

	return NULL;
}
