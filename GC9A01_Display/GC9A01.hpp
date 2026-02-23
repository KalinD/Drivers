#ifndef GC9A01_HPP
#define GC9A01_HPP

#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define ON  1
#define OFF 0

#define MAX_HEIGHT 240U
#define MAX_WIDTH 240U
#define RGB_COUNT 3U

typedef enum {
    PF12BitsPerPixel,
    PF16BitsPerPixel,
    PF18BitsPerPixel,
} PixelFormat;

typedef enum {
#ifdef READ_SUPPORT
    ReadDisplayIdentificationInformation2 = 0x04,
    ReadDisplayStatus = 0x09,
#endif
    EnterSleepMode = 0x10,
    SleepOUT = 0x11,
    PartialMode = 0x12,
    NormalMode = 0x13,
    InversionOFF = 0x20,
    InversionON = 0x21,
    DisplayOFF = 0x28,
    DisplayON = 0x29,
    ColumnAddressSet = 0x2A,
    RowAddressSet = 0x2B,
    MemoryWrite = 0x2C,
    PartialArea = 0x30,
    VerticalScrollingDefinition = 0x33,
    TearingEffectLineOFF = 0x34,
    TearingEffectLineON = 0x35,
    MemoryAccessControl = 0x36,
    VerticalScrollingStartAddress = 0x37,
    IdleModeOFF = 0x38,
    IdleModeON = 0x39,
    COLMODPixelFormatSet = 0x3A,
    WriteMemoryContinue = 0x3C,
    SetTearScanline = 0x44,
#ifdef READ_SUPPORT
    GetScanline = 0x45,
#endif
    WriteDisplayBrightness = 0x51,
    WriteCTRLDisplay = 0x53,
#ifdef READ_SUPPORT
    ReadID1 = 0xDA,
    ReadID2 = 0xDB,
    ReadID3 = 0xDC,
#endif
} RegulativeCommandSet;

typedef enum {
    RGBInterfaceSignalControl = 0xB0,
    BlankingPorchControl = 0xB5,
    DisplayFunctionControl = 0xB6,
    TearingEffectControl = 0xBA,
    InterfaceControl = 0xF6,
} ExtendedCommandSet;

typedef enum {
    FrameRate = 0xE8,
    SPI2DataControl = 0xE9,
    PowerControl1 = 0xC1,
    PowerControl2 = 0xC3,
    PowerControl3 = 0xC4,
    PowerControl4 = 0xC9,
    PowerControl7 = 0xA7,
    InterRegisterEnable1 = 0xFE,
    InterRegisterEnable2 = 0xEF,
    SetGamma1 = 0xF0,
    SetGamma2 = 0xF1,
    SetGamma3 = 0xF2,
    SetGamma4 = 0xF3,
} InterCommandSet;

namespace SPI2DataControlOptions {
    // 65K color 1pixle/transition
    static constexpr unsigned char MDT_65K_1PixelPerTransition     = 0b000U;
    // 262K color 1pixle/transition
    static constexpr unsigned char MDT_262K_1PixelPerTransition    = 0b001U;
    // 262K color 2/3pixle/transition
    static constexpr unsigned char MDT_262K_2Or3PixelPerTransition = 0b010U;
    // 4M color 1pixle/transition
    static constexpr unsigned char MDT_4M_1PixelPerTransition      = 0b100U;
    // 4M color 2/3pixle/transition
    static constexpr unsigned char MDT_4M_2Or3PixelPerTransition   = 0b110U;
}

namespace FrameRateOptions {
    static constexpr unsigned char ColumnInversion   = 0x00U;
    static constexpr unsigned char OneDotInversion   = 0x10U;
    static constexpr unsigned char TwoDotInversion   = 0x20U;
    static constexpr unsigned char FourDotInversion  = 0x30U;
    static constexpr unsigned char EightDotInversion = 0x40U;
}

namespace InterfaceControlOptions {
    // DM [1:0]: Select the display operation mode.
    static constexpr unsigned char DM_InternalClockOperation = 0b00000000U;
    // DM [1:0]: Select the display operation mode.
    static constexpr unsigned char DM_RGBInterfaceMode = 0b00000100U;
    // DM [1:0]: Select the display operation mode.
    static constexpr unsigned char DM_VSYNCInterfaceMode = 0b00001000U;
    // DM [1:0]: Select the display operation mode. (DO NOT USE)
    static constexpr unsigned char DM_SettingDisabled = 0b00001100U;
    
    // RM: Select the interface to access the GRAM.
    static constexpr unsigned char RM_OtherInterfaces = 0b00000000U;
    // Set RM to "1" when writing display data by the RGB interface.
    static constexpr unsigned char RM_RGBInterface = 0b00000010U;
    
    // RIM: Specify the RGB interface mode when the RGB interface is used. These bits should be set before display operation through the RGB interface and should not be set during operation.
    static constexpr unsigned char RIM_18Or16BitRGBInterface = 0b00000000U;
    // RIM: Specify the RGB interface mode when the RGB interface is used. These bits should be set before display operation through the RGB interface and should not be set during operation.
    static constexpr unsigned char RIM_6BitRGBInterface = 0b00000001U;
}

namespace DisplayFunctionControlOptions {
    // Sets the direction of scan by the gate driver in the range determined by SCN [4:0] and NL [4:0]. G1 -> G32
    static constexpr unsigned char GS_OFF = 0b00000000U;
    // Sets the direction of scan by the gate driver in the range determined by SCN [4:0] and NL [4:0]. G32 -> G1
    static constexpr unsigned char GS_ON  = 0b01000000U;

    // Assigns R, G, B dots to the source driver pins from S1 to S360,
    static constexpr unsigned char SS_OFF = 0b00000000U;
    // Assigns R, G, B dots to the source driver pins from S360 to S1
    static constexpr unsigned char SS_ON  = 0b00100000U;
}

// Check GC9A01 datasheet, RGB Interface Signal Control (B0h) for more information
namespace RGBInterfaceSignalControlOptions {
    // DE polarity: High enable for RGB interface
    static constexpr unsigned char EPL_OFF = 0b00000000U;
    // DE polarity: Low enable for RGB interface
    static constexpr unsigned char EPL_ON  = 0b00000001U;

    // DOTCLK polarity: data fetched at the rising time
    static constexpr unsigned char DPL_OFF = 0b00000000U;
    // DOTCLK polarity: data fetched at the falling time
    static constexpr unsigned char DPL_ON  = 0b00000010U;

    // HSYNC polarity: Low level sync clock
    static constexpr unsigned char HSPL_OFF = 0b00000000U;
    // HSYNC polarity: High level sync clock
    static constexpr unsigned char HSPL_ON  = 0b00000100U;

    // VSYNC polarity: Low level sync clock
    static constexpr unsigned char VSPL_OFF = 0b00000000U;
    // VSYNC polarity: High level sync clock
    static constexpr unsigned char VSPL_ON  = 0b00001000U;
}

namespace WriteCTRLDisplayOptions {
    // Brightness registers are 00h
    static constexpr unsigned char BCTRL_OFF = 0b00000000U;
    // Brightness registers are active, according to the DBV[7..0] parameters.
    static constexpr unsigned char BCTRL_ON  = 0b00100000U;
    // Display Dimming is off
    static constexpr unsigned char DD_OFF = 0b00000000U;
    // Display Dimming is on
    static constexpr unsigned char DD_ON  = 0b00001000U;
    // Completely turn off backlight circuit. Control lines must be low.
    static constexpr unsigned char BL_OFF = 0b00000000U;
    // Backlight On
    static constexpr unsigned char BL_ON  = 0b00000100U;
}

namespace MemoryAccessControlOptions {
    // Theses 3 bits control mCU to memory write/read direction

    // Row address order
    static constexpr unsigned char MY = 0b10000000U;
    // Column address order
    static constexpr unsigned char MX = 0b01000000U;
    // Row / Column Exchange address order
    static constexpr unsigned char MV = 0b00100000U;
    // Vertical Regresh Order
    // LCD vertical refresh direction control
    static constexpr unsigned char ML = 0b00100000U;
    // RGB-BGR Order
    // Color selector switch control
    // (0=RGB color filter panel, 1=BGR color filter panel)
    static constexpr unsigned char RGB = 0b00000000U;
    // RGB-BGR Order
    // Color selector switch control
    // (0=RGB color filter panel, 1=BGR color filter panel)
    static constexpr unsigned char BGR = 0b00001000U;
    // Horizontal Refresh ORDER
    // LCD horizontal refresh direction control
    static constexpr unsigned char MH  = 0b00000100U;
}

/* This command sets the pixel format for the RGB image data used by the interface. DPI [2:0] is the
 * pixel format select of RGB interface and DBI [2:0] is the pixel format of MCU interface. If
 * a particular interface, either RGB interface or MCU interface, is not used then the
 * corresponding bits in the parameter are ignored.
 * If using RGB Interface must selction serial interface. */
namespace COLMOD {
    static constexpr unsigned char DPI16BitPerPixel = 0x50U;
    static constexpr unsigned char DPI18BitPerPixel = 0x60U;

    static constexpr unsigned char DBI12BitPerPixel = 0x03U;
    static constexpr unsigned char DBI16BitPerPixel = 0x05U;
    static constexpr unsigned char DBI18BitPerPixel = 0x06U;
}

class GC9A01
{
private:
    bool is_rgb;
    PixelFormat pf;
    spi_inst_t* spi_instance;
    unsigned char miso_pin;
    unsigned char cs_pin;
    unsigned char sck_pin;
    unsigned char mosi_pin;
    unsigned char rst_pin;
    unsigned char dc_pin;
    void ReMapToCorrectPixels(const unsigned char originalPixels[], const size_t pixelCount, unsigned char out[]) const;
    void GetNewImageSize(const size_t pixelCount, size_t* outSize) const;
    void HandlePixels(const unsigned char originalPixels[], size_t * const originalIndex, unsigned char out[], size_t * const outIndex) const;
public:
    GC9A01(spi_inst_t* spi_instance, unsigned char miso_pin, unsigned char cs_pin, unsigned char sck_pin, unsigned char mosi_pin, unsigned char rst_pin, unsigned char dc_pin);
    ~GC9A01();
    /* CSX     ‾‾\__________________/‾‾‾‾‾
    * RESX    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    * D/CX    ‾‾‾\_____/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    * WRX     ‾‾‾‾\___/‾‾‾‾\___/‾‾‾‾‾‾‾‾‾
    * RDX     ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    * D[17:0] ----<Addr>---<Data>--------
    * */
    void WriteCycleSequence(const unsigned char command, const unsigned char data) const;
    void WriteCycleSequence(const unsigned char command, const unsigned char data[], const size_t dataSize) const;
    void Init() const;
    void Adafruit_Init() const;
    void FillArea(unsigned char r, unsigned char g, unsigned char b, unsigned short x0, unsigned short y0, unsigned short w, unsigned short h) const;
    void FillScreen(unsigned char r, unsigned char g, unsigned char b) const;
    void SetAddressWindow(unsigned short x0, unsigned short y0, unsigned short w, unsigned short h) const;
    void SetVerticalScrollArea(unsigned short topFixedArea, unsigned short verticalScrollArea) const;
    void SetPartialArtea(unsigned short startRow, unsigned short endRow) const;
    void FillImage(unsigned char image[], unsigned short x0, unsigned short y0, unsigned short w, unsigned short h) const;
    inline void HardwareReset() const {
        // TODO: Fix HW reset issue
        gpio_put(this->rst_pin, OFF);
        sleep_ms(120);
        gpio_put(this->rst_pin, ON);
    }
    /* This command causes the LCD module to enter the minimum power consumption mode. In this mode e.g. the DC/DC converter
     * is stopped, Internal oscillator is stopped, and panel scanning is stopped Out Blank STOP MCU interface and memory are
     * still working and the memory keeps its contents.
     * 
     * Restriction: This command has no effect when module is already in sleep in mode. Sleep In Mode can only be left by the
     * Sleep Out Command (11h). It will be necessary to wait 5msec before sending next to command, this is to allow time for
     * the supply voltages and clock circuits to stabilize. It will be necessary to wait 120msec after sending Sleep Out command
     * (when in Sleep In Mode) before Sleep In command can be sent.
     *
     */
    inline void Sleep() const {
        this->WriteCycleSequence(RegulativeCommandSet::EnterSleepMode, nullptr, 0);
        sleep_ms(5);
    }
    /* This command turns off sleep mode. the DC/DC converter is enabled, Internal oscillator is started, and panel scanning is started.
     *
     * Restriction: This command has no effect when module is already in sleep out mode. Sleep Out Mode can only be left by the
     * Sleep In Command (10h). It will be necessary to wait 5msec before sending next command, this is to allow time for the supply
     * voltages and clock circuits stabilize. The display module loads all display supplier’s factory default values to the registers
     * during this 5msec and there cannot be any abnormal visual effect on the display image if factory default and register values
     * are same when this load is done and when the display module is already Sleep Out –mode. The display module is doing self-diagnostic
     * functions during this 5msec. It will be necessary to wait 120msec after sending Sleep In command (when in Sleep Out mode) before
     * Sleep Out command can be sent.
     * */
    inline void WakeUp() const { this->WriteCycleSequence(RegulativeCommandSet::SleepOUT, nullptr, 0); }
    /* This command turns on partial mode. The partial mode window is described by the Partial Area command (30H).
     * To leave Partial mode, the Normal Display Mode On command (13H) should be written.
     * Restriction: This command has no effect when Partial mode is active.
     * */
    inline void EnterPartialMode() const { this->WriteCycleSequence(RegulativeCommandSet::PartialMode, nullptr, 0); }
    /* This command returns the display to normal mode. Normal display mode on means Partial mode off.
     * Exit from NORON by the Partial mode On command (12h) 
     * 
     * Restriction: This command has no effect when Normal Display mode is active.
     * */
    inline void EnterNormalMode() const { this->WriteCycleSequence(RegulativeCommandSet::NormalMode, nullptr, 0); }
    /* This command is used to recover from display inversion mode. This command makes no change of the
     * content of frame memory. This command doesn’t change any other status. 
     * 
     * Restriction: This command has no effect when module already is inversion OFF mode.
     * */
    inline void InversionOff() const { this->WriteCycleSequence(RegulativeCommandSet::InversionOFF, nullptr, 0); }
    /* This command is used to enter into display inversion mode. This command makes no change of the content
     * of frame memory. Every bit is inverted from the frame memory to the display. This command doesn’t change
     * any other status. To exit Display inversion mode, the Display inversion OFF command (20h) should be written.. 
     * 
     * Restriction: This command has no effect when module already is inversion ON mode.
     * */
    inline void InversionOn() const { this->WriteCycleSequence(RegulativeCommandSet::InversionON, nullptr, 0); }
    /* This command is used to enter into DISPLAY OFF mode.
     * In this mode, the output from Frame Memory is disabled and blank page inserted.
     * This command makes no change of contents of frame memory.
     * This command does not change any other status.
     * There will be no abnormal visible effect on the display. 
     * 
     * Restriction: This command has no effect when module is already in display off mode.
     * */
    inline void DisplayOff() const { this->WriteCycleSequence(RegulativeCommandSet::DisplayOFF, nullptr, 0); }
    /* This command is used to recover from DISPLAY OFF mode.
     * Output from the Frame Memory is enabled.
     * This command makes no change of contents of frame memory.
     * This command does not change any other status. 
     * 
     * Restriction: This command has no effect when module is already in display on mode.
     * */
    inline void DisplayOn() const { this->WriteCycleSequence(RegulativeCommandSet::DisplayON, nullptr, 0); }
    /* This command is used to turn OFF (Active Low) the Tearing Effect output signal from the TE signal line.
     *
     * Restriction: This command has no effect when Tearing Effect output is already OFF.
     * */
    inline void TearingEffectOff() const { this->WriteCycleSequence(RegulativeCommandSet::TearingEffectLineOFF, nullptr, 0); }
    /* This command is used to turn ON the Tearing Effect output signal from the TE signal line.
     * This output is not affected by changing MADCTL bit B4. The Tearing Effect Line On has one parameter which describes
     * the mode of the Tearing Effect Output Line.
     * 
     * Restriction: This command has no effect when Tearing Effect output is already ON.
     * 
     * @note During Sleep In Mode with Tearing Effect Line On, Tearing Effect Output pin will be active Low.
     *
     * @param hasVandHInfo When true, the Tearing Effect Output line consists of V-Blanking information only. When false, the Tearing Effect Output Line consists of both V-Blanking and H-Blanking information.
     * */
    inline void TearingEffectOn(bool hasVandHInfo) const {
        unsigned char data = (hasVandHInfo ? 0x01U : 0x00U);
        this->WriteCycleSequence(RegulativeCommandSet::TearingEffectLineOFF, &data, 1U);
    }

    /* This command is used together with Vertical Scrolling Definition (33h). These two commands
     * describe the scrolling area and the scrolling mode. The Vertical Scrolling Start Address
     * command has one parameter which describes the address of the line in the Frame Memory that
     * will be written as the first line after the last line of the Top Fixed Area on the display.
     *
     * Restriction: This command has no effect when Tearing Effect output is already ON.
     * 
     * @note (1) When new Pointer position and Picture Data are sent, the result on the display will happen at the next Panel Scan to avoid tearing effect. VSP refers to the Frame Memory line Pointer.
     * @note (2) This command is ignored when the GC9A01 enters Partial mode.
     *  */
    inline void SetVerticalScrollingStartAddress(unsigned short lineAddress) const {
        unsigned char data[2] = {0U};
        data[0] = lineAddress >> 8U;
        data[1] = lineAddress & 0xFFU;
        this->WriteCycleSequence(RegulativeCommandSet::VerticalScrollingStartAddress, data, 2U);
    }
    /* This command is used to recover from Idle mode on.
     * In the idle off mode, LCD can display maximum 262,144 colors. 
     * 
     * Restriction: This command has no effect when module is already in idle off mode.
     * */
    inline void IdleModeOff() const { this->WriteCycleSequence(RegulativeCommandSet::IdleModeOFF, nullptr, 0); }
    /* This command is used to enter into Idle mode on.
     * In the idle on mode, color expression is reduced. The primary and the secondary colors using
     * MSB of each R, G and B in the Frame Memory, 8 color depth data is displayed. 
     * 
     * Restriction: This command has no effect when module is already in idle on mode.
     * */
    inline void IdleModeOn() const { this->WriteCycleSequence(RegulativeCommandSet::IdleModeON, nullptr, 0); }
    /* This command turns on the display Tearing Effect output signal on the TE signal line when the display reaches
     * line equal the value of STS[8:0].
     * 
     * @note that set_tear_scanline with STS is equivalent to set_tear_on with 8+GateN(N=1, 2, 3...240)
     * eg:when the STS[8:0]=8,the TE will output at the position of Gate1.
     *          when the STS[8:0]=9,the TE will output at the position of Gate2.
     *          when the STS[8:0]=10,the TE will output at the position of Gate3.
     *          ...
     * 
     * The Tearing Effect Output line shall be active low when the display module is in Sleep mode
     * 
     * Restriction: This command has no effect when module is already in idle on mode.
     * */
    inline void SetTearLine(unsigned short lineAddress) const {
        unsigned char data[2] = {0U};
        data[0] = lineAddress >> 8U;
        data[1] = lineAddress & 0xFFU;
        this->WriteCycleSequence(RegulativeCommandSet::VerticalScrollingStartAddress, data, 2U);
    }
    /* This command is used to adjust the brightness value of the display.
     * It should be checked what is the relationship between this written value and output brightness of the display.
     * This relationship is defined on the display module specification.
     * In principle relationship is that 00h value means the lowest brightness and FFh value means the highest brightness.
     * 
     * @param brightnessLevel the new bightness value for the display   
     * */
    inline void SetBrightness(unsigned char brightnessLevel) const {
        this->WriteCycleSequence(RegulativeCommandSet::WriteDisplayBrightness, &brightnessLevel, 1U);
    }
    /* This command is used to return brightness setting.
     * 
     * The ctrlOptions should be 0b00<BCTRL>0<DD><BL>00
     * 
     * BCTRL: Brightness Control Block On/Off,
     * '0' = Off (Brightness registers are 00h)
     * '1' = On (Brightness registers are active, according to the DBV[7..0] parameters.)
     * DD: Display Dimming
     * '0' = Display Dimming is off
     * '1' = Display Dimming is on
     * BL: Backlight On/Off
     * '0' = Off (Completely turn off backlight circuit. Control lines must be low. )
     * '1' = On
     * 
     * Restriction: The display module is sending 2nd parameter value on the data lines if the MCU wants to read more than one parameter (= more than 2 RDX cycle) on DBI.
     * Only 2nd parameter is sent on DSI (The 1st parameter is not sent).
     * 
     * @param ctrlOptions the new CTRL Display settings
     * */
    inline void SetCTRLDisplay(unsigned char ctrlOptions) const {
        this->WriteCycleSequence(RegulativeCommandSet::WriteCTRLDisplay, &ctrlOptions, 1U);
    }
    /* @note The first parameter must write,but it is not valid. (Handled internally)
     *
     * The ctrlOptions should be 0bX<GS><SS>XXXXX
     * 
     * SS: Select the shift direction of outputs from the source driver.
     * In addition to the shift direction, the settings for both SS and BGR bits are required to change the assignment of R, G, and B dots to the source driver pins.
     * 
     * GS: Sets the direction of scan by the gate driver in the range determined by SCN [4:0] and NL [4:0]. The scan direction determined by GS = 0 can be reversed by setting GS = 1. GS Gate Output Scan Direction 0 G1→G32 1 G32→G1
     * 
     * SM: Sets the gate driver pin arrangement in combination with the GS bit to select the optimal scan mode for the module
     * 
     * Restriction: EXTC should be high to enable this command
     * */
    inline void SetDisplayFunctionControl(unsigned char ctrlOptions) const {
        unsigned char data[2] = { 0U };
        data[2] = ctrlOptions;
        this->WriteCycleSequence(ExtendedCommandSet::DisplayFunctionControl, data, 2U);
    }
    /* @note During Sleep In Mode with Tearing Effect Line On, Tearing Effect Output pin will be active Low.
     * 
     * Restriction: This command has no effect when Tearing Effect output is already ON.
     * 
     * @param te_pol is used to adjust the Tearing Effect output signal pulse polarity. false: positive pulse; true: negative pulse
     * @param te_width[6:0] is used to adjust the Tearing Effect output signal pulse width with display linesin unit. N: N + 1 line time (max value 127)
     * */
    inline void SetTearingEffectControl(bool te_pol, unsigned char te_width) const {
        unsigned char data = (te_pol ? (1U << 7U) : 0U) | (te_width & 0x7F);
        this->WriteCycleSequence(ExtendedCommandSet::TearingEffectControl, &data, 1U);
    }
    /* The ctrlOptions should be 0b1100<DM[1:0]><RM><RIM>
     * 
     * Restriction: EXTC should be high to enable this command
     * */    
    inline void SetInterfaceControl(unsigned char ctrlOptions) const {
        // Set first two bits to 1s
        ctrlOptions |= 0b11000000;
        // Set second two bits to 0s
        ctrlOptions &= (~0b00110000);
        this->WriteCycleSequence(ExtendedCommandSet::InterfaceControl, &ctrlOptions, 1U);
    }
    /* DINV[3:0] : Set display inversion mode
     * 
     * The frameRate should be 0b<DINV[3:0]>XXXX
     * 
     * Restriction: Inter_command should be set high to enable this command
     * */
    inline void SetFrameRate(unsigned char frameRate) const {
        // TODO: Lower 4 bits should be X (Not care) but if they are not set, display renders incorrectly.
        frameRate |= 0x04;
        this->WriteCycleSequence(InterCommandSet::FrameRate, &frameRate, 1U);
    }
    /* Select the external reference voltage Vci or internal reference voltage VCIR.
     * 
     * @param vcire false: Internal reference voltage 2.5V (default); true: External reference voltage Vci
     * 
     * Restriction: Inter_command should be set high to enable this command
     * */    
    inline void SetPowerControl1(bool vcire) const {
        unsigned char data = (vcire ? 0b10U : 0U);
        this->WriteCycleSequence(InterCommandSet::PowerControl1, &data, 1U);
    }
    /* Set the voltage level value to output the VREG1A and VREG1B OUT level, which is a reference
     * level for the grayscale voltage level.
     * VREG1A = (vrh + vbp_d) * 0.02 + 4
     * VREG1B = vbp_d * 0.02 + 0.3
     * 
     * Restriction: Inter_command should be set high to enable this command
     * */    
    inline void SetPowerControl2(unsigned char vbp_d) const {
        this->WriteCycleSequence(InterCommandSet::PowerControl2, &vbp_d, 1U);
    }
    /* Set the voltage level value to output the VREG2A OUT level, which is a reference level
     * for the grayscale voltage level.
     * VREG2A = (vbn_d - vrh) * 0.02 - 3.4
     * VREG2B = vbn_d * 0.02 + 0.3
     * 
     * Restriction: Inter_command should be set high to enable this command
     * */    
    inline void SetPowerControl3(unsigned char vbn_d) const {
        this->WriteCycleSequence(InterCommandSet::PowerControl3, &vbn_d, 1U);
    }
    /* Set the voltage level value to output the VREG1A OUT level, which is a reference level for the
     * grayscale voltage level.
     * VREG1A = (vrh + vbp_d) * 0.02 + 4
     * VREG2A=(vbn_d - vrh) * 0.02 - 3.4
     * 
     * Restriction: Inter_command should be set high to enable this command
     * */    
    inline void SetPowerControl4(unsigned char vrh) const {
        this->WriteCycleSequence(InterCommandSet::PowerControl4, &vrh, 1U);
    }
    /* Set the voltage level value to output the VCORE level.
     * 
     * vdd_ad[3:0]: VCORE (V)
     * 0: 1.483;     8: 1.994
     * 1: 1.545;     9: 2.109
     * 2: 1.590;     A: 2.193
     * 3: 1.638;     B: 2.286
     * 4: 1.714;     C: 2.385
     * 5: 1.279;     D: 1.713
     * 6: 1.859;     E: 1.713
     * 7: 1.925;     F: 1.713
     * 
     * Restriction: Inter_command should be set high to enable this command
     * */    
    inline void SetPowerControl7(unsigned char vdd_ad) const {
        this->WriteCycleSequence(InterCommandSet::PowerControl7, &vdd_ad, 1U);
    }

    void RainbowTest() const;
    void CheckerboardTest() const;
};

#endif
