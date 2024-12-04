
#include "Core.h"
#include "Effects.h"
#include "Resources.h"

/* 'C' function for method : 'Core::Group.OnGetEmbedded()' */
XBool CoreGroup_OnGetEmbedded( CoreGroup _this )
{
  return (( _this->Super2.viewState & CoreViewStateEmbedded ) == CoreViewStateEmbedded 
    );
}



/* 'C' function for method : 'Core::Group.OnGetEnabled()' */
XBool CoreGroup_OnGetEnabled( CoreGroup _this )
{
  return (( _this->Super2.viewState & CoreViewStateEnabled ) == CoreViewStateEnabled 
    );
}


/* 'C' function for method : 'Core::Group.OnSetAlphaBlended()' */
void CoreGroup_OnSetAlphaBlended( CoreGroup _this, XBool value )
{
  if ( value )
    CoreView__ChangeViewState( _this, CoreViewStateAlphaBlended, 0 );
  else
    CoreView__ChangeViewState( _this, 0, CoreViewStateAlphaBlended );
}


/* 'C' function for method : 'Core::Group.OnSetEmbedded()' */
void CoreGroup_OnSetEmbedded( CoreGroup _this, XBool value )
{
  if ( value )
    CoreView__ChangeViewState( _this, CoreViewStateEmbedded, 0 );
  else
    CoreView__ChangeViewState( _this, 0, CoreViewStateEmbedded );
}


/* 'C' function for method : 'Core::Outline.OnSetFormation()' */
void CoreOutline_OnSetFormation( CoreOutline _this, XEnum value )
{
  if ( value == _this->Formation )
    return;

  _this->Formation = value;

  if ( _this->Super2.Owner != 0 )
  {
    _this->Super2.viewState = _this->Super2.viewState | CoreViewStateUpdateLayout;
    _this->Super2.Owner->Super2.viewState = _this->Super2.Owner->Super2.viewState 
    | CoreViewStatePendingLayout;
    EwPostSignal( EwNewSlot( _this->Super2.Owner, CoreGroup_updateComponent ), ((XObject)_this 
      ));
  }
}

