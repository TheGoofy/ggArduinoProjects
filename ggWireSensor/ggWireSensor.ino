const int mFieldX_Pin = A0;
const int mFieldY_Pin = A1;
const int mFieldZ_Pin = A2;

const int mPhaseXY_Pin = A4;
const int mPhaseYZ_Pin = A5;
const int mPhaseZX_Pin = A6;

int mSignX = 1;
int mSignY = 1;
int mSignZ = 1;

void setup() {
  Serial.begin(115200); 
}

void loop() {
  
  // read unsigned field vector
  int vFieldX = analogRead(mFieldX_Pin);
  int vFieldY = analogRead(mFieldY_Pin);
  int vFieldZ = analogRead(mFieldZ_Pin);
  
  // read phase difference (0 => 0°, 512 => 90°. 1023 => 180°)
  int vPhaseXY = analogRead(mPhaseXY_Pin);
  int vPhaseYZ = analogRead(mPhaseYZ_Pin);
  int vPhaseZX = analogRead(mPhaseZX_Pin);
  
  // calculate signs
  if (vFieldX > vFieldY) {
    if (vFieldX > vFieldZ) {
      // X is largest, keep X sign unchanged and adjust signs for Y and Z
      mSignY = vPhaseXY > 511 ? mSignX : -mSignX;
      mSignZ = vPhaseZX > 511 ? mSignX : -mSignX;
    }
    else {
      // Z is largest, keep Z sign unchanged and adjust signs for X and Y
      mSignX = vPhaseZX > 511 ? mSignZ : -mSignZ;
      mSignY = vPhaseYZ > 511 ? mSignZ : -mSignZ;
    }
  }
  else {
    if (vFieldY > vFieldZ) {
      // Y is largest, keep Y sign unchanged and adjust signs for X and Z
      mSignX = vPhaseXY > 511 ? mSignY : -mSignY;
      mSignZ = vPhaseYZ > 511 ? mSignY : -mSignY;
    }
    else {
      // Z is largest, keep Z sign unchanged and adjust signs for X and Y
      mSignX = vPhaseZX > 511 ? mSignZ : -mSignZ;
      mSignY = vPhaseYZ > 511 ? mSignZ : -mSignZ;
    }
  }
  
  // signed field vector
  vFieldX *= mSignX;
  vFieldY *= mSignY;
  vFieldZ *= mSignZ;
  
  // output
  Serial.print(vFieldX);
  Serial.print(" ");
  Serial.print(vFieldY);
  Serial.print(" ");
  Serial.print(vFieldZ);
  Serial.println();

/*
  Serial.print("Phase ");
  Serial.print(vPhaseXY);
  Serial.print(" ");
  Serial.print(vPhaseYZ);
  Serial.print(" ");
  Serial.print(vPhaseZX);
  Serial.println();
*/
  delay(10);
}
