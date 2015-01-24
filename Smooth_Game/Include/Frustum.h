#pragma once

#include <DirectXMath.h>
#include <exception>

using namespace DirectX;

namespace SmoothGame
{
	class Plane
	{
	public:
		XMVECTOR normal, point;
		float d;

		Plane()
		{

		}

		void Plane::set3Points(XMVECTOR& v1, XMVECTOR& v2, XMVECTOR v3)
		{
			XMVECTOR aux1, aux2;

			aux1 = v1 - v2;
			aux2 = v3 - v2;

			normal = XMVector3Cross(aux2, aux1);
			normal = XMVector3Normalize(normal);

			point = v2;

			XMStoreFloat(&d, XMVector3Dot(normal, point));
			d *= -1;
		}

		float Plane::distance(XMVECTOR& p)
		{
			float a;
			XMStoreFloat(&a, XMVector3Dot(normal, p));
			return d + a;
		}
	};

	class Frustum
	{
		enum
		{
			TOP = 0,
			BOTTOM,
			LEFT,
			RIGHT,
			NEARP,
			FARP
		};

		static enum { OUTSIDE, INTERSECT, INSIDE };

		Plane pl[6];

		XMVECTOR ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
		float nearD, farD, ratio, angle, tang;
		float nw, nh, fw, fh;

	public:

		Frustum()
		{

		}

		void setCamInternals(float angle, float ratio, float nearD, float farD)
		{
			this->ratio = ratio;
			this->angle = angle;
			this->nearD = nearD;
			this->farD = farD;

			tang = (float)tan(angle * 0.5);
			nh = nearD * tang;
			nw = nh * ratio;
			fh = farD  * tang;
			fw = fh * ratio;
		}


		void setCamDef(XMVECTOR& p, XMVECTOR& l, XMVECTOR& u)
		{
			XMVECTOR dir, nc, fc, X, Y, Z;

			Z = p - l;
			Z = XMVector3Normalize(Z);

			X = XMVector3Cross(u, Z);
			X = XMVector3Normalize(X);

			Y = XMVector3Cross(Z, X);

			nc = p - Z * nearD;
			fc = p - Z * farD;

			ntl = nc + Y * nh - X * nw;
			ntr = nc + Y * nh + X * nw;
			nbl = nc - Y * nh - X * nw;
			nbr = nc - Y * nh + X * nw;

			ftl = fc + Y * fh - X * fw;
			ftr = fc + Y * fh + X * fw;
			fbl = fc - Y * fh - X * fw;
			fbr = fc - Y * fh + X * fw;

			pl[TOP].set3Points(ntr, ntl, ftl);
			pl[BOTTOM].set3Points(nbl, nbr, fbr);
			pl[LEFT].set3Points(ntl, nbl, fbl);
			pl[RIGHT].set3Points(nbr, ntr, fbr);
			pl[NEARP].set3Points(ntl, ntr, nbr);
			pl[FARP].set3Points(ftr, ftl, fbl);
		}


		int pointInFrustum(XMVECTOR& p)
		{
			int result = INSIDE;

			for (int i = 0; i < 6; i++)
				if (pl[i].distance(p) < 0)
					return OUTSIDE;

			return(result);

		}

		int sphereInFrustum(XMVECTOR& pos, float radius)
		{
			int result = INSIDE;

			float distance;
			for (int i = 0; i < 6; i++)
			{
				distance = pl[i].distance(pos);

				if (distance < -radius)
					return OUTSIDE;
				else if (distance < radius)
					result = INTERSECT;
			}
			return(result);
		}
	};
}