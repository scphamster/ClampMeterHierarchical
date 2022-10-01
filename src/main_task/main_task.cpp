//
// Created by scpha on 30/09/2022.
//

#ifdef min
#undef min
#endif   // max
#ifdef max
#undef max
#endif   // max
#ifdef printf
#undef printf
#endif
#include <asf.h>
#include "system_init.h"
#include "DSP_functions.h"
#include "MCP23016.h"
#include "signal_conditioning.h"
#include "ILI9486_public.h"
#include "menu_ili9486_kbrd_mngr.h"
#include "menu_ili9486.h"

#include "hclamp_meter.hpp"
#include "main_task.hpp"
#include "clamp_sensor.hpp"

#include "FreeRTOS.h"
#include "task.h"

using Drawer             = ClampMeterDrawer<ILIDrawer>;
using Clamp              = ClampMeter<Drawer, ClampSensor>;
using ClampMeterFreeRTOS = ClampMeterInTaskHandler<Drawer, ClampSensor>;

void
tasks_setup2()
{
    static volatile auto clamp_meter = Clamp{ 56 };
    clamp_meter.StartMeasurementsTask();
    clamp_meter.StartDisplayMeasurementsTask();

    vTaskStartScheduler();

    while (true) { }
}

extern "C" void
ClampMeterMeasurementsTaskWrapper(void *ClampMeterInstance)
{
    auto clmp = static_cast<Clamp *>(ClampMeterInstance);
        auto clamp_meter = ClampMeterFreeRTOS{ *clmp };

    while (true) {
//        clmp->MeasurementsTask();
clamp_meter.MeasurementsTask();
    }
}

extern "C" void
ClampMeterDisplayMeasurementsTaskWrapper(void *ClampMeterInstance)
{
    auto clmp = static_cast<Clamp *>(ClampMeterInstance);
        auto clamp_meter = ClampMeterFreeRTOS{ *clmp };

    while (true) {
//        clmp->DisplayMeasurementsTask();
    clamp_meter.DisplayMeasurementsTask();
    }
}
