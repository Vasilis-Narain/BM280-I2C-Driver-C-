#include "i2c_state_machine.h"
/*
    * Interrupt notes:
    *  Use NVIC_ISER to set/read enabled state of interrupts.
    *
    *  Use NVIC_ICER to clear/read enabled state of interrupts.
    *
    *  Use NVIC_ISPR to set/read pending state of interrupts
    *
    *  Use NVIC_ICPR to clear/read pending state of interrupts
    *
    *  Use NVIC_IABR to show active state of each interrupt
    *
    *  Use NVIC_IPRn to set/read interrupt priorities
    *
    * 
*/

/* Pico specific interrupt notes:
 *
 * First enable correct irq using NVIC_ISER (from above)
 *
 * Use IC_INTR_MASK register to mask/unmask interrupts (0 masked, 1 unmasked)
 * Use IC_INTR_STATUS register to read active status of masked registers (0 inactive, 1 active)
 * Use IC_RAW_INTR_STAT register to read active status of raw registers (0 inactive, 1 active)
 * Use CLR_INTR reigster to clear ALL interrupts (just need to read it)
 * Use IC_CLEAR_* registers to clear individual * interrupts
 *
 * START_DET -> start/restart occured
 * STOP_DET -> stop occured (when the sensor is done)
 * TX_OVER -> transmit buffer is full and processor tries to load extra byte (used to possibly retry on next state machine iter)
 * TX_EMPTY -> at or below threshold value set in IC_TX_TL register HW CLEAR ONLY
 * RX_FULL -> receive buffer is full (used to possibly retry on next state machine iter) HW CLEAR ONLY
 * RX_OVER -> receive buffer is full and bme280 tries to send an extra byte (used to possibly retry on next state machine iter)
 * RX_UNDER -> processor attempts to read receive buffer when empty
 *
*/
static void pump_tx();
static void drain_rx();
static void clear_rx();

//NOTE(vasilis): these are the I2C interrupt handlers:
//void __attribute__((weak, alias("_DEFAULT_Handler"))) I2C0_IRQ_Handler();
//void __attribute__((weak, alias("_DEFAULT_Handler"))) I2C1_IRQ_Handler();
//

// Make sure to `#define SDA_PIN` and `#define SCL_PIN` to be used by pico.
// Defaults are 14 and 15 respectively.
void i2c_init_master() {
    // Setup pads
    hw_clear_bits(&pads_bank0_hw->io[SDA_PIN], PADS_I2C_CLEAR);
    hw_clear_bits(&pads_bank0_hw->io[SCL_PIN], PADS_I2C_CLEAR);
    hw_set_bits(&pads_bank0_hw->io[SDA_PIN], PADS_I2C_SET);
    hw_set_bits(&pads_bank0_hw->io[SCL_PIN], PADS_I2C_SET);

    // Most i2c config requires enable = 0.
    i2c_hw->enable = 0;
    while (i2c_hw->enable_status & I2C_IC_ENABLE_STATUS_IC_EN_BITS) {}

    // Config I2C1 as master
    i2c_hw->con = I2C_INIT_SET;

    // Specify target address
    i2c_hw->tar = BME280_I2C_ADDR_PRIM;

    // The following numbers calculated from specification formulas for 12Mhz clk_sys
    // lcnt and hcnt ar ic_clk settings
    //
    // TODO(vasilis): calculate these at comptime rather than hardcode
    //
    i2c_hw->ss_scl_hcnt = 48;
    i2c_hw->ss_scl_lcnt = 72;
    i2c_hw->fs_spklen = 1;
    i2c_hw->sda_hold = 4;

    i2c_hw->enable = 1;
}

void i2c_blocking_read_command(u8 address, u8 *byte) {
    while (!(i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) {}
    i2c_hw->data_cmd = address;

    while (!(i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) {}
    i2c_hw->data_cmd = I2C_IC_DATA_CMD_CMD_BITS | I2C_IC_DATA_CMD_RESTART_BITS | I2C_IC_DATA_CMD_STOP_BITS;

    while (i2c_hw->rxflr == 0) {}
    *byte = i2c_hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS;
}

b32 i2c_blocking_bulk_read_command(u8 start_address, u8 *buffer, u32 length) {
    while (!(i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) {}
    i2c_hw->data_cmd = start_address;

    u32 issued = 0;
    u32 received = 0;
    while (received < length) {
        if ((issued < length) && (i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) { // same as TX_EMPTY interrupt
            u32 cmd = I2C_IC_DATA_CMD_CMD_BITS;
            if (issued == 0) {
                cmd |= I2C_IC_DATA_CMD_RESTART_BITS;
            }
            if (issued == length - 1) {
                cmd |= I2C_IC_DATA_CMD_STOP_BITS;
            }
            i2c_hw->data_cmd = cmd;
            issued++;
        }

        if (i2c_hw->status & I2C_IC_STATUS_RFNE_BITS) { // same as RX_FULL interrupt
            buffer[received++] = (u8)(i2c_hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS);
        }

        if (i2c_hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_ABRT_BITS) {
            (void)i2c_hw->clr_tx_abrt;
            return 1;
        }
    }
    return 0;
}

volatile i2c_state i2c1_state = I2C_IDLE;
typedef struct {
    u8 *buf;
    u32 issued;
    u32 received;
    u32 length;
    u32 abrt_source;
    u32 fault;
    u8 reg_addr;
} i2c_descriptor;

//static i2c_descriptor i2c0_descriptor;
static i2c_descriptor i2c1_descriptor;

u32 i2c_get_fault() {
    return i2c1_descriptor.fault;
}

u32 i2c_get_received() {
    return i2c1_descriptor.received;
}

u32 i2c_get_abrt_source() {
    return i2c1_descriptor.abrt_source;
}

u32 i2c_abrt_get_dropped() {
    return i2c1_descriptor.length - i2c1_descriptor.received;
}

void i2c_irq_enable(i2c_lane bus_lane) {
    if (bus_lane == I2C0) {
        i2c0_hw->intr_mask = (I2C_IC_INTR_MASK_M_TX_EMPTY_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS |
                              I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS |
                              I2C_IC_INTR_MASK_M_RX_OVER_BITS);
        m33_hw->nvic_iser[1] = 1u << 4;
    } else if (bus_lane == I2C1) {
        i2c1_hw->intr_mask = (I2C_IC_INTR_MASK_M_TX_EMPTY_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS |
                              I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS |
                              I2C_IC_INTR_MASK_M_RX_OVER_BITS);
        m33_hw->nvic_iser[1] = 1u << 5;
    }
}

b32 i2c_start_bulk_read_async(u8 reg_addr, u8 *buf, u32 len) {
    if (i2c1_state != I2C_IDLE) {
        return I2C_BUS_BUSY;
    }

    i2c1_descriptor = (i2c_descriptor){
        .buf = buf,
        .issued = 0,
        .received = 0,
        .length = len,
        .reg_addr = reg_addr,
        .abrt_source = 0,
    };

    i2c1_hw->data_cmd = reg_addr;
    i2c1_state = I2C_READING;
    i2c1_hw->intr_mask |= (I2C_IC_INTR_MASK_M_TX_EMPTY_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS);

    return 0;
}

void I2C1_IRQ_Handler() {
    u32 irq_status = i2c1_hw->intr_stat;

    if (irq_status & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
        u32 abrt_source = i2c1_hw->tx_abrt_source;
        (void)i2c1_hw->clr_tx_abrt;
        (void)i2c1_hw->clr_stop_det;
        if (i2c1_state == I2C_READING) {
            i2c1_descriptor.abrt_source = abrt_source;
            i2c1_hw->intr_mask &= ~(I2C_IC_INTR_MASK_M_TX_EMPTY_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS);
            clear_rx();
            i2c1_descriptor.fault |= I2C_FAULT_ABORT;
            i2c1_state = I2C_ERROR;
        }
        return;
    }

    if (irq_status & I2C_IC_INTR_STAT_R_RX_OVER_BITS) {
        (void)i2c1_hw->clr_rx_over;
        if (i2c1_state == I2C_READING) {
            i2c1_descriptor.fault |= I2C_FAULT_OVERRUN;
            i2c1_state = I2C_ERROR;
        }
    }

    if (irq_status & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
        drain_rx();
    }

    if (irq_status & I2C_IC_INTR_STAT_R_TX_EMPTY_BITS) {
        pump_tx();
    }

    if (irq_status & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
        (void)i2c1_hw->clr_stop_det;
        if (i2c1_state == I2C_READING) {
            drain_rx();
            i2c1_state = (i2c1_descriptor.received == i2c1_descriptor.length) ? I2C_DONE : I2C_ERROR;
        }
    }
}

static void drain_rx() {
    while ((i2c1_descriptor.received < i2c1_descriptor.issued) && (i2c_hw->status & I2C_IC_STATUS_RFNE_BITS)) { // same as RX_FULL interrupt
        i2c1_descriptor.buf[i2c1_descriptor.received++] = (u8)(i2c_hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS);
    }
}

static void pump_tx() {
    while ((i2c1_descriptor.issued < i2c1_descriptor.length) && (i2c_hw->status & I2C_IC_STATUS_TFNF_BITS)) { // same as TX_EMPTY interrupt
        u32 cmd = I2C_IC_DATA_CMD_CMD_BITS;
        if (i2c1_descriptor.issued == 0) {
            cmd |= I2C_IC_DATA_CMD_RESTART_BITS;
        }
        if (i2c1_descriptor.issued == i2c1_descriptor.length - 1) {
            cmd |= I2C_IC_DATA_CMD_STOP_BITS;
        }
        i2c_hw->data_cmd = cmd;
        i2c1_descriptor.issued++;
    }
    if (i2c1_descriptor.issued == i2c1_descriptor.length) {
        i2c1_hw->intr_mask &= ~I2C_IC_INTR_MASK_M_TX_EMPTY_BITS;
    }
}

static void clear_rx() {
    while (i2c1_hw->rxflr) {
        (void)(i2c_hw->data_cmd & I2C_IC_DATA_CMD_DAT_BITS);
    }
}
