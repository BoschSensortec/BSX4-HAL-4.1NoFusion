/*!
 * @section LICENSE
 *
 * (C) Copyright 2011~2015 Bosch Sensortec GmbH All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *------------------------------------------------------------------------------
 *  Disclaimer
 *
 * Common: Bosch Sensortec products are developed for the consumer goods
 * industry. They may only be used within the parameters of the respective valid
 * product data sheet.  Bosch Sensortec products are provided with the express
 * understanding that there is no warranty of fitness for a particular purpose.
 * They are not fit for use in life-sustaining, safety or security sensitive
 * systems or any system or device that may lead to bodily harm or property
 * damage if the system or device malfunctions. In addition, Bosch Sensortec
 * products are not fit for use in products which interact with motor vehicle
 * systems.  The resale and/or use of products are at the purchaser's own risk
 * and his own responsibility. The examination of fitness for the intended use
 * is the sole responsibility of the Purchaser.
 *
 * The purchaser shall indemnify Bosch Sensortec from all third party claims,
 * including any claims for incidental, or consequential damages, arising from
 * any product use not covered by the parameters of the respective valid product
 * data sheet or not approved by Bosch Sensortec and reimburse Bosch Sensortec
 * for all costs in connection with such claims.
 *
 * The purchaser must monitor the market for the purchased products,
 * particularly with regard to product safety and inform Bosch Sensortec without
 * delay of all security relevant incidents.
 *
 * Engineering Samples are marked with an asterisk (*) or (e). Samples may vary
 * from the valid technical specifications of the product series. They are
 * therefore not intended or fit for resale to third parties or for use in end
 * products. Their sole purpose is internal client testing. The testing of an
 * engineering sample may in no way replace the testing of a product series.
 * Bosch Sensortec assumes no liability for the use of engineering samples. By
 * accepting the engineering samples, the Purchaser agrees to indemnify Bosch
 * Sensortec from all claims arising from the use of engineering samples.
 *
 * Special: This software module (hereinafter called "Software") and any
 * information on application-sheets (hereinafter called "Information") is
 * provided free of charge for the sole purpose to support your application
 * work. The Software and Information is subject to the following terms and
 * conditions:
 *
 * The Software is specifically designed for the exclusive use for Bosch
 * Sensortec products by personnel who have special experience and training. Do
 * not use this Software if you do not have the proper experience or training.
 *
 * This Software package is provided `` as is `` and without any expressed or
 * implied warranties, including without limitation, the implied warranties of
 * merchantability and fitness for a particular purpose.
 *
 * Bosch Sensortec and their representatives and agents deny any liability for
 * the functional impairment of this Software in terms of fitness, performance
 * and safety. Bosch Sensortec and their representatives and agents shall not be
 * liable for any direct or indirect damages or injury, except as otherwise
 * stipulated in mandatory applicable law.
 *
 * The Information provided is believed to be accurate and reliable. Bosch
 * Sensortec assumes no responsibility for the consequences of use of such
 * Information nor for any infringement of patents or other rights of third
 * parties which may result from its use.
 *
 * @file         sensord_hwcntl_implement.cpp
 * @date         "Fri Feb 5 15:40:38 2016 +0800"
 * @commit       "666efb6"
 *
 * @brief
 *
 * @detail
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <fcntl.h>
#include <dirent.h>

#if !defined(PLTF_LINUX_ENABLED)
#include<android/log.h>
/*Android utils headers*/
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#endif

#include "BstSensor.h"

#include "axis_remap.h"
#include "sensord_hwcntl.h"
#include "sensord_pltf.h"
#include "sensord_cfg.h"
#include "sensord_algo.h"
#include "util_misc.h"
#include "sensord_hwcntl_iio.h"

/* input event definition
struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

EV_SYN            0x00 EV_KEY            0x01 EV_REL            0x02
EV_ABS            0x03 EV_MSC            0x04 EV_SW            0x05 EV_LED            0x11
EV_SND            0x12 EV_REP            0x14 EV_FF            0x15 EV_PWR            0x16
EV_FF_STATUS        0x17 EV_MAX            0x1f EV_CNT            (EV_MAX+1)
*/


#define FIFO_HEAD_A        0x84
#define FIFO_HEAD_G        0x88
#define FIFO_HEAD_M        0x90

#define BMI160_ACCEL_ODR_RESERVED       0x00
#define BMI160_ACCEL_ODR_0_78HZ         0x01
#define BMI160_ACCEL_ODR_1_56HZ         0x02
#define BMI160_ACCEL_ODR_3_12HZ         0x03
#define BMI160_ACCEL_ODR_6_25HZ         0x04
#define BMI160_ACCEL_ODR_12_5HZ         0x05
#define BMI160_ACCEL_ODR_25HZ           0x06
#define BMI160_ACCEL_ODR_50HZ           0x07
#define BMI160_ACCEL_ODR_100HZ          0x08
#define BMI160_ACCEL_ODR_200HZ          0x09
#define BMI160_ACCEL_ODR_400HZ          0x0A
#define BMI160_ACCEL_ODR_800HZ          0x0B
#define BMI160_ACCEL_ODR_1600HZ         0x0C

/* BMI160 Gyro ODR */
#define BMI160_GYRO_ODR_RESERVED   0x00
#define BMI160_GYRO_ODR_25HZ   0x06
#define BMI160_GYRO_ODR_50HZ   0x07
#define BMI160_GYRO_ODR_100HZ   0x08
#define BMI160_GYRO_ODR_200HZ   0x09
#define BMI160_GYRO_ODR_400HZ   0x0A
#define BMI160_GYRO_ODR_800HZ   0x0B
#define BMI160_GYRO_ODR_1600HZ   0x0C
#define BMI160_GYRO_ODR_3200HZ   0x0D

/* BMI160 Mag ODR */
#define BMI160_MAG_ODR_RESERVED       0x00
#define BMI160_MAG_ODR_0_78HZ         0x01
#define BMI160_MAG_ODR_1_56HZ         0x02
#define BMI160_MAG_ODR_3_12HZ         0x03
#define BMI160_MAG_ODR_6_25HZ         0x04
#define BMI160_MAG_ODR_12_5HZ         0x05
#define BMI160_MAG_ODR_25HZ           0x06
#define BMI160_MAG_ODR_50HZ           0x07
#define BMI160_MAG_ODR_100HZ          0x08
#define BMI160_MAG_ODR_200HZ          0x09
#define BMI160_MAG_ODR_400HZ          0x0A
#define BMI160_MAG_ODR_800HZ          0x0B
#define BMI160_MAG_ODR_1600HZ         0x0C

/**for BMA2x2, ODR = Filter Bandwidth x 2*/
#define BMA2X2_BW_7_81HZ        0x08
#define BMA2X2_BW_15_63HZ       0x09
#define BMA2X2_BW_31_25HZ       0x0A
#define BMA2X2_BW_62_50HZ       0x0B
#define BMA2X2_BW_125HZ         0x0C
#define BMA2X2_BW_250HZ         0x0D
#define BMA2X2_BW_500HZ         0x0E
#define BMA2X2_BW_1000HZ        0x0F
#define BMA2X2_ODR_15_63HZ       BMA2X2_BW_7_81HZ
#define BMA2X2_ODR_31_25HZ       BMA2X2_BW_15_63HZ
#define BMA2X2_ODR_62_50HZ       BMA2X2_BW_31_25HZ
#define BMA2X2_ODR_125HZ         BMA2X2_BW_62_50HZ
#define BMA2X2_ODR_250HZ         BMA2X2_BW_125HZ
#define BMA2X2_ODR_500HZ         BMA2X2_BW_250HZ
#define BMA2X2_ODR_1000HZ        BMA2X2_BW_500HZ
#define BMA2X2_ODR_2000HZ        BMA2X2_BW_1000HZ

/**for BMG160, ODR mapped to Filter Bandwidth according to this table
 * ODR_Hz      Bandwidth
 *  100         32Hz
 *  200         64Hz
 *  100         12Hz    --- optimal for 100Hz
 *  200         23Hz    --- optimal for 200Hz
 *  400         47Hz
 *  1000        116Hz
 *  2000        230Hz   --- optimal for 2000Hz
 *  2000        523Hz(Unfiltered)*/
#define BMG160_BW_12HZ          0x05
#define BMG160_BW_23HZ          0x04
#define BMG160_BW_32HZ          0x07
#define BMG160_BW_47HZ          0x03
#define BMG160_BW_64HZ          0x06
#define BMG160_BW_116HZ         0x02
#define BMG160_BW_230HZ         0x01
#define BMG160_BW_523HZ         0x00
#define BMG160_ODR_100HZ        BMG160_BW_12HZ
#define BMG160_ODR_200HZ        BMG160_BW_23HZ
#define BMG160_ODR_400HZ        BMG160_BW_47HZ
#define BMG160_ODR_1000HZ       BMG160_BW_116HZ
#define BMG160_ODR_2000HZ       BMG160_BW_230HZ

#define BMI160_ACCEL_RANGE_2G   3
#define BMI160_ACCEL_RANGE_4G   5
#define BMI160_ACCEL_RANGE_8G   8
#define BMI160_ACCEL_RANGE_16G  12

#define BMA2X2_RANGE_2G     3
#define BMA2X2_RANGE_4G     5
#define BMA2X2_RANGE_8G     8
#define BMA2X2_RANGE_16G    12

#define BMA2x2_FIFO_PASSBY  0
#define BMA2x2_FIFO_STREAM  2
#define BMG160_FIFO_PASSBY  0
#define BMG160_FIFO_STREAM  2

#define BMA222E_ADC_BITS    8
#define BMA250E_ADC_BITS    10
#define BMA255_ADC_BITS     12
#define BMA280_ADC_BITS     14
#define BMM_COMPVAL_TO_uT (16) //bmm compensation value need divide 16 to become uT
#define AKM09912_COMPVAL_TO_uT (0.15) //AKM09912 output is in unit of 0.15uT
#define AKM09911_COMPVAL_TO_uT (0.6) //AKM09911 output is in unit of 0.6uT
#define YAS5xx_COMPVAL_TO_uT (0.001) //YAS5xx output is in unit of 0.001uT

#define EVENT_CODE_X_VALUE      0x02
#define EVENT_CODE_Y_VALUE      0x03
#define EVENT_CODE_Z_VALUE      0x04

static char iio_dev0_dir_name[128] = { 0 };

/**
 * test by practice, if IIO buffer is set too small, data will loose
 * now hwdata_unit_toread = 10 and default_watermark = 10 work well on 200Hz
 */

static uint32_t hwdata_unit_toread = 10;
static uint32_t default_watermark = 10;

static int32_t poll_timer_fd = -1;
static int32_t accl_scan_size, gyro_scan_size, magn_scan_size;
static int32_t accl_iio_fd = -1;
static int32_t gyro_iio_fd = -1;
static int32_t magn_iio_fd = -1;
static int acc_input_fd = -1;
static int acc_input_num = 0;
static int magn_input_fd = -1;
static int magn_input_num = 0;
static int gyr_input_fd = -1;
static int gyr_input_num = 0;

static char mag_input_dir_name[128] = {0};
static char acc_input_dir_name[128] = {0};
static char gyr_input_dir_name[128] = {0};

static float BMI160_acc_resl = 0.061; //16bit ADC, default range +-2000 mg. algorithm input requires "mg"
static float BMA255_acc_resl = 0.97656; //12bit ADC, default range +-2000 mg. algorithm input requires "mg"

/**
 *
 * @param p_sSensorList
 * @return
 */
uint32_t ap_get_sensorlist(struct sensor_t const** p_sSensorList)
{
    uint64_t avail_sens_regval = 0;
    uint32_t sensor_amount = 0;
    int32_t i;
    int32_t j;

    if (0 == bst_sensorlist.list_len)
    {
        switch(accl_range){
            case ACC_CHIP_RANGCONF_2G:
            case ACC_CHIP_RANGCONF_4G:
            case ACC_CHIP_RANGCONF_8G:
            case ACC_CHIP_RANGCONF_16G:
                bst_all_sensors[SENSORLIST_INX_ACCELEROMETER].maxRange = accl_range * GRAVITY_EARTH;
                bst_all_sensors[SENSORLIST_INX_LINEAR_ACCELERATION].maxRange = accl_range * GRAVITY_EARTH;
                bst_all_sensors[SENSORLIST_INX_GRAVITY].maxRange = accl_range * GRAVITY_EARTH;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_ACCELEROMETER].maxRange = accl_range * GRAVITY_EARTH;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_LINEAR_ACCELERATION].maxRange = accl_range * GRAVITY_EARTH;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_GRAVITY].maxRange = accl_range * GRAVITY_EARTH;
                break;
            default:
                PWARN("Invalid accl_range: %d", accl_range);
                break;
        }

        if(MAG_CHIP_AKM09912 ==  magn_chip || MAG_CHIP_AKM09911 ==  magn_chip)
        {
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].name = "AKM Magnetic Field Sensor";
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].vendor = "AKM";
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].maxRange = 4900.0f;
            if(MAG_CHIP_AKM09912 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].power = 1.0f;
            }else if(MAG_CHIP_AKM09911 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].resolution = 0.6f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].power = 2.4f;
            }
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].name = "AKM Magnetic Field Uncalibrated Sensor";
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].vendor = "AKM";
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].maxRange = 4900.0f;
            if(MAG_CHIP_AKM09912 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].power = 1.0f;
            }else if(MAG_CHIP_AKM09911 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.6f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].power = 2.4f;
            }

            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].name = "AKM Magnetic Field (Wakeup) Sensor";
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].vendor = "AKM";
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].maxRange = 4900.0f;
            if(MAG_CHIP_AKM09912 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].power = 1.0f;
            }else if(MAG_CHIP_AKM09911 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].resolution = 0.6f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].power = 2.4f;
            }
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].name = "AKM Magnetic Field Uncalibrated (Wakeup) Sensor";
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].vendor = "AKM";
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].maxRange = 4900.0f;
            if(MAG_CHIP_AKM09912 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].power = 1.0f;
            }else if(MAG_CHIP_AKM09911 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.6f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].power = 2.4f;
            }
        }

        if(MAG_CHIP_YAS537 == magn_chip || MAG_CHIP_YAS532 == magn_chip)
        {
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].name = "YAS Magnetic Field Sensor";
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].vendor = "YAS";
            if(MAG_CHIP_YAS537 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].maxRange = 2000.0f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].resolution = 0.3f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].power = 1.8f;
            }else if(MAG_CHIP_YAS532 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].maxRange = 1200.0f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].power = 2.6f;
            }
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].name = "YAS Magnetic Field Uncalibrated Sensor";
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].vendor = "YAS";
            if(MAG_CHIP_YAS537 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].maxRange = 2000.0f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.3f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].power = 1.8f;
            }else if(MAG_CHIP_YAS532 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].maxRange = 1200.0f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].power = 2.6f;
            }

            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].name = "YAS Magnetic Field (Wakeup) Sensor";
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].vendor = "YAS";
            if(MAG_CHIP_YAS537 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].maxRange = 2000.0f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].resolution = 0.3f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].power = 1.8f;
            }else if(MAG_CHIP_YAS532 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].maxRange = 1200.0f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD].power = 2.6f;
            }
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].name = "YAS Magnetic Field Uncalibrated (Wakeup) Sensor";
            bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].vendor = "YAS";
            if(MAG_CHIP_YAS537 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].maxRange = 2000.0f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.3f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].power = 1.8f;
            }else if(MAG_CHIP_YAS532 ==  magn_chip){
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].maxRange = 1200.0f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].resolution = 0.15f;
                bst_all_sensors[SENSORLIST_INX_WAKEUP_MAGNETIC_FIELD_UNCALIBRATED].power = 2.6f;
            }
        }

        if(SOLUTION_MDOF == solution_type)
        {
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].minDelay = 20000;
        }
        /*So far compass / m4g subset library support max 25/50Hz on virtual sensors*/
        else if(SOLUTION_ECOMPASS == solution_type)
        {
            bst_all_sensors[SENSORLIST_INX_ORIENTATION].minDelay = 40000;
            bst_all_sensors[SENSORLIST_INX_GRAVITY].minDelay = 40000;
            bst_all_sensors[SENSORLIST_INX_LINEAR_ACCELERATION].minDelay = 40000;
            bst_all_sensors[SENSORLIST_INX_ROTATION_VECTOR].minDelay = 40000;
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_ROTATION_VECTOR].minDelay = 40000;
        }else if(SOLUTION_M4G == solution_type){
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_ORIENTATION].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_GYROSCOPE].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_GRAVITY].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_LINEAR_ACCELERATION].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_ROTATION_VECTOR].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_GYROSCOPE_UNCALIBRATED].minDelay = 20000;
            bst_all_sensors[SENSORLIST_INX_MAGNETIC_ROTATION_VECTOR].minDelay = 20000;
        }

        avail_sens_regval = ( (1 << SENSORLIST_INX_ACCELEROMETER) | (1 << SENSORLIST_INX_GYROSCOPE_UNCALIBRATED) );

        sensor_amount = sensord_popcount_64(avail_sens_regval);

        bst_sensorlist.list = (struct sensor_t *) malloc(sensor_amount * sizeof(struct sensor_t));
        if (NULL == bst_sensorlist.list)
        {

            PERR("fail to malloc %d * sizeof(struct sensor_t)(=%d)",
                    sensor_amount, sizeof(struct sensor_t));
            return 0;
        }

        bst_sensorlist.bsx_list_index = (int32_t *) malloc(sensor_amount * sizeof(int32_t));
        if (NULL == bst_sensorlist.bsx_list_index)
        {

            PERR("fail to malloc %d * 4", sensor_amount);
            free(bst_sensorlist.list);
            return 0;
        }

        for (i = 0, j = 0; i < SENSORLIST_INX_END; i++, avail_sens_regval >>= 1)
        {
            if (0x0 == avail_sens_regval)
            {
                break;
            }

            if (avail_sens_regval & 0x1)
            {
                memcpy(&(bst_sensorlist.list[j]), &(bst_all_sensors[i]), sizeof(struct sensor_t));
                bst_sensorlist.bsx_list_index[j] = i;
                j++;
            }
        }

        bst_sensorlist.list_len = sensor_amount;
    }

    *p_sSensorList = bst_sensorlist.list;
    return bst_sensorlist.list_len;
}

static inline int32_t BMI160_convert_ODR(int32_t bsx_list_inx, float Hz)
{
    if (SENSORLIST_INX_ACCELEROMETER == bsx_list_inx)
    {

        if (Hz > 200)
        {
            return BMI160_ACCEL_ODR_400HZ;
        }
        if (Hz > 100 && Hz <= 200)
        {
            return BMI160_ACCEL_ODR_200HZ;
        }
        if (Hz > 50 && Hz <= 100)
        {
            return BMI160_ACCEL_ODR_100HZ;
        }
        if (Hz > 25 && Hz <= 50)
        {
            return BMI160_ACCEL_ODR_50HZ;
        }
        if (Hz > 12.5 && Hz <= 25)
        {
            return BMI160_ACCEL_ODR_25HZ;
        }
        if (Hz > 6.25 && Hz <= 12.5)
        {
            return BMI160_ACCEL_ODR_12_5HZ;
        }
        if (Hz > 1 && Hz <= 6.25)
        {
            return BMI160_ACCEL_ODR_6_25HZ;
        }
        if (Hz > 0 && Hz <= 1)
        {
            return BMI160_ACCEL_ODR_0_78HZ;
        }

        return BMI160_ACCEL_ODR_RESERVED;
    }
    else if (SENSORLIST_INX_GYROSCOPE_UNCALIBRATED == bsx_list_inx)
    {

        if (Hz > 200)
        {
            return BMI160_GYRO_ODR_400HZ;
        }
        if (Hz > 100 && Hz <= 200)
        {
            return BMI160_GYRO_ODR_200HZ;
        }
        if (Hz > 50 && Hz <= 100)
        {
            return BMI160_GYRO_ODR_100HZ;
        }
        if (Hz > 25 && Hz <= 50)
        {
            return BMI160_GYRO_ODR_50HZ;
        }
        if (Hz > 0 && Hz <= 25)
        {
            return BMI160_GYRO_ODR_25HZ;
        }

        return BMI160_GYRO_ODR_RESERVED;
    }
    else if (SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED == bsx_list_inx)
    {

        if (Hz > 200)
        {
            return BMI160_MAG_ODR_400HZ;
        }
        if (Hz > 100 && Hz <= 200)
        {
            return BMI160_MAG_ODR_200HZ;
        }
        if (Hz > 50 && Hz <= 100)
        {
            return BMI160_MAG_ODR_100HZ;
        }
        if (Hz > 25 && Hz <= 50)
        {
            return BMI160_MAG_ODR_50HZ;
        }
        if (Hz > 12.5 && Hz <= 25)
        {
            return BMI160_MAG_ODR_25HZ;
        }
        if (Hz > 6.25 && Hz <= 12.5)
        {
            return BMI160_MAG_ODR_12_5HZ;
        }
        if (Hz > 1 && Hz <= 6.25)
        {
            return BMI160_MAG_ODR_6_25HZ;
        }
        if (Hz > 0 && Hz <= 1)
        {
            return BMI160_MAG_ODR_0_78HZ;
        }

        return BMI160_MAG_ODR_RESERVED;
    }

    return 0;
}

/**
 * @param Hz
 * @param p_bandwith
 * @return physically support Hz
 */
static inline float BMA2x2_convert_ODR(float Hz, int32_t *p_bandwith)
{
    if (Hz > 200)
    {
        *p_bandwith = BMA2X2_ODR_500HZ;
        return 500.f;
    }
    if (Hz > 100 && Hz <= 200)
    {
        *p_bandwith = BMA2X2_ODR_250HZ;
        return 250.f;
    }
    if (Hz > 50 && Hz <= 100)
    {
        *p_bandwith = BMA2X2_ODR_125HZ;
        return 125.f;
    }
    if (Hz > 25 && Hz <= 50)
    {
        *p_bandwith = BMA2X2_ODR_62_50HZ;
        return 62.5f;
    }
    if (Hz > 12.5 && Hz <= 25)
    {
        *p_bandwith = BMA2X2_ODR_31_25HZ;
        return 31.25f;
    }
    if (Hz > 6.25 && Hz <= 12.5)
    {
        *p_bandwith = BMA2X2_ODR_15_63HZ;
        return 15.63f;
    }
    if (Hz > 1 && Hz <= 6.25)
    {
        *p_bandwith = BMA2X2_ODR_15_63HZ;
        return 15.63f;
    }

    return 0;

}

/**
 * @param Hz
 * @param p_bandwith
 * @return physical support Hz
 */
static inline float BMG160_convert_ODR(float Hz, int32_t *p_bandwith)
{
    if (Hz > 200)
    {
        *p_bandwith = BMG160_ODR_400HZ;
        return 400.f;
    }
    if (Hz > 100 && Hz <= 200)
    {
        *p_bandwith = BMG160_ODR_200HZ;
        return 200.f;
    }
    if (Hz > 1 && Hz <= 100)
    {
        *p_bandwith = BMG160_ODR_100HZ;
        return 100.f;
    }

    return 0;

}


static inline int32_t ap_convert_latency(
        uint16_t alshconf_maxlatency,
        uint8_t alshconf_latencyunit,
        int32_t acc_odr, int32_t gyro_odr, int32_t magn_odr)
{

#if 0
    uint64_t maxlatency_ns = 0;

    switch(alshconf_latencyunit)
    {
        case ALSH_CONFSTR_UNITns:
        maxlatency_ns = (uint64_t)alshconf_maxlatency;
        break;
        case ALSH_CONFSTR_UNITus:
        maxlatency_ns = (uint64_t)alshconf_maxlatency * 1000;
        break;
        case ALSH_CONFSTR_UNITms:
        maxlatency_ns = (uint64_t)alshconf_maxlatency * 1000000;
        break;
        case ALSH_CONFSTR_UNITs:
        maxlatency_ns = (uint64_t)alshconf_maxlatency * 1000000000;
        break;
    }

    //caculate out water mark
    if(maxlatency_ns < xxx)
    if(acc_odr .., gyro_odr .., magn_odr ..)
    return xxx
    else if(maxlatency_ns < xxx)
    if(acc_odr .., gyro_odr .., magn_odr ..)
    return xxx
#else
    (void) alshconf_maxlatency;
    (void) alshconf_latencyunit;
    (void) acc_odr;
    (void) gyro_odr;
    (void) magn_odr;

    return default_watermark;
#endif
}


static int32_t is_acc_open = 0;
static int32_t is_gyr_open = 0;
static int32_t is_mag_open = 0;


static void ap_config_phyACC(bsx_f32_t sample_rate)
{
    int32_t ret = 0;
    int32_t odr_Hz;
    int32_t bandwidth = 0;
    int32_t fifo_data_sel_regval;
    float physical_Hz = 0;

    PINFO("set physical ACC rate %f", sample_rate);

    if(ACC_CHIP_BMI160 == accl_chip)
    {
        ret = rd_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, &fifo_data_sel_regval);
        if (0 != ret)
        {
            PERR("read fifo_data_sel fail, ret = %d, set fifo_data_sel_regval = 0", ret);
            /*Keep on trying*/
            fifo_data_sel_regval = 0;
        }

        if (SAMPLE_RATE_DISABLED == sample_rate)
        {
            if (1 == is_acc_open)
            {
                PDEBUG("shutdown acc");
                /** when closing in BMI160, set op mode firstly. */
                ret = wr_sysfs_oneint("acc_op_mode", iio_dev0_dir_name, SENSOR_PM_SUSPEND);
                fifo_data_sel_regval &= ~(1 << 0);
                wr_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, fifo_data_sel_regval);

                is_acc_open = 0;
            }
        }else
        {
            PDEBUG("set acc odr: %f", sample_rate);
            odr_Hz = BMI160_convert_ODR(SENSORLIST_INX_ACCELEROMETER, sample_rate);
            ret = wr_sysfs_oneint("acc_odr", iio_dev0_dir_name, odr_Hz);

            /*activate is included*/
            if (0 == is_acc_open)
            {
                /** when opening in BMI160, set op mode at last. */
                fifo_data_sel_regval |= (1 << 0);
                wr_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, fifo_data_sel_regval);
                ret = wr_sysfs_oneint("acc_op_mode", iio_dev0_dir_name, SENSOR_PM_NORMAL);

                is_acc_open = 1;
            }
        }
    }else if(ACC_CHIP_BMA2x2 == accl_chip)
    {
        if (SAMPLE_RATE_DISABLED == sample_rate)
        {
            if (1 == is_acc_open)
            {
                PDEBUG("shutdown acc");

                ret = wr_sysfs_oneint("op_mode", acc_input_dir_name, SENSOR_PM_SUSPEND);

                is_acc_open = 0;
            }
        }else
        {
            PDEBUG("set acc odr: %f", sample_rate);

            physical_Hz = BMA2x2_convert_ODR(sample_rate, &bandwidth);
            ret = wr_sysfs_oneint("bandwidth", acc_input_dir_name, bandwidth);

            /*activate is included*/
            if (0 == is_acc_open)
            {
                ret = wr_sysfs_oneint("op_mode", acc_input_dir_name, SENSOR_PM_NORMAL);

                is_acc_open = 1;
            }
        }

    }

    return;
}

static void ap_config_phyGYR(bsx_f32_t sample_rate)
{
    int32_t ret = 0;
    int32_t odr_Hz;
    int32_t bandwidth = 0;
    int32_t fifo_data_sel_regval;
    float physical_Hz = 0;

    PINFO("set physical GYRO rate %f", sample_rate);

    if(GYR_CHIP_BMI160 == gyro_chip)
    {
        ret = rd_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, &fifo_data_sel_regval);
        if (0 != ret)
        {
            PERR("read fifo_data_sel fail, ret = %d, set fifo_data_sel_regval = 0", ret);
            /*Keep on trying*/
            fifo_data_sel_regval = 0;
        }

        if (SAMPLE_RATE_DISABLED == sample_rate)
        {
            if (1 == is_gyr_open)
            {
                PDEBUG("shutdown gyro");
                /** when closing in BMI160, set op mode firstly. */
                ret = wr_sysfs_oneint("gyro_op_mode", iio_dev0_dir_name, SENSOR_PM_SUSPEND);
                fifo_data_sel_regval &= ~(1 << 1);
                wr_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, fifo_data_sel_regval);

                is_gyr_open = 0;
            }
        }else
        {
            PDEBUG("set gyr odr: %f", sample_rate);
            odr_Hz = BMI160_convert_ODR(SENSORLIST_INX_GYROSCOPE_UNCALIBRATED, sample_rate);
            ret = wr_sysfs_oneint("gyro_odr", iio_dev0_dir_name, odr_Hz);

            /*activate is included*/
            if (0 == is_gyr_open)
            {
                /** when opening in BMI160, set op mode at last. */
                fifo_data_sel_regval |= (1 << 1);
                wr_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, fifo_data_sel_regval);
                ret = wr_sysfs_oneint("gyro_op_mode", iio_dev0_dir_name, SENSOR_PM_NORMAL);

                is_gyr_open = 1;
            }
        }
    }else if(GYR_CHIP_BMG160 == gyro_chip)
    {
        if (SAMPLE_RATE_DISABLED == sample_rate)
        {
            if (1 == is_gyr_open)
            {
                PDEBUG("shutdown gyro");

                ret = wr_sysfs_oneint("op_mode", gyr_input_dir_name, SENSOR_PM_SUSPEND);

                is_gyr_open = 0;
            }
        }else
        {
            PDEBUG("set gyr odr: %f", sample_rate);

            physical_Hz = BMG160_convert_ODR(sample_rate, &bandwidth);
            ret = wr_sysfs_oneint("bandwidth", gyr_input_dir_name, bandwidth);

            /*activate is included*/
            if (0 == is_gyr_open)
            {
                ret = wr_sysfs_oneint("op_mode", gyr_input_dir_name, SENSOR_PM_NORMAL);

                is_gyr_open = 1;
            }
        }

    }

    return;
}

static void ap_config_phyMAG(bsx_f32_t sample_rate)
{
    int32_t ret = 0;
    int32_t odr_Hz;
    int32_t fifo_data_sel_regval;
    uint32_t Hz_to_delay_ms = 0;
    struct itimerspec timerspec;
    static bsx_f32_t pre_poll_rate = 0;

    PINFO("set physical MAG rate %f", sample_rate);

    if(MAG_CHIP_BMI160 == magn_chip)
    {
        ret = rd_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, &fifo_data_sel_regval);
        if (0 != ret)
        {
            PERR("read fifo_data_sel fail, ret = %d, set fifo_data_sel_regval = 0", ret);
            /*Keep on trying*/
            fifo_data_sel_regval = 0;
        }

        if (SAMPLE_RATE_DISABLED == sample_rate)
        {
            if (1 == is_mag_open)
            {
                PDEBUG("shutdown magn");
                /** when closing in BMI160, set op mode firstly. */
                ret = wr_sysfs_oneint("mag_op_mode", iio_dev0_dir_name, SENSOR_PM_SUSPEND);
                fifo_data_sel_regval &= ~(1 << 2);
                wr_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, fifo_data_sel_regval);

                is_mag_open = 0;
            }
        }else
        {
            PDEBUG("set magn odr: %f", sample_rate);
            odr_Hz = BMI160_convert_ODR(SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED, sample_rate);
            ret = wr_sysfs_oneint("mag_odr", iio_dev0_dir_name, odr_Hz);

            /*activate is included*/
            if (0 == is_mag_open)
            {
                /** when opening in BMI160, set op mode at last. */
                fifo_data_sel_regval |= (1 << 2);
                wr_sysfs_oneint("fifo_data_sel", iio_dev0_dir_name, fifo_data_sel_regval);
                ret = wr_sysfs_oneint("mag_op_mode", iio_dev0_dir_name, SENSOR_PM_NORMAL);

                is_mag_open = 1;
            }
        }
    }else if(MAG_CHIP_BMM150 == magn_chip || MAG_CHIP_AKM09912 ==  magn_chip || MAG_CHIP_AKM09911 ==  magn_chip ||
            MAG_CHIP_YAS537 ==  magn_chip || MAG_CHIP_YAS532 ==  magn_chip)
    {
        if (SAMPLE_RATE_DISABLED == sample_rate)
        {
            if (1 == is_mag_open)
            {
                PDEBUG("shutdown magn");

                if(SOLUTION_MDOF == solution_type && ACC_CHIP_BMI160 == accl_chip && GYR_CHIP_BMI160 == gyro_chip)
                {
                    timerspec.it_value.tv_sec = 0;
                    timerspec.it_value.tv_nsec = 0;
                    timerspec.it_interval.tv_sec = 0; /*is needed, please ignore the man info*/
                    timerspec.it_interval.tv_nsec = 0;/*is needed, please ignore the man info*/
                    ret = timerfd_settime(poll_timer_fd, 0, &timerspec, NULL);
                }

                pre_poll_rate = 0;

                ret = wr_sysfs_oneint("op_mode", mag_input_dir_name, SENSOR_PM_SUSPEND);

                is_mag_open = 0;
            }
        }else
        {
            PDEBUG("set magn odr: %f", sample_rate);

            /*activate is included*/
            if (0 == is_mag_open)
            {
                ret = wr_sysfs_oneint("op_mode", mag_input_dir_name, SENSOR_PM_NORMAL);

                if(MAG_CHIP_BMM150 == magn_chip){
                    /*set filter parameter to reduce noise.
                     * only can do on our BMM but open this to customer on AKM*/
                    ret = wr_sysfs_oneint("rept_xy", mag_input_dir_name, 4);
                    ret = wr_sysfs_oneint("rept_z", mag_input_dir_name, 15);
                }

                is_mag_open = 1;
            }

            if(pre_poll_rate != sample_rate)
            {
                Hz_to_delay_ms = (uint32_t)(1000.0f / sample_rate);
                if(SOLUTION_MDOF == solution_type && ACC_CHIP_BMI160 == accl_chip && GYR_CHIP_BMI160 == gyro_chip)
                {
                    timerspec.it_value.tv_sec = Hz_to_delay_ms / 1000;
                    timerspec.it_value.tv_nsec = (Hz_to_delay_ms % 1000) * 1000000;
                    timerspec.it_interval.tv_sec = timerspec.it_value.tv_sec;
                    timerspec.it_interval.tv_nsec = timerspec.it_value.tv_nsec;
                    ret = timerfd_settime(poll_timer_fd, 0, &timerspec, NULL);
                }

                pre_poll_rate = sample_rate;
            }
        }
    }

    return;
}


static void ap_config_physensor(bsx_u32_t input_id, bsx_f32_t sample_rate)
{
    int32_t ret = 0;

    if (BSX_PHYSICAL_SENSOR_ID_INVALID == input_id)
    { //TODO: library may has bug
        return;
    }

    switch (input_id)
    {
        case BSX_INPUT_ID_ACCELERATION:
            ap_config_phyACC(sample_rate);
            break;
        case BSX_INPUT_ID_MAGNETICFIELD:
            ap_config_phyMAG(sample_rate);
            break;
        case BSX_INPUT_ID_ANGULARRATE:
            ap_config_phyGYR(sample_rate);
            break;
        default:
            PWARN("unknown input id: %d", input_id);
            ret = 0;
            break;
    }

    if (ret < 0)
    {
        PERR("write_sysfs() fail");
    }

    return;
}



/**
 *
 */
static void ap_send_config(int32_t bsx_list_inx)
{
    const struct sensor_t *p_sensor;
    BSX_SENSOR_CONFIG *p_config;
    bsx_sensor_configuration_t bsx_config_output[2];
    int32_t bsx_supplier_id;
    int32_t list_inx_base;
    bsx_u32_t input_id;

    if (bsx_list_inx <= SENSORLIST_INX_AMBIENT_IAQ)
    {
        list_inx_base = SENSORLIST_INX_GAS_RESIST;
        p_config = BSX_sensor_config_nonwk;
    }
    else
    {
        list_inx_base = SENSORLIST_INX_WAKEUP_SIGNI_PRESSURE;
        p_config = BSX_sensor_config_wk;
    }

    bsx_supplier_id = convert_BSX_ListInx(bsx_list_inx);
    if (BSX_VIRTUAL_SENSOR_ID_INVALID == bsx_supplier_id)
    {
        PWARN("invalid list index: %d, when matching supplier id", bsx_list_inx);
        return;
    }

    {
        bsx_config_output[0].sensor_id = bsx_supplier_id;

        p_sensor = &(bst_all_sensors[bsx_list_inx]);
        if(SENSOR_FLAG_ON_CHANGE_MODE == (p_sensor->flags & REPORTING_MODE_MASK))
        {
            bsx_config_output[0].sample_rate = p_config[bsx_list_inx - list_inx_base].delay_onchange_Hz;
        }else
        {
            CONVERT_DATARATE_CODE(p_config[bsx_list_inx - list_inx_base].data_rate, bsx_config_output[0].sample_rate);
        }


        /**
         * For test app, AR should be defined as a on change sensor,
         * But for BSX4 Library, only can send rate 0 on AR
         */
        if (SENSORLIST_INX_WAKEUP_ACTIVITY == bsx_list_inx)
        {
            bsx_config_output[0].sample_rate = 0;
        }
    }

    {
        switch (bsx_list_inx) {
            case SENSORLIST_INX_ACCELEROMETER:
                input_id = BSX_INPUT_ID_ACCELERATION;
                break;
            case SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED:
                input_id = BSX_INPUT_ID_MAGNETICFIELD;
                break;
            case SENSORLIST_INX_GYROSCOPE_UNCALIBRATED:
                input_id = BSX_INPUT_ID_ANGULARRATE;
                break;
            default:
                PERR("wrong list index: %d", bsx_list_inx);
                return;
        }

        ap_config_physensor(input_id, bsx_config_output[0].sample_rate);
    }

    return;
}

/**
 *
 * @param bsx_list_inx
 */
static void ap_send_disable_config(int32_t bsx_list_inx)
{
    int32_t bsx_supplier_id;
    bsx_u32_t input_id;

    bsx_supplier_id = convert_BSX_ListInx(bsx_list_inx);
    if (BSX_VIRTUAL_SENSOR_ID_INVALID == bsx_supplier_id)
    {
        PWARN("invalid list index: %d, when matching supplier id", bsx_list_inx);
        return;
    }

    {
        switch (bsx_list_inx) {
            case SENSORLIST_INX_ACCELEROMETER:
                input_id = BSX_INPUT_ID_ACCELERATION;
                break;
            case SENSORLIST_INX_MAGNETIC_FIELD_UNCALIBRATED:
                input_id = BSX_INPUT_ID_MAGNETICFIELD;
                break;
            case SENSORLIST_INX_GYROSCOPE_UNCALIBRATED:
                input_id = BSX_INPUT_ID_ANGULARRATE;
                break;
            default:
                PERR("wrong list index: %d", bsx_list_inx);
                return;
        }

        ap_config_physensor(input_id, SAMPLE_RATE_DISABLED);
    }
    return;
}

int32_t ap_activate(int32_t handle, int32_t enabled)
{
    struct sensor_t *p_sensor;
    int32_t bsx_list_inx;
    int32_t ret;

    if (BSX_SENSOR_ID_INVALID == handle)
    {
        /*private sensors which not supported in library and Android yet*/
        return 0;
    }

    PDEBUG("ap_activate(handle: %d, enabled: %d)", handle, enabled);

    get_sensor_t(handle, &p_sensor, &bsx_list_inx);
    if (NULL == p_sensor)
    {
        PWARN("invalid id: %d", handle);
        return -EINVAL;
    }

    /*To adapt BSX4 algorithm's way of configuration string, activate_configref_resort() is employed*/
    ret = activate_configref_resort(bsx_list_inx, enabled);
    if (ret)
    {
        if (enabled)
        {
            ap_send_config(bsx_list_inx);
        }
        else
        {
            ap_send_disable_config(bsx_list_inx);
        }
    }

    return 0;
}

int32_t ap_batch(int32_t handle, int32_t flags, int64_t sampling_period_ns, int64_t max_report_latency_ns)
{

    (void) flags; //Deprecated in SENSORS_DEVICE_API_VERSION_1_3
    struct sensor_t *p_sensor;
    int32_t bsx_list_inx;
    int32_t ret;
    float delay_Hz_onchange = 0;

    //Sensors whoes fifoMaxEventCount==0 can be called batching according to spec
    get_sensor_t(handle, &p_sensor, &bsx_list_inx);
    if (NULL == p_sensor)
    {
        PWARN("invalid id: %d", handle);
        /*If maxReportingLatency is set to 0, this function must succeed*/
        if (0 == max_report_latency_ns)
        {
            return 0;
        }
        else
        {
            return -EINVAL;
        }
    }

    /*One-shot sensors are sometimes referred to as trigger sensors.
     The sampling_period_ns and max_report_latency_ns parameters
     passed to the batch function is ignored.
     Events from one-shot events cannot be stored in hardware FIFOs:
     the events must be reported as soon as they are generated.
     */
    if (SENSOR_FLAG_ONE_SHOT_MODE == (p_sensor->flags & REPORTING_MODE_MASK))
    {
        return 0;
    }

    /*For special reporting-mode sensors, process each one according to spec*/
    if (SENSOR_FLAG_SPECIAL_REPORTING_MODE == (p_sensor->flags & REPORTING_MODE_MASK))
    {
        //...
        return 0;
    }

    PDEBUG("batch(handle: %d, sampling_period_ns = %lld)", handle, sampling_period_ns);

    /*
     For continuous and on-change sensors
     1. if sampling_period_ns is less than sensor_t.minDelay,
     then the HAL implementation must silently clamp it to max(sensor_t.minDelay, 1ms).
     Android does not support the generation of events at more than 1000Hz.
     2. if sampling_period_ns is greater than sensor_t.maxDelay,
     then the HAL implementation must silently truncate it to sensor_t.maxDelay.
     */
    if (sampling_period_ns < (int64_t)(p_sensor->minDelay) * 1000LL)
    {
        sampling_period_ns = (int64_t)(p_sensor->minDelay) * 1000LL;
    }
    else if (sampling_period_ns > (int64_t)(p_sensor->maxDelay) * 1000LL)
    {
        sampling_period_ns = (int64_t)(p_sensor->maxDelay) * 1000LL;
    }

    /**
     * store sampling period as delay(us) for on change type sensor
     */
    if(SENSOR_FLAG_ON_CHANGE_MODE == (p_sensor->flags & REPORTING_MODE_MASK))
    {
        delay_Hz_onchange = (float)1000000000LL / (float)sampling_period_ns;
        sampling_period_ns = 0;
    }

    /*In Android's perspective, a sensor can be configured no matter if it's active.
     To adapt BSX4 algorithm's way of configuration string, batch_configref_resort() is employed*/
    ret = batch_configref_resort(bsx_list_inx, sampling_period_ns, max_report_latency_ns, delay_Hz_onchange);
    if (ret)
    {
        ap_send_config(bsx_list_inx);
    }

    return 0;
}

int32_t ap_flush(BstSensor *bstsensor, int32_t handle)
{
    struct sensor_t *p_sensor;
    int32_t bsx_list_inx;

    get_sensor_t(handle, &p_sensor, &bsx_list_inx);
    if (NULL == p_sensor)
    {
        PWARN("invalid id: %d", handle);
        return -EINVAL;
    }

    /*flush does not apply to one-shot sensors:
     if sensor_handle refers to a one-shot sensor,
     flush must return -EINVAL and not generate any flush complete metadata event.
     */
    if (SENSOR_FLAG_ONE_SHOT_MODE == (p_sensor->flags & REPORTING_MODE_MASK))
    {
        PWARN("invalid flags for id: %d", handle);
        return -EINVAL;
    }


    //Now the driver can not support this specification, so has to work around
    (void) bstsensor->send_flush_event(handle);

    /*If the specified sensor has no FIFO (no buffering possible),
     or if the FIFO was empty at the time of the call,
     flush must still succeed and send a flush complete event for that sensor.
     This applies to all sensors
     (except one-shot sensors, and already filted in former code).
     */

    return 0;
}


static void ap_hw_process_IIO_frame(int32_t header, char *data, BstSimpleList *dest_list)
{

    HW_DATA_UNION *p_hwdata;
    int16_t rawdata_tmp = 0;
    int64_t raw_timestamp = 0;
    int32_t ret;

    p_hwdata = (HW_DATA_UNION *) calloc(1, sizeof(HW_DATA_UNION));
    if (NULL == p_hwdata)
    {
        PERR("malloc fail");
        return;
    }

    /*
     size_from_channelarray bytes 1 name =in_x location=0 bytes=2
     size_from_channelarray bytes 1 name =in_y location=2 bytes=2
     size_from_channelarray bytes 1 name =in_z location=4 bytes=2
     size_from_channelarray bytes 1 name =in_timestamp location=8 bytes=8
     size_from_channelarray bytes=16
     */

    switch (header)
    {
        case FIFO_HEAD_A:
            p_hwdata->id = SENSOR_TYPE_ACCELEROMETER;
            memcpy(&rawdata_tmp, data, sizeof(rawdata_tmp));
            p_hwdata->x = (float) rawdata_tmp * BMI160_acc_resl;

            memcpy(&rawdata_tmp, data + 2, sizeof(rawdata_tmp));
            p_hwdata->y = (float) rawdata_tmp * BMI160_acc_resl;

            memcpy(&rawdata_tmp, data + 4, sizeof(rawdata_tmp));
            p_hwdata->z = (float) rawdata_tmp * BMI160_acc_resl;

            memcpy(&raw_timestamp, data + 8, sizeof(raw_timestamp));
            p_hwdata->timestamp = raw_timestamp * 1000;

            hw_remap_sensor_data(&(p_hwdata->x), &(p_hwdata->y), &(p_hwdata->z), g_place_a);

            ret = dest_list->list_add_rear((void *) p_hwdata);
            if (ret)
            {
                PERR("list_add_rear() fail, ret = %d", ret);
                if(-1 == ret){
                    free(p_hwdata);
                }
            }

            break;
        case FIFO_HEAD_G:
            p_hwdata->id = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;

            memcpy(&rawdata_tmp, data, sizeof(rawdata_tmp));
            p_hwdata->x_uncalib = (float) rawdata_tmp;

            memcpy(&rawdata_tmp, data + 2, sizeof(rawdata_tmp));
            p_hwdata->y_uncalib = (float) rawdata_tmp;

            memcpy(&rawdata_tmp, data + 4, sizeof(rawdata_tmp));
            p_hwdata->z_uncalib = (float) rawdata_tmp;

            memcpy(&raw_timestamp, data + 8, sizeof(raw_timestamp));
            p_hwdata->timestamp = raw_timestamp * 1000;

            hw_remap_sensor_data(&(p_hwdata->x_uncalib), &(p_hwdata->y_uncalib), &(p_hwdata->z_uncalib), g_place_g);

            ret = dest_list->list_add_rear((void *) p_hwdata);
            if (ret)
            {
                PERR("list_add_rear() fail, ret = %d", ret);
                if(-1 == ret){
                    free(p_hwdata);
                }
            }

            break;
        case FIFO_HEAD_M:
            p_hwdata->id = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;

            memcpy(&rawdata_tmp, data, sizeof(rawdata_tmp));
            p_hwdata->x_uncalib = (float) rawdata_tmp * 10 / BMM_COMPVAL_TO_uT; // library requires input is in 0.1uT

            memcpy(&rawdata_tmp, data + 2, sizeof(rawdata_tmp));
            p_hwdata->y_uncalib = (float) rawdata_tmp * 10 / BMM_COMPVAL_TO_uT;

            memcpy(&rawdata_tmp, data + 4, sizeof(rawdata_tmp));
            p_hwdata->z_uncalib = (float) rawdata_tmp * 10 / BMM_COMPVAL_TO_uT;

            memcpy(&raw_timestamp, data + 8, sizeof(raw_timestamp));
            p_hwdata->timestamp = raw_timestamp * 1000;

            hw_remap_sensor_data(&(p_hwdata->x_uncalib), &(p_hwdata->y_uncalib), &(p_hwdata->z_uncalib), g_place_m);

            ret = dest_list->list_add_rear((void *) p_hwdata);
            if (ret)
            {
                PERR("list_add_rear() fail, ret = %d", ret);
                if(-1 == ret){
                    free(p_hwdata);
                }
            }

            break;
        default:
            PERR("Unknown header %d", header);
            free(p_hwdata);

            break;
    }

    return;
}

static void ap_hw_poll_bma2x2(BstSimpleList *dest_list)
{
    int32_t i;
    int32_t ret;
    struct input_event event[6];
    struct timespec tmspec;
    HW_DATA_UNION *p_hwdata;

    while( (ret = read(acc_input_fd, event, sizeof(event))) > 0)
    {
        if(EV_MSC != event[0].type || EV_MSC != event[1].type || EV_MSC != event[2].type || EV_MSC != event[3].type ||
                EV_MSC != event[4].type || EV_SYN != event[5].type)
        {
            PWARN("Unexpected events: %d, %d; %d, %d; %d, %d; %d, %d; %d, %d; %d;",
                    event[0].type, event[0].code, event[1].type, event[1].code, event[2].type, event[2].code,
                    event[3].type, event[3].code, event[4].type, event[4].code,
                    event[5].type);
            continue;
        }

        p_hwdata = (HW_DATA_UNION *) calloc(1, sizeof(HW_DATA_UNION));
        if (NULL == p_hwdata)
        {
            PERR("malloc fail");
            continue;
        }

        p_hwdata->id = SENSOR_TYPE_ACCELEROMETER;
        p_hwdata->x = event[2].value * BMA255_acc_resl;
        p_hwdata->y = event[3].value * BMA255_acc_resl;
        p_hwdata->z = event[4].value * BMA255_acc_resl;
        p_hwdata->timestamp = event[0].value * 1000000000LL +  event[1].value;

        hw_remap_sensor_data(&(p_hwdata->x), &(p_hwdata->y), &(p_hwdata->z), g_place_a);

        ret = dest_list->list_add_rear((void *) p_hwdata);
        if (ret)
        {
            PERR("list_add_rear() fail, ret = %d", ret);
            if(-1 == ret){
                free(p_hwdata);
            }
        }
    }

    return;
}

static void ap_hw_poll_bmg160(BstSimpleList *dest_list)
{

    int32_t i;
    int32_t ret;
    struct input_event event[6];
    struct timespec tmspec;
    HW_DATA_UNION *p_hwdata;

    while( (ret = read(gyr_input_fd, event, sizeof(event))) > 0)
    {
        if(EV_MSC != event[0].type || EV_MSC != event[1].type || EV_MSC != event[2].type || EV_MSC != event[3].type ||
                EV_MSC != event[4].type || EV_SYN != event[5].type)
        {
            PWARN("Unexpected events: %d, %d; %d, %d; %d, %d; %d, %d; %d, %d; %d;",
                    event[0].type, event[0].code, event[1].type, event[1].code, event[2].type, event[2].code,
                    event[3].type, event[3].code, event[4].type, event[4].code,
                    event[5].type);
            continue;
        }

        p_hwdata = (HW_DATA_UNION *) calloc(1, sizeof(HW_DATA_UNION));
        if (NULL == p_hwdata)
        {
            PERR("malloc fail");
            continue;
        }

        p_hwdata->id = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
        p_hwdata->x_uncalib = event[2].value;
        p_hwdata->y_uncalib = event[3].value;
        p_hwdata->z_uncalib = event[4].value;
        p_hwdata->timestamp = event[0].value * 1000000000LL +  event[1].value;

        hw_remap_sensor_data(&(p_hwdata->x_uncalib), &(p_hwdata->y_uncalib), &(p_hwdata->z_uncalib), g_place_g);

        ret = dest_list->list_add_rear((void *) p_hwdata);
        if (ret)
        {
            PERR("list_add_rear() fail, ret = %d", ret);
            if(-1 == ret){
                free(p_hwdata);
            }
        }
    }

    return;
}


/*
static void dump_samples(BstSimpleList *al, BstSimpleList *gl, BstSimpleList *ml)
{
    HW_DATA_UNION *p_hwdata;
    struct list_node *pnode;
    uint32_t i;

    pnode = al->head;
    for (i = 0; i < al->list_len; ++i) {
        p_hwdata = (HW_DATA_UNION *)(pnode->p_data);
        PNOTE("**ACCL tm = %lld", p_hwdata->timestamp);
        pnode = pnode->next;
    }

    pnode = gl->head;
    for (i = 0; i < gl->list_len; ++i) {
        p_hwdata = (HW_DATA_UNION *)(pnode->p_data);
        PNOTE("--GYRO tm = %lld", p_hwdata->timestamp);
        pnode = pnode->next;
    }

    pnode = ml->head;
    for (i = 0; i < ml->list_len; ++i) {
        p_hwdata = (HW_DATA_UNION *)(pnode->p_data);
        PNOTE("##MAGN tm = %lld", p_hwdata->timestamp);
        pnode = pnode->next;
    }

    PNOTE("==============================");

    return;
}
*/



static uint32_t IMU_hw_deliver_sensordata(BstSensor *bstsensor)
{
    int32_t ret;
    uint32_t j;
    struct pollfd poll_fds[2];

    if(ACC_CHIP_BMA2x2 == accl_chip)
    {
        poll_fds[0].fd = acc_input_fd;
        poll_fds[0].events = POLLIN;
    }

    if(GYR_CHIP_BMG160 == gyro_chip)
    {
        poll_fds[1].fd = gyr_input_fd;
        poll_fds[1].events = POLLIN;
    }

    ret = poll(poll_fds, ARRAY_ELEMENTS(poll_fds), -1);
    if (ret <= 0)
    {
        PERR("poll in error: ret=%d", ret);
        return 0;
    }

    for (j = 0; j < ARRAY_ELEMENTS(poll_fds); j++)
    {
        if (POLLIN != poll_fds[j].revents)
        {
            continue;
        }

        switch (j)
        {
            case 0:
                ap_hw_poll_bma2x2(bstsensor->tmplist_hwcntl_acclraw);
                break;

            case 1:
                ap_hw_poll_bmg160(bstsensor->tmplist_hwcntl_gyroraw);
                break;
        }

    }

#if 1
    if (bstsensor->tmplist_hwcntl_acclraw->list_len + bstsensor->tmplist_hwcntl_gyroraw->list_len)
    {
        pthread_mutex_lock(&(bstsensor->shmem_hwcntl.mutex));

        ret = bstsensor->shmem_hwcntl.p_list->list_mount_rear(bstsensor->tmplist_hwcntl_acclraw);
        if(ret){
            PWARN("list mount fail");
        }

        ret = bstsensor->shmem_hwcntl.p_list->list_mount_rear(bstsensor->tmplist_hwcntl_gyroraw);
        if(ret){
            PWARN("list mount fail");
        }

        pthread_cond_signal(&(bstsensor->shmem_hwcntl.cond));
        pthread_mutex_unlock(&(bstsensor->shmem_hwcntl.mutex));
    }
#endif
    return 0;

}


static void ap_show_ver()
{
    const char* accl_chip_name = NULL;
    const char* gyro_chip_name = NULL;
    const char* magn_chip_name = NULL;
    const char* solution_name = NULL;
    char data_log_buf[256] = { 0 };

    switch (accl_chip) {
        case ACC_CHIP_BMI160:
            accl_chip_name = "BMI160";
            break;
        case ACC_CHIP_BMA2x2:
            accl_chip_name = "BMA2x2";
            break;
    }

    switch (gyro_chip) {
        case GYR_CHIP_BMI160:
            gyro_chip_name = "BMI160";
            break;
        case GYR_CHIP_BMG160:
            gyro_chip_name = "BMG160";
            break;
    }

    switch (magn_chip) {
        case MAG_CHIP_BMI160:
            magn_chip_name = "BMI160_Aux";
            break;
        case MAG_CHIP_BMM150:
            magn_chip_name = "BMM150";
            break;
        case MAG_CHIP_AKM09911:
            magn_chip_name = "AKM09911";
            break;
        case MAG_CHIP_AKM09912:
            magn_chip_name = "AKM09912";
            break;
        case MAG_CHIP_YAS532:
            magn_chip_name = "YAS532";
            break;
        case MAG_CHIP_YAS537:
            magn_chip_name = "YAS537";
            break;
    }

    switch (solution_type) {
        case SOLUTION_MDOF:
            solution_name = "MDOF";
            break;
        case SOLUTION_ECOMPASS:
            solution_name = "ECOMPASS";
            gyro_chip_name = NULL;
            break;
        case SOLUTION_IMU:
            solution_name = "IMU";
            magn_chip_name = NULL;
            break;
        case SOLUTION_M4G:
            solution_name = "M4G";
            gyro_chip_name = NULL;
            break;
        case SOLUTION_ACC:
            solution_name = "ACC";
            magn_chip_name = NULL;
            gyro_chip_name = NULL;
            break;
        default:
            solution_name = "Unknown";
            accl_chip_name = NULL;
            magn_chip_name = NULL;
            gyro_chip_name = NULL;
            break;
    }


    PNOTE("\n **************************\n \
* HAL version: %d.%d.%d.%d\n \
* build time: %s, %s\n \
* solution type: %s\n \
* accl_chip: %s\n \
* magn_chip: %s\n \
* gyro_chip: %s\n \
**************************",
            HAL_ver[0], HAL_ver[1], HAL_ver[2], HAL_ver[3],
            __DATE__, __TIME__, solution_name, accl_chip_name, magn_chip_name, gyro_chip_name);

    if(bsx_datalog){
        sprintf(data_log_buf, "<component>HAL version: %d.%d.%d.%d</component>\n",
                HAL_ver[0], HAL_ver[1], HAL_ver[2], HAL_ver[3]);
        bsx_datalog_algo(data_log_buf);
        sprintf(data_log_buf, "<component>build time: %s, %s</component>\n",
                __DATE__, __TIME__);
        bsx_datalog_algo(data_log_buf);
        sprintf(data_log_buf, "<component>solution type: %s</component>\n\n", solution_name);
        bsx_datalog_algo(data_log_buf);

        if(accl_chip_name){
            sprintf(data_log_buf, "<device name=\"%s\" bus=\"IIC\">\n", accl_chip_name);
            bsx_datalog_algo(data_log_buf);
            sprintf(data_log_buf, "</device>\n");
            bsx_datalog_algo(data_log_buf);
        }

        if(magn_chip_name){
            sprintf(data_log_buf, "<device name=\"%s\" bus=\"\" >\n", magn_chip_name);
            bsx_datalog_algo(data_log_buf);
            sprintf(data_log_buf, "</device>\n");
            bsx_datalog_algo(data_log_buf);
        }

        if(gyro_chip_name){
            sprintf(data_log_buf, "<device name=\"%s\" bus=\"IIC\">\n", gyro_chip_name);
            bsx_datalog_algo(data_log_buf);
            sprintf(data_log_buf, "</device>\n\n");
            bsx_datalog_algo(data_log_buf);
        }
    }

    return;
}

static void driver_show_ver(const char* base_path)
{
    char path[64] = {0};
    char driver_info[64] = {0};
    FILE *fp;
    int ret;

    if(NULL == base_path)
    {
        PWARN("base_path is invalid");
        return;
    }

    sprintf(path, "%s/driver_version", base_path);

    fp = fopen(path, "r");
    if (NULL == fp)
    {
        PWARN("read driver version failed");
        return;
    }

    ret = fread(driver_info, 1, sizeof(driver_info), fp);
    fclose(fp);
    if(ret <= 0)
    {
        PERR("fread fail, errno = %d(%s)", errno, strerror(errno));
    }

    PNOTE("\n **************************\n \
* %s \
*****************************", driver_info);

    return;
}

static const char *iio_dir = "/sys/bus/iio/devices/";

static int32_t ap_hwcntl_init_ACC()
{
    int32_t ret = 0;
    int32_t accl_dev_num;
    char accl_dev_dir_name[128];
    char accl_buf_dir_name[128];
    char accl_buffer_access[128];
    const char *accl_device_name = NULL;

    if(ACC_CHIP_BMI160 == accl_chip)
    {
        accl_device_name = "bmi160_accl";

        /* Find the device requested */
        accl_dev_num = get_IIOnum_by_name(accl_device_name, iio_dir);
        if (accl_dev_num < 0)
        {
            PERR("Failed to find the %s\n", accl_device_name);
            return -ENODEV;
        }

        PDEBUG("iio device number being used is acc %d\n", accl_dev_num);
        sprintf(accl_dev_dir_name, "%siio:device%d", iio_dir, accl_dev_num);

        strcpy(iio_dev0_dir_name, accl_dev_dir_name);

        /*driver version only can be accessed in ACC attributes */
        driver_show_ver(accl_dev_dir_name);

        /* Construct the directory name for the associated buffer.*/
        sprintf(accl_buf_dir_name, "%siio:device%d/buffer", iio_dir, accl_dev_num);

        /* Setup ring buffer parameters */
        ret = wr_sysfs_oneint("length", accl_buf_dir_name, hwdata_unit_toread);
        if (ret < 0)
        {
            PERR("wr_sysfs_oneint() fail, ret = %d", ret);
            return 0;
        }

        ret = wr_sysfs_oneint("enable", accl_buf_dir_name, 1);
        if (ret < 0)
        {
            PERR("wr_sysfs_oneint() fail, ret = %d", ret);
            return 0;
        }

        accl_scan_size = 16;

        sprintf(accl_buffer_access, "/dev/iio:device%d", accl_dev_num);
        PDEBUG("accl_buffer_access: %s\n", accl_buffer_access);

        /* Attempt to open non blocking the access dev */
        accl_iio_fd = open(accl_buffer_access, O_RDONLY | O_NONBLOCK);
        if (accl_iio_fd == -1)
        {
            PERR("Failed to open %s\n", accl_buffer_access);
        }

        /*set BMI160 RANGE INT and WM*/
        ret = 0;

        switch(accl_range){
            case ACC_CHIP_RANGCONF_4G:
                ret += wr_sysfs_oneint("acc_range", iio_dev0_dir_name, BMI160_ACCEL_RANGE_4G);
                BMI160_acc_resl *= 2;
                break;
            case ACC_CHIP_RANGCONF_8G:
                ret += wr_sysfs_oneint("acc_range", iio_dev0_dir_name, BMI160_ACCEL_RANGE_8G);
                BMI160_acc_resl *= 4;
                break;
            case ACC_CHIP_RANGCONF_16G:
                ret += wr_sysfs_oneint("acc_range", iio_dev0_dir_name, BMI160_ACCEL_RANGE_16G);
                BMI160_acc_resl *= 8;
                break;
            default:
                break;
        }

        ret += wr_sysfs_twoint("enable_int", iio_dev0_dir_name, 13, 1);
        ret += wr_sysfs_oneint("fifo_watermark", iio_dev0_dir_name, default_watermark);
        if (ret < 0)
        {
            PERR("write_sysfs() fail, ret = %d", ret);
            return ret;
        }
    }else if(ACC_CHIP_BMA2x2 == accl_chip)
    {
        accl_device_name = "bma2x2";

        open_input_by_name(accl_device_name, &acc_input_fd, &acc_input_num);
        if (-1 == acc_input_fd)
        {
            PERR("Failed to open input event\n");
            return -ENODEV;
        }

        PDEBUG("acc input_num = %d", acc_input_num);
        sprintf(acc_input_dir_name, "/sys/class/input/input%d", acc_input_num);

        driver_show_ver(acc_input_dir_name);

        switch(accl_range){
            case ACC_CHIP_RANGCONF_2G:
                ret += wr_sysfs_oneint("range", acc_input_dir_name, BMA2X2_RANGE_2G);
                BMA255_acc_resl *= 1;
                break;
            case ACC_CHIP_RANGCONF_4G:
                ret += wr_sysfs_oneint("range", acc_input_dir_name, BMA2X2_RANGE_4G);
                BMA255_acc_resl *= 2;
                break;
            case ACC_CHIP_RANGCONF_8G:
                ret += wr_sysfs_oneint("range", acc_input_dir_name, BMA2X2_RANGE_8G);
                BMA255_acc_resl *= 4;
                break;
            case ACC_CHIP_RANGCONF_16G:
                ret += wr_sysfs_oneint("range", acc_input_dir_name, BMA2X2_RANGE_16G);
                BMA255_acc_resl *= 8;
                break;
            default:
                break;
        }
    }

    return 0;
}


static int32_t ap_hwcntl_init_GYRO()
{
    int32_t ret;
    int32_t gyro_dev_num;
    char gyro_dev_dir_name[128];
    char gyro_buf_dir_name[128];
    char gyro_buffer_access[128];
    const char *gyro_device_name = NULL;

    if(GYR_CHIP_BMI160 == gyro_chip)
    {
        gyro_device_name = "bmi160_gyro";

        /* Find the device requested */
        gyro_dev_num = get_IIOnum_by_name(gyro_device_name, iio_dir);
        if (gyro_dev_num < 0)
        {
            PERR("Failed to find the %s\n", gyro_device_name);
            return -ENODEV;
        }

        PDEBUG("iio device number being used is gyr %d\n", gyro_dev_num);
        sprintf(gyro_dev_dir_name, "%siio:device%d", iio_dir, gyro_dev_num);

        /* Construct the directory name for the associated buffer.*/
        sprintf(gyro_buf_dir_name, "%siio:device%d/buffer", iio_dir, gyro_dev_num);

        /* Setup ring buffer parameters */
        ret = wr_sysfs_oneint("length", gyro_buf_dir_name, hwdata_unit_toread);
        if (ret < 0)
        {
            PERR("wr_sysfs_oneint() fail");
            return 0;
        }

        ret = wr_sysfs_oneint("enable", gyro_buf_dir_name, 1);
        if (ret < 0)
        {
            PERR("wr_sysfs_oneint() fail");
            return 0;
        }

        gyro_scan_size = 16;

        sprintf(gyro_buffer_access, "/dev/iio:device%d", gyro_dev_num);
        PDEBUG("gyro_buffer_access: %s\n", gyro_buffer_access);

        /* Attempt to open non blocking the access dev */
        gyro_iio_fd = open(gyro_buffer_access, O_RDONLY | O_NONBLOCK);
        if (gyro_iio_fd == -1)
        { /*If it isn't there make the node */
            PERR("Failed to open %s\n", gyro_buffer_access);
        }
    }else if(GYR_CHIP_BMG160 ==  gyro_chip)
    {
        gyro_device_name = "bmg160";

        open_input_by_name(gyro_device_name, &gyr_input_fd, &gyr_input_num);
        if (-1 == gyr_input_fd)
        {
            PERR("Failed to open input event\n");
            return -ENODEV;
        }

        PDEBUG("gyr input_num = %d", gyr_input_num);
        sprintf(gyr_input_dir_name, "/sys/class/input/input%d", gyr_input_num);

    }

    return 0;

}


static int32_t ap_hwcntl_init_MAGN()
{
    int32_t ret;
    int32_t magn_dev_num;
    char magn_dev_dir_name[128];
    char magn_buf_dir_name[128];
    char magn_buffer_access[128];
    const char *magn_device_name = NULL;

    /* Find the device requested */
    if(MAG_CHIP_BMI160 == magn_chip)
    {
        magn_device_name = "bmi160_magn";

        magn_dev_num = get_IIOnum_by_name(magn_device_name, iio_dir);
        if (magn_dev_num < 0)
        {
            PERR("Failed to find the %s\n", magn_device_name);
            return -ENODEV;
        }

        PDEBUG("iio device number being used is mag %d\n", magn_dev_num);

        sprintf(magn_dev_dir_name, "%siio:device%d", iio_dir, magn_dev_num);

        /* Construct the directory name for the associated buffer.*/
        sprintf(magn_buf_dir_name, "%siio:device%d/buffer", iio_dir, magn_dev_num);

        /* Setup ring buffer parameters */
        ret = wr_sysfs_oneint("length", magn_buf_dir_name, hwdata_unit_toread);
        if (ret < 0)
        {
            PERR("wr_sysfs_oneint() fail");
            return 0;
        }

        ret = wr_sysfs_oneint("enable", magn_buf_dir_name, 1);
        if (ret < 0)
        {
            PERR("wr_sysfs_oneint() fail");
            return 0;
        }

        magn_scan_size = 16;

        sprintf(magn_buffer_access, "/dev/iio:device%d", magn_dev_num);
        PDEBUG("magn_buffer_access: %s\n", magn_buffer_access);

        /* Attempt to open non blocking the access dev */
        magn_iio_fd = open(magn_buffer_access, O_RDONLY | O_NONBLOCK);
        if (magn_iio_fd == -1)
        { /*If it isn't there make the node */
            PERR("Failed to open %s\n", magn_buffer_access);
        }

    }else if(MAG_CHIP_BMM150 == magn_chip || MAG_CHIP_AKM09912 == magn_chip || MAG_CHIP_AKM09911 == magn_chip ||
            MAG_CHIP_YAS537 ==  magn_chip || MAG_CHIP_YAS532 ==  magn_chip)
    {
        if(MAG_CHIP_BMM150 == magn_chip){
            magn_device_name = "bmm050";
        }else if(MAG_CHIP_AKM09912 == magn_chip){
            magn_device_name = "akm09912";
        }else if(MAG_CHIP_AKM09911 == magn_chip){
            magn_device_name = "akm09911";
        }else if(MAG_CHIP_YAS537 ==  magn_chip){
            magn_device_name = "yas537";
        }else if( MAG_CHIP_YAS532 ==  magn_chip){
            magn_device_name = "yas532";
        }

        open_input_by_name(magn_device_name, &magn_input_fd, &magn_input_num);
        if (-1 == magn_input_fd)
        {
            PERR("Failed to open input event\n");
            return -ENODEV;
        }
        close(magn_input_fd);

        PDEBUG("magn_input_num = %d", magn_input_num);
        sprintf(mag_input_dir_name, "/sys/class/input/input%d", magn_input_num);
    }

    return 0;

}


int32_t hwcntl_init(BstSensor *bstsensor)
{
    int32_t ret = 0;
    struct itimerspec timerspec;

    ap_show_ver();

    bstsensor->pfun_get_sensorlist = ap_get_sensorlist;
    bstsensor->pfun_activate = ap_activate;
    bstsensor->pfun_batch = ap_batch;
    bstsensor->pfun_flush = ap_flush;
    if(SOLUTION_IMU == solution_type)
    {
        bstsensor->pfun_hw_deliver_sensordata = IMU_hw_deliver_sensordata;
        ret = ap_hwcntl_init_ACC();
        ret = ap_hwcntl_init_GYRO();
    }else
    {
        PERR("Unkown solution type: %d", solution_type);
    }

    if(MAG_CHIP_BMM150 == magn_chip || MAG_CHIP_AKM09912 == magn_chip || MAG_CHIP_AKM09911 == magn_chip ||
                    MAG_CHIP_YAS537 ==  magn_chip || MAG_CHIP_YAS532 ==  magn_chip)
    {
        poll_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if(-1 == poll_timer_fd)
        {
            PERR("Failed to create timer\n");
            return -ENODEV;
        }
    }


    return ret;
}

