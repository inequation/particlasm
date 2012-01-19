/*
particlasm reference C++ implementation
Copyright (C) 2011-2012, Leszek Godlewski <lg@inequation.org>
*/

#include "libparticlasm.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>

// forward declarations
static float FRand();	// uniform distribution, range [0; 1]
static bool SpawnParticle(ptcEmitter *e);
static float GetScalar(ptcScalarDistr *d, float t);
static void GetVector(ptcVectorDistr *d, float t, ptcVector out);
static void GetColour(ptcColourDistr *d, uint32_t flags, float t, ptcColour out);

#ifdef __cplusplus
extern "C"
#endif
uint32_t ref_ptcCompileEmitter(ptcEmitter *emitter) {
	emitter->NumParticles = 0;
	memset(emitter->ParticleBuf, 0, sizeof(ptcParticle) * emitter->MaxParticles);
	return 1;
}

#define MIN_SUBSTEP		0.025f
static inline bool ProcessParticle(ptcEmitter *emitter, ptcParticle *p,
	float step, ptcVector cameraCS[3], ptcVertex *buffer) {
	float s = step;
	do {
		step = std::min(s, MIN_SUBSTEP);
		p->Time += step * p->TimeScale;
			// we've had our time, kill the particle
		if (p->Time > 1.f) {
			p->Active = 0;
			--emitter->NumParticles;
			return false;
		}
		for (ptcModule *m = emitter->Head; m; m = m->Header.Next) {
			switch (m->Header.ModuleID) {
				case ptcMID_Velocity:
					GetVector(&m->Vel.Distr, p->Time, p->Velocity);
					break;
				case ptcMID_Acceleration:
					GetVector(&m->Accel.Distr, p->Time, p->Accel);
					break;
				case ptcMID_Colour:
					GetColour(&m->Col.Distr, m->Col.Flags, p->Time, p->Colour);
					break;
				case ptcMID_Size:
					p->Size = GetScalar(&m->Size.Distr, p->Time);
					break;
				case ptcMID_Gravity:
					{
						ptcVector centre;
						GetVector(&m->Gravity.Centre, p->Time, centre);
						ptcVector diff = {
							centre[0] - p->Location[0],
							centre[1] - p->Location[1],
							centre[2] - p->Location[2]
						};
						const float r2 = diff[0] * diff[0]
										+ diff[1] * diff[1]
										+ diff[2] * diff[2];
						if (r2 < 1e-6)
							break;
						// squared source radius
						const float Rs2 = m->Gravity.Radius * m->Gravity.Radius;
						// 1/r^2
						const float invr2 = 1.f / r2;
						// 1/r
						const float invr = sqrtf(invr2);
						float F;
						if (r2 < Rs2) {
							// inside radius -> linear falloff
							const float Fmax = 6.673e-11 * m->Gravity.SourceMass
								/ ((m->Gravity.Flags & ptcGF_LinearAtt)
										? m->Gravity.Radius : Rs2);
							F = Fmax * sqrtf(r2 / Rs2);
						} else {
							// outside radius -> F = GMm/r^2
							F = 6.673e-11 * m->Gravity.SourceMass
								* ((m->Gravity.Flags & ptcGF_LinearAtt) ? invr : invr2);
						}
						if (m->Gravity.Flags & ptcGF_AxisX)
							p->Accel[0] = diff[0] * invr * F;
						if (m->Gravity.Flags & ptcGF_AxisY)
							p->Accel[1] = diff[1] * invr * F;
						if (m->Gravity.Flags & ptcGF_AxisZ)
							p->Accel[2] = diff[2] * invr * F;
					}
					break;
				default:
					break;
			}
		}
		for (int j = 0; j < 3; ++j)
			p->Velocity[j] += p->Accel[j] * step;
		for (int j = 0; j < 3; ++j)
			p->Location[j] += p->Velocity[j] * step;
		// TODO: rotation
		if (buffer) {
			for (int j = 0; j < 4; ++j) {
				for (int k = 0; k < 4; ++k)
					buffer[j].Colour[k] = p->Colour[k];
				for (int k = 0; k < 3; ++k)
					buffer[j].Location[k] = p->Location[k];
				for (int k = 0; k < 3; ++k) {
					buffer[j].Location[k] += cameraCS[1][k] * p->Size
							* ((j == 0 || j == 3) ? 0.5 : -0.5);
				}
				for (int k = 0; k < 3; ++k) {
					buffer[j].Location[k] += cameraCS[2][k] * p->Size
							* ((j < 2) ? 0.5 : -0.5);
				}
				buffer[j].TexCoords[0] = (j == 0 || j == 3) ? 1 : 0;
				buffer[j].TexCoords[1] = (j < 2) ? 0 : 1;
			}
		}
		s -= MIN_SUBSTEP;
	} while (s > 0.f);
	return true;
}

#ifdef __cplusplus
extern "C"
#endif
uint32_t ref_ptcProcessEmitter(ptcEmitter *emitter, float step,
		ptcVector cameraCS[3], ptcVertex *buffer, uint32_t maxVertices) {
	// particle spawning
	emitter->SpawnTimer += step;
	size_t count = (size_t)floorf(emitter->SpawnTimer * emitter->SpawnRate);
	emitter->SpawnTimer -= (float)count / emitter->SpawnRate;
	count *= emitter->BurstCount;
	if (count > emitter->MaxParticles - emitter->NumParticles)
		count = emitter->MaxParticles - emitter->NumParticles;
	for (size_t i = 0; i < count; ++i)
		SpawnParticle(emitter);

	// particle advancing
	uint32_t verts = 0;
	ptcParticle *p;
	for (size_t i = 0; i < emitter->MaxParticles
		&& verts + 4 < maxVertices; ++i) {
		if (!emitter->ParticleBuf[i].Active)
			continue;
		p = &emitter->ParticleBuf[i];
		if (ProcessParticle(emitter, p, step, cameraCS, buffer + verts))
			verts += 4;
	}
	return verts;
}

#ifdef __cplusplus
extern "C"
#endif
void ref_ptcReleaseEmitter(ptcEmitter *emitter) {
}

static float FRand() {
	return (float)rand() / (float)RAND_MAX;
}

static bool SpawnParticle(ptcEmitter *e) {
	if (e->NumParticles + 1 >= e->MaxParticles)
		// no room to spawn particle
		return false;
	// start looking for a free spot at a random index
	// this improves distribution -> reduces collisions
	size_t i = 0;//rand() % e->MaxParticles;
	while (e->ParticleBuf[i].Active)
		i = (i + 1) % e->MaxParticles;
	++e->NumParticles;
	ptcParticle *p = &e->ParticleBuf[i];
	p->Active = 1;
	p->TimeScale = 1.f / (e->LifeTimeFixed + FRand() * e->LifeTimeRandom);
	p->Time = 0.f;
	p->Colour[0] = p->Colour[1] = p->Colour[2] = p->Colour[3] = 1.f;
	p->Location[0] = p->Location[1] = p->Location[2] = 0.f;
	p->Rotation = 0.f;
	p->Size = 1.f;
	p->Velocity[0] = p->Velocity[1] = p->Velocity[2] = 0.f;
	p->Accel[0] = p->Accel[1] = p->Accel[2] = 0.f;
	for (ptcModule *m = e->Head; m; m = m->Header.Next) {
		switch (m->Header.ModuleID) {
			case ptcMID_InitialLocation:
				GetVector(&m->InitLoc.Distr, 0.f, p->Location);
				break;
			case ptcMID_InitialRotation:
				p->Rotation = GetScalar(&m->InitRot.Distr, 0.f);
				break;
			case ptcMID_InitialSize:
				p->Size = GetScalar(&m->InitRot.Distr, 0.f);
				break;
			case ptcMID_InitialVelocity:
				GetVector(&m->InitVel.Distr, 0.f, p->Velocity);
				break;
			case ptcMID_InitialColour:
				GetColour(&m->InitCol.Distr, ptcCF_SetAlpha | ptcCF_SetRGB,
					0.f, p->Colour);
				break;
			default:
				break;
		}
	}
	return true;
}

static inline float GetBicubicInterp(float t, float targVal) {
	// FIXME
	return targVal;
}

static float GetScalar(ptcScalarDistr *d, float t) {
	switch (d->DistrID) {
		case ptcDID_Constant:
			return d->Constant.Val;
		case ptcDID_Uniform:
			return d->Uniform.Range[0]
				+ FRand() * (d->Uniform.Range[1] - d->Uniform.Range[0]);
		case ptcDID_BicubicInterp:
			return GetBicubicInterp(t, d->BicubicInterp.TargVal);
	}
	return 0.f;
}

static void GetVector(ptcVectorDistr *d, float t, ptcVector out) {
	switch (d->DistrID) {
		case ptcDID_Constant:
			for (int i = 0; i < 3; ++i)
				out[i] = d->Constant.Val[i];
			break;
		case ptcDID_Uniform:
			for (int i = 0; i < 3; ++i)
				out[i] = d->Uniform.Ranges[0][i]
					+ FRand() * (d->Uniform.Ranges[1][i]
						- d->Uniform.Ranges[0][i]);
			break;
		case ptcDID_BicubicInterp:
			for (int i = 0; i < 3; ++i)
				out[i] = GetBicubicInterp(t, d->BicubicInterp.TargVal[i]);
	}
}

static void GetColour(ptcColourDistr *d, uint32_t flags, float t, ptcColour out) {
	switch (d->DistrID) {
		case ptcDID_Constant:
			if (flags & ptcCF_SetRGB) {
				for (int i = 0; i < 3; ++i)
					out[i] = d->Constant.Val[i];
			}
			if (flags & ptcCF_SetAlpha)
				out[3] = d->Constant.Val[3];
			break;
		case ptcDID_Uniform:
			if (flags & ptcCF_SetRGB) {
				for (int i = 0; i < 3; ++i)
					out[i] = d->Uniform.Ranges[0][i]
						+ FRand() * (d->Uniform.Ranges[1][i]
							- d->Uniform.Ranges[0][i]);
			}
			if (flags & ptcCF_SetAlpha)
				out[3] = d->Uniform.Ranges[0][3]
					+ FRand() * (d->Uniform.Ranges[1][3]
						- d->Uniform.Ranges[0][3]);
			break;
		case ptcDID_BicubicInterp:
			if (flags & ptcCF_SetRGB) {
				for (int i = 0; i < 3; ++i)
					out[i] = GetBicubicInterp(t, d->BicubicInterp.TargVal[i]);
			}
			if (flags & ptcCF_SetAlpha)
				out[3] = GetBicubicInterp(t, d->BicubicInterp.TargVal[3]);
			break;
	}
}
