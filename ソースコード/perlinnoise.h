#pragma once
#include "main.h"
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <cmath>

class PerlinNoise
{
private:
	vector<unsigned int> p;

	static double fade(double t)
	{
		return t * t * (t * (t * 6.f - 15.f) + 10.f);
	}

	static double lerp(double t, double a, double b)
	{
		return a + t * (b - a);
	}

	static double grad(int hash, double x, double y, double z = 0.f)
	{
		int h = hash & 15;
		double u = h < 8 ? x : y;
		double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
		return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
	}

public:
    PerlinNoise(const unsigned int seed = 2025)
	{
		p.resize(256);
		iota(p.begin(), p.end(), 0u);

		mt19937 engine(seed);
		shuffle(p.begin(), p.end(), engine);

		p.insert(p.end(), p.begin(), p.end());
	}

    double noise2D(double x, double y, double noiseScale = 1.0) const
    {
        double fx = floor(x);
        double fy = floor(y);
        int X = (int)fx & 255;
        int Y = (int)fy & 255;

        x -= fx;
        y -= fy;

        double u = fade(x);
        double v = fade(y);

        int A = (p[X] + Y) & 255;
        int B = (p[X + 1] + Y) & 255;

        double res = lerp(v,
            lerp(u, grad(p[A], x, y),
                grad(p[B], x - 1, y)),
            lerp(u, grad(p[(A + 1) & 255], x, y - 1),
                grad(p[(B + 1) & 255], x - 1, y - 1))
        );

        return res * noiseScale;
    }

    double Noise(double x, double y, double z, double noiseScale = 1.0) const
    {
        double fx = floor(x);
        double fy = floor(y);
        double fz = floor(z);
        int X = (int)fx & 255;
        int Y = (int)fy & 255;
        int Z = (int)fz & 255;

        x -= fx;
        y -= fy;
        z -= fz;

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int A = (p[X] + Y) & 255;
        int B = (p[X + 1] + Y) & 255;
        int AA = (p[A] + Z) & 255;
        int AB = (p[A + 1] + Z) & 255;
        int BA = (p[B] + Z) & 255;
        int BB = (p[B + 1] + Z) & 255;

        double res = lerp(w,
            lerp(v,
                lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))
            ),
            lerp(v,
                lerp(u, grad(p[(AA + 1) & 255], x, y, z - 1), grad(p[(BA + 1) & 255], x - 1, y, z - 1)),
                lerp(u, grad(p[(AB + 1) & 255], x, y - 1, z - 1), grad(p[(BB + 1) & 255], x - 1, y - 1, z - 1))
            )
        );

        return res * noiseScale;
    }

    double octaveNoise2D(double x, double y, int octaves = 4, double persistence = 0.5f) const
    {
        double total = 0.0;
        double maxValue = 0.0;
        double amplitude = 1.0;
        double frequency = 1.0;

        for (int i = 0; i < octaves; i++)
        {
            // 入力値を安全にラップ
            double nx = fmod(x * frequency, 256.0);
            double ny = fmod(y * frequency, 256.0);

            double val = noise2D(nx, ny) * amplitude;
            if (!isfinite(val)) val = 0.0;
            total += val;
            maxValue += amplitude;

            amplitude *= persistence;
            frequency *= 2.0;
        }

        return total / maxValue;
    }

};
