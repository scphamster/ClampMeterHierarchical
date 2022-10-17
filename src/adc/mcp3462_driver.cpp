#ifdef min
#undef min
#endif   // max
#ifdef max
#undef max
#endif   // max
#ifdef printf
#undef printf
#endif

#include "pio.h"
#include "pmc.h"
#include "spi.h"

#include "mcp3462_driver.hpp"

auto constexpr CLOCK_PCK_ID      = 2;
auto constexpr CLOCK_PCK_PRES    = 0;
auto const CLOCK_PIO             = PIOA;
auto constexpr CLOCK_PIO_PIN_MSK = 0x40000UL;
auto constexpr CLOCK_PIO_PERIPH  = PIO_PERIPH_B;

void
MCP3462_driver::ClockInit() noexcept
{
    pmc_enable_pck(CLOCK_PCK_ID);

    while (!(pmc_is_pck_enabled(CLOCK_PCK_ID))) {
        ;
    }

    if (pmc_switch_pck_to_mainck(CLOCK_PCK_ID, CLOCK_PCK_PRES)) {
        while (1) {   // todo: make better implementation
            ;
        }
    }

    pmc_disable_pck(CLOCK_PCK_ID);
    pio_set_peripheral(CLOCK_PIO, CLOCK_PIO_PERIPH, CLOCK_PIO_PIN_MSK);
}

void
MCP3462_driver::Initialize() noexcept
{
    uint16_t config0_regval = 0;
    uint16_t config1_regval = 0;
    uint16_t config2_regval = 0;
    uint16_t config3_regval = 0;
    uint16_t irq_regval     = 0;
    uint16_t mux_regval     = 0;
    uint16_t firstbyte      = 0;
    uint16_t retval1        = 0;
    uint16_t retval2        = 0;

    //    config0_regval = ADC_MODE(ADC_CONV_MODE) | CS_SEL(CS_SEL_0) | CLK_SEL(CLK_SEL_EXT) | CONFIG0(CONFIG0_ACTIVE);
    //    config1_regval = PRE(PRE_0) | OSR(OSR_256);
    //    config2_regval = BOOST(BOOST_2) | GAIN(GAIN_1) | AZ_MUX(0) | 1;
    //    config3_regval =
    //      CONV_MODE(CONV_MODE_CONT) | DATA_FORMAT(0) | CRC_FORMAT(0) | EN_CRCCOM(0) | EN_OFFCAL(0) | EN_GAINCAL(0);
//    irq_regval = 0x4;
//    mux_regval = MUX_SET_VPOS(REF_CH4) | REF_CH5;


    config0_regval =
      CreateConfig0RegisterValue(false, ClockSelection::CLK_SEL_EXT, CurrentSource::CS_SEL_0, ADC_MODE::Conversion);

    config1_regval = CreateConfig1RegisterValue(Prescaler::PRE_0, OSR::OSR_256);
    config2_regval = CreateConfig2RegisterValue(Boost::BOOST_2, Gain::GAIN_1, AutoZeroingMuxMode::Disabled);
    config3_regval = CreateConfig3RegisterValue(ConversionMode::CONV_MODE_CONT,
                                                DataFormat::Default16Bit,
                                                CRC_Format::CRC_16Only,
                                                false,
                                                false,
                                                false);
    irq_regval = CreateIRQRegisterValue(false, false, IRQ_PinMode::IRQ, IRQ_InactivePinMode::LogicHigh);
    mux_regval = CreateMUXRegisterValue(Reference::CH4, Reference::CH5);

    firstbyte  = DEVICE_ADDR | CMD_ADDR(CONFIG0_REG_ADDR) | CMD_INCREMENTAL_WRITE;

    spi_write(SPI, firstbyte, 0, 0);
    spi_read(SPI, &retval1, nullptr);
    spi_write(SPI, config0_regval, 0, 0);
    spi_write(SPI, config1_regval, 0, 0);
    spi_write(SPI, config2_regval, 0, 0);
    spi_write(SPI, config3_regval, 0, 0);
    spi_write(SPI, irq_regval, 0, 0);
    spi_write(SPI, mux_regval, 0, 0);
    spi_set_lastxfer(SPI);
}

Byte
MCP3462_driver::CreateConfig0RegisterValue(bool                           if_full_shutdown,
                                           MCP3462_driver::ClockSelection clock,
                                           MCP3462_driver::CurrentSource  current_source,
                                           MCP3462_driver::ADC_MODE       adc_mode) noexcept
{
    auto constexpr CONFIG0_REG_ADDR = 0x1;
    auto constexpr CONFIG0_POS      = 0x6;
    auto constexpr CONFIG0_MSK      = 0xC0;
    auto constexpr CLK_SEL_POS      = 0x4;
    auto constexpr CLK_SEL_MSK      = 0x30;
    auto constexpr CS_SEL_POS       = 0x2;
    auto constexpr CS_SEL_MSK       = 0xC;
    auto constexpr ADC_MODE_POS     = 0x0;
    auto constexpr ADC_MODE_MSK     = 0x3;

    return (ADC_MODE_MSK bitand (static_cast<Byte>(adc_mode) << ADC_MODE_POS)) bitor
           (CS_SEL_MSK bitand (static_cast<std::underlying_type_t<CurrentSource>>(current_source) << CS_SEL_POS)) bitor
           (CLK_SEL_MSK bitand (static_cast<std::underlying_type_t<ClockSelection>>(clock) << CLK_SEL_POS)) bitor
           (CONFIG0_MSK bitand (static_cast<Byte>(if_full_shutdown) << CONFIG0_POS));
}
Byte
MCP3462_driver::CreateConfig1RegisterValue(MCP3462_driver::Prescaler prescaler,
                                           MCP3462_driver::OSR       oversampling_ratio) noexcept
{
    auto constexpr PRE_POS = 0x6;
    auto constexpr PRE_MSK = 0xC0;
    auto constexpr OSR_POS = 0x2;
    auto constexpr OSR_MSK = 0x3C;

    return (PRE_MSK bitand (static_cast<std::underlying_type_t<Prescaler>>(prescaler) << PRE_POS)) bitor
           (OSR_MSK bitand (static_cast<std::underlying_type_t<OSR>>(oversampling_ratio) << OSR_POS));
}
Byte
MCP3462_driver::CreateConfig2RegisterValue(MCP3462_driver::Boost              boost_value,
                                           MCP3462_driver::Gain               gain,
                                           MCP3462_driver::AutoZeroingMuxMode autozero) noexcept
{
    auto constexpr BOOST_POS    = 0x6;
    auto constexpr BOOST_MSK    = 0xC0;
    auto constexpr GAIN_POS     = 0x3;
    auto constexpr GAIN_MSK     = 0x38;
    auto constexpr AZ_MUX_POS   = 0x2;
    auto constexpr AZ_MUX_MSK   = 0x4;
    auto constexpr overall_mask = 0xfe;

    return ((BOOST_MSK bitand (static_cast<std::underlying_type_t<Boost>>(boost_value) << BOOST_POS)) bitor
            (GAIN_MSK bitand (static_cast<std::underlying_type_t<Gain>>(gain) << GAIN_POS)) bitor
            (AZ_MUX_MSK bitand (static_cast<std::underlying_type_t<AutoZeroingMuxMode>>(autozero) << AZ_MUX_POS))) bitand
           overall_mask;
}
Byte
MCP3462_driver::CreateConfig3RegisterValue(ConversionMode conversion_mode,
                                           DataFormat     data_format,
                                           CRC_Format     crc_format,
                                           bool           enable_crc_checksum,
                                           bool           enable_offset_calibration,
                                           bool           enable_gain_calibration) noexcept
{
    auto constexpr CONV_MODE_POS   = 0x6;
    auto constexpr CONV_MODE_MSK   = 0xC0;
    auto constexpr DATA_FORMAT_POS = 0x4;
    auto constexpr DATA_FORMAT_MSK = 0x30;
    auto constexpr CRC_FORMAT_POS  = 0x3;
    auto constexpr CRC_FORMAT_MSK  = 0x8;
    auto constexpr EN_CRCCOM_POS   = 0x2;
    auto constexpr EN_CRCCOM_MSK   = 0x4;
    auto constexpr EN_OFFCAL_POS   = 0x1;
    auto constexpr EN_OFFCAL_MSK   = 0x2;
    auto constexpr EN_GAINCAL_POS  = 0x0;
    auto constexpr EN_GAINCAL_MSK  = 0x1;

    // clang-format off
    return (CONV_MODE_MSK bitand (static_cast<std::underlying_type_t<ConversionMode>>(conversion_mode) << CONV_MODE_POS)) bitor
           (DATA_FORMAT_MSK bitand (static_cast<std::underlying_type_t<DataFormat>>(data_format) << DATA_FORMAT_POS)) bitor
           (EN_CRCCOM_MSK bitand (static_cast<std::underlying_type_t<CRC_Format>>(crc_format) << CRC_FORMAT_POS)) bitor
           (EN_CRCCOM_MSK bitand (static_cast<Byte>(enable_crc_checksum) << EN_CRCCOM_POS)) bitor
           (EN_OFFCAL_MSK bitand (static_cast<Byte>(enable_offset_calibration) << EN_OFFCAL_POS)) bitor
           (EN_GAINCAL_MSK bitand (static_cast<Byte>(enable_gain_calibration) << EN_GAINCAL_POS));
    // clang-format on
}
Byte
MCP3462_driver::CreateIRQRegisterValue(bool                enable_conversion_start_interrupt,
                                       bool                enable_fast_command,
                                       IRQ_PinMode         irq_pin_mode,
                                       IRQ_InactivePinMode inactive_irq_pin_mode) noexcept
{
    auto constexpr WRITE_MASK                   = 0x0f;
    auto constexpr conversion_start_int_mask    = 0x01;
    auto constexpr conversion_start_int_pos     = 0;
    auto constexpr fast_cmd_mask                = 0x02;
    auto constexpr fast_cmd_pos                 = 1;
    auto constexpr irq_inactive_pin_config_mask = 0x04;
    auto constexpr irq_inactive_pin_config_pos  = 2;
    auto constexpr irq_pin_mode_mask            = 0x08;
    auto constexpr irq_pin_mode_pos             = 3;

    return (conversion_start_int_mask bitand
            (static_cast<Byte>(enable_conversion_start_interrupt) << conversion_start_int_pos)) bitor
           (fast_cmd_mask bitand (static_cast<Byte>(enable_fast_command) << fast_cmd_pos)) bitor
           (irq_inactive_pin_config_mask bitand
            (static_cast<std::underlying_type_t<IRQ_InactivePinMode>>(inactive_irq_pin_mode)
             << irq_inactive_pin_config_pos)) bitor
           (irq_pin_mode_mask bitand (static_cast<std::underlying_type_t<IRQ_PinMode>>(irq_pin_mode))
                                       << irq_pin_mode_pos);
}
Byte
MCP3462_driver::CreateMUXRegisterValue(MCP3462_driver::Reference positive_channel,
                                       MCP3462_driver::Reference negative_channel) noexcept
{
    auto constexpr positive_channel_pos  = 4;
    auto constexpr positive_channel_mask = 0xf0;
    auto constexpr negative_channel_mask = 0x0f;
    auto constexpr negative_channel_pos  = 0;

    return (positive_channel_mask bitand
            (static_cast<std::underlying_type_t<Reference>>(positive_channel) << positive_channel_pos)) bitor
           (negative_channel_mask bitand
            (static_cast<std::underlying_type_t<Reference>>(negative_channel) << negative_channel_pos));
}

// void
// MCP3462_init(void)
//{
//     clock_init();
//     send_configuration();
// }
//
// void
// MCP3462_set_gain(gain_type_t gain)
//{
//     uint32_t timeout_counter = SPI_TIMEOUT;
//     uint16_t firstbyte;
//     uint16_t config2_byte;
//     uint16_t retval;
//     firstbyte    = DEVICE_ADDR | CMD_ADDR(CONFIG2_REG_ADDR) | CMD_INCREMENTAL_WRITE;
//     config2_byte = BOOST(BOOST_2) | GAIN(gain) | AZ_MUX(0) | 1;
//
//     while ((!spi_is_tx_ready(SPI)) && (timeout_counter--))
//         ;
//
//     if (!timeout_counter)
//         return;
//
//     spi_configure_cs_behavior(SPI, 0, SPI_CS_RISE_FORCED);
//     spi_set_bits_per_transfer(SPI, 0, SPI_CSR_BITS_16_BIT);
//
//     spi_write(SPI, (firstbyte << 8) | config2_byte, 0, 0);
//     spi_set_lastxfer(SPI);
// }
//
// void
// MCP3462_set_mux(uint8_t positive_ch, uint8_t negative_ch)
//{
//     uint32_t timeout_counter = SPI_TIMEOUT;
//     uint16_t firstbyte;
//     uint16_t mux_byte;
//     uint16_t retval;
//
//     if ((positive_ch > REF_CH5) || (negative_ch > REF_CH5))
//         return;
//
//     firstbyte = DEVICE_ADDR | CMD_ADDR(MUX_REG_ADDR) | CMD_INCREMENTAL_WRITE;
//     mux_byte  = MUX_SET_VPOS(positive_ch) | negative_ch;
//
//     while ((!spi_is_tx_ready(SPI)) && (timeout_counter--))
//         ;
//
//     if (!timeout_counter)
//         return;
//
//     spi_configure_cs_behavior(SPI, 0, SPI_CS_RISE_FORCED);
//     spi_set_bits_per_transfer(SPI, 0, SPI_CSR_BITS_16_BIT);
//
//     spi_write(SPI, (firstbyte << 8) | mux_byte, 0, 0);
//     spi_set_lastxfer(SPI);
// }
//
// int32_t
// MCP3462_read(uint16_t data)
//{
//     uint32_t timeout_counter = SPI_TIMEOUT;
//     uint8_t  firstbyte       = 0;
//     uint16_t readout         = 0;
//     int32_t  retval          = 0;
//
//     uint16_t data0 = 0;
//     uint16_t data1 = 0;
//     uint16_t data2 = 0;
//
//     firstbyte = DEVICE_ADDR | CMD_ADDR(ADCDATA_ADDR) | CMD_STATIC_READ;
//
//     while ((!spi_is_tx_ready(SPI)) && (timeout_counter--))
//         ;
//
//     if (!timeout_counter)
//         return 0;
//
//     spi_set_bits_per_transfer(SPI, 0, SPI_CSR_BITS_8_BIT);
//     spi_configure_cs_behavior(SPI, 0, SPI_CS_KEEP_LOW);
//
//     spi_write(SPI, firstbyte, 0, 0);
//     spi_read(SPI, &readout, NULL);
//     spi_write(SPI, 0, 0, 0);
//     spi_read(SPI, &data0, NULL);
//     spi_write(SPI, 0, 0, 0);
//     spi_read(SPI, &data1, NULL);
//     spi_configure_cs_behavior(SPI, 0, SPI_CS_RISE_FORCED);
//     spi_write(SPI, 0, 0, 0);
//     spi_read(SPI, &data2, NULL);
//     spi_set_lastxfer(SPI);
//
//     retval = data2 | (data1 << 8) | (data0 << 16);
//
//     // sign bit
//     if (retval & (1 << 23)) {
//         retval--;
//         retval = -(~retval & (0xffffff));
//     }
//
//     return retval;
// }
//
// void
// MCP3462_enable_clock(void)
//{
//     pmc_enable_pck(CLOCK_PCK_ID);
//     // TODO add feedback about current status
// }
//
// void
// MCP3462_disable_clock(void)
//{
//     pmc_disable_pck(CLOCK_PCK_ID);
//     // TODO add feedback about current status
// }
