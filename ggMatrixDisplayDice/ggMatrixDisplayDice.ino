#include "ggMatrixDisplay.h"

const byte mCols = 8;
const byte mRows = 8;
const byte mColPins[mCols] = {4,A3,6,A2,11,7,13,8};
const byte mRowPins[mRows] = {12,A4,A5,9,5,10,A0,A1};

const byte mPixmap1[8] = {B00000000, B00000000, B00000000, B00011000, B00011000, B00000000, B00000000, B00000000};
const byte mPixmap2[8] = {B11000000, B11000000, B00000000, B00000000, B00000000, B00000000, B00000011, B00000011};
const byte mPixmap3[8] = {B11000000, B11000000, B00000000, B00011000, B00011000, B00000000, B00000011, B00000011};
const byte mPixmap4[8] = {B11000011, B11000011, B00000000, B00000000, B00000000, B00000000, B11000011, B11000011};
const byte mPixmap5[8] = {B11000011, B11000011, B00000000, B00011000, B00011000, B00000000, B11000011, B11000011};
const byte mPixmap6[8] = {B11000011, B11000011, B00000000, B11000011, B11000011, B00000000, B11000011, B11000011};

ggMatrixDisplay mDisplay(mCols, mColPins,
                         mRows, mRowPins);

void ZeigeAugen(byte aZahl)
{
  switch (aZahl) {
    case 1: mDisplay.SetPixmap(mPixmap1); break;
    case 2: mDisplay.SetPixmap(mPixmap2); break;
    case 3: mDisplay.SetPixmap(mPixmap3); break;
    case 4: mDisplay.SetPixmap(mPixmap4); break;
    case 5: mDisplay.SetPixmap(mPixmap5); break;
    case 6: mDisplay.SetPixmap(mPixmap6); break;
    default: break;
  }
}

byte ZufallsZahl()
{
  // zufällige Zahl zwischen 1 und 6 berechnen
  byte vZahl = random(6) + 1;
  // berechnete Zahl zurückgeben
  return vZahl;
}

void TasterVorbereiten()
{
  // Pin als Eingang verwenden
  pinMode(2, INPUT);
  // internen Pull-Up-Widerstand einschalten
  digitalWrite(2, HIGH);
}

boolean TasterIstGedrueckt()
{
  // Wert vom Eingangs-Pin lesen
  byte vEingang = digitalRead(2);
  // Wenn die Taste gedrueckt ist, dann ist der Einganswert tief (LOW), ...
  if (vEingang == LOW) return true;
  // ... und wenn der Eingangswert nicht tief ist, dann ist die Taste nicht gedrueckt.
  return false;
}

long mZeitBeiWurf = 0;
long mDauerBisZurNaechstenZahl = 20;
long mZeitBeiLetzterZahl = 0;

byte mAugenZahl = 1;

void WuerfelWerfen()
{
  mZeitBeiWurf = millis();
  mDauerBisZurNaechstenZahl = 20;
  mAugenZahl = (mZeitBeiWurf % 6) + 1;
}

boolean WuerfelRollt()
{
  long vZeitJetzt = millis();
  long vDauerSeitWurf = vZeitJetzt - mZeitBeiWurf;
  return vDauerSeitWurf < 3000;
}

boolean WuerfelDarfZurNaechstenZahlRollen()
{
  long vZeitJetzt = millis();
  long vDauerSeitLetzerZahl = vZeitJetzt - mZeitBeiLetzterZahl;
  if (vDauerSeitLetzerZahl > mDauerBisZurNaechstenZahl) {
    mZeitBeiLetzterZahl = vZeitJetzt;
    mDauerBisZurNaechstenZahl *= 1.15f;
    return true;
  }
  else {
    return false;
  }
}

void RolleWuerfelZurNaechstenZahl()
{
  if (mAugenZahl < 6) {
    mAugenZahl++;
  }
  else {
    mAugenZahl = 1;
  }
}

void setup()
{
  TasterVorbereiten();
  mDisplay.begin();
}

void loop()
{
  if (TasterIstGedrueckt()) {
    WuerfelWerfen();
  }
  
  if (WuerfelRollt()) {
    if (WuerfelDarfZurNaechstenZahlRollen()) {
      RolleWuerfelZurNaechstenZahl();
    }
  }
  
  ZeigeAugen(mAugenZahl);
  
  // rendering (call this as often as possible)
  mDisplay.run();  
}

