// xBRZ image upscaler/filter - modified for the Kega Fusion Emulator

// Original xbrz.cpp :
// ****************************************************************************
// * This file is part of the HqMAME project. It is distributed under         *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html         *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved          *
// *                                                                          *
// * Additionally and as a special exception, the author gives permission     *
// * to link the code of this program with the MAME library (or with modified *
// * versions of MAME that use the same license as MAME), and distribute      *
// * linked combinations including the two. You must obey the GNU General     *
// * Public License in all respects for all of the code used other than MAME. *
// * If you modify this file, you may extend this exception to your version   *
// * of the file, but you are not obligated to do so. If you do not wish to   *
// * do so, delete this exception statement from your version.                *
// ****************************************************************************

// rpi-plugin creation and C++11 unraveling and some cleaning of the original code
// by "milo1012" (milo1012 AT freenet DOT de)
// no real code changes besides removing lambda functions and some preprocessor mess,
// removing <cstdint> (uint32_t) plus compressing whitespace (tabs instead space,
// bracket style)
// (do we really need lambda functions for this small code and break compatibility
// with the majority of the decent, well-tried, but older C++ compilers out there?
// - I don't think so - we can even get speedups (check the readme))
//
// other code changes:
// - aggressive speedup by removing unnecessary sub-routines for color
//   distance function and converting to float -> double precision is overkill
//   for an 8 bit resolution color model -> MSVC 7.1 et al. seem to favor it
//   -> same function: we probably don't want to calculate three subtractions
//   and two divisions with known constants in every call -> unfold/precalc it
// - removal of elaborate fillBlock() and sub functions -> in place seems faster
// - removal of std::max and std::min -> unnecessary: "std::max()" == "(a<b)?b:a"
//   -> "((x-1<0)?(0):(x-1))" can be changed to "x-((x>0)?(1):(0))"
//   -> definitely less code produced, theoret. faster
// - removal of ScalerCfg() and cfg vars -> we use xBRZ defaults anyway
// - removal of safety checks at scaleImage() start
// ... overall ~2-8 % faster (depending on machine)
//
// currently only for Windows
// for other systems: replace the stuff in DllMain and change or remove
// the threading in scale() and RenderPluginOutput() first

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#ifdef _WINDOWS
	#define WIN32_LEAN_AND_MEAN
	//#define NOMINMAX // if using std::max or std:.min
	#include <windows.h>
	//#undef NOMINMAX
#endif
#include <limits>
//#include <algorithm> // if using std::max or std:.min
#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#elif defined __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

//////////////
// main config: scaling factor (2,3,4) and threading
// don't use other values for _SCALER_ as above (5 not implemented)
const unsigned char _SCALER_ = 2;
// number of threads = image slices scaled parallel in xBRZ - src image size
// must be a multiple of it! - prob. no more gain above 4 slices
const unsigned char NUM_SLICE = 4;
#define _XBRZ_MT_
#undef _XBRZ_MT_
//////////////

////////////////////////////////////////////////////////////////////////////////

typedef union {
	unsigned int rgbint;
	uint8_t rgbarr[4];
} rgbpixel;

////////////////////////////////////////////////////////////////////////////////
// global vars
rgbpixel* picture_in32;
rgbpixel* picture_out32;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


namespace xbrz {
#ifdef _XBRZ_MT_
	DWORD WINAPI scale(LPVOID);
#else
	void scale();
#endif
} // namespace


namespace {

const unsigned int redMask   = 0xff0000;
const unsigned int greenMask = 0x00ff00;
const unsigned int blueMask  = 0x0000ff;
const unsigned int redblueMask = 0xff00ff;

enum RotationDegree { //clock-wise
	ROT_0,
	ROT_90,
	ROT_180,
	ROT_270
};

enum BlendType {
	BLEND_NONE = 0,
	BLEND_NORMAL,   //a normal indication to blend
	BLEND_DOMINANT, //a strong indication to blend
	//attention: BlendType must fit into the value range of 2 bit!!!
};

struct BlendResult {
	BlendType
	/**/blend_f, blend_g,
	/**/blend_j, blend_k;
};

struct Kernel_4x4 { //kernel for preprocessing step
	unsigned int
	/**/a, b, c, d,
	/**/e, f, g, h,
	/**/i, j, k, l,
	/**/m, n, o, p;
};

struct Kernel_3x3 {
	unsigned int
	/**/a,  b,  c,
	/**/d,  e,  f,
	/**/g,  h,  i;
};




template <unsigned int N, unsigned int M> inline
void alphaBlend(unsigned int& dst, unsigned int col) { //blend color over destination with opacity N / M
	dst = (redMask   & ((col & redMask  ) * N + (dst & redMask  ) * (M - N)) / M) | //this works because 8 upper bits are free
		(greenMask & ((col & greenMask) * N + (dst & greenMask) * (M - N)) / M) |
		(blueMask  & ((col & blueMask ) * N + (dst & blueMask ) * (M - N)) / M);
}
//n0p - doesn't work as expected :(
/*
void alphaBlend(unsigned int& dst, unsigned int col) {
	dst = (redblueMask   & ((col & redblueMask  ) * N  + (dst & redblueMask ) * (M - N)) / M) | 
		(greenMask & ((col & greenMask) * N + (dst & greenMask) * (M - N)) / M); }
*/
//calculate input matrix coordinates after rotation at compile time
template <RotationDegree rotDeg, size_t I, size_t J, size_t N>
struct MatrixRotation;

template <size_t I, size_t J, size_t N>
struct MatrixRotation<ROT_0, I, J, N> {
	static const size_t I_old = I;
	static const size_t J_old = J;
};


template <RotationDegree rotDeg, size_t I, size_t J, size_t N> //(i, j) = (row, col) indices, N = size of (square) matrix
struct MatrixRotation {
	static const size_t I_old = N - 1 - MatrixRotation<static_cast<RotationDegree>(rotDeg - 1), I, J, N>::J_old; //old coordinates before rotation!
	static const size_t J_old =         MatrixRotation<static_cast<RotationDegree>(rotDeg - 1), I, J, N>::I_old; //
};


template <size_t N, RotationDegree rotDeg>
class OutputMatrix {
	public:
		OutputMatrix(unsigned int* out, int outWidth) : //access matrix area, top-left at position "out" for image with given width
			out_(out),
			outWidth_(outWidth) {}
	template <size_t I, size_t J>
	unsigned int& ref() const {
		static const size_t I_old = MatrixRotation<rotDeg, I, J, N>::I_old;
		static const size_t J_old = MatrixRotation<rotDeg, I, J, N>::J_old;
		return *(out_ + J_old + I_old * outWidth_);
	}
	private:
		unsigned int* out_;
		const int outWidth_;
};

/*
FORCE_INLINE
float distYCbCr(const unsigned int& pix1, const unsigned int& pix2) {
	if (pix1 == pix2) //about 8% perf boost
		return 0;
	//http://en.wikipedia.org/wiki/YCbCr#ITU-R_BT.601_conversion
	//YCbCr conversion is a matrix multiplication => take advantage of linearity by subtracting first!	


	//int r_diff = *((unsigned char*)&pix1+2) - *((unsigned char*)&pix2+2); //we may delay division by 255 to after matrix multiplication
	//int g_diff = *((unsigned char*)&pix1+1) - *((unsigned char*)&pix2+1); //
	//int b_diff = *((unsigned char*)&pix1) - *((unsigned char*)&pix2); //substraction for int is noticeable faster than for double!

//n0p - faster code
//	int r_diff = (pix1>>16) - (pix2>>16); //we may delay division by 255 to after matrix multiplication
//	int g_diff = ((pix1>>8)&0xFF)-((pix2>>8)&0xFF); //
//	int b_diff = (pix1&0xFF)-(pix2&0xFF); //substraction for int is noticeable faster than for double!

//n0p - produces even smaller and faster code
	unsigned int tmp1 = pix1;
	unsigned int tmp2 = pix2;
	int b_diff = (tmp1&0xFF)-(tmp2&0xFF);
	tmp1>>=8; tmp2>>=8;
	int g_diff = (tmp1&0xFF)-(tmp2&0xFF);	
        tmp1>>=8; tmp2>>=8;
	int r_diff = tmp1 - tmp2;

	//ITU-R BT.709 conversion
	const float y   = 0.2126F * r_diff + 0.7152F * g_diff + 0.0722F * b_diff; //[!], analog YCbCr!
	const float c_b = (b_diff - y) * 0.5389F;
	const float c_r = (r_diff - y) * 0.635F;
	//we skip division by 255 to have similar range like other distance functions
	return sqrt(y*y + c_b*c_b +  c_r*c_r);
}

*/

FORCE_INLINE
float distYCbCr(const unsigned int& pix1, const unsigned int& pix2)
{
  if (pix1 == pix2) return 0;	
  unsigned int tmp1 = pix1;
  unsigned int tmp2 = pix2;
  int b_diff = (tmp1&0xFF)-(tmp2&0xFF);
  tmp1>>=8; tmp2>>=8; 
  int g_diff = (tmp1&0xFF)-(tmp2&0xFF);	
  tmp1>>=8; tmp2>>=8;
  int r_diff = tmp1 - tmp2;
  long rmean = ( (long)tmp1 + (long)tmp2 ) / 2;
  return sqrt((((512+rmean)*r_diff*r_diff)>>8) + ((g_diff*g_diff)<<4) + (((767-rmean)*b_diff*b_diff)>>8));
}


/*
input kernel area naming convention:
-----------------
| A | B | C | D |
----|---|---|---|
| E | F | G | H |   //evalute the four corners between F, G, J, K
----|---|---|---|   //input pixel is at position F
| I | J | K | L |
----|---|---|---|
| M | N | O | P |
-----------------
*/
FORCE_INLINE //detect blend direction
BlendResult preProcessCorners(const Kernel_4x4& ker) { //result: F, G, J, K corners of "GradientType"
	BlendResult result = {};
	if ((ker.f == ker.g && ker.j == ker.k) || (ker.f == ker.j && ker.g == ker.k))
		return result;
	//auto dist = [&](unsigned int col1, unsigned int col2) { return colorDist(col1, col2, cfg.luminanceWeight_); };
	//const int weight = 4;
	float jg = ((ker.i == ker.f)?(0):(distYCbCr(ker.i, ker.f))) + ((ker.f == ker.c)?(0):(distYCbCr(ker.f, ker.c))) + ((ker.n == ker.k)?(0):(distYCbCr(ker.n, ker.k))) + ((ker.k == ker.h)?(0):(distYCbCr(ker.k, ker.h))) + ((ker.j == ker.g)?(0):(4 * distYCbCr(ker.j, ker.g)));
	float fk = ((ker.e == ker.j)?(0):(distYCbCr(ker.e, ker.j))) + ((ker.j == ker.o)?(0):(distYCbCr(ker.j, ker.o))) + ((ker.b == ker.g)?(0):(distYCbCr(ker.b, ker.g))) + ((ker.g == ker.l)?(0):(distYCbCr(ker.g, ker.l))) + ((ker.f == ker.k)?(0):(4 * distYCbCr(ker.f, ker.k)));
	if (jg < fk) { //test sample: 70% of values max(jg, fk) / min(jg, fk) are between 1.1 and 3.7 with median being 1.8
		const bool dominantGradient = 3.6F * jg < fk;
		if (ker.f != ker.g && ker.f != ker.j)
			result.blend_f = dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL;
		if (ker.k != ker.j && ker.k != ker.g)
			result.blend_k = dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL;
	}
	else if (fk < jg) {
		const bool dominantGradient = 3.6F * fk < jg;
		if (ker.j != ker.f && ker.j != ker.k)
			result.blend_j = dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL;
		if (ker.g != ker.f && ker.g != ker.k)
			result.blend_g = dominantGradient ? BLEND_DOMINANT : BLEND_NORMAL;
	}
	return result;
}


//compress four blend types into a single byte
inline BlendType getTopL(unsigned char b) {
	return static_cast<BlendType>(0x3 & b);
}
inline BlendType getTopR(unsigned char b) {
	return static_cast<BlendType>(0x3 & (b >> 2));
}
inline BlendType getBottomR(unsigned char b) {
	return static_cast<BlendType>(0x3 & (b >> 4));
}
inline BlendType getBottomL(unsigned char b) {
	return static_cast<BlendType>(0x3 & (b >> 6));
}
inline void setTopL(unsigned char& b, BlendType bt) {
	b |= bt;
} //buffer is assumed to be initialized before preprocessing!
inline void setTopR(unsigned char& b, BlendType bt) {
	b |= (bt << 2);
}
inline void setBottomR(unsigned char& b, BlendType bt) {
	b |= (bt << 4);
}
inline void setBottomL(unsigned char& b, BlendType bt) {
	b |= (bt << 6);
}


template <RotationDegree rotDeg> inline
unsigned char rotateBlendInfo(unsigned char b) {
	return b;
}
template <> inline unsigned char rotateBlendInfo<ROT_90 >(unsigned char b) {
	return ((b << 2) | (b >> 6)) & 0xff;
}
template <> inline unsigned char rotateBlendInfo<ROT_180>(unsigned char b) {
	return ((b << 4) | (b >> 4)) & 0xff;
}
template <> inline unsigned char rotateBlendInfo<ROT_270>(unsigned char b) {
	return ((b << 6) | (b >> 2)) & 0xff;
}


template <RotationDegree rotDeg> unsigned int inline get_a(const Kernel_3x3& ker) { return ker.a; }
template <RotationDegree rotDeg> unsigned int inline get_b(const Kernel_3x3& ker) { return ker.b; }
template <RotationDegree rotDeg> unsigned int inline get_c(const Kernel_3x3& ker) { return ker.c; }
template <RotationDegree rotDeg> unsigned int inline get_d(const Kernel_3x3& ker) { return ker.d; }
template <RotationDegree rotDeg> unsigned int inline get_e(const Kernel_3x3& ker) { return ker.e; }
template <RotationDegree rotDeg> unsigned int inline get_f(const Kernel_3x3& ker) { return ker.f; }
template <RotationDegree rotDeg> unsigned int inline get_g(const Kernel_3x3& ker) { return ker.g; }
template <RotationDegree rotDeg> unsigned int inline get_h(const Kernel_3x3& ker) { return ker.h; }
template <RotationDegree rotDeg> unsigned int inline get_i(const Kernel_3x3& ker) { return ker.i; }

template <> inline unsigned int get_a<ROT_90>(const Kernel_3x3& ker) { return ker.g; }
template <> inline unsigned int get_b<ROT_90>(const Kernel_3x3& ker) { return ker.d; }
template <> inline unsigned int get_c<ROT_90>(const Kernel_3x3& ker) { return ker.a; }
template <> inline unsigned int get_d<ROT_90>(const Kernel_3x3& ker) { return ker.h; }
template <> inline unsigned int get_e<ROT_90>(const Kernel_3x3& ker) { return ker.e; }
template <> inline unsigned int get_f<ROT_90>(const Kernel_3x3& ker) { return ker.b; }
template <> inline unsigned int get_g<ROT_90>(const Kernel_3x3& ker) { return ker.i; }
template <> inline unsigned int get_h<ROT_90>(const Kernel_3x3& ker) { return ker.f; }
template <> inline unsigned int get_i<ROT_90>(const Kernel_3x3& ker) { return ker.c; }

template <> inline unsigned int get_a<ROT_180>(const Kernel_3x3& ker) { return ker.i; }
template <> inline unsigned int get_b<ROT_180>(const Kernel_3x3& ker) { return ker.h; }
template <> inline unsigned int get_c<ROT_180>(const Kernel_3x3& ker) { return ker.g; }
template <> inline unsigned int get_d<ROT_180>(const Kernel_3x3& ker) { return ker.f; }
template <> inline unsigned int get_e<ROT_180>(const Kernel_3x3& ker) { return ker.e; }
template <> inline unsigned int get_f<ROT_180>(const Kernel_3x3& ker) { return ker.d; }
template <> inline unsigned int get_g<ROT_180>(const Kernel_3x3& ker) { return ker.c; }
template <> inline unsigned int get_h<ROT_180>(const Kernel_3x3& ker) { return ker.b; }
template <> inline unsigned int get_i<ROT_180>(const Kernel_3x3& ker) { return ker.a; }

template <> inline unsigned int get_a<ROT_270>(const Kernel_3x3& ker) { return ker.c; }
template <> inline unsigned int get_b<ROT_270>(const Kernel_3x3& ker) { return ker.f; }
template <> inline unsigned int get_c<ROT_270>(const Kernel_3x3& ker) { return ker.i; }
template <> inline unsigned int get_d<ROT_270>(const Kernel_3x3& ker) { return ker.b; }
template <> inline unsigned int get_e<ROT_270>(const Kernel_3x3& ker) { return ker.e; }
template <> inline unsigned int get_f<ROT_270>(const Kernel_3x3& ker) { return ker.h; }
template <> inline unsigned int get_g<ROT_270>(const Kernel_3x3& ker) { return ker.a; }
template <> inline unsigned int get_h<ROT_270>(const Kernel_3x3& ker) { return ker.d; }
template <> inline unsigned int get_i<ROT_270>(const Kernel_3x3& ker) { return ker.g; }


/*
input kernel area naming convention:
-------------
| A | B | C |
----|---|---|
| D | E | F | //input pixel is at position E
----|---|---|
| G | H | I |
-------------
*/
template <class Scaler, RotationDegree rotDeg>
FORCE_INLINE //perf: quite worth it!
void scalePixel(const Kernel_3x3& ker,unsigned int* target, int trgWidth,
	unsigned char blendInfo //result of preprocessing all four corners of pixel "e"
	) {
	const unsigned char blend = rotateBlendInfo<rotDeg>(blendInfo);
	if (getBottomR(blend) >= BLEND_NORMAL) {
		unsigned int ee = get_e<rotDeg>(ker);
		unsigned int ff = get_f<rotDeg>(ker);
		unsigned int hh = get_h<rotDeg>(ker);
		unsigned int gg = get_g<rotDeg>(ker);
		unsigned int cc = get_c<rotDeg>(ker);
		unsigned int ii = get_i<rotDeg>(ker);
		//auto eq   = [&](unsigned int col1, unsigned int col2) { return colorDist(col1, col2, cfg.luminanceWeight_) < cfg.equalColorTolerance_; };
		//auto dist = [&](unsigned int col1, unsigned int col2) { return colorDist(col1, col2, cfg.luminanceWeight_); };
		bool doLineBlend = true;
		if (getBottomR(blend) >= BLEND_DOMINANT)
			doLineBlend = true;
		//make sure there is no second blending in an adjacent rotation for this pixel: handles insular pixels, mario eyes
		if (getTopR(blend) != BLEND_NONE && !((ee == gg)?(1):(distYCbCr(ee, gg) < 30))) //but support double-blending for 90° corners
			doLineBlend = false;
		if (getBottomL(blend) != BLEND_NONE && !((ee == cc)?(1):(distYCbCr(ee, cc) < 30)))
			doLineBlend = false;
		//no full blending for L-shapes; blend corner only (handles "mario mushroom eyes")
		if (((gg == hh)?(1):(distYCbCr(gg, hh) < 30)) && ((hh == ii)?(1):(distYCbCr(hh, ii) < 30)) && ((ii == ff)?(1):(distYCbCr(ii, ff) < 30)) && ((ff == cc)?(1):(distYCbCr(ff, cc) < 30)) && !((ee == ii)?(1):(distYCbCr(ee, ii) < 30)))
			doLineBlend = false;
		const unsigned int px = ((ee == ff)?(0):(distYCbCr(ee, ff))) <= ((ee == hh)?(0):(distYCbCr(ee, hh))) ? ff : hh; //choose most similar color
		OutputMatrix<Scaler::scale, rotDeg> out(target, trgWidth);
		if (doLineBlend) {
			const float fg = ((ff == gg)?(0):(distYCbCr(ff, gg))); //test sample: 70% of values max(fg, hc) / min(fg, hc) are between 1.1 and 3.7 with median being 1.9
			const float hc = ((hh == cc)?(0):(distYCbCr(hh, cc))); //
			const bool haveShallowLine = 2.2F * fg <= hc && ee != gg && get_d<rotDeg>(ker) != gg;
			const bool haveSteepLine   = 2.2F * hc <= fg && ee != cc && get_b<rotDeg>(ker) != cc;
			if (haveShallowLine) {
				if (haveSteepLine)
					Scaler::blendLineSteepAndShallow(px, out);
				else
					Scaler::blendLineShallow(px, out);
			}
			else {
				if (haveSteepLine)
					Scaler::blendLineSteep(px, out);
				else
					Scaler::blendLineDiagonal(px,out);
			}
		}
		else
			Scaler::blendCorner(px, out);
	}
}


template <class Scaler> //scaler policy: see "Scaler2x" reference implementation
void scaleImage(const unsigned int* src, unsigned int* trg, int srcWidth, int srcHeight, int yFirst, int yLast, int trgPitch) {
	/*yFirst = ((yFirst<0)?(0):(yFirst));
	yLast  = ((!(srcHeight<yLast))?(yLast):(srcHeight));
	if (yFirst >= yLast || srcWidth <= 0)
		return;*/
	const int trgWidth = srcWidth * Scaler::scale;
	//n0p - we get pitch in pixels.
	const int trgIWidth = trgPitch;
	const int trgIOffset = (trgIWidth - trgWidth) / 2;
	//fprintf (stderr,"%d %d\n",trgIWidth,trgIOffset);
	//"use" space at the end of the image as temporary buffer for "on the fly preprocessing": we even could use larger area of
	//"sizeof(uint32_t) * srcWidth * (yLast - yFirst)" bytes without risk of accidental overwriting before accessing
	const int bufferSize = srcWidth;

	//unsigned char* preProcBuffer = reinterpret_cast<unsigned char*>(trg + yLast * Scaler::scale * trgWidth) - bufferSize;
	unsigned char* preProcBuffer = reinterpret_cast<unsigned char*>(trg + yLast * Scaler::scale * trgIWidth + trgIOffset) - bufferSize;

	//std::fill(preProcBuffer, preProcBuffer + bufferSize, 0);
	//initialize preprocessing buffer for first row: detect upper left and right corner blending
	//this cannot be optimized for adjacent processing stripes; we must not allow for a memory race condition!
	if (yFirst > 0) {
		const int y = yFirst - 1;
		const unsigned int* s_m1 = src + srcWidth * ((y-1<0)?(0):(y-1));
		const unsigned int* s_0  = src + srcWidth * y; //center line
		const unsigned int* s_p1 = src + srcWidth * ((!(srcHeight-1<y+1))?(y+1):(srcHeight-1));
		const unsigned int* s_p2 = src + srcWidth * ((!(srcHeight-1<y+2))?(y+2):(srcHeight-1));
		for (int x = 0; x < srcWidth; ++x) {
			const int x_m1 = ((x-1<0)?(0):(x-1));
			const int x_p1 = ((!(srcWidth-1<x+1))?(x+1):(srcWidth-1));
			const int x_p2 = ((!(srcWidth-1<x+2))?(x+2):(srcWidth-1));
			Kernel_4x4 ker = {}; //perf: initialization is negligable
			ker.a = s_m1[x_m1]; //read sequentially from memory as far as possible
			ker.b = s_m1[x];
			ker.c = s_m1[x_p1];
			ker.d = s_m1[x_p2];
			ker.e = s_0[x_m1];
			ker.f = s_0[x];
			ker.g = s_0[x_p1];
			ker.h = s_0[x_p2];
			ker.i = s_p1[x_m1];
			ker.j = s_p1[x];
			ker.k = s_p1[x_p1];
			ker.l = s_p1[x_p2];
			ker.m = s_p2[x_m1];
			ker.n = s_p2[x];
			ker.o = s_p2[x_p1];
			ker.p = s_p2[x_p2];
			const BlendResult res = preProcessCorners(ker);
			/*
			preprocessing blend result:
			---------
			| F | G |   //evalute corner between F, G, J, K
			----|---|   //input pixel is at position F
			| J | K |
			---------
			*/
			setTopR(preProcBuffer[x], res.blend_j);
			if (x + 1 < srcWidth)
				setTopL(preProcBuffer[x + 1], res.blend_k);
		}
	}
	//------------------------------------------------------------------------------------
	unsigned int* tt; // new
	unsigned int ii, jj; // new
	for (int y = yFirst; y < yLast; ++y) {
		//n0p
		//unsigned int* out = trg + Scaler::scale * y * trgWidth; //consider MT "striped" access
		unsigned int* out = trg + trgIOffset + Scaler::scale * y * trgIWidth;

		const unsigned int* s_m1 = src + srcWidth * (y-((y>0)?(1):(0)));
		const unsigned int* s_0  = src + srcWidth * y; //center line
		const unsigned int* s_p1 = src + srcWidth * ((!(srcHeight-1<y+1))?(y+1):(srcHeight-1));
		const unsigned int* s_p2 = src + srcWidth * ((!(srcHeight-1<y+2))?(y+2):(srcHeight-1));
		unsigned char blend_xy1 = 0; //corner blending for current (x, y + 1) position
		for (int x = 0; x < srcWidth; ++x, out += Scaler::scale) {
			//all those bounds checks have only insignificant impact on performance!
			const int x_m1 = x-((x>0)?(1):(0)); //perf: prefer array indexing to additional pointers!
			const int x_p1 = ((!(x+1<srcWidth-1))?(srcWidth-1):(x+1));
			const int x_p2 = ((!(x+2<srcWidth-1))?(srcWidth-1):(x+2));
			//evaluate the four corners on bottom-right of current pixel
			unsigned char blend_xy = 0; { //for current (x, y) position
				Kernel_4x4 ker = {}; //perf: initialization is negligable
				ker.a = s_m1[x_m1]; //read sequentially from memory as far as possible
				ker.b = s_m1[x];
				ker.c = s_m1[x_p1];
				ker.d = s_m1[x_p2];
				ker.e = s_0[x_m1];
				ker.f = s_0[x];
				ker.g = s_0[x_p1];
				ker.h = s_0[x_p2];
				ker.i = s_p1[x_m1];
				ker.j = s_p1[x];
				ker.k = s_p1[x_p1];
				ker.l = s_p1[x_p2];
				ker.m = s_p2[x_m1];
				ker.n = s_p2[x];
				ker.o = s_p2[x_p1];
				ker.p = s_p2[x_p2];
				const BlendResult res = preProcessCorners(ker);
				/*
				preprocessing blend result:
				---------
				| F | G |   //evalute corner between F, G, J, K
				----|---|   //current input pixel is at position F
				| J | K |
				---------
				*/
				blend_xy = preProcBuffer[x];
				setBottomR(blend_xy, res.blend_f); //all four corners of (x, y) have been determined at this point due to processing sequence!
				setTopR(blend_xy1, res.blend_j); //set 2nd known corner for (x, y + 1)
				preProcBuffer[x] = blend_xy1; //store on current buffer position for use on next row
				blend_xy1 = 0;
				setTopL(blend_xy1, res.blend_k); //set 1st known corner for (x + 1, y + 1) and buffer for use on next column
				if (x + 1 < srcWidth) //set 3rd known corner for (x + 1, y)
					setBottomL(preProcBuffer[x + 1], res.blend_g);
			}
			//fill block of size scale * scale with the given color
			//fillBlock(out, trgWidth * sizeof(unsigned int), s_0[x], Scaler::scale); //place *after* preprocessing step, to not overwrite the results while processing the the last pixel!
			// new start
			for (ii = 0, tt = out; ii < Scaler::scale; ++ii, tt += trgIWidth)
				for (jj = 0; jj < Scaler::scale; ++jj)
					tt[jj] = s_0[x];
			// end new
			//blend four corners of current pixel
			if (blend_xy != 0) { //good 20% perf-improvement
				Kernel_3x3 ker = {}; //perf: initialization is negligable
				ker.a = s_m1[x_m1]; //read sequentially from memory as far as possible
				ker.b = s_m1[x];
				ker.c = s_m1[x_p1];
				ker.d = s_0[x_m1];
				ker.e = s_0[x];
				ker.f = s_0[x_p1];
				ker.g = s_p1[x_m1];
				ker.h = s_p1[x];
				ker.i = s_p1[x_p1];
				scalePixel<Scaler, ROT_0  >(ker, out, trgIWidth, blend_xy);
				scalePixel<Scaler, ROT_90 >(ker, out, trgIWidth, blend_xy);
				scalePixel<Scaler, ROT_180>(ker, out, trgIWidth, blend_xy);
				scalePixel<Scaler, ROT_270>(ker, out, trgIWidth, blend_xy);
			}
		}
	}
}


struct Scaler2x {
	static const int scale = 2;
	template <class OutputMatrix>
	static void blendLineShallow(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<scale - 1, 0>(), col);
		alphaBlend<3, 4>(out.template ref<scale - 1, 1>(), col);
	}
	template <class OutputMatrix>
	static void blendLineSteep(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<0, scale - 1>(), col);
		alphaBlend<3, 4>(out.template ref<1, scale - 1>(), col);
	}
	template <class OutputMatrix>
	static void blendLineSteepAndShallow(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<1, 0>(), col);
		alphaBlend<1, 4>(out.template ref<0, 1>(), col);
		alphaBlend<5, 6>(out.template ref<1, 1>(), col); //[!] fixes 7/8 used in xBR
	}
	template <class OutputMatrix>
	static void blendLineDiagonal(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 2>(out.template ref<1, 1>(), col);
	}
	template <class OutputMatrix>
	static void blendCorner(unsigned int col, OutputMatrix& out) {
		//model a round corner
		alphaBlend<21, 100>(out.template ref<1, 1>(), col); //exact: 1 - pi/4 = 0.2146018366
	}
};


struct Scaler3x {
	static const int scale = 3;
	template <class OutputMatrix>
	static void blendLineShallow(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<scale - 1, 0>(), col);
		alphaBlend<1, 4>(out.template ref<scale - 2, 2>(), col);
		alphaBlend<3, 4>(out.template ref<scale - 1, 1>(), col);
		out.template ref<scale - 1, 2>() = col;
	}
	template <class OutputMatrix>
	static void blendLineSteep(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<0, scale - 1>(), col);
		alphaBlend<1, 4>(out.template ref<2, scale - 2>(), col);
		alphaBlend<3, 4>(out.template ref<1, scale - 1>(), col);
		out.template ref<2, scale - 1>() = col;
	}
	template <class OutputMatrix>
	static void blendLineSteepAndShallow(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<2, 0>(), col);
		alphaBlend<1, 4>(out.template ref<0, 2>(), col);
		alphaBlend<3, 4>(out.template ref<2, 1>(), col);
		alphaBlend<3, 4>(out.template ref<1, 2>(), col);
		out.template ref<2, 2>() = col;
	}
	template <class OutputMatrix>
	static void blendLineDiagonal(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 8>(out.template ref<1, 2>(), col);
		alphaBlend<1, 8>(out.template ref<2, 1>(), col);
		alphaBlend<7, 8>(out.template ref<2, 2>(), col);
	}
	template <class OutputMatrix>
	static void blendCorner(unsigned int col, OutputMatrix& out) {
		//model a round corner
		alphaBlend<45, 100>(out.template ref<2, 2>(), col); //exact: 0.4545939598
		//alphaBlend<14, 1000>(out.template ref<2, 1>(), col); //0.01413008627 -> negligable
		//alphaBlend<14, 1000>(out.template ref<1, 2>(), col); //0.01413008627
	}
};


struct Scaler4x {
	static const int scale = 4;
	template <class OutputMatrix>
	static void blendLineShallow(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<scale - 1, 0>(), col);
		alphaBlend<1, 4>(out.template ref<scale - 2, 2>(), col);
		alphaBlend<3, 4>(out.template ref<scale - 1, 1>(), col);
		alphaBlend<3, 4>(out.template ref<scale - 2, 3>(), col);
		out.template ref<scale - 1, 2>() = col;
		out.template ref<scale - 1, 3>() = col;
	}
	template <class OutputMatrix>
	static void blendLineSteep(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 4>(out.template ref<0, scale - 1>(), col);
		alphaBlend<1, 4>(out.template ref<2, scale - 2>(), col);
		alphaBlend<3, 4>(out.template ref<1, scale - 1>(), col);
		alphaBlend<3, 4>(out.template ref<3, scale - 2>(), col);
		out.template ref<2, scale - 1>() = col;
		out.template ref<3, scale - 1>() = col;
	}
	template <class OutputMatrix>
	static void blendLineSteepAndShallow(unsigned int col, OutputMatrix& out) {
		alphaBlend<3, 4>(out.template ref<3, 1>(), col);
		alphaBlend<3, 4>(out.template ref<1, 3>(), col);
		alphaBlend<1, 4>(out.template ref<3, 0>(), col);
		alphaBlend<1, 4>(out.template ref<0, 3>(), col);
		alphaBlend<1, 3>(out.template ref<2, 2>(), col); //[!] fixes 1/4 used in xBR
		out.template ref<3, 3>() = out.template ref<3, 2>() = out.template ref<2, 3>() = col;
	}
	template <class OutputMatrix>
	static void blendLineDiagonal(unsigned int col, OutputMatrix& out) {
		alphaBlend<1, 2>(out.template ref<scale - 1, scale / 2    >(), col);
		alphaBlend<1, 2>(out.template ref<scale - 2, scale / 2 + 1>(), col);
		out.template ref<scale - 1, scale - 1>() = col;
	}
	template <class OutputMatrix>
	static void blendCorner(unsigned int col, OutputMatrix& out) {
		//model a round corner
		alphaBlend<68, 100>(out.template ref<3, 3>(), col); //exact: 0.6848532563
		alphaBlend< 9, 100>(out.template ref<3, 2>(), col); //0.08677704501
		alphaBlend< 9, 100>(out.template ref<2, 3>(), col); //0.08677704501
	}
};


} // namespace

#if 0
////////////////////////////////////////////////////////////////////////////////
// Main caller
#ifdef _XBRZ_MT_
	DWORD WINAPI xbrz::scale(LPVOID lpParam) {
		unsigned char slice = *(unsigned char*)lpParam;
		if(_SCALER_ == 2)
			scaleImage<Scaler2x>((unsigned int*)picture_in32, (unsigned int*)picture_out32, MyRPO->SrcW, MyRPO->SrcH, (MyRPO->SrcH/NUM_SLICE)*slice, (MyRPO->SrcH/NUM_SLICE)*(slice+1));
		else if(_SCALER_ == 3)
			scaleImage<Scaler3x>((unsigned int*)picture_in32, (unsigned int*)picture_out32, MyRPO->SrcW, MyRPO->SrcH, (MyRPO->SrcH/NUM_SLICE)*slice, (MyRPO->SrcH/NUM_SLICE)*(slice+1));
		else // 4 or sth. else
			scaleImage<Scaler4x>((unsigned int*)picture_in32, (unsigned int*)picture_out32, MyRPO->SrcW, MyRPO->SrcH, (MyRPO->SrcH/NUM_SLICE)*slice, (MyRPO->SrcH/NUM_SLICE)*(slice+1));
		WORD* dst = (WORD*)MyRPO->DstPtr;
		unsigned int ccnt;
		ccnt=((MyRPO->DstH*MyRPO->DstW)/NUM_SLICE)*slice;
		dst+=(MyRPO->DstH/NUM_SLICE)*slice*pitchd;
		unsigned int i, j;
		// complete vmode unravel, since otherwise we'd need to check it in every line or pixel -> performance
		if(VideoFormat) {
			for(i=0; i<(MyRPO->DstH/NUM_SLICE); i++) {
				for(j=0; j<MyRPO->DstW; j++) {
					picture_out32[ccnt].rgbarr[2] = picture_out32[ccnt].rgbarr[2] >> 3;
					picture_out32[ccnt].rgbarr[1] = picture_out32[ccnt].rgbarr[1] >> 3;
					picture_out32[ccnt].rgbarr[0] = picture_out32[ccnt].rgbarr[0] >> 3;
					dst[j]=(picture_out32[ccnt].rgbarr[2] << 10) | (picture_out32[ccnt].rgbarr[1] << 5) | picture_out32[ccnt].rgbarr[0];
					ccnt++;
				}
				dst+=pitchd;
			}
		}
		else {
			for(i=0; i<(MyRPO->DstH/NUM_SLICE); i++) {
				for(j=0; j<MyRPO->DstW; j++) {
					picture_out32[ccnt].rgbarr[2] = picture_out32[ccnt].rgbarr[2] >> 3;
					picture_out32[ccnt].rgbarr[1] = picture_out32[ccnt].rgbarr[1] >> 2;
					picture_out32[ccnt].rgbarr[0] = picture_out32[ccnt].rgbarr[0] >> 3;
					dst[j]=(picture_out32[ccnt].rgbarr[2] << 11) | (picture_out32[ccnt].rgbarr[1] << 5) | picture_out32[ccnt].rgbarr[0];
					ccnt++;
				}
				dst+=pitchd;
			}
		}
		return 0;
	}
#else
	void xbrz::scale() {
		if(_SCALER_ == 2)
			scaleImage<Scaler2x>((unsigned int*)picture_in32, (unsigned int*)picture_out32, 320, 240, 0, MyRPO->SrcH);
		else if(_SCALER_ == 3)
			scaleImage<Scaler3x>((unsigned int*)picture_in32, (unsigned int*)picture_out32, 320, MyRPO->SrcH, 0, MyRPO->SrcH);
		else // 4 or sth. else
			scaleImage<Scaler4x>((unsigned int*)picture_in32, (unsigned int*)picture_out32, 320, MyRPO->SrcH, 0, MyRPO->SrcH);
	}
#endif
#endif

rgbpixel* g_input;
rgbpixel* g_output;
int       g_pitch;


//Single-threaded
extern "C" void xBRZScale_2X (rgbpixel* input, rgbpixel* output, int pitch) {
           scaleImage<Scaler2x>((unsigned int*)input, (unsigned int*)output, 320, 224, 0, 224, pitch);
}

extern "C" void xBRZScale_3X (rgbpixel* input, rgbpixel* output, int pitch) {
           scaleImage<Scaler3x>((unsigned int*)input, (unsigned int*)output, 320, 224, 0, 224, pitch);
}

extern "C" void xBRZScale_4X (rgbpixel* input, rgbpixel* output, int pitch) {
           scaleImage<Scaler4x>((unsigned int*)input, (unsigned int*)output, 320, 224, 0, 224, pitch);
}


//Double-threaded
void *SliceOne2X(void *vargp) {
        scaleImage<Scaler2x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 0, 224/2, g_pitch);
}

extern "C" void xBRZScale_2X_MT (rgbpixel* input, rgbpixel* output, int pitch) {
pthread_t tid;
	   g_input = input;
	   g_output = output;
           g_pitch = pitch;
           pthread_create(&tid, NULL, SliceOne2X, NULL);
           scaleImage<Scaler2x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 224/2, 224, g_pitch);
    	   pthread_join(tid, NULL);
}

void *SliceOne3X(void *vargp) {
        scaleImage<Scaler3x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 0, 224/2, g_pitch);
}

extern "C" void xBRZScale_3X_MT (rgbpixel* input, rgbpixel* output, int pitch) {
pthread_t tid;
	   g_input = input;
	   g_output = output;
           g_pitch = pitch;
           pthread_create(&tid, NULL, SliceOne3X, NULL);
           scaleImage<Scaler3x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 224/2, 224, g_pitch);
    	   pthread_join(tid, NULL);
}

void *SliceOne4X(void *vargp) {
        scaleImage<Scaler4x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 0, 224/2, g_pitch);
}

extern "C" void xBRZScale_4X_MT (rgbpixel* input, rgbpixel* output, int pitch) {
pthread_t tid;
	   g_input = input;
	   g_output = output;
           g_pitch = pitch;
           pthread_create(&tid, NULL, SliceOne4X, NULL);
           scaleImage<Scaler4x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 224/2, 224, g_pitch);
    	   pthread_join(tid, NULL);
}


//------------------------------------------------------Quad

void *SliceOne2X2(void *vargp) {
        scaleImage<Scaler2x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 0, 56, g_pitch);
}

void *SliceTwo2X2(void *vargp) {
        scaleImage<Scaler2x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 56, 112, g_pitch);
}

void *SliceThree2X2(void *vargp) {
        scaleImage<Scaler2x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 112, 168, g_pitch);
}

extern "C" void xBRZScale_2X_MT2 (rgbpixel* input, rgbpixel* output, int pitch) {
pthread_t tid1;
pthread_t tid2;
pthread_t tid3;
	   g_input = input;
	   g_output = output;
           g_pitch = pitch;
           pthread_create(&tid1, NULL, SliceOne2X2, NULL);
           pthread_create(&tid2, NULL, SliceTwo2X2, NULL);
           pthread_create(&tid3, NULL, SliceThree2X2, NULL);
           scaleImage<Scaler2x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 168, 224, g_pitch);
    	   pthread_join(tid1, NULL); pthread_join(tid2, NULL); pthread_join(tid3, NULL);
}

void *SliceOne3X2(void *vargp) {
        scaleImage<Scaler3x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 0, 56, g_pitch);
}

void *SliceTwo3X2(void *vargp) {
        scaleImage<Scaler3x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 56, 112, g_pitch);
}

void *SliceThree3X2(void *vargp) {
        scaleImage<Scaler3x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 112, 168, g_pitch);
}

extern "C" void xBRZScale_3X_MT2 (rgbpixel* input, rgbpixel* output, int pitch) {
pthread_t tid1;
pthread_t tid2;
pthread_t tid3;
	   g_input = input;
	   g_output = output;
           g_pitch = pitch;
           pthread_create(&tid1, NULL, SliceOne3X2, NULL);
           pthread_create(&tid2, NULL, SliceTwo3X2, NULL);
           pthread_create(&tid3, NULL, SliceThree3X2, NULL);
           scaleImage<Scaler3x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 168, 224, g_pitch);
    	   pthread_join(tid1, NULL); pthread_join(tid2, NULL); pthread_join(tid3, NULL);
}
void *SliceOne4X2(void *vargp) {
        scaleImage<Scaler4x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 0, 56, g_pitch);
}

void *SliceTwo4X2(void *vargp) {
        scaleImage<Scaler4x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 56, 112, g_pitch);
}

void *SliceThree4X2(void *vargp) {
        scaleImage<Scaler4x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 112, 168, g_pitch);
}

extern "C" void xBRZScale_4X_MT2 (rgbpixel* input, rgbpixel* output, int pitch) {
pthread_t tid1;
pthread_t tid2;
pthread_t tid3;
	   g_input = input;
	   g_output = output;
           g_pitch = pitch;
           pthread_create(&tid1, NULL, SliceOne4X2, NULL);
           pthread_create(&tid2, NULL, SliceTwo4X2, NULL);
           pthread_create(&tid3, NULL, SliceThree4X2, NULL);
           scaleImage<Scaler4x>((unsigned int*)g_input, (unsigned int*)g_output, 320, 224, 168, 224, g_pitch);
    	   pthread_join(tid1, NULL); pthread_join(tid2, NULL); pthread_join(tid3, NULL);
}

extern "C" void xBRZScale_Init() {
//stub
}