#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "modulesdk.h"
#include <stdio.h>
#include <netinet/in.h>
#include "xop/RtmpServer.h"
#include "xop/HttpFlvServer.h"
#include "xop/RtmpPublisher.h"
#include "xop/RtmpClient.h"
#include "xop/HttpFlvServer.h"
#include "xop/H264Parser.h"
#include "net/EventLoop.h"
#include "cJSON.h"
#include "log.h"
// ServeInvoke主要是解析来自客户端的请求，解析其中的参数，然后根据参数连接rtmp服务器，若此处需要做一个重定向，可以获取rtmp的连接地址，然后做一个更改
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
const char*conf_path="conf/rtmp_config.json";

void *run(void *adsfjiaf)
{
    //加载配置文件
    std::string rtmpaddress, httpaddress;
    int rtmpport, httpport, enablehttp;
    if (access(conf_path, F_OK))
    {
        extrapidLog(LOG_WARN, "RTMP", "未找到配置文件，正在生成默认配置文件");
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "RtmpBindAddress", "0.0.0.0");
        cJSON_AddNumberToObject(json, "RtmpPort", 1935);
        cJSON_AddBoolToObject(json, "EnableHttpFlv", cJSON_True);
        cJSON_AddStringToObject(json, "HttpFlvAddress", "0.0.0.0");
        cJSON_AddNumberToObject(json, "HttpFlvPort", 1936);
        char *p = cJSON_Print(json);
        FILE *fp = fopen(conf_path, "w");
        if (fp == NULL)
        {
            extrapidLog(LOG_ERROR, "RTMP", "无法保存配置文件!RTMP模块启动失败");
            free(p);
            cJSON_Delete(json);
            return NULL;
        }
        fprintf(fp, "%s", p);
        fclose(fp);
        cJSON_Delete(json);
        free(p);
        extrapidLog(LOG_INFO, "RTMP", "配置文件%s保存成功",conf_path);
        rtmpaddress = "0.0.0.0";
        rtmpport = 1935;
        enablehttp = 1;
        httpaddress = "0.0.0.0";
        httpport = 1936;
    }
    else
    {
        FILE *fp = fopen(conf_path, "r");
        if (fp == NULL)
        {
            extrapidLog(LOG_ERROR, "RTMP", "配置文件%s加载失败:%s,RTMP模块启动失败", conf_path,strerror(errno));
            return NULL;
        }
        char *jsondata = (char *)malloc(1024 * 2);
        fread(jsondata, 1, 1024 * 2, fp);
        fclose(fp);
        cJSON *json = cJSON_Parse(jsondata);
        free(jsondata);
        if (json == NULL)
        {
            extrapidLog(LOG_ERROR, "RTMP", "%s加载失败:%s,RTMP模块启动失败",conf_path, cJSON_GetErrorPtr());
            return NULL;
        }
        cJSON *temp = cJSON_GetObjectItem(json, "RtmpBindAddress");
        if (temp == NULL || temp->type != cJSON_String || temp->valuestring == NULL)
        {
            extrapidLog(LOG_ERROR, "RTMP", "%s加载失败,RtmpBindAddress错误,RTMP模块启动失败",conf_path);
            cJSON_Delete(json);
            return NULL;
        }
        rtmpaddress = temp->valuestring;
        temp = cJSON_GetObjectItem(json, "RtmpPort");
        if (temp == NULL || temp->type != cJSON_Number)
        {
            extrapidLog(LOG_ERROR, "RTMP", "%s加载失败,RtmpPort错误,RTMP模块启动失败",conf_path);
            cJSON_Delete(json);
            return NULL;
        }
        rtmpport = temp->valueint;
        temp = cJSON_GetObjectItem(json, "EnableHttpFlv");
        if (temp == NULL)
        {
            extrapidLog(LOG_ERROR, "RTMP", "%s加载失败,EnableHttpFlv错误,RTMP模块启动失败",conf_path);
            cJSON_Delete(json);
            return NULL;
        }
        enablehttp = temp->valueint;
        temp = cJSON_GetObjectItem(json, "HttpFlvAddress");
        if (temp == NULL || temp->type != cJSON_String)
        {
            extrapidLog(LOG_ERROR, "RTMP", "%s加载失败,HttpFlvAddress错误,RTMP模块启动失败",conf_path);
            cJSON_Delete(json);
            return NULL;
        }
        httpaddress = temp->valuestring;
        temp = cJSON_GetObjectItem(json, "HttpFlvPort");
        if (temp == NULL || temp->type != cJSON_Number)
        {
            extrapidLog(LOG_ERROR, "RTMP", "%s加载失败,HttpFlvPort错误,RTMP模块启动失败",conf_path);
            cJSON_Delete(json);
            return NULL;
        }
        httpport = temp->valueint;
        cJSON_Delete(json);
    }
    int count = 1;
    xop::EventLoop event_loop(count);

    /* rtmp server example */
    auto rtmp_server = xop::RtmpServer::Create(&event_loop);
    rtmp_server->SetChunkSize(60000);
    rtmp_server->SetEventCallback([](std::string type, std::string stream_path)
                                  { extrapidLog(LOG_INFO, "RTMP", "事件:%s,流:%s", type.c_str(), stream_path.c_str()); });
    if (!rtmp_server->Start(rtmpaddress, rtmpport))
    {
        extrapidLog(LOG_ERROR, "RTMP", "RTMP服务创建失败");
    }
    if (enablehttp == 1)
    {
        xop::HttpFlvServer*http_flv_server=new xop::HttpFlvServer;
        http_flv_server->Attach(rtmp_server);

        if (!http_flv_server->Start(httpaddress, httpport))
        {
            extrapidLog(LOG_ERROR, "RTMP", "HttpFlv服务创建失败");
        }
    }
    else
    {
        extrapidLog(LOG_INFO, "RTMP", "HttpFlv服务已禁用");
    }
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);
    pthread_mutex_lock(&lock);
    return NULL;
}
extern "C"
{
    void thread()
    {
        pthread_t tid;
        pthread_create(&tid, NULL, run, NULL);
        pthread_detach(tid);
    }
    ModuleType_t _init_(int i)
    {
        logInit("logs/rtmp");
        ModuleType_t mod = SDK_CreateModule("rtmp", "一个rtmp模块，原作者:https://github.com/PHZ76/rtmp", 1);
        SDK_AddFunction(&mod, RUN_AT_START, (void *)(thread), 10);
        return mod;
    }
}
