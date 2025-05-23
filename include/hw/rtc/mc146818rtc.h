/*
 * QEMU MC146818 RTC emulation
 *
 * Copyright (c) 2003-2004 Fabrice Bellard
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HW_RTC_MC146818RTC_H
#define HW_RTC_MC146818RTC_H

#include "qapi/qapi-types-machine.h"
#include "qemu/queue.h"
#include "qemu/timer.h"
#include "hw/isa/isa.h"
#include "qom/object.h"

#define TYPE_MC146818_RTC "mc146818rtc"
OBJECT_DECLARE_SIMPLE_TYPE(MC146818RtcState, MC146818_RTC)

struct MC146818RtcState {
    ISADevice parent_obj;

    MemoryRegion io;
    MemoryRegion coalesced_io;
    uint8_t cmos_data[256];
    uint8_t cmos_index;
    uint8_t isairq;
    uint16_t io_base;
    int32_t base_year;
    uint64_t base_rtc;
    uint64_t last_update;
    int64_t offset;
    qemu_irq irq;
    int it_shift;
    /* periodic timer */
    QEMUTimer *periodic_timer;
    int64_t next_periodic_time;
    /* update-ended timer */
    QEMUTimer *update_timer;
    uint64_t next_alarm_time;
    uint16_t irq_reinject_on_ack_count;
    uint32_t irq_coalesced;
    uint32_t period;
    QEMUTimer *coalesced_timer;
    Notifier clock_reset_notifier;
    LostTickPolicy lost_tick_policy;
    Notifier suspend_notifier;
    QLIST_ENTRY(MC146818RtcState) link;
};

#define RTC_ISA_IRQ 8

MC146818RtcState *mc146818_rtc_init(ISABus *bus, int base_year,
                                    qemu_irq intercept_irq);
void mc146818rtc_set_cmos_data(MC146818RtcState *s, int addr, int val);
int mc146818rtc_get_cmos_data(MC146818RtcState *s, int addr);
void rtc_reset_reinjection(MC146818RtcState *rtc);

#endif /* HW_RTC_MC146818RTC_H */
