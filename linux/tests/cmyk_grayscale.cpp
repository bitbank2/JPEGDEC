// Host regression test; libjpeg is used only to generate fixtures and references.
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <jpeglib.h>
#undef DCTSIZE // libjpeg and JPEGDEC use different units for this macro.
#include <JPEGDEC.h>

static const int width = 73, height = 65;
static int outputWidth, outputHeight, pixelsWritten;
static std::vector<unsigned char> output;

static void require(bool condition, const char *message)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

static int draw(JPEGDRAW *block)
{
    const unsigned char *pixels = (const unsigned char *)block->pPixels;
    for (int y = 0; y < block->iHeight; y++) {
        for (int x = 0; x < block->iWidthUsed; x++) {
            require(block->x + x >= 0 && block->x + x < outputWidth &&
                    block->y + y >= 0 && block->y + y < outputHeight,
                    "draw coordinates are out of bounds");
            output[(block->y + y) * outputWidth + block->x + x] = pixels[y * block->iWidth + x];
            pixelsWritten++;
        }
    }
    return 1;
}

static std::vector<unsigned char> encode(bool ycck, bool adobe, int restart,
                                         bool subsampled = false, bool progressive = false,
                                         J_COLOR_SPACE inputColor = JCS_CMYK)
{
    jpeg_compress_struct jpeg = {};
    jpeg_error_mgr errors;
    jpeg.err = jpeg_std_error(&errors);
    jpeg_create_compress(&jpeg);
    unsigned char *bytes = NULL;
    unsigned long length = 0;
    jpeg_mem_dest(&jpeg, &bytes, &length);
    jpeg.image_width = width;
    jpeg.image_height = height;
    jpeg.input_components = inputColor == JCS_CMYK ? 4 : (inputColor == JCS_RGB ? 3 : 1);
    jpeg.in_color_space = inputColor;
    jpeg_set_defaults(&jpeg);
    if (inputColor == JCS_CMYK)
        jpeg_set_colorspace(&jpeg, ycck ? JCS_YCCK : JCS_CMYK);
    for (int i = 0; i < jpeg.num_components; i++) {
        jpeg.comp_info[i].h_samp_factor = 1;
        jpeg.comp_info[i].v_samp_factor = 1;
    }
    if (subsampled) {
        jpeg.comp_info[0].h_samp_factor = jpeg.comp_info[0].v_samp_factor = 2;
        jpeg.comp_info[3].h_samp_factor = jpeg.comp_info[3].v_samp_factor = 2;
    }
    jpeg.write_Adobe_marker = adobe;
    jpeg.restart_interval = restart;
    if (progressive)
        jpeg_simple_progression(&jpeg);
    jpeg_set_quality(&jpeg, 95, TRUE);
    jpeg_start_compress(&jpeg, TRUE);
    std::vector<unsigned char> row(width * jpeg.input_components);
    while (jpeg.next_scanline < jpeg.image_height) {
        for (int x = 0; x < width; x++) {
            for (int component = 0; component < jpeg.input_components; component++) {
                const int y = jpeg.next_scanline;
                const int value = component == 0 ? x * 255 / (width - 1) :
                                  component == 1 ? y * 255 / (height - 1) :
                                  component == 2 ? (x * 17 + y * 31) % 256 :
                                                   (x + y) * 255 / (width + height - 2);
                row[x * jpeg.input_components + component] =
                    (inputColor == JCS_CMYK && !adobe) ? 255 - value : value;
            }
        }
        JSAMPROW pointer = row.data();
        jpeg_write_scanlines(&jpeg, &pointer, 1);
    }
    jpeg_finish_compress(&jpeg);
    std::vector<unsigned char> result(bytes, bytes + length);
    std::free(bytes);
    jpeg_destroy_compress(&jpeg);
    return result;
}

static std::vector<unsigned char> reference(const std::vector<unsigned char> &bytes, bool adobe)
{
    jpeg_decompress_struct jpeg = {};
    jpeg_error_mgr errors;
    jpeg.err = jpeg_std_error(&errors);
    jpeg_create_decompress(&jpeg);
    jpeg_mem_src(&jpeg, bytes.data(), bytes.size());
    jpeg_read_header(&jpeg, TRUE);
    jpeg.out_color_space = JCS_CMYK;
    jpeg_start_decompress(&jpeg);
    std::vector<unsigned char> result(width * height), row(width * 4);
    while (jpeg.output_scanline < jpeg.output_height) {
        const int y = jpeg.output_scanline;
        JSAMPROW pointer = row.data();
        jpeg_read_scanlines(&jpeg, &pointer, 1);
        for (int x = 0; x < width; x++) {
            const unsigned char *pixel = &row[x * 4];
            double channels[4];
            for (int i = 0; i < 4; i++)
                channels[i] = adobe ? pixel[i] : 255 - pixel[i];
            result[y * width + x] = (unsigned char)std::lround(
                (0.299 * channels[0] + 0.587 * channels[1] + 0.114 * channels[2]) * channels[3] / 255.0);
        }
    }
    jpeg_finish_decompress(&jpeg);
    jpeg_destroy_decompress(&jpeg);
    return result;
}

static void decode(std::vector<unsigned char> &bytes, int scale)
{
    JPEGDEC jpeg;
    require(jpeg.openRAM(bytes.data(), bytes.size(), draw) == 1, "fixture should open");
    const int divisor = scale ? scale : 1;
    outputWidth = (width + divisor - 1) / divisor;
    outputHeight = (height + divisor - 1) / divisor;
    output.assign(outputWidth * outputHeight, 0);
    pixelsWritten = 0;
    jpeg.setPixelType(EIGHT_BIT_GRAYSCALE);
    require(jpeg.decode(0, 0, scale) == 1, "fixture should decode");
    require(pixelsWritten == outputWidth * outputHeight, "all pixels should be returned");
    jpeg.close();
}

int main()
{
    const int scales[] = {0, JPEG_SCALE_HALF, JPEG_SCALE_QUARTER, JPEG_SCALE_EIGHTH};
    int cases = 0;
    for (int variant = 0; variant < 5; variant++) {
        const bool ycck = variant < 2, adobe = variant != 4;
        std::vector<unsigned char> bytes = encode(ycck, adobe, variant == 1 || variant == 3 ? 7 : 0);
        const std::vector<unsigned char> expected = reference(bytes, adobe);
        for (int scale : scales) {
            decode(bytes, scale);
            if (scale == 0) {
                double error = 0;
                for (size_t i = 0; i < output.size(); i++)
                    error += std::abs((int)output[i] - (int)expected[i]);
                require(error / output.size() < 2.0, "grayscale should match the libjpeg reference");
            }
            cases++;
        }
        JPEGDEC color;
        require(color.openRAM(bytes.data(), bytes.size(), draw) == 1, "CMYK header should open");
        color.setPixelType(RGB565_LITTLE_ENDIAN);
        require(color.decode(0, 0, 0) == 0 && color.getLastError() == JPEG_UNSUPPORTED_FEATURE,
                "unsupported color output should be rejected");
        color.close();
        cases++;
    }
    for (int variant = 0; variant < 2; variant++) {
        std::vector<unsigned char> bytes = encode(true, true, 0, variant == 0, variant == 1);
        JPEGDEC jpeg;
        require(jpeg.openRAM(bytes.data(), bytes.size(), draw) == 0 &&
                jpeg.getLastError() == JPEG_UNSUPPORTED_FEATURE,
                "unsupported four-component layouts should be rejected");
        cases++;
    }
    for (J_COLOR_SPACE inputColor : {JCS_RGB, JCS_GRAYSCALE}) {
        std::vector<unsigned char> bytes = encode(false, false, 0, false, false, inputColor);
        for (int scale : scales) {
            decode(bytes, scale);
            cases++;
        }
    }
    std::printf("PASS: %d CMYK/YCCK and RGB/grayscale regression cases\n", cases);
}
