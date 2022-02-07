#pragma once

class ggCoilStage
{
public:

  ggCoilStage(int aPinProjectileSen,
              int aPinCoilFire)
  : mProjectileSen(aPinProjectileSen, true/*aInverted*/, true/*aEnablePullUp*/),
    mCoilFire(aPinCoilFire, true/*aInverted*/) {
  }

  void Begin() {
    mProjectileSen.Begin();
    mCoilFire.Begin();
  }

  ggInput mProjectileSen;
  ggOutput mCoilFire;

private:
  
};
