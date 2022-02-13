/*******************************************************************************
 * Copyright (c) 2020 Renesas Electronics Corporation
 * All Rights Reserved.
 *
 * This code is proprietary to Renesas, and is license pursuant to the terms and
 * conditions that may be accessed at:
 * https://www.renesas.com/eu/en/document/msc/idt-software-license-terms-gas-sensor-software
 *
 ******************************************************************************/

/**
 * @file    zmod4510_config_oaq2.h
 * @brief   This is the configuration for ZMOD4510 module - oaq_2nd_gen library
 * @version 3.0.0
 * @author Renesas Electronics Corporation
 */

#ifndef _ZMOD4510_CONFIG_OAQ_2ND_GEN_H_
#define _ZMOD4510_CONFIG_OAQ_2ND_GEN_H_

#include <stdio.h>
#include <zmod4xxx_types.h>

#define INIT    0
#define MEASURE 1

/**********************************/
/* < Define product ID > */
#define ZMOD4510_PID 0x6320

/**********************************/
/* < Define I2C slase address > */
#define ZMOD4510_I2C_ADDR 0x33

/**********************************/
/* < Define product data length > */
#define ZMOD4510_PROD_DATA_LEN 10

/*************************************/
/* < Define ADC result data length > */
#define ZMOD4510_ADC_DATA_LEN 18

/* < Define limit of counter > */
#define ZMOD4510_OAQ2_COUNTER_LIMIT 10U

#define ZMOD4XXX_H_ADDR 0x40
#define ZMOD4XXX_D_ADDR 0x50
#define ZMOD4XXX_M_ADDR 0x60
#define ZMOD4XXX_S_ADDR 0x68

uint8_t data_set_4510_init[] = {
                                0x00, 0x50,
                                0x00, 0x28, 0xC3, 0xE3,
                                0x00, 0x00, 0x80, 0x40};

uint8_t data_set_4510_oaq_2nd_gen[] = {
                                0x00, 0x50, 0xFE, 0x70,
                                0x00, 0x10,
                                0x23, 0x03,
                                0x00, 0x00, 0x06, 0x41,
                                0x06, 0x41, 0x06, 0x41,
                                0x06, 0x41, 0x06, 0x41,
                                0x06, 0x41, 0x06, 0x41,
                                0x86, 0x41
                                };

zmod4xxx_conf zmod_oaq_sensor_type[] = {
    [INIT] = {
        .start = 0x80,
        .h = { .addr = ZMOD4XXX_H_ADDR, .len = 2, .data_buf = &data_set_4510_init[0]},
        .d = { .addr = ZMOD4XXX_D_ADDR, .len = 2, .data_buf = &data_set_4510_init[2]},
        .m = { .addr = ZMOD4XXX_M_ADDR, .len = 2, .data_buf = &data_set_4510_init[4]},
        .s = { .addr = ZMOD4XXX_S_ADDR, .len = 4, .data_buf = &data_set_4510_init[6]},
        .r = { .addr = 0x97, .len = 4},
    },

    [MEASURE] = {
        .start = 0x80,
        .h = {.addr = ZMOD4XXX_H_ADDR, .len = 4, .data_buf = &data_set_4510_oaq_2nd_gen[0]},
        .d = {.addr = ZMOD4XXX_D_ADDR, .len = 2, .data_buf = &data_set_4510_oaq_2nd_gen[4]},
        .m = {.addr = ZMOD4XXX_M_ADDR, .len = 2, .data_buf = &data_set_4510_oaq_2nd_gen[6]},
        .s = {.addr = ZMOD4XXX_S_ADDR, .len = 18, .data_buf = &data_set_4510_oaq_2nd_gen[8]},
        .r = {.addr = 0x97, .len = 18},
        .prod_data_len = ZMOD4510_PROD_DATA_LEN,
    },
};

#endif //_ZMOD4510_CONFIG_OAQ_2ND_GEN_H_
