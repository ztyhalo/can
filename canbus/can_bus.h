#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "can_relate.h"

#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "e_poll.h"
#include <sys/eventfd.h>
#include <bitset>
#include "timers.h"
#include "pro_data.h"
#include "zprint.h"
#include "zevent.h"


using namespace std;

#define CAN_DATA_LEN  8
#define CANBUS_TX_QSIZE 64
#define CANBUS_RX_QSIZE 64

#define CAN_MAX_NUM 1

#ifndef CAN_MAX_DLEN
#define CAN_MAX_DLEN 8
#endif


#define MESSAGE_LEN 8



typedef struct
{
    unsigned int StdId;		            //标准帧ID
    unsigned int ExtId;		            //扩展帧ID
    unsigned char IDE;		            //帧类型 0：标准帧 1：扩展帧
    unsigned char RTR;		            //数据类型 0：数据帧  1：远程帧
    unsigned char DLC;		            //数据长度
    unsigned char Data[CAN_DATA_LEN];	//数据组

}CANDATAFORM;


typedef struct
{
    unsigned int Baudrate;
}CANDEVPARA;


#define READBUF  Pth_Buf_T<CanFrame, CANBUS_TX_QSIZE, CanDriver>
#define WRITEBUF  Pth_Buf_T<CANDATAFORM, CANBUS_TX_QSIZE, CanDriver>

class CanDriver:public NCbk_Poll
{
public:
    int      interval;
    int      CanFileP;
    void *   father_p;
    READBUF  canread;
    WRITEBUF canwrite;
public:
    CanDriver(int fnum = 1):NCbk_Poll(fnum){
        father_p = NULL;
        interval = 5;
    }
    ~CanDriver(){
        zprintf3("destory CanDriver!\n");
        if(CanFileP > 0){
            shutdown(CanFileP, SHUT_RDWR);
            close(CanFileP);
            CanFileP = 0;
        }
    }

    int can_bus_init(int registerdev ,int brate = 100000);
    void run();
    int write_send_data(CANDATAFORM  & Msg);
    int writeframe(const CANDATAFORM& f);
    int writeframe(const CanFrame& f);

};


class Can_Bus:public FILE_POLL<CanFrame>
{
public:
    int                     canfile_fd;
    Z_EVENT<CanFrame, 8>    buf;
public:
    Can_Bus(int id = 0)
    {
        can_bus_init(id, 100000);
        this->f_bind(this, &Can_Bus::can_read_data);
    }
    int can_bus_init(int registerdev ,int brate = 100000);
    void can_read_data(CanFrame data);
    template<class SLOT_OWN>
    void buf_bind(SLOT_OWN* pObj, void (SLOT_OWN::*func)(CanFrame))
    {
        buf.f_bind(pObj, func);
    }
};

CANDATAFORM lawdata_to_prodata(const CanFrame & f);
#endif // CAN_BUS_H
