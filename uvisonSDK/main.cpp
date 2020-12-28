#include <QCoreApplication>
#include <iostream>
#include "HCNetSDK.h"
using namespace std;




int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 初始化
    NET_DVR_Init();

    return a.exec();
}
