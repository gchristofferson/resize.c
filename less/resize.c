// Resizes a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    char *nString = argv[1];
    int n = atoi(nString);
    char *infile = argv[2];
    char *outfile = argv[3];

    // validate user input
    // check if n is between 0 and 100
    int err = 0;
    if ( n < 0 || n > 100)
    {
        fprintf(stderr, "Usage: n must be a postive integer less than or equal to 100\n");
        err++;
    }
    // check if infile is a valid .bmp
    // find the extension
    int ret = 1;
    char *ext = strrchr(infile, '.');
    if (!ext)
    {
        fprintf(stderr, "Usage: infile must be the name of a BMP to be resized\n");
        err++;
    }
    else
    {
        ret = strncmp(ext, ".bmp", 3);
        if (ret != 0)
        {
            fprintf(stderr, "Usage: infile must be the name of a BMP to be resized\n");
            err++;
        }
    }
    // check if outfile is a valid .bmp
    // find the extension
    ext = strrchr(outfile, '.');

    if (!ext)
    {
        fprintf(stderr, "Usage: outfile must be the name of a BMP to be resized\n");
        err++;
    }
    else
    {
        ret = strncmp(ext, ".bmp", 3);
        if (ret != 0)
        {
            fprintf(stderr, "Usage: outfile must be the name of a BMP to be resized\n");
            err++;
        }
    }
    // if any of the 3 arguments don't validate, exit program
    if (err > 0)
    {
        return 1;
    }

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // determine padding for infile scanlines
    int inPadding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // preserve infile bi.biWidth and bi.biHeight
    int inBiWidth = bi.biWidth;
    int inBiHeight = bi.biHeight;
    int posInBiHeight = abs(inBiHeight);

    // update width and height in file headers for outfile
    bi.biWidth = bi.biWidth * n;
    bi.biHeight = bi.biHeight * n;
    // fprintf(stderr,"inBiWidth = %i\n", inBiWidth);

    // define variables used in equations for updating biSizeImage and bfSize
    int infileW = bi.biWidth;
    int infileH = bi.biHeight;

    // convert to positive integers for use in equations
    int posBiWidth = abs(infileW);
    int posBiHeight= abs(infileH);

    // determine padding for outfile scanlines
    int outPadding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // update biSizeImage and bfSize in file headers for outfile
    // convert to positive integers for use in equations
    int posOutPadding = outPadding;
    posOutPadding = abs(posOutPadding);

    // update biSizeImage
    int biSizeImageInt = 3 * (posBiWidth * posBiHeight) + (posOutPadding * posBiHeight);
    bi.biSizeImage = biSizeImageInt;

    // update bfSize
    int bfSizeInt = 54 + biSizeImageInt;
    bf.bfSize = bfSizeInt;

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // iterate over infile's scanlines
    for (int i = 0; i < posInBiHeight; i++)
    {
        int repeat = -1;
        do {

        // iterate over pixels in scanline
        for (int j = 0; j < inBiWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // scale pixels horizontally by n
            for (int l = 0; l < n; l++)
            {
                fwrite(&triple.rgbtBlue, sizeof(triple.rgbtBlue), 1, outptr);
                fwrite(&triple.rgbtGreen, sizeof(triple.rgbtGreen), 1, outptr);
                fwrite(&triple.rgbtRed, sizeof(triple.rgbtRed), 1, outptr);
            }
        }

        // add padding to outfile if needed
        for (int k = 0; k < outPadding; k++)
        {
            // printf("0x00 ");
            fputc(0x00, outptr);
        }

        // scale vertically by n
        repeat ++;
        if (repeat != n - 1)
        {
            // send infile cursor back to the beginning of the row
            fseek(inptr, 0 - (inBiWidth * 3), SEEK_CUR);
        }
        }
        while (repeat != n - 1);

        // skip over padding, if any
        fseek(inptr, inPadding, SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
