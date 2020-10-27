/*
 * P1_PWM.c
 *
 * S-Function device driver for the Pendulum 1 model. 
 * RTDAC4/PCI board
 *
 * Copyright (c) 2002 by InTeCo/2K/AP
 * All Rights Reserved
 *
 */


#define S_FUNCTION_NAME P1_PWM
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#include <conio.h>
#include <math.h>


static int WriteByte( int port, int value ) { 
    return( _outpd( (unsigned short)port, value ) );
}
static unsigned short WriteWord( int port, unsigned int value ) { 
    return( _outpd( (unsigned short)port, (unsigned short)value ) );
}
static int ReadByte( int port ) { 
    return( _inpd( (unsigned short)port ) );
}
static unsigned int ReadWord( int port ) { 
    return( _inpd( (unsigned short)port ) );
}
static unsigned int ReadDWord( int port ) { 
    return( _inpd( (unsigned short)port ) );
}
/* Input Arguments */
#define BASE_ADDRESS_ARG        (ssGetSFcnParam(S,0))
#define SAMPLE_TIME_ARG         (ssGetSFcnParam(S,1))
#define NUMBER_OF_ARGS          (2)
#define NSAMPLE_TIMES           (1)

#define BASE_ADDRESS            ((real_T) mxGetPr(BASE_ADDRESS_ARG)[0])
#define SAMPLE_TIME             ((real_T) mxGetPr(SAMPLE_TIME_ARG)[0])

static void mdlInitializeSizes(SimStruct *S)
{
    ssSetNumSFcnParams(S, NUMBER_OF_ARGS);
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        return; /* Parameter mismatch will be reported by Simulink */
    }

    if (!ssSetNumInputPorts(S, 1)) return;
    ssSetInputPortWidth(S, 0, 3);
    ssSetInputPortDirectFeedThrough(S, 0, 0);

    if (!ssSetNumOutputPorts(S, 1)) return;
    ssSetOutputPortWidth(S, 0, 3);

    ssSetNumSampleTimes(S, 1);

    /* Take care when specifying exception free code */
    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
}
 
 
/* Function to initialize sample times */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, SAMPLE_TIME);
    ssSetOffsetTime(S, 0, 0);
}
 

/* Function to compute outputs */
static void mdlOutputs(SimStruct *S, int_T tid)
{
  InputRealPtrsType uPtrs = ssGetInputPortRealSignalPtrs(S,0);
  real_T *y               = ssGetOutputPortRealSignal(S,0);

#ifndef MATLAB_MEX_FILE
  int BaseAddress = (int)BASE_ADDRESS;
  int Dir, Brake;

  if ( BaseAddress == 0 )  
  {
    y[0] = y[1] = y[2] = 0.0;
    return;
  }

  Dir   = ReadByte( BaseAddress+4*0x24 );
  Brake = ReadByte( BaseAddress+4*0x25 );

  if( fabs( (*uPtrs[0]) <= 1 ) )
  {
    WriteWord ( BaseAddress+4*0x21, (int)(fabs( (*uPtrs[0])*1023 )) );
    y[0] = ReadWord( BaseAddress+4*0x21 );
    if( y[0] == 0.0 )  Brake = Brake | 0x01; else Brake = Brake & 0xFE;
    if( (*uPtrs[0]) <  0.0 )  Dir   = Dir   | 0x01;
    if( (*uPtrs[0]) >= 0.0 )  Dir   = Dir   & 0xFE;
    if( (*uPtrs[0]) <  0.0 )  y[0]  = -y[0];
  }
  if( fabs( (*uPtrs[1]) <= 1 ) )
  {
    WriteWord ( BaseAddress+4*0x22, (int)(fabs( (*uPtrs[1])*1023 )) );
    y[1] = ReadWord( BaseAddress+4*0x22 );
    if( y[1] == 0.0 )  Brake = Brake | 0x02; else Brake = Brake & 0xFD;
    if( (*uPtrs[1]) <  0.0 )  Dir   = Dir   | 0x02;
    if( (*uPtrs[1]) >= 0.0 )  Dir   = Dir   & 0xFD;
    if( (*uPtrs[1]) < 0.0 )   y[1]  = -y[1];
  }
  if( fabs( (*uPtrs[2]) <= 1 ) )
  {
    WriteWord ( BaseAddress+4*0x23, (int)(fabs( (*uPtrs[2])*1023 )) );
    y[2] = ReadWord( BaseAddress+4*0x23 );
    if( y[2] == 0.0 )  Brake = Brake | 0x04; else Brake = Brake & 0xFB;
    if( (*uPtrs[2]) <  0.0 )  Dir   = Dir   | 0x04;
    if( (*uPtrs[2]) >= 0.0 )  Dir   = Dir   & 0xFB;
    if( (*uPtrs[2]) < 0.0 )   y[2]  = -y[2];
  }

  WriteByte( BaseAddress+4*0x24, Dir ); 
  WriteByte( BaseAddress+4*0x25, Brake );
#else
  y[0] = (*uPtrs[0]);
  y[1] = (*uPtrs[1]);
  y[2] = (*uPtrs[2]);
#endif
}


/* Function to perform cleanup at execution termination */
static void mdlTerminate(SimStruct *S)
{
#ifndef MATLAB_MEX_FILE
  int BaseAddress = (int)BASE_ADDRESS;
  int Brake;

  Brake = ReadByte( BaseAddress+4*0x25 );
  Brake = Brake | 0x07;
  WriteByte( BaseAddress+4*0x25, Brake );
#endif
}

#ifdef MATLAB_MEX_FILE  /* Is this file being compiled as a MEX-file? */
#include "simulink.c"        /* MEX-File interface mechanism */
#else
#include "cg_sfun.h"    /* Code generation registration function */
#endif
