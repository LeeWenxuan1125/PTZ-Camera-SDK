#include <QCoreApplication>

#include "dhnetsdk.h"

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <QTime>
#include<ctime>

using namespace std;

static BOOL g_bNetSDKInitFlag = FALSE;
static LLONG g_lLoginHandle = 0;
static char g_szUserName[64] = "admin";
static char g_szPasswd[64] = "admin12345";
static WORD g_nPort = 80;
char g_szDevIp[64] = "192.168.1.4";

void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    printf("Call DisConnectFunc: lLoginID[0x%x]\n", lLoginID);
}

void CALLBACK HaveReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    printf("Call HaveReConnect\n");
    printf("lLoginID[0x%x]", lLoginID);
    if (NULL != pchDVRIP)
    {
        printf("pchDVRIP[%s]\n", pchDVRIP);
    }
    printf("nDVRPort[%d]\n", nDVRPort);
    printf("dwUser[%p]\n", dwUser);
    printf("\n");
}

// 云台状态回调函数
void CALLBACK PTZStatusCbFunc(LLONG lLoginID, LLONG lAttachHandle, void* pBuf, int nBufLen, LDWORD dwUser);

void RunTest()
{
    int	 nChoose  = 0;
    int  nChannel = 0;
    BOOL bRet = TRUE;
    std::cout << MAX_CATEGORY_LEN;
    cout<<"请选择需要的操作.............."<<endl;
    cout<<"1:直接获取云台状态"<<endl;
    cout<<"2:订阅方式获取云台状态(云台状态变化才会上报)"<<endl;
    cout<<"3:三维精确定位"<<endl;
    cout<<"4:设置绝对聚焦值"<<endl;
    cout<<"5:转动到阈值位"<<endl;
    cin >> nChoose;

    if (1 == nChoose)
    {
        DH_PTZ_LOCATION_INFO *stLocationInfo = new DH_PTZ_LOCATION_INFO();
        memset(stLocationInfo, 0, sizeof(*stLocationInfo));
        cout<<"请输入通道号.............."<<endl;
        cin>> stLocationInfo->nChannelID;
        int nRetLen = 0;

        bRet = CLIENT_QueryDevState(g_lLoginHandle, DH_DEVSTATE_PTZ_LOCATION,(char *)stLocationInfo,sizeof(DH_PTZ_LOCATION_INFO),&nRetLen,3000);

        if(TRUE != bRet)
        {
            cout << "get failed" << endl;
        }
        else
        {
            cout << "get succeed" << endl;

            cout<<"水平位置【0~3600】:"<<stLocationInfo->nPTZPan<<endl;
            cout<<"垂直位置【-1800~1800】:"<<stLocationInfo->nPTZTilt<<endl;
            cout<<"倍率值【0,128】:"<<stLocationInfo->nPTZZoom<<endl;
            cout<<"聚焦位置为:"<<stLocationInfo->fFocusPosition<<endl;
            cout<<"真实变倍率:"<<stLocationInfo->nZoomValue<<endl;
            cout<<"阈值点位:" <<stLocationInfo->dwPresetID<<endl;

            if(stLocationInfo->bState == 0)
                cout<<"云台处于:未知状态"<<endl;
            else if(stLocationInfo->bState == 1)
                cout<<"云台处于:运动状态"<<endl;
            else if(stLocationInfo->bState == 2)
               cout<<"云台处于:空闲状态"<<endl;

        }
        delete stLocationInfo;

    }
    else if (2 == nChoose)
    {
        NET_IN_PTZ_STATUS_PROC inParams;
        inParams.dwSize = sizeof(NET_IN_PTZ_STATUS_PROC);
        inParams.cbPTZStatusProc = PTZStatusCbFunc;
        inParams.dwUser = 0;
        cout<<"请输入通道号.............."<<endl;
        cin>> inParams.nChannel;

        NET_OUT_PTZ_STATUS_PROC outParams;
        outParams.dwSize = sizeof(NET_OUT_PTZ_STATUS_PROC);

        LLONG attachHandle = CLIENT_AttachPTZStatusProc(g_lLoginHandle, &inParams,  &outParams, 3000);
        if(attachHandle)
        {
            cout<<"attach success."<<endl;
        }
        else
        {
            cout<<"attach failed."<<endl;
        }
    }
    else if (3 == nChoose)
    {
        int parm1 = 0;
        int parm2 = 0;
        int parm3 = 0;
        int parm4 = 0;

        cout << "parm1(0~3600 水平):" ;
        cin >> parm1;
        cout << "parm2(0~900 垂直):" ;
        cin >> parm2;
        cout << "parm3(1~128 倍数):" ;
        cin >> parm3;

        CLIENT_DHPTZControlEx2(g_lLoginHandle,0,DH_EXTPTZ_EXACTGOTO,parm1,parm2,parm3,FALSE,(void*)&parm4);
    }
    else if (4 == nChoose)
    {
        int parm1 = 0;
        int parm2 = 0;
        int parm3 = 0;

        PTZ_FOCUS_ABSOLUTELY parm4= {0};

        cout << "dwValue(0~8191 聚焦位置):" ;
        cin >> parm4.dwValue;
        parm4.dwSpeed = 7;

        CLIENT_DHPTZControlEx2(g_lLoginHandle,0,DH_EXTPTZ_FOCUS_ABSOLUTELY,parm1,parm2,parm3,FALSE,(void*)&parm4);
    }
    else if(5 == nChoose)
    {
        int parm1 = 0;
        int parm2 = 1;
        int parm3 = 0;
        int parm4 = 0;
        cout << "阈值点位置:" ;
        cin >> parm2;

        CLIENT_DHPTZControlEx2(g_lLoginHandle,0,DH_PTZ_POINT_MOVE_CONTROL,parm1,parm2,parm3,FALSE,(void*)&parm4);
    }

    getchar();


}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // 初始化SDK
    g_bNetSDKInitFlag = CLIENT_Init(DisConnectFunc, 0);
    if (FALSE == g_bNetSDKInitFlag)
    {

        printf("Initialize client SDK fail; \n");
        return a.exec();
    }
    else
    {
        printf("Initialize client SDK done; \n");
    }

    // 设置断线重连回调接口，设置过断线重连成功回调函数后，当设备出现断线情况，SDK内部会自动进行重连操作
    // 此操作为可选操作，但建议用户进行设置
    CLIENT_SetAutoReconnect(&HaveReConnect, 0);

    NET_DEVICEINFO_Ex stDevInfoEx = {0};
    int nError = 0;

    g_lLoginHandle = CLIENT_LoginEx2(g_szDevIp, g_nPort, g_szUserName, g_szPasswd, EM_LOGIN_SPEC_CAP_TCP, NULL, &stDevInfoEx, &nError);

    if(0 == g_lLoginHandle)
    {
       // 根据错误码，可以在dhnetsdk.h中找到相应的解释，此处打印的是16进制，头文件中是十进制，其中的转换需注意
       // 例如：
       // #define NET_NOT_SUPPORTED_EC(23) // 当前SDK未支持该功能，对应的错误码为 0x80000017, 23对应的16进制为0x17
        printf("CLIENT_LoginEx2 %s[%d]Failed!Last Error[%x]\n" , g_szDevIp , g_nPort , CLIENT_GetLastError());
    }
    else
    {
        printf("CLIENT_LoginEx2 %s[%d] Success\n" , g_szDevIp , g_nPort);
        while(true)
        {
            RunTest();
        }
    }
    return a.exec();
}

void CALLBACK PTZStatusCbFunc(LLONG lLoginID, LLONG lAttachHandle, void* pBuf, int nBufLen, LDWORD dwUser)
{
    DH_PTZ_LOCATION_INFO *ptzInfo = NULL;
    if (nBufLen > 0)
    {
        ptzInfo = (DH_PTZ_LOCATION_INFO*)pBuf;
        cout<<"水平位置【0~3600】:"<<ptzInfo->nPTZPan<<endl;
        cout<<"垂直位置【-1800~1800】:"<<ptzInfo->nPTZTilt<<endl;

        if(ptzInfo->bState == 0)
            cout<<"云台处于:未知状态"<<endl;
        else if(ptzInfo->bState == 1)
            cout<<"云台处于:运动状态"<<endl;
        else if(ptzInfo->bState == 2)
            cout<<"云台处于:空闲状态"<<endl;
    }
}
