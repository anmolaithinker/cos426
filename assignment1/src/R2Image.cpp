 // Source file for image class



// Include files 

#include "R2/R2.h"
#include "R2Pixel.h"
#include "R2Image.h"
#include <iostream>
#include <vector>




////////////////////////////////////////////////////////////////////////
// Constructors/Destructors
////////////////////////////////////////////////////////////////////////


R2Image::
R2Image(void)
  : pixels(NULL),
    npixels(0),
    width(0), 
    height(0)
{
}



R2Image::
R2Image(const char *filename)
  : pixels(NULL),
    npixels(0),
    width(0), 
    height(0)
{
  // Read image
  Read(filename);
}



R2Image::
R2Image(int width, int height)
  : pixels(NULL),
    npixels(width * height),
    width(width), 
    height(height)
{
  // Allocate pixels
  pixels = new R2Pixel [ npixels ];
  assert(pixels);
}



R2Image::
R2Image(int width, int height, const R2Pixel *p)
  : pixels(NULL),
    npixels(width * height),
    width(width), 
    height(height)
{
  // Allocate pixels
  pixels = new R2Pixel [ npixels ];
  assert(pixels);

  // Copy pixels 
  for (int i = 0; i < npixels; i++) 
    pixels[i] = p[i];
}



R2Image::
R2Image(const R2Image& image)
  : pixels(NULL),
    npixels(image.npixels),
    width(image.width), 
    height(image.height)
    
{
  // Allocate pixels
  pixels = new R2Pixel [ npixels ];
  assert(pixels);

  // Copy pixels 
  for (int i = 0; i < npixels; i++) 
    pixels[i] = image.pixels[i];
}



R2Image::
~R2Image(void)
{
  // Free image pixels
  if (pixels) delete [] pixels;
}



R2Image& R2Image::
operator=(const R2Image& image)
{
  // Delete previous pixels
  if (pixels) { delete [] pixels; pixels = NULL; }

  // Reset width and height
  npixels = image.npixels;
  width = image.width;
  height = image.height;

  // Allocate new pixels
  pixels = new R2Pixel [ npixels ];
  assert(pixels);

  // Copy pixels 
  for (int i = 0; i < npixels; i++) 
    pixels[i] = image.pixels[i];

  // Return image
  return *this;
}



////////////////////////////////////////////////////////////////////////
// Image processing functions
// YOU IMPLEMENT THE FUNCTIONS IN THIS SECTION
////////////////////////////////////////////////////////////////////////

// Per-pixel Operations ////////////////////////////////////////////////

void R2Image::
Brighten(double factor)
{
  // Brighten the image by multiplying each pixel component by the factor,
  // then clamping the result to a valid range.

  // MAY ADD CODE HERE FROM ASSIGNMENT 0. NO CREDIT FOR THIS FEATURE
  for (int i = 0; i < npixels; i++) {
    pixels[i] *= factor;
    pixels[i].Clamp();
  }
}

void R2Image::
AddNoise(double factor)
{
  // Add noise to an image.  The amount of noise is given by the factor
  // in the range [0.0..1.0].  0.0 adds no noise.  1.0 adds a lot of noise.

	// MAY FILL IN IMPLEMENTATION HERE (REMOVE PRINT STATEMENT WHEN DONE)
  fprintf(stderr, "AddNoise(%g) not implemented\n", factor);
}

void R2Image::
ChangeContrast(double factor)
{
  // Change the contrast of an image by interpolating between the image
  // and a constant gray image with the average luminance.
  // Interpolation reduces constrast, extrapolation boosts constrast,
  // and negative factors generate inverted images.

	double avglumi=0;
  for (int i = 0; i < npixels; i++) {
		avglumi += pixels[i].Luminance();
	}
	avglumi /= npixels;
	R2Pixel greypixel(avglumi, avglumi, avglumi, 1.0);
  
	for (int i = 0; i < npixels; i++) {
		pixels[i] = (1-factor)*greypixel + factor*pixels[i];
		pixels[i].Clamp();
  }
}

void R2Image::
ChangeSaturation(double factor)
{
  // Changes the saturation of an image by interpolating between the
  // image and a gray level version of the image.  Interpolation
  // decreases saturation, extrapolation increases it, negative factors
  // preserve luminance  but invert the hue of the input image.

  for (int i = 0; i < npixels; i++) {
		double lumi = pixels[i].Luminance();
		R2Pixel bwpixel(lumi, lumi, lumi, 1.0);
		pixels[i] = (1-factor)*bwpixel + factor*pixels[i];
		pixels[i].Clamp();
  }
}

void R2Image::
ApplyGamma(double exponent)
{
  // Apply a gamma correction with exponent to each pixel

  if(exponent < 0) {
		fprintf(stderr, "Gamma exponent (%f) negative\n", exponent);
		return;
	}
  
  for (int i = 0; i < npixels; i++) {
		pixels[i].SetRed(pow(pixels[i].Red(), exponent));
		pixels[i].SetGreen(pow(pixels[i].Green(), exponent));
		pixels[i].SetBlue(pow(pixels[i].Blue(), exponent));
  }
}

void R2Image::
BlackAndWhite(void)
{
  // Replace each pixel with its luminance value
  // Put this in each channel,  so the result is grayscale

	for (int i = 0; i < npixels; i++) {
		double lumi = pixels[i].Luminance();
		pixels[i].SetRed(lumi);
		pixels[i].SetGreen(lumi);
		pixels[i].SetBlue(lumi);
	}
}

void R2Image::
ExtractChannel(int channel)
{
  // Extracts a channel of an image (e.g., R2_IMAGE_RED_CHANNEL).  
  // Leaves the specified channel intact, 
  // and sets all the other ones to zero.

	
	for (int i = 0; i < npixels; i++) {
		R2Pixel p(0.0, 0.0, 0.0, 1.0);
		if(channel == R2_IMAGE_RED_CHANNEL) {
			p.SetRed(pixels[i].Red());
		} else if(channel == R2_IMAGE_GREEN_CHANNEL) {
			p.SetGreen(pixels[i].Green());
		} else if(channel == R2_IMAGE_BLUE_CHANNEL) {
			p.SetBlue(pixels[i].Blue());
		} else if(channel == R2_IMAGE_ALPHA_CHANNEL) {
			p.SetAlpha(pixels[i].Alpha());
		} else {
			fprintf(stderr, "Invalid channel (%d)\n", channel);
			return;
		}
		pixels[i] = p;
	}
}

// Linear filtering ////////////////////////////////////////////////


void R2Image::
Blur(double sigma)
{
  // Blur an image with a Gaussian filter with a given sigma.
	if(sigma == 0) return;

	ApplyGamma(2.2);
	R2Image original(*this);

	int size = 3*sigma;
	if(size < 1) size = 1;
	std::vector<double> col(size+1, 0.0);
	std::vector<std::vector<double> > gaussiankernel(size+1, col);
	for(int i=0; i<=size; i++) {
		for(int j=0; j<=size; j++) {
			double distancesquared = i*i + j*j;
			gaussiankernel[i][j] = exp(-distancesquared / 2.0 / sigma / sigma);
		}
	}

	for (int i = 0; i < npixels; i++) {
		R2Pixel p(0.0, 0.0, 0.0, pixels[i].Alpha());
		int x0 = i/height;
		int y0 = i%height;
		double total=0;

		for(int x = (x0 - size < 0 ? 0 : x0 - size); x < (x0 + size + 1 > width ? width : x0 + size + 1); x++) {
			for(int y = (y0 - size < 0 ? 0 : y0 - size); y < (y0 + size + 1 > height ? height : y0 + size + 1); y++) {
				double g = gaussiankernel[abs(x-x0)][abs(y-y0)];
				p += g*original.Pixel(x,y);
				total += g;
			}
		}
		p /= total;
		pixels[i] = p;
	}

	ApplyGamma(1.0/2.2);
}


void R2Image::
Sharpen()
{
  // Sharpen an image using a linear filter
	R2Image blurredImage(*this);
	blurredImage.Blur(2.0);

	double factor=2.0;
	for (int i = 0; i < npixels; i++) {
		pixels[i] = (1-factor)*blurredImage.Pixels()[i] + factor*pixels[i];
		pixels[i].Clamp();
  }
}

void R2Image::
Sobel(void)
{
  // Detect edges in an image.
	ApplyGamma(2.2);
	R2Image orig(*this);
	
	for (int i = 0; i < npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;
		double gx=0;
		double gy=0;

		for(int x = x0 - 1; x <= x0 + 1; x++) {
			if(x < 0) continue;
			if(x >= width) continue;
			for(int y = y0 - 1; y <= y0 + 1; y++) {
				if(y < 0) continue;
				if(y >= height) continue;

				if(x != x0) { //gx
					double sign = (x < x0) ? -1 : 1;
					double magnitude = (y == y0) ? 2 : 1;
					gx += orig.Pixel(x,y).Luminance() * sign * magnitude;
				}
				if(y != y0) { //gx
					double sign = (y < y0) ? 1 : -1;
					double magnitude = (x == x0) ? 2 : 1;
					gy += orig.Pixel(x,y).Luminance() * sign * magnitude;
				}
			}
		}
		double lumi = sqrt(gx*gx + gy*gy)/4.0;
		pixels[i].SetBlue(lumi);
		pixels[i].SetRed(lumi);
		pixels[i].SetGreen(lumi);
	}

	ApplyGamma(1.0/2.2);
}

void R2Image::
EdgeDetect(void)
{
  // Detect edges in an image.
	ApplyGamma(2.2);
	R2Image orig(*this);
	
	for (int i = 0; i < npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;
		
		R2Pixel p(0.0, 0.0, 0.0, 1.0);
		double counter = 0;

		for(int x = x0 - 1; x <= x0 + 1; x++) {
			if(x < 0) continue;
			if(x >= width) continue;
			for(int y = y0 - 1; y <= y0 + 1; y++) {
				if(y < 0) continue;
				if(y >= height) continue;

				if(x == x0 && y == y0) {
					p += 8*orig.Pixel(x,y);
					counter += 8;
				} else {
					p += -1.0*orig.Pixel(x,y);
					counter++;
				}
			}
		}
		
		p /= counter;
		pixels[i] = p;
	}

	ApplyGamma(1.0/2.2);
}

void R2Image::
MotionBlur(int amount)
{
  // Perform horizontal motion blur

  // convolve in X direction with a linear ramp of amount non-zero pixels
  // the image should be strongest on the right hand side (see example)

	ApplyGamma(2.2);
	R2Image orig(*this);
	
	for (int i = 0; i < npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;

		R2Pixel p(0.0,0.0,0.0, orig.Pixel(x0,y0).Alpha());
		int counter = 0;
		double total = 0;
		for(int x = x0 - amount; x <= x0; x++) {
			if(x < 0) continue;
			counter++;
			p += orig.Pixel(x,y0) * counter;
			total += counter;
		}
		p /= total;
		pixels[i] = p;
	}

	ApplyGamma(1.0/2.2);
}


// Non-Linear filtering ////////////////////////////////////////////////

void R2Image::
MedianFilter(double sigma)
{
  // Perform median filtering with a given width
	R2Image orig(*this);

	for (int i = 0; i < npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;

		std::vector<double> rs, gs, bs;
		for(int x = x0 - sigma; x <= x0 + sigma; x++) {
			if(x < 0) continue;
			if(x >= width) continue;
			for(int y = y0 - sigma; y <= y0 + sigma; y++) {
				if(y < 0) continue;
				if(y >= height) continue;

				rs.push_back(orig.Pixel(x,y).Red());
				bs.push_back(orig.Pixel(x,y).Blue());
				gs.push_back(orig.Pixel(x,y).Green());
			}
		}
		sort(rs.begin(), rs.end());
		sort(bs.begin(), bs.end());
		sort(gs.begin(), gs.end());

		double r, g, b;
		if(rs.size() % 2 == 0) {
			r = 0.5*(rs[rs.size()/2 - 1] + rs[rs.size()/2]);
			g = 0.5*(gs[rs.size()/2 - 1] + gs[rs.size()/2]);
			b = 0.5*(bs[rs.size()/2 - 1] + bs[rs.size()/2]);
		} else {
			r = rs[rs.size()/2];
			g = gs[rs.size()/2];
			b = bs[rs.size()/2];
		}

		pixels[i] = R2Pixel(r, g, b, 1.0);
	}
}

void R2Image::
BilateralFilter(double rangesigma, double domainsigma)
{
  // Perform bilateral filtering with a given range and domain widths.
	R2Image orig(*this);

	int size = 3*domainsigma;
	if(size < 1) size = 1;

	std::vector<double> col(size+1, 0.0);
	std::vector<std::vector<double> > gaussiankernel(size+1, col);
	for(int i=0; i<=size; i++) {
		for(int j=0; j<=size; j++) {
			double distancesquared = i*i + j*j;
			gaussiankernel[i][j] = exp(-distancesquared / 2.0 / domainsigma / domainsigma);
		}
	}

	for (int i = 0; i < npixels; i++) {
		R2Pixel p(0.0, 0.0, 0.0, pixels[i].Alpha());
		int x0 = i/height;
		int y0 = i%height;
		double total=0;

		for(int x = (x0 - size < 0 ? 0 : x0 - size); x < (x0 + size +1 > width ? width : x0 + size + 1); x++) {
			for(int y = (y0 - size < 0 ? 0 : y0 - size); y < (y0 + size + 1 > height ? height : y0 + size + 1); y++) {
				double closeness = gaussiankernel[abs(x-x0)][abs(y-y0)];

				R2Pixel pixeldiff = orig.Pixel(x,y) - pixels[i];
				double pixeldiffsquared = pow(pixeldiff.Blue(), 2) + pow(pixeldiff.Red(), 2) + pow(pixeldiff.Green(), 2);
				double similarity = exp(-pixeldiffsquared / 2.0 / rangesigma / rangesigma);

				p += closeness*similarity*orig.Pixel(x,y);
				total += closeness*similarity;
			}
		}
		p /= total;
		pixels[i] = p;
	}
}


// Resampling operations  ////////////////////////////////////////////////
R2Pixel R2Image::Sample(double x0, double y0, int sampling_method, double sigma_x, double sigma_y)
{
	if(sampling_method == R2_IMAGE_POINT_SAMPLING) {
		int x_orig = lround(x0);
		int y_orig = lround(y0);
		// Just in case for range violations
		if(x_orig < 0) x_orig = 0;
		if(x_orig >= width) x_orig = width - 1;
		if(y_orig < 0) y_orig = 0;
		if(y_orig >= height) y_orig = height - 1;
		return Pixel(x_orig, y_orig);
	}
	else if(sampling_method == R2_IMAGE_GAUSSIAN_SAMPLING) {
		int x_orig = lround(x0);
		int y_orig = lround(y0);

		R2Pixel p(0.0, 0.0, 0.0, 1.0);
		double total=0;
		int size_x = sigma_x * 3;
		if(size_x < 1) size_x = 1;
		int size_y = sigma_y * 3;
		if(size_y < 1) size_y = 1;

		for(int x = (x_orig - size_x < 0 ? 0 : x_orig - size_x); x < (x_orig + size_x + 1 > width ? width : x_orig + size_x + 1); x++) {
			for(int y = (y_orig - size_y < 0 ? 0 : y_orig - size_y); y < (y_orig + size_y + 1 > height ? height : y_orig + size_y + 1); y++) {
				double g = exp(-(x-x0)*(x-x0) / 2.0 / sigma_x / sigma_x) * exp(-(y-y0)*(y-y0) / 2.0 / sigma_y / sigma_y);
				p += g*Pixel(x,y);
				total += g;
			}
		}
		p /= total;
		return p;
	}
	else if(sampling_method == R2_IMAGE_BILINEAR_SAMPLING) {
		int xlow = floor(x0) > 0 ? floor(x0) : 0;
		int xhigh = ceil(x0) < width ? ceil(x0) : width - 1;
		int ylow = floor(y0) > 0 ? floor(y0) : 0;
		int yhigh = ceil(y0) < height ? ceil(y0) : height - 1;

		R2Pixel a = (1.0 - (x0-xlow))*Pixel(xlow, yhigh) + (x0-xlow)*Pixel(xhigh, yhigh);
		R2Pixel b = (1.0 - (x0-xlow))*Pixel(xlow, ylow) + (x0-xlow)*Pixel(xhigh, ylow);
		R2Pixel dst = (1.0 - (y0-ylow))*b + (y0-ylow)*a;
		return dst;
	}
	else {
		fprintf(stderr, "Invalid sampling method (%d)\n", sampling_method);
		return R2Pixel();
	}
}

void R2Image::
Scale(double sx, double sy, int sampling_method)
{
  // Scale an image in x by sx, and y by sy.
	R2Image orig(*this);
	width = lround(sx*orig.width);
	height = lround(sy*orig.height);

	delete [] pixels;
	npixels = width*height;
	pixels = new R2Pixel[npixels];

	double xoffset = 0.5 * ((double)orig.width) / ((double)width) - 0.5;
	double yoffset = 0.5 * ((double)orig.height) / ((double)height) - 0.5;
	
	for(int i=0; i<npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;

		double x_orig = ((double)orig.width) / ((double)width) * x0 + xoffset; 
		double y_orig = ((double)orig.height) / ((double)height) * y0 + yoffset; 

		double sigma_x = 1.0/3.0/sx, sigma_y = 1.0/3.0/sy;
		if(sx > 1.0) { sigma_x = 0.5; }
		if(sy > 1.0) { sigma_y = 0.5; }
		pixels[i] = orig.Sample(x_orig, y_orig, sampling_method, sigma_x, sigma_y);
	}
}


void R2Image::
Rotate(double angle, int sampling_method)
{
  // Rotate an image by the given angle.
	
	// Expand canvas
	R2Image orig(*this);
	width = sqrt(orig.width*orig.width + orig.height*orig.height);
	height = sqrt(orig.width*orig.width + orig.height*orig.height);

	delete [] pixels;
	npixels = width*height;
	pixels = new R2Pixel[npixels];

	// Copy image across onto expanded canvas so we have black edge to sample against for smooth line
	R2Image orig2(width, height);
	for(int x=0; x<orig2.width; x++) {
		for(int y=0; y<orig2.height; y++) {
			int x0 = x - orig2.width/2 + orig.width/2;
			int y0 = y - orig2.height/2 + orig.height/2;
			if(x0 >= orig.width || y0 >= orig.height || x0 < 0 || y0 < 0)
				continue;
			orig2.SetPixel(x,y, orig.Pixel(x0,y0));
		}
	}

	// Middle of image to perform rotation around
	double yoffset = ((double)height)/2;
	double xoffset = ((double)width)/2;
	
	for(int i=0; i<npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;
		
		double x_orig = (x0-xoffset)*cos(-angle) - (y0-yoffset)*sin(-angle) + xoffset;
		double y_orig = (x0-xoffset)*sin(-angle) + (y0-yoffset)*cos(-angle) + yoffset;
		//double x_orig = x0, y_orig = y0;

		if(x_orig < 0 || x_orig >= width || y_orig < 0 || y_orig >= height) {
			pixels[i] = R2Pixel(0,0,0,1);
		}
		else {
			double sigma_x = 0.5, sigma_y = 0.5;
			pixels[i] = orig2.Sample(x_orig, y_orig, sampling_method, sigma_x, sigma_y);
		}
	}
}


void R2Image::
Fun(int sampling_method)
{
  // Warp an image using a creative filter of your choice.
	
	R2Image orig2(*this);

	for(int i=0; i<npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;
		
		double x_orig = x0 + 10.0*cos(2*M_PI * ((double)y0) / 20.0);
		double y_orig = y0;

		if(x_orig < 0 || x_orig >= width || y_orig < 0 || y_orig >= height) {
			pixels[i] = R2Pixel(0,0,0,1);
		}
		else {
			double sigma_x = 0.5, sigma_y = 0.5;
			pixels[i] = orig2.Sample(x_orig, y_orig, sampling_method, sigma_x, sigma_y);
		}
	}
}


// Dither operations ////////////////////////////////////////////////

void R2Image::
Quantize (int nbits)
{
  // Quantizes an image with "nbits" bits per channel.
	for(int i=0; i<npixels; i++) {
		double r, g, b, a;
		pixels[i].Clamp(1.0);
		r = round(pixels[i].Red()*(pow(2.0, nbits)-1)) / (pow(2.0, nbits) - 1);
		g = round(pixels[i].Green()*(pow(2.0, nbits)-1)) / (pow(2.0, nbits) - 1);
		b = round(pixels[i].Blue()*(pow(2.0, nbits)-1)) / (pow(2.0, nbits) - 1);
		a = round(pixels[i].Alpha()*(pow(2.0, nbits)-1)) / (pow(2.0, nbits) - 1);
		pixels[i].Reset(r, g, b, a);
	}
}



void R2Image::
RandomDither(int nbits)
{
  // Converts and image to nbits per channel using random dither.
	for(int i=0; i<npixels; i++) {
		double val;

		for(int j=0; j<4; j++) { //One for each channel
			val = pixels[i][j]*(pow(2.0, nbits)-1);
			double e = val - floor(val);
			if(e >= ((double)rand())/ RAND_MAX)
				val = ceil(val);
			else
				val = floor(val);
			pixels[i][j] = val / (pow(2.0, nbits)-1);
		}
	}
}

void R2Image::
OrderedDither(int nbits)
{
  // Converts an image to nbits per channel using ordered dither, 
  // with a 4x4 Bayer's pattern matrix.
	std::vector<double> col(4, 0.0);
	std::vector<std::vector<double> > pattern(4, col);
	pattern[0][0] = 1;
	pattern[0][1] = 13;
	pattern[0][2] = 4;
	pattern[0][3] = 16;
	pattern[1][0] = 9;
	pattern[1][1] = 5;
	pattern[1][2] = 12;
	pattern[1][3] = 8;
	pattern[2][0] = 3;
	pattern[2][1] = 15;
	pattern[2][2] = 2;
	pattern[2][3] = 14;
	pattern[3][0] = 11;
	pattern[3][1] = 7;
	pattern[3][2] = 10;
	pattern[3][3] = 6;

	for(int i=0; i<npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;
		double val;

		for(int j=0; j<4; j++) { //One for each channel
			val = pixels[i][j]*(pow(2.0, nbits)-1);
			double e = val - floor(val);
			if(e >= pattern[x0%4][y0%4]/16.0)
				val = ceil(val);
			else
				val = floor(val);
			pixels[i][j] = val / (pow(2.0, nbits) - 1);
		}
	}

}



void R2Image::
FloydSteinbergDither(int nbits)
{
  // Converts an image to nbits per channel using Floyd-Steinberg dither.
  // with error diffusion.

	for(int y0=0; y0<height; y0++) {
		for(int x0=0; x0<width; x0++) {
			double prev;
			Pixel(x0, y0).Clamp(1);
			for(int j=0; j<4; j++) { //One for each channel
				prev = Pixel(x0, y0)[j];
				Pixel(x0, y0)[j] = round(prev*(pow(2.0, nbits)-1)) / (pow(2.0, nbits)-1);

				double e = prev - Pixel(x0, y0)[j];

				if(x0 + 1 < width)
					Pixel(x0+1, y0)[j] += 7.0/16.0 * e;
				if(x0 - 1 >= 0 && y0 + 1 < height)
					Pixel(x0-1, y0+1)[j]+= 3.0/16.0 * e;
				if(y0 + 1 < height)
					Pixel(x0, y0+1)[j] += 5.0/16.0 * e;
				if(x0 + 1 < width && y0 + 1 < height)
					Pixel(x0+1, y0+1)[j] += 1.0/16.0 * e;
			}
		}
	}
}



// Miscellaneous operations ////////////////////////////////////////////////

void R2Image::
CopyChannel(const R2Image& from_image, int from_channel, int to_channel)
{
  // Copies one channel of an image (e.g., R2_IMAGE_RED_CHANNEL).  
  // to another channel

	for(int x0=0; x0<width; x0++) {
		for(int y0=0; y0<height; y0++) {
			Pixel(x0,y0)[to_channel] = from_image.Pixel(x0,y0)[from_channel];
		}
	}
}

void R2Image::
Composite(const R2Image& top, int operation)
{
  // Composite passed image on top of this one using operation OVER
	
	for(int i=0; i<npixels; i++) {
		double alphatop = top.pixels[i].Alpha();
		double alphabottom = pixels[i].Alpha();

		pixels[i] = alphatop * top.pixels[i] + alphabottom * (1 - alphatop) * pixels[i];
		pixels[i].SetAlpha(alphatop + alphabottom * (1-alphatop));
	}
}

void R2Image::
Morph(const R2Image& target, 
  R2Segment *source_segments, R2Segment *target_segments, int nsegments, 
  double t, int sampling_method)
{
  // Morph this source image towards a passed target image by t using pairwise line segment correspondences
	R2Image orig_s(*this);
	
	for(int i=0; i<npixels; i++) {
		pixels[i].Reset(0,0,0,1);
		int x0 = i/height;
		int y0 = i%height;

		R2Vector Dsum(0,0);
		double weightsum = 0;
		R2Point X(x0, y0);

		for(int j=0; j<nsegments; j++) {
			R2Point P = target_segments[j].Start();
			R2Point Q = target_segments[j].End();

			double u = (X-P).Dot(Q-P) / (Q-P).Dot(Q-P);

			R2Vector perp = Q-P;
			perp.Rotate(M_PI/2.0);
			double v = (X-P).Dot(perp) / sqrt((Q-P).Dot(Q-P));

			R2Point P_ = t*source_segments[j].Start() + (1-t)*target_segments[j].Start();
			R2Point Q_ = t*source_segments[j].End() + (1-t)*target_segments[j].End();
			R2Vector perp_ = Q_ - P_;
			perp_.Rotate(M_PI/2.0);
			R2Point X_ = P_ + u*(Q_ - P_) + v*perp_/sqrt((Q_-P_).Dot(Q_-P_));

			R2Vector D = X_ - X;
			R2Point closestpoint = target_segments[j].Line().ClosestPoint(X);
			double dist = sqrt((X - closestpoint).Dot(X-closestpoint));

			double a = 0.5;
			double b = 1;
			double p = 0.5;
			double weight = pow(pow(target_segments[j].Length(), p) / (a+dist), b);
			Dsum += D*weight;
			weightsum += weight;

		}
		R2Point X_ = X + Dsum / weightsum;

		int x1 = X_.X();
		int y1 = X_.Y();
		if(x1 > 0 && x1 < width && y1 > 0 && y1 < height)
			pixels[i] = orig_s.Sample(x1, y1, sampling_method, 0.5, 0.5);
	}
	
	R2Image targetc(target);
	R2Image targetn(width, height);
	for(int i=0; i<npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;

		R2Vector Dsum(0,0);
		double weightsum = 0;
		R2Point X(x0, y0);

		for(int j=0; j<nsegments; j++) {
			R2Point P = source_segments[j].Start();
			R2Point Q = source_segments[j].End();

			double u = (X-P).Dot(Q-P) / (Q-P).Dot(Q-P);

			R2Vector perp = Q-P;
			perp.Rotate(M_PI/2.0);
			double v = (X-P).Dot(perp) / sqrt((Q-P).Dot(Q-P));

			R2Point P_ = t*source_segments[j].Start() + (1-t)*target_segments[j].Start();
			R2Point Q_ = t*source_segments[j].End() + (1-t)*target_segments[j].End();
			R2Vector perp_ = Q_ - P_;
			perp_.Rotate(M_PI/2.0);
			R2Point X_ = P_ + u*(Q_ - P_) + v*perp_/sqrt((Q_-P_).Dot(Q_-P_));

			R2Vector D = X_ - X;
			R2Point closestpoint = source_segments[j].Line().ClosestPoint(X);
			double dist = sqrt((X - closestpoint).Dot(X-closestpoint));

			double a = 0.5;
			double b = 1;
			double p = 0.5;
			double weight = pow(pow(source_segments[j].Length(), p) / (a+dist), b);
			Dsum += D*weight;
			weightsum += weight;

		}
		R2Point X_ = X + Dsum / weightsum;

		int x1 = X_.X();
		int y1 = X_.Y();
		if(x1 > 0 && x1 < width && y1 > 0 && y1 < height)
			targetn.Pixel(x0,y0) = targetc.Sample(x1, y1, sampling_method, 0.5, 0.5);
	}

	for(int i=0; i<npixels; i++) {
		int x0 = i/height;
		int y0 = i%height;
		pixels[i] = (1-t) * pixels[i] + t*targetn.Pixel(x0,y0);
	}
}

void R2Image::
Crop(int x, int y, int w, int h)
{
  // Extracts a sub image from the image, 
  // at position (x, y), width w, and height h.

  // MAY FILL IN IMPLEMENTATION HERE (REMOVE PRINT STATEMENT WHEN DONE) (NO CREDIT FOR ASSIGNMENT)
  fprintf(stderr, "Crop(%d %d  %d %d) not implemented\n", x, y, w, h);
}

void R2Image::
Add(const R2Image& image)
{
  // Add passed image pixel-by-pixel.

  // MAY FILL IN IMPLEMENTATION HERE (REMOVE PRINT STATEMENT WHEN DONE) (NO CREDIT FOR ASSIGNMENT)
  fprintf(stderr, "Add not implemented\n");
}

void R2Image::
Subtract(const R2Image& image)
{
  // Subtract passed image from this image.

  // MAY FILL IN IMPLEMENTATION HERE (REMOVE PRINT STATEMENT WHEN DONE) (NO CREDIT FOR ASSIGNMENT)
  fprintf(stderr, "Subtract not implemented\n");
}


////////////////////////////////////////////////////////////////////////
// I/O Functions
////////////////////////////////////////////////////////////////////////

int R2Image::
Read(const char *filename)
{
  // Initialize everything
  if (pixels) { delete [] pixels; pixels = NULL; }
  npixels = width = height = 0;

  // Parse input filename extension
  char *input_extension;
  if (!(input_extension = (char*)strrchr(filename, '.'))) {
    fprintf(stderr, "Input file has no extension (e.g., .jpg).\n");
    return 0;
  }
  
  // Read file of appropriate type
  if (!strncmp(input_extension, ".bmp", 4)) return ReadBMP(filename);
  else if (!strncmp(input_extension, ".ppm", 4)) return ReadPPM(filename);
  else if (!strncmp(input_extension, ".jpg", 4)) return ReadJPEG(filename);
  else if (!strncmp(input_extension, ".jpeg", 5)) return ReadJPEG(filename);
  
  // Should never get here
  fprintf(stderr, "Unrecognized image file extension");
  return 0;
}



int R2Image::
Write(const char *filename) const
{
  // Parse input filename extension
  char *input_extension;
  if (!(input_extension = (char*)strrchr(filename, '.'))) {
    fprintf(stderr, "Input file has no extension (e.g., .jpg).\n");
    return 0;
  }
  
  // Write file of appropriate type
  if (!strncmp(input_extension, ".bmp", 4)) return WriteBMP(filename);
  else if (!strncmp(input_extension, ".ppm", 4)) return WritePPM(filename, 1);
  else if (!strncmp(input_extension, ".jpg", 5)) return WriteJPEG(filename);
  else if (!strncmp(input_extension, ".jpeg", 5)) return WriteJPEG(filename);

  // Should never get here
  fprintf(stderr, "Unrecognized image file extension");
  return 0;
}



////////////////////////////////////////////////////////////////////////
// BMP I/O
////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32)

typedef struct tagBITMAPFILEHEADER {
  unsigned short int bfType;
  unsigned int bfSize;
  unsigned short int bfReserved1;
  unsigned short int bfReserved2;
  unsigned int bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  unsigned int biSize;
  int biWidth;
  int biHeight;
  unsigned short int biPlanes;
  unsigned short int biBitCount;
  unsigned int biCompression;
  unsigned int biSizeImage;
  int biXPelsPerMeter;
  int biYPelsPerMeter;
  unsigned int biClrUsed;
  unsigned int biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBTRIPLE {
  unsigned char rgbtBlue;
  unsigned char rgbtGreen;
  unsigned char rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
  unsigned char rgbBlue;
  unsigned char rgbGreen;
  unsigned char rgbRed;
  unsigned char rgbReserved;
} RGBQUAD;

#endif

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

#define BMP_BF_TYPE 0x4D42 /* word BM */
#define BMP_BF_OFF_BITS 54 /* 14 for file header + 40 for info header (not sizeof(), but packed size) */
#define BMP_BI_SIZE 40 /* packed size of info header */


static unsigned short int WordReadLE(FILE *fp)
{
  // Read a unsigned short int from a file in little endian format 
  unsigned short int lsb, msb;
  lsb = getc(fp);
  msb = getc(fp);
  return (msb << 8) | lsb;
}



static void WordWriteLE(unsigned short int x, FILE *fp)
{
  // Write a unsigned short int to a file in little endian format
  unsigned char lsb = (unsigned char) (x & 0x00FF); putc(lsb, fp); 
  unsigned char msb = (unsigned char) (x >> 8); putc(msb, fp);
}



static unsigned int DWordReadLE(FILE *fp)
{
  // Read a unsigned int word from a file in little endian format 
  unsigned int b1 = getc(fp);
  unsigned int b2 = getc(fp);
  unsigned int b3 = getc(fp);
  unsigned int b4 = getc(fp);
  return (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
}



static void DWordWriteLE(unsigned int x, FILE *fp)
{
  // Write a unsigned int to a file in little endian format 
  unsigned char b1 = (x & 0x000000FF); putc(b1, fp);
  unsigned char b2 = ((x >> 8) & 0x000000FF); putc(b2, fp);
  unsigned char b3 = ((x >> 16) & 0x000000FF); putc(b3, fp);
  unsigned char b4 = ((x >> 24) & 0x000000FF); putc(b4, fp);
}



static int LongReadLE(FILE *fp)
{
  // Read a int word from a file in little endian format 
  int b1 = getc(fp);
  int b2 = getc(fp);
  int b3 = getc(fp);
  int b4 = getc(fp);
  return (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
}



static void LongWriteLE(int x, FILE *fp)
{
  // Write a int to a file in little endian format 
  char b1 = (x & 0x000000FF); putc(b1, fp);
  char b2 = ((x >> 8) & 0x000000FF); putc(b2, fp);
  char b3 = ((x >> 16) & 0x000000FF); putc(b3, fp);
  char b4 = ((x >> 24) & 0x000000FF); putc(b4, fp);
}



int R2Image::
ReadBMP(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open image file: %s\n", filename);
    return 0;
  }

  /* Read file header */
  BITMAPFILEHEADER bmfh;
  bmfh.bfType = WordReadLE(fp);
  bmfh.bfSize = DWordReadLE(fp);
  bmfh.bfReserved1 = WordReadLE(fp);
  bmfh.bfReserved2 = WordReadLE(fp);
  bmfh.bfOffBits = DWordReadLE(fp);
  
  /* Check file header */
  assert(bmfh.bfType == BMP_BF_TYPE);
  /* ignore bmfh.bfSize */
  /* ignore bmfh.bfReserved1 */
  /* ignore bmfh.bfReserved2 */
  assert(bmfh.bfOffBits == BMP_BF_OFF_BITS);
  
  /* Read info header */
  BITMAPINFOHEADER bmih;
  bmih.biSize = DWordReadLE(fp);
  bmih.biWidth = LongReadLE(fp);
  bmih.biHeight = LongReadLE(fp);
  bmih.biPlanes = WordReadLE(fp);
  bmih.biBitCount = WordReadLE(fp);
  bmih.biCompression = DWordReadLE(fp);
  bmih.biSizeImage = DWordReadLE(fp);
  bmih.biXPelsPerMeter = LongReadLE(fp);
  bmih.biYPelsPerMeter = LongReadLE(fp);
  bmih.biClrUsed = DWordReadLE(fp);
  bmih.biClrImportant = DWordReadLE(fp);
  
  // Check info header 
  assert(bmih.biSize == BMP_BI_SIZE);
  assert(bmih.biWidth > 0);
  assert(bmih.biHeight > 0);
  assert(bmih.biPlanes == 1);
  assert(bmih.biBitCount == 24);  /* RGB */
  assert(bmih.biCompression == BI_RGB);   /* RGB */
  int lineLength = bmih.biWidth * 3;  /* RGB */
  if ((lineLength % 4) != 0) lineLength = (lineLength / 4 + 1) * 4;
  assert(bmih.biSizeImage == (unsigned int) lineLength * (unsigned int) bmih.biHeight);

  // Assign width, height, and number of pixels
  width = bmih.biWidth;
  height = bmih.biHeight;
  npixels = width * height;

  // Allocate unsigned char buffer for reading pixels
  int rowsize = 3 * width;
  if ((rowsize % 4) != 0) rowsize = (rowsize / 4 + 1) * 4;
  int nbytes = bmih.biSizeImage;
  unsigned char *buffer = new unsigned char [nbytes];
  if (!buffer) {
    fprintf(stderr, "Unable to allocate temporary memory for BMP file");
    fclose(fp);
    return 0;
  }

  // Read buffer 
  fseek(fp, (long) bmfh.bfOffBits, SEEK_SET);
  if (fread(buffer, 1, bmih.biSizeImage, fp) != bmih.biSizeImage) {
    fprintf(stderr, "Error while reading BMP file %s", filename);
    return 0;
  }

  // Close file
  fclose(fp);

  // Allocate pixels for image
  pixels = new R2Pixel [ width * height ];
  if (!pixels) {
    fprintf(stderr, "Unable to allocate memory for BMP file");
    fclose(fp);
    return 0;
  }

  // Assign pixels
  for (int j = 0; j < height; j++) {
    unsigned char *p = &buffer[j * rowsize];
    for (int i = 0; i < width; i++) {
      double b = (double) *(p++) / 255;
      double g = (double) *(p++) / 255;
      double r = (double) *(p++) / 255;
      R2Pixel pixel(r, g, b, 1);
      SetPixel(i, j, pixel);
    }
  }

  // Free unsigned char buffer for reading pixels
  delete [] buffer;

  // Return success
  return 1;
}



int R2Image::
WriteBMP(const char *filename) const
{
  // Open file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Unable to open image file: %s\n", filename);
    return 0;
  }

  // Compute number of bytes in row
  int rowsize = 3 * width;
  if ((rowsize % 4) != 0) rowsize = (rowsize / 4 + 1) * 4;

  // Write file header 
  BITMAPFILEHEADER bmfh;
  bmfh.bfType = BMP_BF_TYPE;
  bmfh.bfSize = BMP_BF_OFF_BITS + rowsize * height;
  bmfh.bfReserved1 = 0;
  bmfh.bfReserved2 = 0;
  bmfh.bfOffBits = BMP_BF_OFF_BITS;
  WordWriteLE(bmfh.bfType, fp);
  DWordWriteLE(bmfh.bfSize, fp);
  WordWriteLE(bmfh.bfReserved1, fp);
  WordWriteLE(bmfh.bfReserved2, fp);
  DWordWriteLE(bmfh.bfOffBits, fp);

  // Write info header 
  BITMAPINFOHEADER bmih;
  bmih.biSize = BMP_BI_SIZE;
  bmih.biWidth = width;
  bmih.biHeight = height;
  bmih.biPlanes = 1;
  bmih.biBitCount = 24;       /* RGB */
  bmih.biCompression = BI_RGB;    /* RGB */
  bmih.biSizeImage = rowsize * (unsigned int) bmih.biHeight;  /* RGB */
  bmih.biXPelsPerMeter = 2925;
  bmih.biYPelsPerMeter = 2925;
  bmih.biClrUsed = 0;
  bmih.biClrImportant = 0;
  DWordWriteLE(bmih.biSize, fp);
  LongWriteLE(bmih.biWidth, fp);
  LongWriteLE(bmih.biHeight, fp);
  WordWriteLE(bmih.biPlanes, fp);
  WordWriteLE(bmih.biBitCount, fp);
  DWordWriteLE(bmih.biCompression, fp);
  DWordWriteLE(bmih.biSizeImage, fp);
  LongWriteLE(bmih.biXPelsPerMeter, fp);
  LongWriteLE(bmih.biYPelsPerMeter, fp);
  DWordWriteLE(bmih.biClrUsed, fp);
  DWordWriteLE(bmih.biClrImportant, fp);

  // Write image, swapping blue and red in each pixel
  int pad = rowsize - width * 3;
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      const R2Pixel& pixel = (*this)[i][j];
      double r = 255.0 * pixel.Red();
      double g = 255.0 * pixel.Green();
      double b = 255.0 * pixel.Blue();
      if (r >= 255) r = 255;
      if (g >= 255) g = 255;
      if (b >= 255) b = 255;
      fputc((unsigned char) b, fp);
      fputc((unsigned char) g, fp);
      fputc((unsigned char) r, fp);
    }

    // Pad row
    for (int i = 0; i < pad; i++) fputc(0, fp);
  }
  
  // Close file
  fclose(fp);

  // Return success
  return 1;  
}



////////////////////////////////////////////////////////////////////////
// PPM I/O
////////////////////////////////////////////////////////////////////////

int R2Image::
ReadPPM(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open image file: %s\n", filename);
    return 0;
  }

  // Read PPM file magic identifier
  char buffer[128];
  if (!fgets(buffer, 128, fp)) {
    fprintf(stderr, "Unable to read magic id in PPM file");
    fclose(fp);
    return 0;
  }

  // skip comments
  int c = getc(fp);
  while (c == '#') {
    while (c != '\n') c = getc(fp);
    c = getc(fp);
  }
  ungetc(c, fp);

  // Read width and height
  if (fscanf(fp, "%d%d", &width, &height) != 2) {
    fprintf(stderr, "Unable to read width and height in PPM file");
    fclose(fp);
    return 0;
  }

  npixels = width * height;
	
  // Read max value
  double max_value;
  if (fscanf(fp, "%lf", &max_value) != 1) {
    fprintf(stderr, "Unable to read max_value in PPM file");
    fclose(fp);
    return 0;
  }
	
  // Allocate image pixels
  pixels = new R2Pixel [ width * height ];
  if (!pixels) {
    fprintf(stderr, "Unable to allocate memory for PPM file");
    fclose(fp);
    return 0;
  }

  // Check if raw or ascii file
  if (!strcmp(buffer, "P6\n")) {
    // Read up to one character of whitespace (\n) after max_value
    int c = getc(fp);
    if (!isspace(c)) putc(c, fp);

    // Read raw image data 
    // First ppm pixel is top-left, so read in opposite scan-line order
    for (int j = height-1; j >= 0; j--) {
      for (int i = 0; i < width; i++) {
        double r = (double) getc(fp) / max_value;
        double g = (double) getc(fp) / max_value;
        double b = (double) getc(fp) / max_value;
        R2Pixel pixel(r, g, b, 1);
        SetPixel(i, j, pixel);
      }
    }
  }
  else {
    // Read asci image data 
    // First ppm pixel is top-left, so read in opposite scan-line order
    for (int j = height-1; j >= 0; j--) {
      for (int i = 0; i < width; i++) {
	// Read pixel values
	int red, green, blue;
	if (fscanf(fp, "%d%d%d", &red, &green, &blue) != 3) {
	  fprintf(stderr, "Unable to read data at (%d,%d) in PPM file", i, j);
	  fclose(fp);
	  return 0;
	}

	// Assign pixel values
	double r = (double) red / max_value;
	double g = (double) green / max_value;
	double b = (double) blue / max_value;
        R2Pixel pixel(r, g, b, 1);
        SetPixel(i, j, pixel);
      }
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R2Image::
WritePPM(const char *filename, int ascii) const
{
  // Check type
  if (ascii) {
    // Open file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
      fprintf(stderr, "Unable to open image file: %s\n", filename);
      return 0;
    }

    // Print PPM image file 
    // First ppm pixel is top-left, so write in opposite scan-line order
    fprintf(fp, "P3\n");
    fprintf(fp, "%d %d\n", width, height);
    fprintf(fp, "255\n");
    for (int j = height-1; j >= 0 ; j--) {
      for (int i = 0; i < width; i++) {
        const R2Pixel& p = (*this)[i][j];
        int r = (int) (255 * p.Red());
        int g = (int) (255 * p.Green());
        int b = (int) (255 * p.Blue());
        fprintf(fp, "%-3d %-3d %-3d  ", r, g, b);
        if (((i+1) % 4) == 0) fprintf(fp, "\n");
      }
      if ((width % 4) != 0) fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    // Close file
    fclose(fp);
  }
  else {
    // Open file
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
      fprintf(stderr, "Unable to open image file: %s\n", filename);
      return 0;
    }
    
    // Print PPM image file 
    // First ppm pixel is top-left, so write in opposite scan-line order
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", width, height);
    fprintf(fp, "255\n");
    for (int j = height-1; j >= 0 ; j--) {
      for (int i = 0; i < width; i++) {
        const R2Pixel& p = (*this)[i][j];
        int r = (int) (255 * p.Red());
        int g = (int) (255 * p.Green());
        int b = (int) (255 * p.Blue());
        fprintf(fp, "%c%c%c", r, g, b);
      }
    }
    
    // Close file
    fclose(fp);
  }

  // Return success
  return 1;  
}



////////////////////////////////////////////////////////////////////////
// JPEG I/O
////////////////////////////////////////////////////////////////////////


extern "C" { 
#   define XMD_H // Otherwise, a conflict with INT32
#   undef FAR // Otherwise, a conflict with windows.h
#   include "jpeg/jpeglib.h"
};



int R2Image::
ReadJPEG(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open image file: %s\n", filename);
    return 0;
  }

  // Initialize decompression info
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  // Remember image attributes
  width = cinfo.output_width;
  height = cinfo.output_height;
  npixels = width * height;
  int ncomponents = cinfo.output_components;

  // Allocate pixels for image
  pixels = new R2Pixel [ npixels ];
  if (!pixels) {
    fprintf(stderr, "Unable to allocate memory for BMP file");
    fclose(fp);
    return 0;
  }

  // Allocate unsigned char buffer for reading image
  int rowsize = ncomponents * width;
  if ((rowsize % 4) != 0) rowsize = (rowsize / 4 + 1) * 4;
  int nbytes = rowsize * height;
  unsigned char *buffer = new unsigned char [nbytes];
  if (!buffer) {
    fprintf(stderr, "Unable to allocate temporary memory for JPEG file");
    fclose(fp);
    return 0;
  }

  // Read scan lines 
  // First jpeg pixel is top-left, so read pixels in opposite scan-line order
  while (cinfo.output_scanline < cinfo.output_height) {
    int scanline = cinfo.output_height - cinfo.output_scanline - 1;
    unsigned char *row_pointer = &buffer[scanline * rowsize];
    jpeg_read_scanlines(&cinfo, &row_pointer, 1);
  }

  // Free everything
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  // Close file
  fclose(fp);

  // Assign pixels
  for (int j = 0; j < height; j++) {
    unsigned char *p = &buffer[j * rowsize];
    for (int i = 0; i < width; i++) {
      double r, g, b, a;
      if (ncomponents == 1) {
        r = g = b = (double) *(p++) / 255;
        a = 1;
      }
      else if (ncomponents == 1) {
        r = g = b = (double) *(p++) / 255;
        a = 1;
        p++;
      }
      else if (ncomponents == 3) {
        r = (double) *(p++) / 255;
        g = (double) *(p++) / 255;
        b = (double) *(p++) / 255;
        a = 1;
      }
      else if (ncomponents == 4) {
        r = (double) *(p++) / 255;
        g = (double) *(p++) / 255;
        b = (double) *(p++) / 255;
        a = (double) *(p++) / 255;
      }
      else {
        fprintf(stderr, "Unrecognized number of components in jpeg image: %d\n", ncomponents);
        return 0;
      }
      R2Pixel pixel(r, g, b, a);
      SetPixel(i, j, pixel);
    }
  }

  // Free unsigned char buffer for reading pixels
  delete [] buffer;

  // Return success
  return 1;
}


	

int R2Image::
WriteJPEG(const char *filename) const
{
  // Open file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Unable to open image file: %s\n", filename);
    return 0;
  }

  // Initialize compression info
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, fp);
  cinfo.image_width = width; 	/* image width and height, in pixels */
  cinfo.image_height = height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  cinfo.dct_method = JDCT_ISLOW;
  jpeg_set_defaults(&cinfo);
  cinfo.optimize_coding = TRUE;
  jpeg_set_quality(&cinfo, 100, TRUE);
  jpeg_start_compress(&cinfo, TRUE);
	
  // Allocate unsigned char buffer for reading image
  int rowsize = 3 * width;
  if ((rowsize % 4) != 0) rowsize = (rowsize / 4 + 1) * 4;
  int nbytes = rowsize * height;
  unsigned char *buffer = new unsigned char [nbytes];
  if (!buffer) {
    fprintf(stderr, "Unable to allocate temporary memory for JPEG file");
    fclose(fp);
    return 0;
  }

  // Fill buffer with pixels
  for (int j = 0; j < height; j++) {
    unsigned char *p = &buffer[j * rowsize];
    for (int i = 0; i < width; i++) {
      const R2Pixel& pixel = (*this)[i][j];
      int r = (int) (255 * pixel.Red());
      int g = (int) (255 * pixel.Green());
      int b = (int) (255 * pixel.Blue());
      if (r > 255) r = 255;
      if (g > 255) g = 255;
      if (b > 255) b = 255;
      *(p++) = r;
      *(p++) = g;
      *(p++) = b;
    }
  }



  // Output scan lines
  // First jpeg pixel is top-left, so write in opposite scan-line order
  while (cinfo.next_scanline < cinfo.image_height) {
    int scanline = cinfo.image_height - cinfo.next_scanline - 1;
    unsigned char *row_pointer = &buffer[scanline * rowsize];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }

  // Free everything
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  // Close file
  fclose(fp);

  // Free unsigned char buffer for reading pixels
  delete [] buffer;

  // Return number of bytes written
  return 1;
}






