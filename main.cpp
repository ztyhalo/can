#include <QCoreApplication>
#include "canbus/can_bus.h"
#include "zevent.h"

class READ_CAN
{
public:
    void zprintf_can(CanFrame val)
    {
        zprintf1("can id 0x%x\n", val.can_id);
    }
};

Can_Bus mcan(0);

READ_CAN rcan;

//Z_EVENT tes;

int main(int argc, char *argv[])
{

//    mcan.f_bind(&rcan, &READ_CAN::zprintf_can);
//    mcan.f_bind(&mcan, &Can_Bus::can_read_data);
    mcan.buf_bind(&rcan, &READ_CAN::zprintf_can);
    while (1) {
       sleep(4);
    }
}
