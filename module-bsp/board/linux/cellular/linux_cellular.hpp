
/*
 * @file linux_cellular.hpp
 * @author Mateusz Piesta (mateusz.piesta@mudita.com)
 * @date 14.06.19
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */


#ifndef PUREPHONE_LINUX_CELLULAR_HPP
#define PUREPHONE_LINUX_CELLULAR_HPP

#include "cellular/bsp_cellular.hpp"


#include <cstring>
#include <sys/epoll.h>
#include "termios.h"
#include <sys/ioctl.h>


namespace bsp {

    class LinuxCellular: public Cellular  {

    public:

        LinuxCellular(const char* term);
        ~LinuxCellular();

        void PowerUp() override final;

        void PowerDown() override final;

        uint32_t Wait(uint32_t timeout) override final;

        ssize_t Read(void *buf, size_t nbytes) override final;

        uint32_t Write(void *buf, size_t nbytes) override final;

    private:

        int set_interface_attribs();

        void set_mincount(int mcount);


        static constexpr speed_t baud_bits[] = {
                0, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600, B1500000, B2000000, B3000000, B4000000
        };

        static const uint32_t portBaudRate = 115200;

        static const uint32_t MAX_EVENTS = 1;

        int fd;

        int epoll_fd;

        struct epoll_event event, events[MAX_EVENTS];


    };

}


#endif //PUREPHONE_LINUX_CELLULAR_HPP
