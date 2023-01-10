#include <stdlib.h>
#include <stdbool.h>
#include "myfunction1.h"
#include "showBMP.h"

/*
* General notes:
* Not using max, min, or calcIndex - costly functions
* not passing/using "dim" - it's always =m, which is known.
*/

/*
 * assign_sum_to_pixel - Truncates pixel's new value to match the range [0,255]
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum, int kernelScale) {

	unsigned char red, blue, green;
	// divide by kernel's weight
	sum.red /= kernelScale;
	sum.green /= kernelScale;
	sum.blue /= kernelScale;

	// truncate each pixel's color values to match the range [0,255]

	// Convert sum values to unsigned char, to test if they've changed during casting
	red = (unsigned char) sum.red;
	green = (unsigned char) sum.green;
	blue = (unsigned char) sum.blue;

	// If value changed by cast, keep 0 (when new value < 255) or 255 (else)
	current_pixel->red = (red == sum.red) ? red : 255*(sum.red >= 255);
	current_pixel->green = (green == sum.green) ? green : 255*(sum.green >= 255);
	current_pixel->blue = (blue == sum.blue) ? blue : 255*(sum.blue >= 255);
	return;
}

/*
* sum_pixels_by_weight - Sums pixel values, scaled by given weight
*/
static void sum_pixels_by_weight(pixel_sum *sum, pixel p, int weight) {
	sum->red += (int) p.red * weight;
	sum->green += (int) p.green * weight;
	sum->blue += (int) p.blue * weight;	
	return;
}

/*
 *  Applies kernel for pixel at (i,j)
 */
static void applyKernel(int i, int j, pixel *src, pixel *dst, int offset, int *kernel, int kernelScale, bool filter) {

	pixel_sum sum;
	// initializing sum, no need to call separate function for one liner
	sum.red = sum.green = sum.blue = 0;
	int src_idx, left, right;

	// i and j are never outside of bounds due to loop in smooth(), no need to make sure they're in bounds.
	// jj only ever iterates over 3 values, no need for loops
	left = j-1;
	right = j+1;

	// Unrolled loop; kernels are all 3x3.
	src_idx = (i-1) * m; // Row 1
	sum_pixels_by_weight(&sum, src[src_idx + left], kernel[0]);
	sum_pixels_by_weight(&sum, src[src_idx + j],  kernel[1]);
	sum_pixels_by_weight(&sum, src[src_idx + right], kernel[2]);

	src_idx += m; // Row 2
	sum_pixels_by_weight(&sum, src[src_idx + left], kernel[3]);
	sum_pixels_by_weight(&sum, src[src_idx + j],  kernel[4]);
	sum_pixels_by_weight(&sum, src[src_idx + right], kernel[5]);

	src_idx += m; // Row 3
	sum_pixels_by_weight(&sum, src[src_idx + left], kernel[6]);
	sum_pixels_by_weight(&sum, src[src_idx + j],  kernel[7]);
	sum_pixels_by_weight(&sum, src[src_idx + right], kernel[8]);


	if (filter) {
		// Lazy definitions+inits of vars, only needed when using the filter
		int ii, jj;
		int loop_pixel_sum;
		int min_intensity = 766; // arbitrary value that is higher than maximum possible intensity, which is 255*3=765
		int max_intensity = -1; // arbitrary value that is lower than minimum possible intensity, which is 0
		int min_row, min_col, max_row, max_col;
		pixel loop_pixel;

		// find min and max coordinates
		src_idx = (i-1) * m;
		for(ii = i-1; ii <= i+1; ii++) {
			for(jj = left; jj <= right; jj++) {
				// check if smaller than min or higher than max and update
				loop_pixel = src[src_idx + jj];
				// Calculate sum once
				loop_pixel_sum = loop_pixel.red + loop_pixel.green + loop_pixel.blue;
				if (loop_pixel_sum <= min_intensity) {
					min_intensity = loop_pixel_sum;
					min_row = ii;
					min_col = jj;
				}
				if (loop_pixel_sum > max_intensity) {
					max_intensity = loop_pixel_sum;
					max_row = ii;
					max_col = jj;
				}
			}
			src_idx += m; // Next row
		}
		// filter out min and max
		sum_pixels_by_weight(&sum, src[min_row*m + min_col], -1);
		sum_pixels_by_weight(&sum, src[max_row*m + max_col], -1);
	}

	// assign kernel's result to pixel at [i,j]
	assign_sum_to_pixel(&dst[offset], sum, kernelScale);
}

/*
* Apply the kernel over each pixel.
* Ignore pixels where the kernel exceeds bounds. These are pixels with row index smaller than kernelSize/2 and/or
* column index smaller than kernelSize/2
*/
void smooth(pixel *src, pixel *dst, int *kernel, int kernelScale, bool filter) {

	int i, j, dim_kS_half, index_calc;
	dim_kS_half = m - 1;
	index_calc = m;
	for (i = 1 ; i < dim_kS_half; i++) {
		for (j = 1 ; j < dim_kS_half ; j++)
			// Apply kernel in-place on dst, instead of getting value back from func and assigning it
			applyKernel(i, j, src, dst, index_calc+j, kernel, kernelScale, filter);
		index_calc += m; // next row
	}
}

void charsToPixels(Image *charsImg, pixel* pixels) {

	int i, j, mn;
	mn = m*n;
	for (i = 0, j = 0 ; i < mn ; i++, j+=3) {
		pixels[i].red = (int) charsImg->data[j];
		pixels[i].green = (int) charsImg->data[j + 1];
		pixels[i].blue = (int) charsImg->data[j + 2];
	}
}

void pixelsToChars(pixel* pixels, Image *charsImg) {
	int i, j, mn;
	mn = m*n;
	for (i = 0, j = 0 ; i < mn ; i++, j+=3) {
		charsImg->data[j] = pixels[i].red;
		charsImg->data[j + 1] = pixels[i].green;
		charsImg->data[j + 2] = pixels[i].blue;
	}
}

void copyPixels(pixel* src, pixel* dst) {
	int i, mn;
	mn = m*n;
	// copy structs by-assign, instead of by-members
	for (i = 0 ; i < mn ; i++)
		dst[i] = src[i];
}

void doConvolution(Image *image, int kernelSize, int *kernel, int kernelScale, bool filter) {

	int malloc_size;
	malloc_size = m*n*sizeof(pixel);
	pixel* pixelsImg = malloc(malloc_size);
	pixel* backupOrg = malloc(malloc_size);

	charsToPixels(image, pixelsImg);
	copyPixels(pixelsImg, backupOrg);
	*backupOrg = *pixelsImg;

	smooth(backupOrg, pixelsImg, kernel, kernelScale, filter);

	pixelsToChars(pixelsImg, image);

	free(pixelsImg);
	free(backupOrg);
}

