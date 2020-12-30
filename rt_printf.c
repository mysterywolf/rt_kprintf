/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-15     Meco Man     first version
 */

#include <rtthread.h>
#include <rthw.h>
#include <stdio.h>

#ifdef RT_USING_CONSOLE
/**
 * This function will print a formatted string on system console (thread safe)
 *
 * @param fmt the format
 */
void rt_printf(const char *fmt, ...)
{
    va_list args;
    rt_size_t length;
    rt_device_t console_dev;
    static char rt_console_buf[RT_CONSOLEBUF_SIZE];
#if defined RT_USING_MUTEX || defined RT_USING_SEMAPHORE
    static unsigned char kprintf_init_flag = RT_FALSE;
#if defined RT_USING_MUTEX
    static struct rt_mutex kprintf_mutex;
#elif defined RT_USING_SEMAPHORE
    static struct rt_semaphore kprintf_sem;
#endif /*defined RT_USING_MUTEX*/
#endif /*defined RT_USING_MUTEX || defined RT_USING_SEMAPHORE*/

    if(rt_interrupt_get_nest() == 0u && rt_thread_self() != RT_NULL)
    {
    #if defined RT_USING_MUTEX || defined RT_USING_SEMAPHORE
        if(kprintf_init_flag == RT_FALSE)
        {
        #if defined RT_USING_MUTEX
            rt_mutex_init(&kprintf_mutex, "kprintf", RT_IPC_FLAG_FIFO);
        #elif defined RT_USING_SEMAPHORE
            rt_sem_init(&kprintf_sem, "kprintf", 1, RT_IPC_FLAG_FIFO);
        #endif /*defined RT_USING_MUTEX*/
            kprintf_init_flag = RT_TRUE;
        }
    #if defined RT_USING_MUTEX
        rt_mutex_take(&kprintf_mutex, RT_WAITING_FOREVER);
    #elif defined RT_USING_SEMAPHORE
        rt_sem_take(&kprintf_sem, RT_WAITING_FOREVER);
    #endif /*defined RT_USING_MUTEX*/
    #else
        rt_enter_critical();
    #endif /*defined RT_USING_MUTEX || defined RT_USING_SEMAPHORE*/
    }

    console_dev = rt_console_get_device();

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_console_buf, we have to adjust the output
     * length. */
    length = vsnprintf(rt_console_buf, sizeof(rt_console_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
#ifdef RT_USING_DEVICE
    if (console_dev == RT_NULL)
    {
        rt_hw_console_output(rt_console_buf);
    }
    else
    {
        rt_uint16_t old_flag = console_dev->open_flag;

        console_dev->open_flag |= RT_DEVICE_FLAG_STREAM;
        rt_device_write(console_dev, 0, rt_console_buf, length);
        console_dev->open_flag = old_flag;
    }
#else
    rt_hw_console_output(rt_console_buf);
#endif /*RT_USING_DEVICE*/

    va_end(args);

    if(rt_interrupt_get_nest() == 0u &&
#if defined RT_USING_MUTEX || defined RT_USING_SEMAPHORE
        kprintf_init_flag == RT_TRUE &&
#endif
        rt_thread_self() != RT_NULL)
    {
    #if defined RT_USING_MUTEX || defined RT_USING_SEMAPHORE
    #if defined RT_USING_MUTEX
        rt_mutex_release(&kprintf_mutex);
    #elif defined RT_USING_SEMAPHORE
        rt_sem_release(&kprintf_sem);
    #endif /*defined RT_USING_MUTEX*/
    #else
        rt_exit_critical();
    #endif /*defined RT_USING_MUTEX || defined RT_USING_SEMAPHORE*/
    }
}
#endif /*RT_USING_CONSOLE*/