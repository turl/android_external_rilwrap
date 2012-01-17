#define LOG_NDEBUG 0
#define LOG_TAG "RILWrap"

#include <dlfcn.h>
#include <cutils/properties.h>
#include <telephony/ril.h>
#include <utils/Log.h>
#include "rilwrap.h"

/* Wrapped RIL_env functions */
void (*OnRequestComplete_wrapee) (RIL_Token, RIL_Errno, void *, size_t);
void (*OnUnsolicitedResponse_wrapee) (int, const void *, size_t);
void (*RequestTimedCallback_wrapee) (RIL_TimedCallback, void *, const struct timeval *);

/* RIL_env wrappers */
void OnRequestComplete_wrapper (RIL_Token t, RIL_Errno e,
	void *response, size_t responselen)
{
	/* Debug code */
	RequestInfo *pRI = (RequestInfo *)t;
	LOGV("OnRequestComplete token='%u' errno='%u' responselen='%u'",
		pRI->token, e, responselen);

	/* Call the orig function */
	OnRequestComplete_wrapee(t, e, response, responselen);
}

void OnUnsolicitedResponse_wrapper (int unsolResponse, const void *data,
	size_t datalen)
{
	/* Debug code */
	LOGV("OnUnsolicitedResponse unsolResponse='%d' datalen='%u'",
		unsolResponse, datalen);

	/* Call the orig function */
	OnUnsolicitedResponse_wrapee(unsolResponse, data, datalen);
}

void RequestTimedCallback_wrapper (RIL_TimedCallback callback,
	void *param, const struct timeval *relativeTime)
{
	/* Debug code */
	LOGV("RequestTimedCallback relativeTime='%lu'",
		(relativeTime->tv_sec * 1000) + (relativeTime->tv_usec / 1000));

	/* Call the orig function */
	RequestTimedCallback_wrapee(callback, param, relativeTime);
}

/* Wrapped RIL_RadioFunctions functions */
RIL_RequestFunc RIL_RequestFunc_wrapee;
RIL_RadioStateRequest RIL_RadioStateRequest_wrapee;
RIL_Supports RIL_Supports_wrapee;
RIL_Cancel RIL_Cancel_wrapee;
RIL_GetVersion RIL_GetVersion_wrapee;

/* RIL_RadioFunctions wrappers */
void RIL_RequestFunc_wrapper (int request, void *data, size_t datalen, RIL_Token t)
{
	/* Debug code */
	RequestInfo *pRI = (RequestInfo *)t;
	LOGV("RIL_RequestFunc request='%d' datalen='%u' token='%u'",
		request, datalen, pRI->token);
	/* Call the orig function */
	RIL_RequestFunc_wrapee(request, data, datalen, t);
}

RIL_RadioState RIL_RadioStateRequest_wrapper (int argc, char *argv[])
{
	/* Debug code */
	LOGV("RIL_RadioStateRequest");

	/* Call the orig function */
	return RIL_RadioStateRequest_wrapee(argc, argv);
}

int RIL_Supports_wrapper (int requestCode)
{
	/* Debug code */
	LOGV("RIL_Supports requestCode='%d'", requestCode);

	/* Call the orig function */
	return RIL_Supports_wrapee(requestCode);
}

void RIL_Cancel_wrapper (RIL_Token t)
{
	/* Debug code */
	RequestInfo *pRI = (RequestInfo *)t;
	LOGV("RIL_Cancel token='%d'", pRI->token);

	/* Call the orig function */
	return RIL_Cancel_wrapee(t);
}

const char * RIL_GetVersion_wrapper (void)
{
	LOGV("RIL_GetVersion");
	/* Call the orig function */
	return RIL_GetVersion_wrapee();
}


RIL_RadioFunctions *RIL_RadioFunctions_orig;
RIL_RadioFunctions RIL_RadioFunctions_wrap;
struct RIL_Env RIL_Env_wrap;

/* Main function - magic happens here! */
const RIL_RadioFunctions *RIL_Init (const struct RIL_Env *env, int argc, char **argv)
{
	void *handle;
	RIL_RadioFunctions *(*RIL_Init_wrap)(const struct RIL_Env *, int, char **);

	char wraplib[PROPERTY_VALUE_MAX];

	/* Read lib path */
	property_get("rild.wraplib", wraplib, "none");

	handle = dlopen(wraplib, RTLD_NOW);
	if (!handle) {
		exit(1);
	}

	RIL_Init_wrap = dlsym(handle, "RIL_Init");
	if (dlerror() != NULL)
		exit(2);

	/* Set up wrapees */
	OnRequestComplete_wrapee = env->OnRequestComplete;
	OnUnsolicitedResponse_wrapee = env->OnUnsolicitedResponse;
	RequestTimedCallback_wrapee = env->RequestTimedCallback;

	/* Set up new env to pass to RIL */
	RIL_Env_wrap.OnRequestComplete = OnRequestComplete_wrapper;
	RIL_Env_wrap.OnUnsolicitedResponse = OnUnsolicitedResponse_wrapper;
	RIL_Env_wrap.RequestTimedCallback = RequestTimedCallback_wrapper;

	RIL_RadioFunctions_orig = RIL_Init_wrap(&RIL_Env_wrap, argc, argv);

	/* Set up RadioFunctions wrapees */
	RIL_RequestFunc_wrapee = RIL_RadioFunctions_orig->onRequest;
	RIL_RadioStateRequest_wrapee = RIL_RadioFunctions_orig->onStateRequest;
	RIL_Supports_wrapee = RIL_RadioFunctions_orig->supports;
	RIL_Cancel_wrapee = RIL_RadioFunctions_orig->onCancel;
	RIL_GetVersion_wrapee = RIL_RadioFunctions_orig->getVersion;

	/* Set up new RadioFunctions to return */
	RIL_RadioFunctions_wrap.version = RIL_RadioFunctions_orig->version;
	RIL_RadioFunctions_wrap.onRequest = RIL_RequestFunc_wrapper;
	RIL_RadioFunctions_wrap.onStateRequest = RIL_RadioStateRequest_wrapper;
	RIL_RadioFunctions_wrap.supports = RIL_Supports_wrapper;
	RIL_RadioFunctions_wrap.onCancel = RIL_Cancel_wrapper;
	RIL_RadioFunctions_wrap.getVersion = RIL_GetVersion_wrapper;

	LOGV("RIL_Init wrapper returning!");

	return &RIL_RadioFunctions_wrap;
}
