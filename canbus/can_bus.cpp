#include "can_bus.h"
#include <stdio.h>
#include "timers.h"
#include <sstream>


using namespace std;

int sigsize = 0;



//CanBusMag  CanBusMag::canpoint;



CanFrame prodata_to_lawdata(const CANDATAFORM & f)
{
    CanFrame cansend;
    memset(&cansend, 0 , sizeof(CanFrame));

    if(f.IDE)
    {
        cansend.can_id = (f.IDE<<31)|(f.RTR<<30)|(f.ExtId);
    }
    else
    {
        cansend.can_id = (f.RTR<<30)|(f.StdId);
    }

    cansend.can_dlc = f.DLC;

    memcpy(cansend.data, f.Data, cansend.can_dlc);

    return cansend;

}

CANDATAFORM lawdata_to_prodata(const CanFrame & f)
{
    CANDATAFORM rxcan;

    memset(&rxcan, 0x00, sizeof(CANDATAFORM));
    rxcan.IDE = (f.can_id>>31)&1;
    rxcan.RTR = (f.can_id>>30)&1;
    if(rxcan.IDE)
    {
        rxcan.ExtId = f.can_id&0x1FFFFFFF;
    }
    else
    {
       rxcan.StdId = f.can_id&0x7FF;
    }

    if(rxcan.RTR&1)
    {
       rxcan.DLC = 0;
    }
    else
    {
       rxcan.DLC = f.can_dlc;
       memcpy(rxcan.Data, f.data, rxcan.DLC);
    }
    return rxcan;
}

CANDATAFORM lawdata_to_prodata(CanFrame * f)
{
    CANDATAFORM rxcan;

    memset(&rxcan, 0x00, sizeof(CANDATAFORM));
    rxcan.IDE = (f->can_id>>31)&1;
    rxcan.RTR = (f->can_id>>30)&1;
    if(rxcan.IDE)
    {
        rxcan.ExtId = f->can_id&0x1FFFFFFF;
    }
    else
    {
       rxcan.StdId = f->can_id&0x7FF;
    }

    if(rxcan.RTR&1)
    {
       rxcan.DLC = 0;
    }
    else
    {
       rxcan.DLC = f->can_dlc;
       memcpy(rxcan.Data, f->data, rxcan.DLC);
    }
    return rxcan;
}


int  call_write_back(CanDriver * pro, CANDATAFORM data)
{

        pro->writeframe(data);
//        linuxDly(pro->interval);
        return 0;
}

 int CanDriver::can_bus_init(int registerdev, int brate)
 {
     int  ret;
     struct sockaddr_can addr;
     struct ifreq ifr;
     char canname[6];
     ostringstream canset;
//     canset << "/opt/canbrateset.sh can" << registerdev << " " << brate<< endl;

//     memset(canname, 0, sizeof(canname));

//     ret = system(canset.str().c_str());
//     if(ret != 0)
//     {
//         printf("can brate set fail!\n");
//         return -1;
//     }

     /* socketCAN连接 */
     CanFileP = socket(PF_CAN, SOCK_RAW, CAN_RAW);
     if(CanFileP < 0)
     {
         zprintf1("Socket PF_CAN failed!\n");
         return -1;
     }


     sprintf(canname, "can%d", registerdev);
     strcpy(ifr.ifr_name, canname);

     ret = ioctl(CanFileP, SIOCGIFINDEX, &ifr);
     if(ret<0)
     {
         zprintf1("Ioctl failed!\n");
         return -2;
     }

     addr.can_family = PF_CAN;
     addr.can_ifindex = ifr.ifr_ifindex;
     ret = ::bind(CanFileP, (struct sockaddr *)&addr, sizeof(addr));
     if(ret<0)
     {
         zprintf1("Bind failed!\n");
         return -3;
     }

     // 设置CAN滤波器
     struct can_filter rfilter[1];
     rfilter[0].can_id= 0x00;
     rfilter[0].can_mask = 0x00;

     ret = setsockopt(CanFileP, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
     if(ret < 0)
     {
         zprintf1("Set sockopt failed!\n");
         return -4;
     }

     e_poll_add(CanFileP);
     canwrite.z_pthread_init(call_write_back, this);
//     start();

     return 0;
 }

void CanDriver::run()
{
    zprintf3("can driver start\n");
    struct epoll_event events[get_epoll_size()];
    char buf[sizeof(CanFrame)];

    for (;  ; )
    {
        memset(&events, 0, sizeof(events));
        if(wait_fd_change(5000) != -1)
        {
//            printf("receive can frame\n");
           while(read(CanFileP, buf,sizeof(CanFrame)) == sizeof(CanFrame))
           {
               canread.buf_write_data((CanFrame*)buf);
           }
        }
        else
        {
            zprintf1("wati over!\n");
        }
    }
}

int CanDriver::writeframe(const CanFrame& f)
{
    int nbytes;
    int retry_times = 10;

    int total_write = sizeof(CanFrame);


    while(retry_times)
    {
        nbytes = ::write(CanFileP, &f, total_write);
        if(nbytes==total_write)
        {
//           timeprf("send successful %d!\n", f.can_dlc);
//            unsigned int midid = f.can_id&0x7FF;
//            if(midid == 0x421)
//            {
//            struct timeval tv;
//            gettimeofday(&tv, NULL);
//            printf("can bus tv_sec; %d tv usec %d\n", (int)tv.tv_sec, (int)tv.tv_usec);
//            }
           break;
        }

        if(nbytes>0)
        {
            retry_times--;
            linuxDly(2);
             continue;
        }
        if(nbytes<0)
        {
//            printf("can bus write error %d!\n",errno);
//            printf("%s\n",strerror(errno));
            linuxDly(2);
            retry_times--;
            continue;
        }
    }
    if(retry_times==0)
    {
           printf("can bus may be full!");
    }
    return nbytes;
}

int CanDriver::writeframe(const CANDATAFORM& f)
{
    return writeframe(prodata_to_lawdata(f));
}

int CanDriver::write_send_data(CANDATAFORM  & Msg)
{
    canwrite.buf_write_data(Msg);
    return 1;
}



int Can_Bus::can_bus_init(int canid, int brate)
{
    int  ret;
    struct sockaddr_can addr;
    struct ifreq ifr;
    char canname[6];
//    ostringstream canset;
//     canset << "/opt/canbrateset.sh can" << registerdev << " " << brate<< endl;

//     memset(canname, 0, sizeof(canname));

//     ret = system(canset.str().c_str());
//     if(ret != 0)
//     {
//         printf("can brate set fail!\n");
//         return -1;
//     }

    /* socketCAN连接 */
    canfile_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if(canfile_fd < 0)
    {
        zprintf1("Socket PF_CAN failed!\n");
        return -1;
    }


    sprintf(canname, "can%d", canid);
    strcpy(ifr.ifr_name, canname);

    ret = ioctl(canfile_fd, SIOCGIFINDEX, &ifr);
    if(ret<0)
    {
        zprintf1("Ioctl failed!\n");
        return -2;
    }

    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = ::bind(canfile_fd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret<0)
    {
        zprintf1("Bind failed!\n");
        return -3;
    }

    // 设置CAN滤波器
    struct can_filter rfilter[1];
    rfilter[0].can_id= 0x00;
    rfilter[0].can_mask = 0x00;

    ret = setsockopt(canfile_fd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    if(ret < 0)
    {
        zprintf1("Set sockopt failed!\n");
        return -4;
    }

    e_poll_add(canfile_fd);
    return 0;
}


 void Can_Bus::can_read_data(CanFrame data)
 {
//     zprintf3("can read 0x%x\n", data.can_id);
     buf.send_msg(data);
 }


