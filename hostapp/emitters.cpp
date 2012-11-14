#include <cstdlib>
#include <cstring>
#include "../libparticlasm/libparticlasm.h"

// returns an array of emitter definitions for the fire effect
#ifdef __cplusplus
extern "C"
#endif
size_t Fire(ptcEmitter **emitters) {
	static ptcMod_InitialVelocity initvel;
	static ptcMod_InitialColour initcol;
	static ptcMod_InitialSize initsize;
	static ptcMod_Acceleration accel;
	static ptcMod_Colour alpha;
	static ptcMod_Gravity gravity;
	static ptcMod_InitialLocation s_il;
	static ptcMod_InitialVelocity s_iv;
	static ptcMod_InitialColour s_ic;
	static ptcMod_InitialSize s_is;
	static ptcMod_Acceleration s_acc;
	static ptcMod_Colour s_alpha;
	static ptcEmitter emit[2];

	memset(emit, 0, sizeof(emit));

	// fire emitter
	emit[0].Config.SpawnRate = 500.f;
	emit[0].Config.BurstCount = 15;
	emit[0].Config.LifeTimeFixed = 1;
	emit[0].Config.LifeTimeRandom = 1;
	emit[0].InternalPtr1 = NULL;
	emit[0].InternalPtr2 = NULL;
	emit[0].InternalPtr3 = NULL;
	emit[0].Head = (ptcModule *)&initvel;
	emit[0].ParticleBuf = NULL;
	emit[0].NumParticles = 0;

	initvel.Header.ModuleID = ptcMID_InitialVelocity;
	initvel.Header.Next = (ptcModule *)&initcol;
	initvel.Distr.Uniform.DistrID = ptcDID_Uniform;
	initvel.Distr.Uniform.Ranges[0][0] = -200;
	initvel.Distr.Uniform.Ranges[0][1] = 80;
	initvel.Distr.Uniform.Ranges[0][2] = -200;
	initvel.Distr.Uniform.Ranges[1][0] = 200;
	initvel.Distr.Uniform.Ranges[1][1] = 160;
	initvel.Distr.Uniform.Ranges[1][2] = 200;

	initcol.Header.ModuleID = ptcMID_InitialColour;
	initcol.Header.Next = (ptcModule *)&initsize;
	initcol.Distr.Uniform.DistrID = ptcDID_Uniform;
	initcol.Distr.Uniform.Ranges[0][0] = 0.8;
	initcol.Distr.Uniform.Ranges[0][1] = 0.f;
	initcol.Distr.Uniform.Ranges[0][2] = 0.f;
	initcol.Distr.Uniform.Ranges[0][3] = 1.f;
	initcol.Distr.Uniform.Ranges[1][0] = 1.f;
	initcol.Distr.Uniform.Ranges[1][1] = 0.8;
	initcol.Distr.Uniform.Ranges[1][2] = 0.f;
	initcol.Distr.Uniform.Ranges[1][3] = 1.f;

	initsize.Header.ModuleID = ptcMID_InitialSize;
	initsize.Header.Next = (ptcModule *)&accel;
	initsize.Distr.Uniform.DistrID = ptcDID_Uniform;
	initsize.Distr.Uniform.Range[0] = 3.f;
	initsize.Distr.Uniform.Range[1] = 6.f;

	accel.Header.ModuleID = ptcMID_Acceleration;
	accel.Header.Next = (ptcModule *)&alpha;
	accel.Distr.Uniform.DistrID = ptcDID_Uniform;
	accel.Distr.Uniform.Ranges[0][0] = 0.f;
	accel.Distr.Uniform.Ranges[0][1] = -50.f;
	accel.Distr.Uniform.Ranges[0][2] = 0.f;
	accel.Distr.Uniform.Ranges[1][0] = 0.f;
	accel.Distr.Uniform.Ranges[1][1] = -80.f;
	accel.Distr.Uniform.Ranges[1][2] = 0.f;

	alpha.Header.ModuleID = ptcMID_Colour;
	alpha.Header.Next = NULL;//(ptcModule *)&gravity;
	alpha.Distr.Constant.DistrID = ptcDID_Constant;
	alpha.Distr.Constant.Val[3] = 1.f;
	alpha.Flags = ptcCF_SetAlpha;

	gravity.Header.ModuleID = ptcMID_Gravity;
	gravity.Header.Next = NULL;
	gravity.Centre.Constant.DistrID = ptcDID_Constant;
	gravity.Centre.Constant.Val[0] = 0.f;
	gravity.Centre.Constant.Val[1] = 0.f;
	gravity.Centre.Constant.Val[2] = 0.f;
	gravity.Radius = 0.2;
	gravity.SourceMass = 4e14;
	gravity.Flags = ptcGF_AxisX | ptcGF_AxisZ | ptcGF_LinearAtt;

	// smoke emitter
	emit[1].Config.SpawnRate = 200.f;
	emit[1].Config.BurstCount = 8;
	emit[1].Config.LifeTimeFixed = 1;
	emit[1].Config.LifeTimeRandom = 1;
	emit[1].InternalPtr1 = NULL;
	emit[1].InternalPtr2 = NULL;
	emit[1].InternalPtr3 = NULL;
	emit[1].Head = (ptcModule *)&s_il;
	emit[1].ParticleBuf = NULL;
	emit[1].NumParticles = 0;

	s_il.Header.ModuleID = ptcMID_InitialLocation;
	s_il.Header.Next = (ptcModule *)&s_iv;
	s_il.Distr.Uniform.DistrID = ptcDID_Uniform;
	s_il.Distr.Uniform.Ranges[0][0] = -15;
	s_il.Distr.Uniform.Ranges[0][1] = 40;
	s_il.Distr.Uniform.Ranges[0][2] = -15;
	s_il.Distr.Uniform.Ranges[1][0] = 15;
	s_il.Distr.Uniform.Ranges[1][1] = 80;
	s_il.Distr.Uniform.Ranges[1][2] = 15;

	s_iv.Header.ModuleID = ptcMID_InitialVelocity;
	s_iv.Header.Next = (ptcModule *)&s_ic;
	s_iv.Distr.Uniform.DistrID = ptcDID_Uniform;
	s_iv.Distr.Uniform.Ranges[0][0] = -50;
	s_iv.Distr.Uniform.Ranges[0][1] = 40;
	s_iv.Distr.Uniform.Ranges[0][2] = -50;
	s_iv.Distr.Uniform.Ranges[1][0] = 50;
	s_iv.Distr.Uniform.Ranges[1][1] = 80;
	s_iv.Distr.Uniform.Ranges[1][2] = 50;

	s_ic.Header.ModuleID = ptcMID_InitialColour;
	s_ic.Header.Next = (ptcModule *)&s_is;
	s_ic.Distr.Constant.DistrID = ptcDID_Constant;
	s_ic.Distr.Constant.Val[0] = 0.5;
	s_ic.Distr.Constant.Val[1] = 0.5;
	s_ic.Distr.Constant.Val[2] = 0.5;
	s_ic.Distr.Constant.Val[3] = 1.f;

	s_is.Header.ModuleID = ptcMID_InitialSize;
	s_is.Header.Next = (ptcModule *)&s_acc;
	s_is.Distr.Uniform.DistrID = ptcDID_Uniform;
	s_is.Distr.Uniform.Range[0] = 6.f;
	s_is.Distr.Uniform.Range[1] = 12.f;

	s_acc.Header.ModuleID = ptcMID_Acceleration;
	s_acc.Header.Next = (ptcModule *)&s_alpha;
	s_acc.Distr.Uniform.DistrID = ptcDID_Uniform;
	s_acc.Distr.Uniform.Ranges[0][0] = 0.f;
	s_acc.Distr.Uniform.Ranges[0][1] = -50.f;
	s_acc.Distr.Uniform.Ranges[0][2] = 0.f;
	s_acc.Distr.Uniform.Ranges[1][0] = 0.f;
	s_acc.Distr.Uniform.Ranges[1][1] = -50.f;
	s_acc.Distr.Uniform.Ranges[1][2] = 0.f;

	s_alpha.Header.ModuleID = ptcMID_Colour;
	s_alpha.Header.Next = NULL;
	s_alpha.Distr.Constant.DistrID = ptcDID_Constant;
	s_alpha.Distr.Constant.Val[3] = 1.f;
	s_alpha.Flags = ptcCF_SetAlpha;

	*emitters = emit;

	return 1;//sizeof(emit) / sizeof(emit[0]);
}
