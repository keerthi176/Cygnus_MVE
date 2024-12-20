/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This file was generated automatically by Embedded Wizard Studio.
*
* Please do not make any modifications of this file! The modifications are lost
* when the file is generated again by Embedded Wizard Studio!
*
* The template of this heading text can be found in the file 'head.ewt' in the
* directory 'Platforms' of your Embedded Wizard installation directory. If you
* wish to adapt this text, please copy the template file 'head.ewt' into your
* project directory and edit the copy only. Please avoid any modifications of
* the original template file!
*
* Version  : 9.00
* Profile  : STM32F746
* Platform : STM.STM32.RGB565
*
*******************************************************************************/

#include "ewlocale.h"
#include "_CoreTimer.h"
#include "_EffectsBoolEffect.h"
#include "_EffectsEffect.h"
#include "_EffectsEffectTimerClass.h"
#include "_EffectsFloatEffect.h"
#include "_EffectsInt32Effect.h"
#include "_EffectsPointEffect.h"
#include "_EffectsTimingList.h"
#include "Effects.h"

/* Constant values used in this 'C' module only. */
static const XPoint _Const0000 = { 100, 100 };

#ifndef EW_DONT_CHECK_INDEX
  /* This function is used to check the indices when accessing an array.
     If you don't want this verification add the define EW_DONT_CHECK_INDEX
     to your Makefile or project settings. */
  static int EwCheckIndex( int aIndex, int aRange, const char* aFile, int aLine )
  {
    if (( aIndex < 0 ) || ( aIndex >= aRange ))
    {
      EwPrint( "[FATAL ERROR in %s:%d] Array index %d out of bounds %d",
                aFile, aLine, aIndex, aRange );
      EwPanic();
    }
    return aIndex;
  }

  #define EwCheckIndex( aIndex, aRange ) \
    EwCheckIndex( aIndex, aRange, __FILE__, __LINE__ )
#else
  #define EwCheckIndex( aIndex, aRange ) aIndex
#endif

/* Initializer for the class 'Effects::Effect' */
void EffectsEffect__Init( EffectsEffect _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( EffectsEffect );

  /* ... and initialize objects, variables, properties, etc. */
  _this->invCycleDuration = 0.001000f;
  _this->Elasticity = 0.500000f;
  _this->Bounces = 3;
  _this->Oscillations = 3;
  _this->Amplitude = 0.500000f;
  _this->Exponent = 3.000000f;
  _this->Noise = 0.000000f;
  _this->TimingCustom2 = 0.000000f;
  _this->TimingCustom1 = 0.000000f;
  _this->Timing = EffectsTimingLinear;
  _this->CycleDuration = 1000;
}

/* Re-Initializer for the class 'Effects::Effect' */
void EffectsEffect__ReInit( EffectsEffect _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Effects::Effect' */
void EffectsEffect__Done( EffectsEffect _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( EffectsEffect );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Effects::Effect' */
void EffectsEffect__Mark( EffectsEffect _this )
{
  EwMarkObject( _this->timingList );
  EwMarkSlot( _this->privateOnFinished );
  EwMarkObject( _this->timer );
  EwMarkSlot( _this->OnFinished );
  EwMarkSlot( _this->OnAnimate );

  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* 'C' function for method : 'Effects::Effect.timerSlot()' */
void EffectsEffect_timerSlot( EffectsEffect _this, XObject sender )
{
  XBool done;

  /* Dummy expressions to avoid the 'C' warning 'unused argument'. */
  EW_UNUSED_ARG( sender );

  if ( _this->timer == 0 )
    return;

  if ( _this->startDirection == 0 )
  {
    if ( _this->Reversed )
      _this->startDirection = -1;
    else
      _this->startDirection = 1;

    _this->direction = _this->startDirection;
    _this->startTime = _this->timer->Time;
    _this->cycleCounter = 0;
    _this->lastFrame = -1.000000f;
  }

  done = 0;

  if (( _this->startDirection > 0 ) && ( _this->direction > 0 ))
    done = EffectsEffect_runFwdFwd( _this );
  else
    if (( _this->startDirection < 0 ) && ( _this->direction < 0 ))
      done = EffectsEffect_runRevRev( _this );
    else
      if ( _this->startDirection > 0 )
        done = EffectsEffect_runFwdRev( _this );
      else
        done = EffectsEffect_runRevFwd( _this );

  if ( done )
  {
    EffectsEffect_OnSetEnabled( _this, 0 );
    EwSignal( _this->privateOnFinished, ((XObject)_this ));
    EwSignal( _this->OnFinished, ((XObject)_this ));
  }
}

/* 'C' function for method : 'Effects::Effect.run()' */
void EffectsEffect_run( EffectsEffect _this, XFloat aFrame )
{
  if (( _this->timingList == 0 ) && ((( _this->Timing == EffectsTimingBounce_In 
      ) || ( _this->Timing == EffectsTimingBounce_Out )) || ( _this->Timing == EffectsTimingBounce_InOut 
      )))
  {
    XInt32 bounces = _this->Bounces + 1;
    XFloat elasticity = EwMathSqrt( _this->Elasticity );
    XFloat sum = 0.500000f;
    XInt32 i;
    XFloat invSum;
    _this->timingList = EwNewObject( EffectsTimingList, 0 );
    _this->timingList->spans[ 0 ] = 1.000000f;
    _this->timingList->amps[ 0 ] = 1.000000f;

    for ( i = 1; i < bounces; i = i + 1 )
    {
      _this->timingList->spans[ EwCheckIndex( i, 12 )] = _this->timingList->spans[ 
      EwCheckIndex( i - 1, 12 )] * elasticity;
      _this->timingList->amps[ EwCheckIndex( i, 12 )] = _this->timingList->amps[ 
      EwCheckIndex( i - 1, 12 )] * _this->Elasticity;
      sum = sum + _this->timingList->spans[ EwCheckIndex( i, 12 )];
    }

    invSum = 1.000000f / sum;
    _this->timingList->spans[ 0 ] = 0.500000f;
    sum = 0.000000f;

    for ( i = 0; i < bounces; i = i + 1 )
    {
      _this->timingList->spans[ EwCheckIndex( i, 12 )] = _this->timingList->spans[ 
      EwCheckIndex( i, 12 )] * invSum;
      sum = sum + _this->timingList->spans[ EwCheckIndex( i, 12 )];
    }

    _this->timingList->spans[ EwCheckIndex( bounces, 12 )] = _this->timingList->spans[ 
    EwCheckIndex( bounces, 12 )] + ( 1.000000f - sum );
  }

  if ( _this->Symmetric )
  {
    if ( aFrame < 0.500000f )
      aFrame = aFrame * 2.000000f;
    else
      aFrame = 2.000000f - ( aFrame * 2.000000f );
  }

  switch ( _this->Timing )
  {
    case EffectsTimingPower_In :
    {
      aFrame = EwMathPow( aFrame, _this->Exponent );
    }
    break;

    case EffectsTimingPower_Out :
    {
      aFrame = 1.000000f - aFrame;
      aFrame = 1.000000f - EwMathPow( aFrame, _this->Exponent );
    }
    break;

    case EffectsTimingPower_InOut :
    {
      aFrame = aFrame * 2.000000f;

      if ( aFrame < 1.000000f )
        aFrame = EwMathPow( aFrame, _this->Exponent ) / 2.000000f;
      else
      {
        aFrame = 2.000000f - aFrame;
        aFrame = ( 2.000000f - EwMathPow( aFrame, _this->Exponent )) * 0.500000f;
      }
    }
    break;

    case EffectsTimingExp_In :
    {
      aFrame = ( EwMathPow( 2.718000f, _this->Exponent * aFrame ) - 1.000000f ) 
      / ( EwMathPow( 2.718000f, _this->Exponent ) - 1.000000f );
    }
    break;

    case EffectsTimingExp_Out :
    {
      aFrame = 1.000000f - aFrame;
      aFrame = 1.000000f - (( EwMathPow( 2.718000f, _this->Exponent * aFrame ) - 
      1.000000f ) / ( EwMathPow( 2.718000f, _this->Exponent ) - 1.000000f ));
    }
    break;

    case EffectsTimingExp_InOut :
    {
      aFrame = aFrame * 2.000000f;

      if ( aFrame < 1.000000f )
        aFrame = (( EwMathPow( 2.718000f, _this->Exponent * aFrame ) - 1.000000f 
        ) / ( EwMathPow( 2.718000f, _this->Exponent ) - 1.000000f )) / 2.000000f;
      else
      {
        aFrame = 2.000000f - aFrame;
        aFrame = ( 2.000000f - (( EwMathPow( 2.718000f, _this->Exponent * aFrame 
        ) - 1.000000f ) / ( EwMathPow( 2.718000f, _this->Exponent ) - 1.000000f 
        ))) * 0.500000f;
      }
    }
    break;

    case EffectsTimingSine_In :
    {
      aFrame = 1.000000f - EwMathCos( aFrame * 90.000000f );
    }
    break;

    case EffectsTimingSine_Out :
    {
      aFrame = EwMathSin( aFrame * 90.000000f );
    }
    break;

    case EffectsTimingSine_InOut :
    {
      aFrame = aFrame * 2.000000f;

      if ( aFrame < 1.000000f )
      {
        aFrame = 1.000000f - EwMathCos( aFrame * 90.000000f );
        aFrame = aFrame * 0.500000f;
      }
      else
      {
        aFrame = 2.000000f - aFrame;
        aFrame = 1.000000f - EwMathCos( aFrame * 90.000000f );
        aFrame = ( 2.000000f - aFrame ) * 0.500000f;
      }
    }
    break;

    case EffectsTimingCircle_In :
    {
      aFrame = 1.000000f - EwMathSqrt( 1.000000f - ( aFrame * aFrame ));
    }
    break;

    case EffectsTimingCircle_Out :
    {
      aFrame = 1.000000f - aFrame;
      aFrame = EwMathSqrt( 1.000000f - ( aFrame * aFrame ));
    }
    break;

    case EffectsTimingCircle_InOut :
    {
      aFrame = aFrame * 2.000000f;

      if ( aFrame < 1.000000f )
      {
        aFrame = 1.000000f - EwMathSqrt( 1.000000f - ( aFrame * aFrame ));
        aFrame = aFrame * 0.500000f;
      }
      else
      {
        aFrame = 2.000000f - aFrame;
        aFrame = 1.000000f - EwMathSqrt( 1.000000f - ( aFrame * aFrame ));
        aFrame = ( 2.000000f - aFrame ) * 0.500000f;
      }
    }
    break;

    case EffectsTimingBack_In :
    {
      aFrame = (( aFrame * aFrame ) * aFrame ) - (( aFrame * _this->Amplitude ) 
      * EwMathSin( aFrame * 180.000000f ));
    }
    break;

    case EffectsTimingBack_Out :
    {
      aFrame = 1.000000f - aFrame;
      aFrame = (( aFrame * aFrame ) * aFrame ) - (( aFrame * _this->Amplitude ) 
      * EwMathSin( aFrame * 180.000000f ));
      aFrame = 1.000000f - aFrame;
    }
    break;

    case EffectsTimingBack_InOut :
    {
      aFrame = aFrame * 2.000000f;

      if ( aFrame < 1.000000f )
      {
        aFrame = (( aFrame * aFrame ) * aFrame ) - (( aFrame * _this->Amplitude 
        ) * EwMathSin( aFrame * 180.000000f ));
        aFrame = aFrame * 0.500000f;
      }
      else
      {
        aFrame = 2.000000f - aFrame;
        aFrame = (( aFrame * aFrame ) * aFrame ) - (( aFrame * _this->Amplitude 
        ) * EwMathSin( aFrame * 180.000000f ));
        aFrame = ( 2.000000f - aFrame ) * 0.500000f;
      }
    }
    break;

    case EffectsTimingElastic_In :
    {
      aFrame = (( aFrame * aFrame ) * aFrame ) * EwMathSin(( aFrame * 90.000000f 
      ) * (XFloat)( 1 + ( 4 * _this->Oscillations )));
    }
    break;

    case EffectsTimingElastic_Out :
    {
      aFrame = 1.000000f - aFrame;
      aFrame = 1.000000f - ((( aFrame * aFrame ) * aFrame ) * EwMathSin(( aFrame 
      * 90.000000f ) * (XFloat)( 1 + ( 4 * _this->Oscillations ))));
    }
    break;

    case EffectsTimingElastic_InOut :
    {
      aFrame = aFrame * 2.000000f;

      if ( aFrame < 1.000000f )
      {
        aFrame = (( aFrame * aFrame ) * aFrame ) * EwMathSin(( aFrame * 90.000000f 
        ) * (XFloat)( 1 + ( 4 * _this->Oscillations )));
        aFrame = aFrame * 0.500000f;
      }
      else
      {
        aFrame = 2.000000f - aFrame;
        aFrame = (( aFrame * aFrame ) * aFrame ) * EwMathSin(( aFrame * 90.000000f 
        ) * (XFloat)( 1 + ( 4 * _this->Oscillations )));
        aFrame = ( 2.000000f - aFrame ) * 0.500000f;
      }
    }
    break;

    case EffectsTimingBounce_In :
    {
      EffectsTimingList list = _this->timingList;
      XInt32 i = 0;
      XFloat t1 = 0.000000f;
      XFloat t2 = list->spans[ 0 ];
      XFloat t = 1.000000f - aFrame;

      while ( t > t2 )
      {
        i = i + 1;
        t1 = t2;
        t2 = t2 + list->spans[ EwCheckIndex( i, 12 )];
      }

      aFrame = ( t - t1 ) / ( t2 - t1 );

      if ( i == 0 )
        aFrame = 1.000000f - ( aFrame * aFrame );
      else
      {
        aFrame = ( 2.000000f * aFrame ) - 1.000000f;
        aFrame = list->amps[ EwCheckIndex( i, 12 )] * ( 1.000000f - ( aFrame * aFrame 
        ));
      }
    }
    break;

    case EffectsTimingBounce_Out :
    {
      EffectsTimingList list = _this->timingList;
      XInt32 i = 0;
      XFloat t1 = 0.000000f;
      XFloat t2 = list->spans[ 0 ];

      while ( aFrame > t2 )
      {
        i = i + 1;
        t1 = t2;
        t2 = t2 + list->spans[ EwCheckIndex( i, 12 )];
      }

      aFrame = ( aFrame - t1 ) / ( t2 - t1 );

      if ( i == 0 )
        aFrame = aFrame * aFrame;
      else
      {
        aFrame = ( 2.000000f * aFrame ) - 1.000000f;
        aFrame = 1.000000f - ( list->amps[ EwCheckIndex( i, 12 )] * ( 1.000000f 
        - ( aFrame * aFrame )));
      }
    }
    break;

    case EffectsTimingBounce_InOut :
    {
      EffectsTimingList list = _this->timingList;
      XInt32 i = 0;
      XFloat t1 = 0.000000f;
      XFloat t2 = list->spans[ 0 ];
      XFloat f = aFrame * 2.000000f;
      XFloat t = f - 1.000000f;

      if ( f < 1.000000f )
        t = -t;

      while ( t > t2 )
      {
        i = i + 1;
        t1 = t2;
        t2 = t2 + list->spans[ EwCheckIndex( i, 12 )];
      }

      t = ( t - t1 ) / ( t2 - t1 );

      if ( i == 0 )
        t = t * t;
      else
      {
        t = ( 2.000000f * t ) - 1.000000f;
        t = 1.000000f - ( list->amps[ EwCheckIndex( i, 12 )] * ( 1.000000f - ( t 
        * t )));
      }

      if ( f < 1.000000f )
        aFrame = 0.500000f * ( 1.000000f - t );
      else
        aFrame = 0.500000f * ( 1.000000f + t );
    }
    break;

    default : 
    {
      if ( _this->useBezier2 )
      {
        XFloat f = aFrame;
        XFloat nf = 1.000000f - f;
        aFrame = (( nf * f ) * ( _this->curveFactor1 + 1.000000f )) + ( f * f );
      }
      else
        if ( _this->useBezier3 )
        {
          XFloat f = aFrame;
          XFloat nf = 1.000000f - f;
          XFloat f2 = f * f;
          XFloat nf2 = nf * nf;
          aFrame = ((( nf2 * f ) * ( _this->curveFactor1 + 1.000000f )) + (( nf 
          * f2 ) * ( _this->curveFactor2 + 2.000000f ))) + ( f2 * f );
        }
    }
  }

  if ((( _this->Noise > 0.000000f ) && ( aFrame > 0.000000f )) && ( aFrame < 1.000000f 
      ))
    aFrame = aFrame + EwMathRandFloat( -_this->Noise * 0.100000f, _this->Noise * 
    0.100000f );

  EffectsEffect__Animate( _this, aFrame );
  EwSignal( _this->OnAnimate, ((XObject)_this ));
}

/* 'C' function for method : 'Effects::Effect.runRevRev()' */
XBool EffectsEffect_runRevRev( EffectsEffect _this )
{
  XInt32 time = (XInt32)( _this->timer->Time - _this->startTime );
  XInt32 delay;
  XInt32 period0;
  XInt32 periodN;
  XBool done;
  XFloat frame;

  if ( time < 0 )
    time = -time;

  delay = _this->InitialDelay;
  period0 = _this->InitialDelay + _this->CycleDuration;
  periodN = _this->InterCycleDelay + _this->CycleDuration;
  done = 0;
  frame = _this->lastFrame;

  if (( _this->cycleCounter == 0 ) && ( time >= period0 ))
  {
    _this->cycleCounter = 1;
    time = time - period0;
    _this->startTime = _this->startTime + period0;
  }

  if (( _this->cycleCounter > 0 ) && ( time >= periodN ))
  {
    XInt32 cycles = time / periodN;
    _this->cycleCounter = _this->cycleCounter + cycles;
    time = time - ( cycles * periodN );
    _this->startTime = _this->startTime + ( cycles * periodN );
  }

  if (( _this->cycleCounter > 2 ) && ( _this->NoOfCycles == 0 ))
    _this->cycleCounter = 1;

  if ( _this->cycleCounter > 0 )
    delay = _this->InterCycleDelay;

  if (( _this->cycleCounter >= _this->NoOfCycles ) && ( _this->NoOfCycles > 0 ))
  {
    done = 1;
    frame = 0.000000f;
  }
  else
    if ( time >= delay )
      frame = 1.000000f - ((XFloat)( time - delay ) * _this->invCycleDuration );
    else
      if ( frame >= 0.000000f )
        frame = 0.000000f;

  if ( frame != _this->lastFrame )
  {
    _this->lastFrame = frame;
    EffectsEffect_run( _this, frame );
  }

  return done;
}

/* 'C' function for method : 'Effects::Effect.runRevFwd()' */
XBool EffectsEffect_runRevFwd( EffectsEffect _this )
{
  XInt32 time = (XInt32)( _this->startTime - _this->timer->Time );
  XInt32 delay = _this->InitialDelay;
  XInt32 period0 = _this->InitialDelay + _this->CycleDuration;
  XInt32 periodN = _this->InterCycleDelay + _this->CycleDuration;
  XBool done = 0;
  XFloat frame = _this->lastFrame;

  if (( _this->cycleCounter > 1 ) && ( time < 0 ))
  {
    XInt32 cycles = (( -time + periodN ) - 1 ) / periodN;

    if (( _this->cycleCounter - cycles ) <= 0 )
      cycles = _this->cycleCounter - 1;

    _this->cycleCounter = _this->cycleCounter - cycles;
    time = time + ( cycles * periodN );
    _this->startTime = _this->startTime + ( cycles * periodN );
  }

  if ((( _this->cycleCounter == 1 ) && ( time < 0 )) && ( _this->NoOfCycles > 0 
      ))
  {
    _this->cycleCounter = 0;
    time = time + period0;
    _this->startTime = _this->startTime + period0;
  }

  if ((( _this->cycleCounter == 1 ) && ( time < 0 )) && ( _this->NoOfCycles == 0 
      ))
  {
    XInt32 cycles = (( -time + periodN ) - 1 ) / periodN;
    time = time + ( cycles * periodN );
    _this->startTime = _this->startTime + ( cycles * periodN );
  }

  if ( _this->cycleCounter > 0 )
    delay = _this->InterCycleDelay;

  if ( time < 0 )
  {
    done = 1;
    frame = 1.000000f;
  }
  else
    if ( time >= delay )
      frame = 1.000000f - ((XFloat)( time - delay ) * _this->invCycleDuration );
    else
      if ( frame >= 0.000000f )
        frame = 1.000000f;

  if ( frame != _this->lastFrame )
  {
    _this->lastFrame = frame;
    EffectsEffect_run( _this, frame );
  }

  return done;
}

/* 'C' function for method : 'Effects::Effect.runFwdRev()' */
XBool EffectsEffect_runFwdRev( EffectsEffect _this )
{
  XInt32 time = (XInt32)( _this->startTime - _this->timer->Time );
  XInt32 delay = _this->InitialDelay;
  XInt32 period0 = _this->InitialDelay + _this->CycleDuration;
  XInt32 periodN = _this->InterCycleDelay + _this->CycleDuration;
  XBool done = 0;
  XFloat frame = _this->lastFrame;

  if (( _this->cycleCounter > 1 ) && ( time < 0 ))
  {
    XInt32 cycles = (( -time + periodN ) - 1 ) / periodN;

    if (( _this->cycleCounter - cycles ) <= 0 )
      cycles = _this->cycleCounter - 1;

    _this->cycleCounter = _this->cycleCounter - cycles;
    time = time + ( cycles * periodN );
    _this->startTime = _this->startTime + ( cycles * periodN );
  }

  if ((( _this->cycleCounter == 1 ) && ( time < 0 )) && ( _this->NoOfCycles > 0 
      ))
  {
    _this->cycleCounter = 0;
    time = time + period0;
    _this->startTime = _this->startTime + period0;
  }

  if ((( _this->cycleCounter == 1 ) && ( time < 0 )) && ( _this->NoOfCycles == 0 
      ))
  {
    XInt32 cycles = (( -time + periodN ) - 1 ) / periodN;
    time = time + ( cycles * periodN );
    _this->startTime = _this->startTime + ( cycles * periodN );
  }

  if ( _this->cycleCounter > 0 )
    delay = _this->InterCycleDelay;

  if ( time < 0 )
  {
    done = 1;
    frame = 0.000000f;
  }
  else
    if ( time >= delay )
      frame = (XFloat)( time - delay ) * _this->invCycleDuration;
    else
      if ( frame >= 0.000000f )
        frame = 0.000000f;

  if ( frame != _this->lastFrame )
  {
    _this->lastFrame = frame;
    EffectsEffect_run( _this, frame );
  }

  return done;
}

/* 'C' function for method : 'Effects::Effect.runFwdFwd()' */
XBool EffectsEffect_runFwdFwd( EffectsEffect _this )
{
  XInt32 time = (XInt32)( _this->timer->Time - _this->startTime );
  XInt32 delay;
  XInt32 period0;
  XInt32 periodN;
  XBool done;
  XFloat frame;

  if ( time < 0 )
    time = -time;

  delay = _this->InitialDelay;
  period0 = _this->InitialDelay + _this->CycleDuration;
  periodN = _this->InterCycleDelay + _this->CycleDuration;
  done = 0;
  frame = _this->lastFrame;

  if (( _this->cycleCounter == 0 ) && ( time >= period0 ))
  {
    _this->cycleCounter = 1;
    time = time - period0;
    _this->startTime = _this->startTime + period0;
  }

  if (( _this->cycleCounter > 0 ) && ( time >= periodN ))
  {
    XInt32 cycles = time / periodN;
    _this->cycleCounter = _this->cycleCounter + cycles;
    time = time - ( cycles * periodN );
    _this->startTime = _this->startTime + ( cycles * periodN );
  }

  if (( _this->cycleCounter > 2 ) && ( _this->NoOfCycles == 0 ))
    _this->cycleCounter = 1;

  if ( _this->cycleCounter > 0 )
    delay = _this->InterCycleDelay;

  if (( _this->cycleCounter >= _this->NoOfCycles ) && ( _this->NoOfCycles > 0 ))
  {
    done = 1;
    frame = 1.000000f;
  }
  else
    if ( time >= delay )
      frame = (XFloat)( time - delay ) * _this->invCycleDuration;
    else
      if ( frame >= 0.000000f )
        frame = 1.000000f;

  if ( frame != _this->lastFrame )
  {
    _this->lastFrame = frame;
    EffectsEffect_run( _this, frame );
  }

  return done;
}

/* 'C' function for method : 'Effects::Effect.OnSetReversed()' */
void EffectsEffect_OnSetReversed( EffectsEffect _this, XBool value )
{
  if ( _this->Reversed == value )
    return;

  _this->Reversed = value;

  if ( !_this->Enabled || ( _this->startDirection == 0 ))
    return;

  if ( value )
    _this->direction = -1;
  else
    _this->direction = 1;

  _this->startTime = ( CoreTimer_GetCurrentTime( _this->timer ) * 2 ) - _this->startTime;
}

/* 'C' function for method : 'Effects::Effect.OnSetTiming()' */
void EffectsEffect_OnSetTiming( EffectsEffect _this, XEnum value )
{
  if ( _this->Timing == value )
    return;

  _this->Timing = value;
  _this->timingList = 0;

  switch ( value )
  {
    case EffectsTimingEaseIn_EaseOut :
    {
      _this->curveFactor1 = -1.100000f;
      _this->curveFactor2 = 1.100000f;
    }
    break;

    case EffectsTimingEaseIn_FastOut :
    {
      _this->curveFactor1 = -1.000000f;
      _this->curveFactor2 = -2.000000f;
    }
    break;

    case EffectsTimingFastIn_EaseOut :
    {
      _this->curveFactor1 = 2.000000f;
      _this->curveFactor2 = 1.000000f;
    }
    break;

    case EffectsTimingFastIn_FastOut :
    {
      _this->curveFactor1 = 1.100000f;
      _this->curveFactor2 = -1.100000f;
    }
    break;

    case EffectsTimingLinear :
    {
      _this->curveFactor1 = 0.000000f;
      _this->curveFactor2 = 0.000000f;
    }
    break;

    default : 
    {
      _this->curveFactor1 = _this->TimingCustom1;
      _this->curveFactor2 = _this->TimingCustom2;
    }
  }

  _this->useBezier2 = (XBool)(( _this->curveFactor1 == _this->curveFactor2 ) && 
  ( _this->curveFactor1 != 0.000000f ));
  _this->useBezier3 = (XBool)( !_this->useBezier2 && ( _this->curveFactor1 != _this->curveFactor2 
  ));
}

/* 'C' function for method : 'Effects::Effect.OnSetNoOfCycles()' */
void EffectsEffect_OnSetNoOfCycles( EffectsEffect _this, XInt32 value )
{
  if ( value < 0 )
    value = 0;

  _this->NoOfCycles = value;
}

/* 'C' function for method : 'Effects::Effect.OnSetCycleDuration()' */
void EffectsEffect_OnSetCycleDuration( EffectsEffect _this, XInt32 value )
{
  if ( value < 15 )
    value = 15;

  _this->CycleDuration = value;
  _this->invCycleDuration = 1.000000f / (XFloat)value;
}

/* 'C' function for method : 'Effects::Effect.OnSetEnabled()' */
void EffectsEffect_OnSetEnabled( EffectsEffect _this, XBool value )
{
  if ( _this->Enabled == value )
    return;

  _this->Enabled = value;

  if ( !value && ( _this->timer != 0 ))
  {
    EwDetachObjObserver( EwNewSlot( _this, EffectsEffect_timerSlot ), (XObject)_this->timer, 
      0 );
    _this->timer = 0;
  }

  if ( value )
  {
    _this->timer = ((CoreTimer)EwGetAutoObject( &EffectsEffectTimer, EffectsEffectTimerClass 
    ));
    EwAttachObjObserver( EwNewSlot( _this, EffectsEffect_timerSlot ), (XObject)_this->timer, 
      0 );
    _this->startDirection = 0;
    EwPostSignal( EwNewSlot( _this, EffectsEffect_timerSlot ), ((XObject)_this ));
  }
}

/* 'C' function for method : 'Effects::Effect.Animate()' */
void EffectsEffect_Animate( EffectsEffect _this, XFloat aProgress )
{
  /* Dummy expressions to avoid the 'C' warning 'unused argument'. */
  EW_UNUSED_ARG( _this );
  EW_UNUSED_ARG( aProgress );
}

/* Wrapper function for the virtual method : 'Effects::Effect.Animate()' */
void EffectsEffect__Animate( void* _this, XFloat aProgress )
{
  ((EffectsEffect)_this)->_VMT->Animate((EffectsEffect)_this, aProgress );
}

/* Variants derived from the class : 'Effects::Effect' */
EW_DEFINE_CLASS_VARIANTS( EffectsEffect )
EW_END_OF_CLASS_VARIANTS( EffectsEffect )

/* Virtual Method Table (VMT) for the class : 'Effects::Effect' */
EW_DEFINE_CLASS( EffectsEffect, XObject, "Effects::Effect" )
  EffectsEffect_Animate,
EW_END_OF_CLASS( EffectsEffect )

/* Initializer for the class 'Effects::Int32Effect' */
void EffectsInt32Effect__Init( EffectsInt32Effect _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  EffectsEffect__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( EffectsInt32Effect );

  /* ... and initialize objects, variables, properties, etc. */
  _this->Value2 = 255;
}

/* Re-Initializer for the class 'Effects::Int32Effect' */
void EffectsInt32Effect__ReInit( EffectsInt32Effect _this )
{
  /* At first re-initialize the super class ... */
  EffectsEffect__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Effects::Int32Effect' */
void EffectsInt32Effect__Done( EffectsInt32Effect _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( EffectsInt32Effect );

  /* Don't forget to deinitialize the super class ... */
  EffectsEffect__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Effects::Int32Effect' */
void EffectsInt32Effect__Mark( EffectsInt32Effect _this )
{
  EwMarkRef( _this->Outlet );

  /* Give the super class a chance to mark its objects and references */
  EffectsEffect__Mark( &_this->_Super );
}

/* 'C' function for method : 'Effects::Int32Effect.Animate()' */
void EffectsInt32Effect_Animate( EffectsInt32Effect _this, XFloat aProgress )
{
  _this->Value = _this->Value1 + (XInt32)EwMathRound((XFloat)( _this->Value2 - _this->Value1 
  ) * aProgress );

  if ( _this->Outlet.Object != 0 )
    EwOnSetInt32( _this->Outlet, _this->Value );
}

/* Variants derived from the class : 'Effects::Int32Effect' */
EW_DEFINE_CLASS_VARIANTS( EffectsInt32Effect )
EW_END_OF_CLASS_VARIANTS( EffectsInt32Effect )

/* Virtual Method Table (VMT) for the class : 'Effects::Int32Effect' */
EW_DEFINE_CLASS( EffectsInt32Effect, EffectsEffect, "Effects::Int32Effect" )
  EffectsInt32Effect_Animate,
EW_END_OF_CLASS( EffectsInt32Effect )

/* Initializer for the class 'Effects::PointEffect' */
void EffectsPointEffect__Init( EffectsPointEffect _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  EffectsEffect__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( EffectsPointEffect );

  /* ... and initialize objects, variables, properties, etc. */
  _this->Value2 = _Const0000;
}

/* Re-Initializer for the class 'Effects::PointEffect' */
void EffectsPointEffect__ReInit( EffectsPointEffect _this )
{
  /* At first re-initialize the super class ... */
  EffectsEffect__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Effects::PointEffect' */
void EffectsPointEffect__Done( EffectsPointEffect _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( EffectsPointEffect );

  /* Don't forget to deinitialize the super class ... */
  EffectsEffect__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Effects::PointEffect' */
void EffectsPointEffect__Mark( EffectsPointEffect _this )
{
  EwMarkRef( _this->Outlet );

  /* Give the super class a chance to mark its objects and references */
  EffectsEffect__Mark( &_this->_Super );
}

/* 'C' function for method : 'Effects::PointEffect.Animate()' */
void EffectsPointEffect_Animate( EffectsPointEffect _this, XFloat aProgress )
{
  XInt32 x = _this->Value1.X;
  XInt32 y = _this->Value1.Y;

  x = x + (XInt32)EwMathRound((XFloat)( _this->Value2.X - x ) * aProgress );
  y = y + (XInt32)EwMathRound((XFloat)( _this->Value2.Y - y ) * aProgress );
  _this->Value = EwNewPoint( x, y );

  if ( _this->Outlet.Object != 0 )
    EwOnSetPoint( _this->Outlet, _this->Value );
}

/* Variants derived from the class : 'Effects::PointEffect' */
EW_DEFINE_CLASS_VARIANTS( EffectsPointEffect )
EW_END_OF_CLASS_VARIANTS( EffectsPointEffect )

/* Virtual Method Table (VMT) for the class : 'Effects::PointEffect' */
EW_DEFINE_CLASS( EffectsPointEffect, EffectsEffect, "Effects::PointEffect" )
  EffectsPointEffect_Animate,
EW_END_OF_CLASS( EffectsPointEffect )

/* Initializer for the class 'Effects::FloatEffect' */
void EffectsFloatEffect__Init( EffectsFloatEffect _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  EffectsEffect__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( EffectsFloatEffect );

  /* ... and initialize objects, variables, properties, etc. */
  _this->Value2 = 1.000000f;
  _this->Value1 = 0.000000f;
}

/* Re-Initializer for the class 'Effects::FloatEffect' */
void EffectsFloatEffect__ReInit( EffectsFloatEffect _this )
{
  /* At first re-initialize the super class ... */
  EffectsEffect__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Effects::FloatEffect' */
void EffectsFloatEffect__Done( EffectsFloatEffect _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( EffectsFloatEffect );

  /* Don't forget to deinitialize the super class ... */
  EffectsEffect__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Effects::FloatEffect' */
void EffectsFloatEffect__Mark( EffectsFloatEffect _this )
{
  EwMarkRef( _this->Outlet );

  /* Give the super class a chance to mark its objects and references */
  EffectsEffect__Mark( &_this->_Super );
}

/* 'C' function for method : 'Effects::FloatEffect.Animate()' */
void EffectsFloatEffect_Animate( EffectsFloatEffect _this, XFloat aProgress )
{
  _this->Value = _this->Value1 + (( _this->Value2 - _this->Value1 ) * aProgress 
  );

  if ( _this->Outlet.Object != 0 )
    EwOnSetFloat( _this->Outlet, _this->Value );
}

/* Variants derived from the class : 'Effects::FloatEffect' */
EW_DEFINE_CLASS_VARIANTS( EffectsFloatEffect )
EW_END_OF_CLASS_VARIANTS( EffectsFloatEffect )

/* Virtual Method Table (VMT) for the class : 'Effects::FloatEffect' */
EW_DEFINE_CLASS( EffectsFloatEffect, EffectsEffect, "Effects::FloatEffect" )
  EffectsFloatEffect_Animate,
EW_END_OF_CLASS( EffectsFloatEffect )

/* Initializer for the class 'Effects::BoolEffect' */
void EffectsBoolEffect__Init( EffectsBoolEffect _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( EffectsBoolEffect );

  /* ... and initialize objects, variables, properties, etc. */
  _this->Value2 = 1;
  _this->CycleDuration = 1000;
  _this->InterCycleDelay = 1000;
}

/* Re-Initializer for the class 'Effects::BoolEffect' */
void EffectsBoolEffect__ReInit( EffectsBoolEffect _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Effects::BoolEffect' */
void EffectsBoolEffect__Done( EffectsBoolEffect _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( EffectsBoolEffect );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Effects::BoolEffect' */
void EffectsBoolEffect__Mark( EffectsBoolEffect _this )
{
  EwMarkSlot( _this->privateOnFinished );
  EwMarkObject( _this->timer );
  EwMarkRef( _this->Outlet );
  EwMarkSlot( _this->OnFinished );

  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* 'C' function for method : 'Effects::BoolEffect.timerSlot()' */
void EffectsBoolEffect_timerSlot( EffectsBoolEffect _this, XObject sender )
{
  XInt32 time;
  XInt32 period0;
  XInt32 periodN;
  XInt32 delay;
  XInt32 counter;
  XBool done;

  /* Dummy expressions to avoid the 'C' warning 'unused argument'. */
  EW_UNUSED_ARG( sender );

  if ( _this->timer == 0 )
    return;

  if ( _this->cycleCounter < 0 )
  {
    _this->startTime = _this->timer->Time;
    _this->cycleCounter = 0;
  }

  time = (XInt32)( _this->timer->Time - _this->startTime );
  period0 = _this->InitialDelay + _this->CycleDuration;
  periodN = _this->InterCycleDelay + _this->CycleDuration;
  delay = _this->InitialDelay;
  counter = _this->cycleCounter;

  if (( counter == 0 ) && ( time >= period0 ))
  {
    counter = 1;
    time = time - period0;
  }

  if (( counter > 0 ) && ( time >= periodN ))
  {
    XInt32 cycles = time / periodN;
    time = time - ( cycles * periodN );
    counter = counter + cycles;
  }

  if (( counter > 2 ) && ( _this->NoOfCycles == 0 ))
    counter = 1;

  if ( counter != _this->cycleCounter )
  {
    _this->startTime = _this->timer->Time - (XUInt32)time;
    _this->cycleCounter = counter;
  }

  if ( counter > 0 )
    delay = _this->InterCycleDelay;

  done = (XBool)(( counter >= _this->NoOfCycles ) && ( _this->NoOfCycles > 0 ));

  if ( _this->Outlet.Object != 0 )
  {
    if ( !done && ( time >= delay ))
      EwOnSetBool( _this->Outlet, _this->Value1 );

    if ( done || (( time < delay ) && ( counter > 0 )))
      EwOnSetBool( _this->Outlet, _this->Value2 );
  }

  if ( done )
  {
    EffectsBoolEffect_OnSetEnabled( _this, 0 );
    EwSignal( _this->privateOnFinished, ((XObject)_this ));
    EwSignal( _this->OnFinished, ((XObject)_this ));
  }
}

/* 'C' function for method : 'Effects::BoolEffect.OnSetCycleDuration()' */
void EffectsBoolEffect_OnSetCycleDuration( EffectsBoolEffect _this, XInt32 value )
{
  if ( value < 100 )
    value = 100;

  _this->CycleDuration = value;
}

/* 'C' function for method : 'Effects::BoolEffect.OnSetInterCycleDelay()' */
void EffectsBoolEffect_OnSetInterCycleDelay( EffectsBoolEffect _this, XInt32 value )
{
  if ( value < 0 )
    value = 0;

  _this->InterCycleDelay = value;
}

/* 'C' function for method : 'Effects::BoolEffect.OnSetEnabled()' */
void EffectsBoolEffect_OnSetEnabled( EffectsBoolEffect _this, XBool value )
{
  if ( _this->Enabled == value )
    return;

  _this->Enabled = value;

  if ( !value && ( _this->timer != 0 ))
  {
    EwDetachObjObserver( EwNewSlot( _this, EffectsBoolEffect_timerSlot ), (XObject)_this->timer, 
      0 );
    _this->timer = 0;
  }

  if ( value )
  {
    _this->timer = ((CoreTimer)EwGetAutoObject( &EffectsEffectTimer, EffectsEffectTimerClass 
    ));
    EwAttachObjObserver( EwNewSlot( _this, EffectsBoolEffect_timerSlot ), (XObject)_this->timer, 
      0 );
    _this->cycleCounter = -1;
  }
}

/* Variants derived from the class : 'Effects::BoolEffect' */
EW_DEFINE_CLASS_VARIANTS( EffectsBoolEffect )
EW_END_OF_CLASS_VARIANTS( EffectsBoolEffect )

/* Virtual Method Table (VMT) for the class : 'Effects::BoolEffect' */
EW_DEFINE_CLASS( EffectsBoolEffect, XObject, "Effects::BoolEffect" )
EW_END_OF_CLASS( EffectsBoolEffect )

/* Initializer for the class 'Effects::EffectTimerClass' */
void EffectsEffectTimerClass__Init( EffectsEffectTimerClass _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  CoreTimer__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( EffectsEffectTimerClass );
}

/* Re-Initializer for the class 'Effects::EffectTimerClass' */
void EffectsEffectTimerClass__ReInit( EffectsEffectTimerClass _this )
{
  /* At first re-initialize the super class ... */
  CoreTimer__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Effects::EffectTimerClass' */
void EffectsEffectTimerClass__Done( EffectsEffectTimerClass _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( EffectsEffectTimerClass );

  /* Don't forget to deinitialize the super class ... */
  CoreTimer__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Effects::EffectTimerClass' */
void EffectsEffectTimerClass__Mark( EffectsEffectTimerClass _this )
{
  /* Give the super class a chance to mark its objects and references */
  CoreTimer__Mark( &_this->_Super );
}

/* The method Trigger() will be invoked when the timer is expired (when the interval 
   defined in @Begin or @Period is elapsed). The method can be overridden and implemented 
   in derived classes. The default implementation of this method sends a signal 
   to the slot method stored in the @OnTrigger property. */
void EffectsEffectTimerClass_Trigger( EffectsEffectTimerClass _this )
{
  CoreTimer_Trigger((CoreTimer)_this );
  EwNotifyObjObservers((XObject)_this, 0 );
}

/* Variants derived from the class : 'Effects::EffectTimerClass' */
EW_DEFINE_CLASS_VARIANTS( EffectsEffectTimerClass )
EW_END_OF_CLASS_VARIANTS( EffectsEffectTimerClass )

/* Virtual Method Table (VMT) for the class : 'Effects::EffectTimerClass' */
EW_DEFINE_CLASS( EffectsEffectTimerClass, CoreTimer, "Effects::EffectTimerClass" )
  EffectsEffectTimerClass_Trigger,
EW_END_OF_CLASS( EffectsEffectTimerClass )

/* User defined auto object: 'Effects::EffectTimer' */
EW_DEFINE_AUTOOBJECT( EffectsEffectTimer, EffectsEffectTimerClass )

/* Initializer for the auto object 'Effects::EffectTimer' */
void EffectsEffectTimer__Init( EffectsEffectTimerClass _this )
{
  CoreTimer_OnSetPeriod((CoreTimer)_this, 15 );
  CoreTimer_OnSetEnabled((CoreTimer)_this, 1 );
}

/* Table with links to derived variants of the auto object : 'Effects::EffectTimer' */
EW_DEFINE_AUTOOBJECT_VARIANTS( EffectsEffectTimer )
EW_END_OF_AUTOOBJECT_VARIANTS( EffectsEffectTimer )

/* Initializer for the class 'Effects::TimingList' */
void EffectsTimingList__Init( EffectsTimingList _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( EffectsTimingList );
}

/* Re-Initializer for the class 'Effects::TimingList' */
void EffectsTimingList__ReInit( EffectsTimingList _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Effects::TimingList' */
void EffectsTimingList__Done( EffectsTimingList _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( EffectsTimingList );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Effects::TimingList' */
void EffectsTimingList__Mark( EffectsTimingList _this )
{
  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* Variants derived from the class : 'Effects::TimingList' */
EW_DEFINE_CLASS_VARIANTS( EffectsTimingList )
EW_END_OF_CLASS_VARIANTS( EffectsTimingList )

/* Virtual Method Table (VMT) for the class : 'Effects::TimingList' */
EW_DEFINE_CLASS( EffectsTimingList, XObject, "Effects::TimingList" )
EW_END_OF_CLASS( EffectsTimingList )

/* Embedded Wizard */
