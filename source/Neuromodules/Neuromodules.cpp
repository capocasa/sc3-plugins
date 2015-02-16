/*
	SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com
 
 
	Neuromodules
	by Frank Pasemann and Julian Rohrhuber
 
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "SC_PlugIn.h"
#include <stdio.h>

#ifndef MAXFLOAT
# include <float.h>
# define MAXFLOAT FLT_MAX
#endif

static InterfaceTable *ft;

#define MAXCHANNELS 32

struct DNeuromodule2 : public Unit
{
	double x1, x2;
};

struct DNeuromodule : public Unit
{
	int m_size;
	
	double *m_theta;
	double *m_x;
	double *m_weights;
	double *m_x_temp;
};

extern "C"
{
	void DNeuromodule2_Ctor(DNeuromodule2 *unit);
	void DNeuromodule2_reset(DNeuromodule2 *unit, int inNumSamples);
	void DNeuromodule2_end(DNeuromodule2 *unit);
	void DNeuromodule2_next(DNeuromodule2 *unit, int inNumSamples);
	
	void DNeuromodule_Ctor(DNeuromodule *unit);
	void DNeuromodule_Dtor(DNeuromodule *unit);
	void DNeuromodule_reset(DNeuromodule *unit, int inNumSamples);
	void DNeuromodule_end(DNeuromodule *unit);
	void DNeuromodule_next(DNeuromodule *unit, int inNumSamples);
	void Neuromodule_initInputs(DNeuromodule *unit, int size);
	
}


enum {
	theta1 = 0, theta2, x1, x2, w11, w12, w21, w22 // these seem to be unreliable.
};


void DNeuromodule2_next(DNeuromodule2 *unit, int inNumSamples)
{
	double x1, x2;
	
	if(inNumSamples) {
		double x1_limit = std::tanh(unit->x1);
		double x2_limit = std::tanh(unit->x2);
		
		/*
		 double theta1 = (double) DEMANDINPUT_A(theta1, inNumSamples);
		 double theta2 = (double) DEMANDINPUT_A(theta1, inNumSamples);
		 
		 double w11 = (double) DEMANDINPUT_A(w11, inNumSamples);
		 double w12 = (double) DEMANDINPUT_A(w12, inNumSamples);
		 double w21 = (double) DEMANDINPUT_A(w21, inNumSamples);
		 double w22 = (double) DEMANDINPUT_A(w22, inNumSamples);
		 */
		
		double theta1 = (double) DEMANDINPUT_A(0, inNumSamples);
		if(sc_isnan(theta1)) { DNeuromodule2_end(unit); return; }
		double theta2 = (double) DEMANDINPUT_A(1, inNumSamples);
		if(sc_isnan(theta2)) { DNeuromodule2_end(unit); return; }
		
		double w11 = (double) DEMANDINPUT_A(4, inNumSamples);
		if(sc_isnan(w11)) { DNeuromodule2_end(unit); return; }
		double w12 = (double) DEMANDINPUT_A(5, inNumSamples);
		if(sc_isnan(w12)) { DNeuromodule2_end(unit); return; }
		double w21 = (double) DEMANDINPUT_A(6, inNumSamples);
		if(sc_isnan(w21)) { DNeuromodule2_end(unit); return; }
		double w22 = (double) DEMANDINPUT_A(7, inNumSamples);
		if(sc_isnan(w22)) { DNeuromodule2_end(unit); return; }
		
		
		x1 = theta1 + w11 * x1_limit + w12 * x2_limit;
		x2 = theta2 + w21 * x1_limit + w22 * x2_limit;
		
		//	printf("x1 = %f  x2 = %f\n", x1, x2);
		//	printf("x1 = %f  x2 = %f, theta1 = %f, theat2 = %f, w11 = %f, w12 = %f, w21 = %f, w22 = %f\n", x1, x2, theta1, theta2, w11, w12, w21, w22);
		
		x1 = zapgremlins(x1);
		x2 = zapgremlins(x2);
		
		
		unit->x1 = x1;
		unit->x2 = x2;
		
		
		OUT0(0) = (float) x1;
		OUT0(1) = (float) x2;
		
	} else {
		
		DNeuromodule2_reset(unit, inNumSamples);
		
	}
	
	
}

void DNeuromodule2_reset(DNeuromodule2 *unit, int inNumSamples)
{
	float x1, x2;
	for(int i =	0; i < 8; i++) {
		RESETINPUT(i);
	}
	x1 = DEMANDINPUT_A(2, inNumSamples);
	x2 = DEMANDINPUT_A(3, inNumSamples);
	
	unit->x1 = (double) x1;
	unit->x2 = (double) x2;
	
	OUT0(0) = x1;
	OUT0(1) = x2;
	
	unit->x1 = (double) x1;
	unit->x2 = (double) x2;
	
}

void DNeuromodule2_end(DNeuromodule2 *unit)
{
	
	for(int i =	0; i < 2; i++) {
		OUT0(i) = NAN;
	}
	
}



void DNeuromodule2_Ctor(DNeuromodule2 *unit)
{
	
	SETCALC(DNeuromodule2_next);
	DNeuromodule2_reset(unit, 0);
}









//////////////////////////////////////////////////////

void Neuromodule_initInputs(DNeuromodule *unit, int size)
{
	int memsize = size * sizeof(double);
	int numWeights = size * size;
	
	
	unit->m_theta = (double*)RTAlloc(unit->mWorld, memsize);
	memset(unit->m_theta, 0, memsize);
	
	unit->m_x = (double*)RTAlloc(unit->mWorld, memsize);
	memset(unit->m_x, 0, memsize);
	
	unit->m_weights = (double*)RTAlloc(unit->mWorld, numWeights);
	memset(unit->m_weights, 0, numWeights);
	
	unit->m_x_temp = (double*)RTAlloc(unit->mWorld, memsize);
	memset(unit->m_x_temp, 0, memsize);

	
}

void DNeuromodule_next(DNeuromodule *unit, int inNumSamples)
{
	
	
	if(inNumSamples) {
		
		int size = unit->m_size;
		int numWeights = size * size;
		
		
		// printf("theta: ");
		
		// THETA
		for(int i=0; i < size; i++) {
			double val = (double) DEMANDINPUT_A(i + 1, inNumSamples);
			if(sc_isnan(val)) { DNeuromodule_end(unit); return; }
			//printf("%f ", val);
			unit->m_theta[i] = val;
		}
		
		// printf("x: ");
		
		
		
		// printf("weights: ");
		
		// WEIGHTS
		for(int i=0; i < numWeights; i++) {
			double val = (double) DEMANDINPUT_A(i + 1 + size + size, inNumSamples);
			if(sc_isnan(val)) { DNeuromodule_end(unit); return; }
		// 	printf("%f ", val);
			unit->m_weights[i] = val;
		}
		
		
		// keep normalized old values in a temporary array
		// so that we can overwrite the original directly
		
		for(int i=0; i < size; i++) {
			unit->m_x_temp[i] = std::tanh(unit->m_x[i]);
		}
		
		// iterate over all other values for each values and calcuate their contribution by weights.
		
		// printf("outputs: \n");
		for(int i=0; i < size; i++) {
			double xval;
			xval = unit->m_theta[i];
			for(int j=0; j < size; j++) {
				// printf("x = %f + %f * %f\n", xval, unit->m_weights[i * size + j], unit->m_x_temp[j]);
				xval += unit->m_weights[i * size + j] * unit->m_x_temp[j];
			}
			xval = zapgremlins(xval);
			unit->m_x[i] = xval;
			// printf("-> %f ", xval);
			OUT0(i) = (float) xval;
		}
		
		//printf("\n");

		
	} else {
		
		DNeuromodule_reset(unit, inNumSamples);
		
	}
	
	
}

void DNeuromodule_reset(DNeuromodule *unit, int inNumSamples)
{
	int size = unit->m_size;
	for(int i =	0; i < size; i++) {
		RESETINPUT(i + 1);
		
	}
	// initial X
	for(int i=0; i < size; i++) {
		double val = (double) DEMANDINPUT_A(i + 1 + size, inNumSamples);
		if(sc_isnan(val)) { DNeuromodule_end(unit); return; }
		// printf("%f ", val);
		unit->m_x[i] = val;
		OUT0(i) = val;
	}
}

void DNeuromodule_end(DNeuromodule *unit)
{
	
	for(int i =	0; i < unit->m_size; i++) {
		OUT0(i) = NAN;
	}
	
}


void DNeuromodule_Ctor(DNeuromodule *unit)
{
	
	SETCALC(DNeuromodule_next);
	unit->m_size = sc_max((int) IN0(0), 0);
	Neuromodule_initInputs(unit, unit->m_size);
	
	
	DNeuromodule_reset(unit, 0);
}

void DNeuromodule_Dtor(DNeuromodule *unit)
{
	RTFree(unit->mWorld, unit->m_theta);
	RTFree(unit->mWorld, unit->m_x);
	RTFree(unit->mWorld, unit->m_weights);
	RTFree(unit->mWorld, unit->m_x_temp);
}


//////////////////////////////////////////////////////



PluginLoad(TagSystem)
{
	ft = inTable;
	DefineSimpleUnit(DNeuromodule2);
	DefineDtorUnit(DNeuromodule);
	
}
