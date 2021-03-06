/***************************************************************************
Copyright (c) 2020, The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "common.h"
#if !defined(DOUBLE)
#define RVV_EFLOAT RVV_E32
#define RVV_M RVV_M4
#define FLOAT_V_T float32xm4_t
#define VLEV_FLOAT vlev_float32xm4
#define VLSEV_FLOAT vlsev_float32xm4
#define VFREDSUM_FLOAT vfredsumvs_float32xm4
#define VFMACCVV_FLOAT vfmaccvv_float32xm4
#define VFMVVF_FLOAT vfmvvf_float32xm4
#define VFDOTVV_FLOAT vfdotvv_float32xm4
#else
#define RVV_EFLOAT RVV_E64
#define RVV_M RVV_M4
#define FLOAT_V_T float64xm4_t
#define VLEV_FLOAT vlev_float64xm4
#define VLSEV_FLOAT vlsev_float64xm4
#define VFREDSUM_FLOAT vfredsumvs_float64xm4
#define VFMACCVV_FLOAT vfmaccvv_float64xm4
#define VFMVVF_FLOAT vfmvvf_float64xm4
#define VFDOTVV_FLOAT vfdotvv_float64xm4
#endif

#if defined(DSDOT)
double CNAME(BLASLONG n, FLOAT *x, BLASLONG inc_x, FLOAT *y, BLASLONG inc_y)
#else
FLOAT CNAME(BLASLONG n, FLOAT *x, BLASLONG inc_x, FLOAT *y, BLASLONG inc_y)
#endif
{
	BLASLONG i=0, j=0;
	double dot = 0.0 ;

	if ( n < 0 )  return(dot);

        FLOAT_V_T vr, vx, vy;
        unsigned int gvl = 0;
        if(inc_x == 1 && inc_y == 1){
                gvl = vsetvli(n, RVV_EFLOAT, RVV_M);
                vr = VFMVVF_FLOAT(0, gvl);
                for(i=0,j=0; i<n/gvl; i++){
                        vx = VLEV_FLOAT(&x[j], gvl);
                        vy = VLEV_FLOAT(&y[j], gvl);
                        vr = VFMACCVV_FLOAT(vr, vx, vy, gvl);
                        j += gvl;
                }
                if(j > 0){
                        vx = VFMVVF_FLOAT(0, gvl);
                        vx = VFREDSUM_FLOAT(vr, vx, gvl);
                        dot += vx[0];
                }
                //tail
                if(j < n){
                        gvl = vsetvli(n-j, RVV_EFLOAT, RVV_M);
                        vx = VLEV_FLOAT(&x[j], gvl);
                        vy = VLEV_FLOAT(&y[j], gvl);
                        FLOAT_V_T vz = VFMVVF_FLOAT(0, gvl);
                        //vr = VFDOTVV_FLOAT(vx, vy, gvl);
                        vr = VFMACCVV_FLOAT(vz, vx, vy, gvl);
                        vx = VFREDSUM_FLOAT(vr, vz, gvl);
                        dot += vx[0];
                }
        }else if(inc_y == 1){
                gvl = vsetvli(n, RVV_EFLOAT, RVV_M);
                vr = VFMVVF_FLOAT(0, gvl);
                unsigned int stride_x = inc_x * sizeof(FLOAT);
                for(i=0,j=0; i<n/gvl; i++){
                        vx = VLSEV_FLOAT(&x[j*inc_x], stride_x, gvl);
                        vy = VLEV_FLOAT(&y[j], gvl);
                        vr = VFMACCVV_FLOAT(vr, vx, vy, gvl);
                        j += gvl;
                }
                if(j > 0){
                        vx = VFMVVF_FLOAT(0, gvl);
                        vx = VFREDSUM_FLOAT(vr, vx, gvl);
                        dot += vx[0];
                }
                //tail
                if(j < n){
                        gvl = vsetvli(n-j, RVV_EFLOAT, RVV_M);
                        vx = VLSEV_FLOAT(&x[j*inc_x], stride_x, gvl);
                        vy = VLEV_FLOAT(&y[j], gvl);
                        FLOAT_V_T vz = VFMVVF_FLOAT(0, gvl);
                        //vr = VFDOTVV_FLOAT(vx, vy, gvl);
                        vr = VFMACCVV_FLOAT(vz, vx, vy, gvl);
                        vx = VFREDSUM_FLOAT(vr, vz, gvl);
                        dot += vx[0];
                }
        }else if(inc_x == 1){
                gvl = vsetvli(n, RVV_EFLOAT, RVV_M);
                vr = VFMVVF_FLOAT(0, gvl);
                unsigned int stride_y = inc_y * sizeof(FLOAT);
                for(i=0,j=0; i<n/gvl; i++){
                        vx = VLEV_FLOAT(&x[j], gvl);
                        vy = VLSEV_FLOAT(&y[j*inc_y], stride_y, gvl);
                        vr = VFMACCVV_FLOAT(vr, vx, vy, gvl);
                        j += gvl;
                }
                if(j > 0){
                        vx = VFMVVF_FLOAT(0, gvl);
                        vx = VFREDSUM_FLOAT(vr, vx, gvl);
                        dot += vx[0];
                }
                //tail
                if(j < n){
                        gvl = vsetvli(n-j, RVV_EFLOAT, RVV_M);
                        vx = VLEV_FLOAT(&x[j], gvl);
                        vy = VLSEV_FLOAT(&y[j*inc_y], stride_y, gvl);
                        FLOAT_V_T vz = VFMVVF_FLOAT(0, gvl);
                        //vr = VFDOTVV_FLOAT(vx, vy, gvl);
                        vr = VFMACCVV_FLOAT(vz, vx, vy, gvl);
                        vx = VFREDSUM_FLOAT(vr, vz, gvl);
                        dot += vx[0];
                }
        }else{
                gvl = vsetvli(n, RVV_EFLOAT, RVV_M);
                vr = VFMVVF_FLOAT(0, gvl);
                unsigned int stride_x = inc_x * sizeof(FLOAT);
                unsigned int stride_y = inc_y * sizeof(FLOAT);
                for(i=0,j=0; i<n/gvl; i++){
                        vx = VLSEV_FLOAT(&x[j*inc_x], stride_x, gvl);
                        vy = VLSEV_FLOAT(&y[j*inc_y], stride_y, gvl);
                        vr = VFMACCVV_FLOAT(vr, vx, vy, gvl);
                        j += gvl;
                }
                if(j > 0){
                        vx = VFMVVF_FLOAT(0, gvl);
                        vx = VFREDSUM_FLOAT(vr, vx, gvl);
                        dot += vx[0];
                }
                //tail
                if(j < n){
                        gvl = vsetvli(n-j, RVV_EFLOAT, RVV_M);
                        vx = VLSEV_FLOAT(&x[j*inc_x], stride_x, gvl);
                        vy = VLSEV_FLOAT(&y[j*inc_y], stride_y, gvl);
                        FLOAT_V_T vz = VFMVVF_FLOAT(0, gvl);
                        //vr = VFDOTVV_FLOAT(vx, vy, gvl);
                        vr = VFMACCVV_FLOAT(vz, vx, vy, gvl);
                        vx = VFREDSUM_FLOAT(vr, vz, gvl);
                        dot += vx[0];
                }
        }
	return(dot);
}


