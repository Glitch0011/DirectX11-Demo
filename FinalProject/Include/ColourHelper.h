#include <DirectXMath.h>

using namespace DirectX;

//From http://www.geekymonkey.com/Programming/CSharp/RGB2HSL_HSL2RGB.htm
class ColorRGB
{
public:
	char R;
	char G;
	char B;

	ColorRGB()
	{
		this->R = 0;
		this->G = 0;
		this->B = 0;
	}

	ColorRGB(XMFLOAT3 value)
	{
		this->R = value.x;
		this->G = value.y;
		this->B = value.z;
	}
};

ColorRGB HSL2RGB(double h, double sl, double l)
{
	double v;
	double r, g, b;

	r = l;   // default to gray
	g = l;
	b = l;
	v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
	if (v > 0)
	{
		double m;
		double sv;
		int sextant;
		double fract, vsf, mid1, mid2;

		m = l + l - v;
		sv = (v - m) / v;
		h *= 6.0;

		sextant = (int)h;
		fract = h - sextant;
		vsf = v * sv * fract;
		mid1 = m + vsf;
		mid2 = v - vsf;

		switch (sextant)
		{
		case 0:
			r = v;
			g = mid1;
			b = m;
			break;
		case 1:
			r = mid2;
			g = v;
			b = m;
			break;
		case 2:
			r = m;
			g = v;
			b = mid1;
			break;
		case 3:
			r = m;
			g = mid2;
			b = v;
			break;
		case 4:
			r = mid1;
			g = m;
			b = v;
			break;
		case 5:
			r = v;
			g = m;
			b = mid2;
			break;
		}
	}

	ColorRGB rgb;

	rgb.R = r;// *255.0f;
	rgb.G = g;// *255.0f;
	rgb.B = b;// *255.0f;

	return rgb;
}