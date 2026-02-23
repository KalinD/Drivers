#include "GC9A01.hpp"

GC9A01::GC9A01(spi_inst_t* spi_instance, unsigned char miso_pin, unsigned char cs_pin, unsigned char sck_pin, unsigned char mosi_pin, unsigned char rst_pin, unsigned char dc_pin)
 : spi_instance(spi_instance), cs_pin(cs_pin), sck_pin(sck_pin), mosi_pin(mosi_pin), rst_pin(rst_pin), dc_pin(dc_pin), pf(PF12BitsPerPixel), is_rgb(true) {
    spi_init(this->spi_instance, 40 * 1000 * 1000);   // 40 MHz is fine
    spi_set_format(
        this->spi_instance,
        8,              // bits
        SPI_CPOL_0,     // CPOL = 0
        SPI_CPHA_0,     // CPHA = 0
        SPI_MSB_FIRST
    );

    gpio_set_function(dc_pin,   GPIO_FUNC_SIO);
    gpio_set_function(cs_pin,   GPIO_FUNC_SIO);
    gpio_set_function(rst_pin,  GPIO_FUNC_SIO);
    gpio_set_function(sck_pin,  GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_set_dir(rst_pin, GPIO_OUT);
    gpio_set_dir(dc_pin, GPIO_OUT);
    gpio_put(cs_pin, ON);
    gpio_put(rst_pin, ON);
}

GC9A01::~GC9A01() { }

void GC9A01::WriteCycleSequence(const unsigned char command, const unsigned char data) const { 
    gpio_put(this->rst_pin, ON);
    
    gpio_put(this->cs_pin, OFF);
    gpio_put(this->dc_pin, OFF);

    spi_write_blocking(this->spi_instance, &command, 1U);
    
    gpio_put(this->dc_pin, ON);
    
    spi_write_blocking(this->spi_instance, &data, 1U);

    gpio_put(this->cs_pin, ON);
}

void GC9A01::WriteCycleSequence(const unsigned char command, const unsigned char data[], const size_t dataSize) const { 
    gpio_put(this->rst_pin, ON);
    
    gpio_put(this->cs_pin, OFF);
    gpio_put(this->dc_pin, OFF);

    spi_write_blocking(this->spi_instance, &command, 1U);
    
    gpio_put(this->dc_pin, ON);
    
    if(0 < dataSize) {
        spi_write_blocking(this->spi_instance, data, dataSize);
    }

    gpio_put(this->cs_pin, ON);
}

void GC9A01::SetAddressWindow(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1) const {
    unsigned char data[4U];

    // Column address (X)
    data[0] = ((x0 & 0xFF00U) >> 8U);
    data[1] = (x0 & 0xFFU);
    data[2] = ((x1 & 0xFF00U) >> 8U);
    data[3] = (x1 & 0xFFU);
    this->WriteCycleSequence(RegulativeCommandSet::ColumnAddressSet, data, 4U);

    // Row address (Y)
    data[0] = ((y0 & 0xFF00U) >> 8U);
    data[1] = (y0 & 0xFFU);
    data[2] = ((y1 & 0xFF00U) >> 8U);
    data[3] = (y1 & 0xFFU);
    this->WriteCycleSequence(RegulativeCommandSet::RowAddressSet, data, 4U);
}

void GC9A01::SetVerticalScrollArea(unsigned short topFixedArea, unsigned short verticalScrollArea) const {
    unsigned char data[4U];

    // Column address (X)
    data[0] = topFixedArea >> 8U;
    data[1] = topFixedArea & 0xFFU;
    data[2] = verticalScrollArea >> 8U;
    data[3] = verticalScrollArea & 0xFFU;
    this->WriteCycleSequence(RegulativeCommandSet::VerticalScrollingDefinition, data, 4U);
}

void GC9A01::GetNewImageSize(const size_t pixelCount, size_t* outSize) const {
    switch (this->pf)
    {
    case PF12BitsPerPixel:
        // Two pixels become 3 bytes
        *outSize = (pixelCount / 2) * 3;
        break;
    case PF16BitsPerPixel:
        // One pixel is two bytes
        *outSize = (pixelCount / 1) * 2;
        break;
    case PF18BitsPerPixel:
        // One pixel is three bytes
        *outSize = (pixelCount / 1) * 3;
        break;
    default:
        break;
    }
}

void GC9A01::HandlePixels(const unsigned char originalPixels[], size_t * const originalIndex, unsigned char out[], size_t * const outIndex) const {
    const unsigned char GREEN_SHIFT = 0U;
    const unsigned char BLUE_SHIFT = (this->is_rgb ? 1U : 2U);
    const unsigned char RED_SHIFT = (this->is_rgb ? 2U : 1U);

    switch (this->pf)
    {
    case PF12BitsPerPixel:
    {
        // Two pixels become 3 bytes
        const unsigned char red1 = originalPixels[*originalIndex + RED_SHIFT];
        const unsigned char green1 = originalPixels[*originalIndex + GREEN_SHIFT];
        const unsigned char blue1 = originalPixels[*originalIndex + BLUE_SHIFT];
        const unsigned char red2 = originalPixels[*originalIndex + 3U + RED_SHIFT];
        const unsigned char green2 = originalPixels[*originalIndex + 3U + GREEN_SHIFT];
        const unsigned char blue2 = originalPixels[*originalIndex + 3U + BLUE_SHIFT];
        // printf("red1: %d, green1: %d, blue1: %d, red2: %d, green2: %d, blue2: %d", red1, green1, blue1, red2, green2, blue2);
        *originalIndex += 6U;
        if (this->is_rgb) {
            out[*outIndex] = (green1 & 0xF0U) | ((blue1 & 0xF0U) >> 4U); // Green 1 + Blue 1
            ++*outIndex;
            out[*outIndex] = (red1 & 0xF0U) | ((green2 & 0xF0U) >> 4U); // Red 1 + Green 2
            ++*outIndex;
            out[*outIndex] = (blue2 & 0xF0U) | ((red2 & 0xF0U) >> 4U); // Blue 2 + Red 2
            ++*outIndex;
        } else {
            out[*outIndex] = (green1 & 0xF0U) | ((red1 & 0xF0U) >> 4U); // Green 1 + Red 1
            ++*outIndex;
            out[*outIndex] = (blue1 & 0xF0U) | ((green2 & 0xF0U) >> 4U); // Blue 1 + Green 2
            ++*outIndex;
            out[*outIndex] = (red2 & 0xF0U) | ((blue2 & 0xF0U) >> 4U); // Red 2 + Blue 2
            ++*outIndex;
        }
        break;
    }
    case PF16BitsPerPixel: // RGB 565
    {
        unsigned char red = originalPixels[*originalIndex + RED_SHIFT];
        unsigned char green = originalPixels[*originalIndex + GREEN_SHIFT];
        unsigned char blue = originalPixels[*originalIndex + BLUE_SHIFT];
        *originalIndex += 3U;
        if (this->is_rgb) {
            // out[outIndex] = (originalPixels[i] >> 3U) & 0x1FU; // Red (5 bits)
            // out[outIndex] = (out[outIndex] << 3U) | (originalPixels[i + 1U] >> 5U); // Green (3bits)
            out[*outIndex] = (green >> 2U) | (blue >> 6U); // Green (6 bits) + Blue (2 bits)
            ++*outIndex;
            // out[outIndex] = (originalPixels[i + 1U] >> 2U) & 0b111U; // Green (3bits)
            // out[outIndex] = (out[outIndex] << 5U) | ((originalPixels[i + 2U] >> 3U) & 0x1FU); // Blue (5 bits)
            out[*outIndex] = ((blue >> 3U) & 0x03U) | (red >> 3U); // Blue (3 bits) + Red (5 bits)
            ++*outIndex;
        } else {
            out[*outIndex] = (green >> 2U) | (red >> 6U); // Green (6 bits) + Blue (2 bits)
            ++*outIndex;
            out[*outIndex] = ((red >> 3U) & 0x03U) | (blue >> 3U); // Blue (3 bits) + Red (5 bits)
            ++*outIndex;
        }
        break;
    }
    case PF18BitsPerPixel:
    {
        // TODO: Fix this logic for the ixels
        // 4 pixels per 9 bytes
        out[*outIndex] = (originalPixels[*originalIndex] >> 2U) & 0x3FU; // Red (6 bits)
        out[*outIndex] = (originalPixels[*originalIndex + 1U] >> 6U) & 0x03U; // Green (2 bits)
        ++*outIndex;
        out[*outIndex] = (originalPixels[*originalIndex + 1U] >> 2U) & 0x0FU; // Green (4 bits)
        out[*outIndex] = (originalPixels[*originalIndex + 2U] >> 4U) & 0x0FU; // Blue (4 bits)
        ++*outIndex;
        out[*outIndex] = (originalPixels[*originalIndex + 2U] >> 4U) & 0x0FU; // Blue (2 bits)
        ++*outIndex;
        break;
    }
    default:
    {
        break;
    }
    }
}

void GC9A01::ReMapToCorrectPixels(const unsigned char originalPixels[], const size_t pixelCount, unsigned char out[]) const {
    const unsigned char GREEN_SHIFT = 0U;
    const unsigned char BLUE_SHIFT = (this->is_rgb ? 1U : 2U);
    const unsigned char RED_SHIFT = (this->is_rgb ? 2U : 1U);
    // Green, Blue, Red
    size_t outIndex = 0U;
    const size_t loopsCount = pixelCount * 3;

    for (size_t i = 0U; i < loopsCount; ) {
        // i increase is handled inside HandlePixels
        this->HandlePixels(originalPixels, &i, out, &outIndex);
    }
}

void GC9A01::SetPartialArtea(unsigned short startRow, unsigned short endRow) const {
    unsigned char data[4];

    // Row address (Y)
    data[0] = startRow >> 8U;
    data[1] = startRow & 0xFF;
    data[2] = endRow >> 8U;
    data[3] = endRow & 0xFF;
    this->WriteCycleSequence(RegulativeCommandSet::PartialArea, data, 4);
}

void GC9A01::FillImage(unsigned char image[], unsigned short x0, unsigned short y0, unsigned short w, unsigned short h) const {
    const size_t pixelCount = w * h;
    size_t outSize = 0U;
    this->GetNewImageSize(pixelCount, &outSize);
    unsigned char out[outSize];
    ReMapToCorrectPixels(image, pixelCount, out);
    this->SetAddressWindow(x0, y0, (x0 + w - 1U), (y0 + h - 1U));
    this->WriteCycleSequence(RegulativeCommandSet::MemoryWrite, &out[0U], outSize);
}

void GC9A01::FillScreen(unsigned char r, unsigned char g, unsigned char b) const {
    this->FillArea(r, g, b, 0, 0, 239, 239);
}

// TODO: Rework to use pixels properly
void GC9A01::FillArea(unsigned char r, unsigned char g, unsigned char b, unsigned short x0, unsigned short y0, unsigned short w, unsigned short h) const {
    const size_t loopsCount = MAX_HEIGHT * MAX_HEIGHT * RGB_COUNT;
    unsigned char pixels[loopsCount] = {0U};
    size_t jumpSize = 0U;
    size_t outIndex = 0U;
    unsigned char pixelArray[6U] = {0U};
    
    switch (this->pf)
    {
    case PF12BitsPerPixel:
        jumpSize = 6U;
        pixelArray[0] = r;
        pixelArray[1] = g;
        pixelArray[2] = b;
        pixelArray[3] = r;
        pixelArray[4] = g;
        pixelArray[5] = b;
        break;
    case PF16BitsPerPixel:
        jumpSize = 3U;
        pixelArray[0] = r;
        pixelArray[1] = g;
        pixelArray[2] = b;
        break;
    case PF18BitsPerPixel:
        // TODO:
        // jumpSize
        break;
    default:
        break;
    }

    size_t pixelArrayIndex = 0U;
    for (size_t i = 0U; i < loopsCount; i += jumpSize) {
        this->HandlePixels(pixelArray, &pixelArrayIndex, pixels, &outIndex);
        pixelArrayIndex = 0U;
    }
    
    this->SetAddressWindow(x0, y0, (x0 + w - 1U), (y0 + h - 1U));
    this->WriteCycleSequence(RegulativeCommandSet::MemoryWrite, pixels, outIndex);
}

void GC9A01::CheckerboardTest() const {
    unsigned char pixels[MAX_WIDTH * MAX_HEIGHT * RGB_COUNT] = {0U};
    unsigned char color[6U] = {0U};
    unsigned char blackArray[6] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
    unsigned char whiteArray[6] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    size_t outSize = 0U;
    this->GetNewImageSize(MAX_WIDTH * MAX_HEIGHT, &outSize);
    size_t colorIndex = 0U;
    for (size_t x = 0; x < MAX_WIDTH; ++x) {
        for (size_t y = 0; y < MAX_HEIGHT; ++y) {
            size_t position = x * MAX_WIDTH * RGB_COUNT + y * RGB_COUNT;
            if ((x / 10) % 2 ==  (y / 10) % 2) {
                this->HandlePixels(whiteArray, &colorIndex, pixels, &position);
            } else {
                this->HandlePixels(blackArray, &colorIndex, pixels, &position);
            }
            colorIndex = 0U;
        }
    }
    this->SetAddressWindow(0U, 0U, MAX_WIDTH - 1U, MAX_HEIGHT - 1U);
    this->WriteCycleSequence(RegulativeCommandSet::MemoryWrite, pixels, outSize);
}

void GC9A01::RainbowTest() const {
    constexpr size_t pixelCount = MAX_WIDTH * MAX_HEIGHT;
    unsigned char color[6U] = {0U};
    size_t outSize = 0U;
    size_t colorIndex = 0U;
    float frequency = 0.026;
    
    this->GetNewImageSize(pixelCount, &outSize);

    unsigned char pixels[pixelCount * RGB_COUNT] = {0U};

    for (size_t x = 0U; x < MAX_WIDTH; ++x) {
        color[0] = sin(frequency * x + 0) * 127 + 128;
        color[1] = sin(frequency * x + 2) * 127 + 128;
        color[2] = sin(frequency * x + 4) * 127 + 128;
        color[3] = sin(frequency * (x + 1) + 0) * 127 + 128;
        color[4] = sin(frequency * (x + 1) + 2) * 127 + 128;
        color[5] = sin(frequency * (x + 1) + 4) * 127 + 128;
        for (size_t y = 0U; y < MAX_HEIGHT; ++y) {
            size_t position = x * MAX_WIDTH * RGB_COUNT + y * RGB_COUNT;
            this->HandlePixels(color, &colorIndex, pixels, &position);
            colorIndex = 0U;
        }
    }
    this->SetAddressWindow(0U, 0U, MAX_WIDTH - 1U, MAX_HEIGHT - 1U);
    this->WriteCycleSequence(RegulativeCommandSet::MemoryWrite, pixels, outSize);
}

void GC9A01::Init() const {
    // this->HardwareReset();
    this->WriteCycleSequence(0x01, nullptr, 0); // Reset?

    this->WriteCycleSequence(InterCommandSet::InterRegisterEnable1, nullptr, 0); // Inter Register Enable1
    this->WriteCycleSequence(InterCommandSet::InterRegisterEnable2, nullptr, 0); // Inter Register Enable2
	
    // Undocumented in datasheet registers
    this->WriteCycleSequence(0xEB, 0x14);
    this->WriteCycleSequence(0x84, 0x60);
    this->WriteCycleSequence(0x85, 0xF7);
    this->WriteCycleSequence(0x86, 0xFC);
    this->WriteCycleSequence(0x87, 0x28);
    this->WriteCycleSequence(0x8E, 0x0F);
    this->WriteCycleSequence(0x8F, 0xFC);
    this->WriteCycleSequence(0x88, 0x0A);
    this->WriteCycleSequence(0x89, 0x21);
    this->WriteCycleSequence(0x8A, 0x00);
    this->WriteCycleSequence(0x8B, 0x80);
    this->WriteCycleSequence(0x8C, 0x01);
    this->WriteCycleSequence(0x8D, 0x03);
    
    this->SetDisplayFunctionControl(DisplayFunctionControlOptions::GS_OFF | DisplayFunctionControlOptions::SS_OFF);

    // this->WriteCycleSequence(RegulativeCommandSet::MemoryAccessControl, MemoryAccessControlOptions::BGR | MemoryAccessControlOptions::MX);
    this->WriteCycleSequence(RegulativeCommandSet::MemoryAccessControl, MemoryAccessControlOptions::RGB | MemoryAccessControlOptions::MX);
    this->WriteCycleSequence(RegulativeCommandSet::COLMODPixelFormatSet, COLMOD::DBI12BitPerPixel | COLMOD::DPI16BitPerPixel);
    // this->WriteCycleSequence(RegulativeCommandSet::COLMODPixelFormatSet, COLMOD::DBI16BitPerPixel | COLMOD::DPI16BitPerPixel);

    // Undocumented in datasheet registers
	unsigned char seqReg90[] = {0x08, 0x08, 0x08, 0x08};
    this->WriteCycleSequence(0x90, seqReg90, 4U);

    this->SetTearingEffectControl(false, 0x01);

    this->WriteCycleSequence(0xBD, 0x06);
    this->WriteCycleSequence(0xBC, 0x00);

	unsigned char seqRegFF[] = {0x60, 0x01, 0x04};
    this->WriteCycleSequence(0xFF, seqRegFF, 3U);

    this->WriteCycleSequence(InterCommandSet::PowerControl2, 0x48);
    this->WriteCycleSequence(InterCommandSet::PowerControl3, 0x48);
    this->WriteCycleSequence(InterCommandSet::PowerControl4, 0x25);

    // Undocumented in datasheet register
    this->WriteCycleSequence(0xBE, 0x11);
	unsigned char seqRegE1[] = {0x10, 0x0E};
    this->WriteCycleSequence(0xE1, seqRegE1, 2U);

	unsigned char seqRegDF[] = {0x21, 0x10, 0x02};
    this->WriteCycleSequence(0xDF, seqRegDF, 3U);

	// gamma control sequence
	unsigned char seqGamma1_3[] = {0x4b, 0x0F, 0x0A, 0x0B, 0x15, 0x30};
    this->WriteCycleSequence(InterCommandSet::SetGamma1, seqGamma1_3, 6U);
	unsigned char seqGamma2_4[] = {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F};
    this->WriteCycleSequence(InterCommandSet::SetGamma2, seqGamma2_4, 6U);
    this->WriteCycleSequence(InterCommandSet::SetGamma3, seqGamma1_3, 6U);
    this->WriteCycleSequence(InterCommandSet::SetGamma4, seqGamma2_4, 6U);
    
	unsigned char seqTearScanline[] = {0x00, 0x00};
    this->WriteCycleSequence(RegulativeCommandSet::SetTearScanline, seqTearScanline, 2U);
    
	// Undocumented in datasheet register
	unsigned char seqRegED[] = {0x1B, 0x0B};
    this->WriteCycleSequence(0xED, seqRegED, 2U);

    this->WriteCycleSequence(0xAC, 0x47);
    this->WriteCycleSequence(0xAE, 0x77);
    this->WriteCycleSequence(0xCD, 0x63);
    
    // Unsure what this line (from manufacturer's boilerplate code) is
    // meant to do, but users reported issues, seems to work OK without:
	unsigned char seqReg70[] = {0x07, 0x09, 0x04, 0x0C, 0x0D, 0x09, 0x07, 0x08, 0x03};
    this->WriteCycleSequence(0x70, seqReg70, 9U);

    // Frame Rate
    this->SetFrameRate(FrameRateOptions::FourDotInversion);

	// Undocumented in datasheet registers
	unsigned char seqReg60[] = {0x38, 0x0B, 0x76, 0x62, 0x39, 0xF0, 0x76, 0x62};
    this->WriteCycleSequence(0x60, seqReg60, 8U);

	unsigned char seqReg61[] = {0x38, 0xF6, 0x76, 0x62, 0x38, 0xF7, 0x76, 0x62};
    this->WriteCycleSequence(0x61, seqReg61, 8U);

	unsigned char seqReg62[] = {0x38, 0x0D, 0x71, 0xED, 0x76, 0x62, 0x38, 0x0F, 0x71, 0xEF, 0x76, 0x62};
    this->WriteCycleSequence(0x62, seqReg62, 12U);

	unsigned char seqReg63[] = {0x38, 0x11, 0x71, 0xF1, 0x76, 0x62, 0x38, 0x13, 0x71, 0xF3, 0x76, 0x62};
    this->WriteCycleSequence(0x63, seqReg63, 12U);

	unsigned char seqReg64[] = {0x3B, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x0A};
    this->WriteCycleSequence(0x64, seqReg64, 7U);

	unsigned char seqReg66[] = {0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00};
    this->WriteCycleSequence(0x66, seqReg66, 10U);
    
	unsigned char seqReg67[] = {0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98};
    this->WriteCycleSequence(0x67, seqReg67, 10U);

	unsigned char seqPorchCtrl[] = {0x08, 0x09, 0x14};
    this->WriteCycleSequence(ExtendedCommandSet::BlankingPorchControl, seqPorchCtrl, 3U);

	// Undocumented in datasheet registers
	unsigned char seqReg74[] = {0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00};
    this->WriteCycleSequence(0x74, seqReg74, 7U);
    
	unsigned char seqReg98[] = {0x3E, 0x07};
    this->WriteCycleSequence(0x98, seqReg98, 2U);

    this->EnterNormalMode();
    this->WriteCycleSequence(RegulativeCommandSet::TearingEffectLineOFF, nullptr, 0);
    this->IdleModeOff();
    this->TearingEffectOn(false);
    this->InversionOn();
    this->WakeUp();
    sleep_ms(120);
    this->DisplayOn();
}

void GC9A01::Adafruit_Init() const {
    this->WriteCycleSequence(InterCommandSet::InterRegisterEnable2, nullptr, 0); // Inter Register Enable2

    this->WriteCycleSequence(0xEB, 0x14);

    this->WriteCycleSequence(InterCommandSet::InterRegisterEnable1, nullptr, 0); // Inter Register Enable1
    this->WriteCycleSequence(InterCommandSet::InterRegisterEnable2, nullptr, 0); // Inter Register Enable2
	
    // Undocumented in datasheet registers
    this->WriteCycleSequence(0xEB, 0x14);
    this->WriteCycleSequence(0x84, 0x40);
    this->WriteCycleSequence(0x85, 0xFF);
    this->WriteCycleSequence(0x86, 0xFF);
    this->WriteCycleSequence(0x87, 0xFF);
    this->WriteCycleSequence(0x88, 0x0A);
    this->WriteCycleSequence(0x89, 0x21);
    this->WriteCycleSequence(0x8A, 0x00);
    this->WriteCycleSequence(0x8B, 0x80);
    this->WriteCycleSequence(0x8C, 0x01);
    this->WriteCycleSequence(0x8D, 0x01);
    this->WriteCycleSequence(0x8E, 0xFF);
    this->WriteCycleSequence(0x8F, 0xFF);

    this->WriteCycleSequence(RegulativeCommandSet::MemoryAccessControl, MemoryAccessControlOptions::RGB | MemoryAccessControlOptions::MX);

    this->WriteCycleSequence(RegulativeCommandSet::COLMODPixelFormatSet, COLMOD::DBI16BitPerPixel);

    // Undocumented in datasheet registers
	const unsigned char seqReg90[] = {0x08, 0x08, 0x08, 0x08};
    this->WriteCycleSequence(0x90, seqReg90, 4U);

    this->WriteCycleSequence(0xBD, 0x06);
    this->WriteCycleSequence(0xBC, 0x00);

	const unsigned char seqRegFF[] = {0x60, 0x01, 0x04};
    this->WriteCycleSequence(0xFF, seqRegFF, 3U);
    
    this->WriteCycleSequence(InterCommandSet::PowerControl2, 0x13);
    this->WriteCycleSequence(InterCommandSet::PowerControl3, 0x13);
    this->WriteCycleSequence(InterCommandSet::PowerControl4, 0x22);
    
    // Undocumented in datasheet register
    this->WriteCycleSequence(0xBE, 0x11);
	const unsigned char seqRegE1[] = {0x10, 0x0E};
    this->WriteCycleSequence(0xE1, seqRegE1, 2U);

	unsigned char seqRegDF[] = {0x21, 0x0C, 0x02};
    this->WriteCycleSequence(0xDF, seqRegDF, 3U);

	// gamma control sequence
	unsigned char seqGamma1_3[] = {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A};
    this->WriteCycleSequence(InterCommandSet::SetGamma1, seqGamma1_3, 6U);
	unsigned char seqGamma2_4[] = {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F};
    this->WriteCycleSequence(InterCommandSet::SetGamma2, seqGamma2_4, 6U);
    this->WriteCycleSequence(InterCommandSet::SetGamma3, seqGamma1_3, 6U);
    this->WriteCycleSequence(InterCommandSet::SetGamma4, seqGamma2_4, 6U);
    
	// Undocumented in datasheet register
	unsigned char seqRegED[] = {0x1B, 0x0B};
    this->WriteCycleSequence(0xED, seqRegED, 2U);

    this->WriteCycleSequence(0xAE, 0x77);
    this->WriteCycleSequence(0xCD, 0x63);
    
    // Unsure what this line (from manufacturer's boilerplate code) is
    // meant to do, but users reported issues, seems to work OK without:
	// unsigned char seqReg70[] = {0x07, 0x09, 0x04, 0x0C, 0x0D, 0x09, 0x07, 0x08, 0x03};
    // this->WriteCycleSequence(0x70, seqReg70, 9U);

    // Frame Rate
    this->SetFrameRate(FrameRateOptions::FourDotInversion);

	// Undocumented in datasheet registers
	unsigned char seqReg62[] = {0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70};
    this->WriteCycleSequence(0x62, seqReg62, 12U);

	unsigned char seqReg63[] = {0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70};
    this->WriteCycleSequence(0x63, seqReg63, 12U);

	unsigned char seqReg64[] = {0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07};
    this->WriteCycleSequence(0x64, seqReg64, 7U);

	unsigned char seqReg66[] = {0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00};
    this->WriteCycleSequence(0x66, seqReg66, 10U);
    
	unsigned char seqReg67[] = {0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98};
    this->WriteCycleSequence(0x67, seqReg67, 10U);

	// Undocumented in datasheet registers
	unsigned char seqReg74[] = {0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00};
    this->WriteCycleSequence(0x74, seqReg74, 7U);
    
	unsigned char seqReg98[] = {0x3E, 0x07};
    this->WriteCycleSequence(0x98, seqReg98, 2U);

    this->TearingEffectOn(false);
    this->InversionOn();
    this->WakeUp();
    sleep_ms(120);
    this->DisplayOn();
}
