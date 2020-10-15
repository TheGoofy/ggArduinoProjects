'use strict';


class ggVector {

  mX;
  mY;

  constructor(aX, aY) {
    this.mX = aX ? aX : 0;
    this.mY = aY ? aY : 0;
  }

  get Length() {
    return Math.hypot(this.mX, this.mY);
  }

  Add(aOther) {
    return new ggVector(this.mX + aOther.mX, this.mY + aOther.mY);
  }

  static Add(aVectorA, aVectorB) {
    return new ggVector(aVectorA.mX + aVectorB.mX, aVectorA.mY + aVectorB.mY);
  }
}


class ggTransition {

  mValueStart = 0;
  mValueEnd = 0;
  mValueDelta = 0;

  mTimeStart;
  mTimeEnd;
  mTimeDeltaMS = 500;

  constructor(aValue, aTimeDeltaMS) {
    if (aValue) {
      this.mValueStart = aValue;
      this.mValueEnd = aValue;
    }
    if (aTimeDeltaMS) {
      this.mTimeDeltaMS = aTimeDeltaMS;
    }
  }

  set TimeDeltaMS(aTimeDeltaMS) {
    this.mTimeDeltaMS = aTimeDeltaMS;
    if (!this.Finished) {
      this.mTimeEnd = this.mTimeStart + this.mTimeDeltaMS;
    }
  }

  get TimeDeltaMS() {
    return this.mTimeDeltaMS;
  }

  set Value(aValue) {
    if (aValue != this.mValueEnd) {
      this.mValueStart = this.Value;
      this.mValueEnd = aValue;
      this.mValueDelta = this.mValueEnd - this.mValueStart;
      this.mTimeStart = new Date();
      this.mTimeEnd = this.mTimeStart + this.mTimeDeltaMS;
    }
  }

  get Value() {
    if (this.Finished) return this.mValueEnd;
    let vTime = new Date();
    let vTimePassedMS = vTime - this.mTimeStart;
    if (vTimePassedMS && vTimePassedMS > this.mTimeDeltaMS) {
      this.mTimeStart = this.mTimeEnd;
      return this.mValueEnd;
    }
    let vT = vTimePassedMS / this.mTimeDeltaMS;
    let vValue = this.mValueStart + vT * this.mValueDelta;
    return vValue;
  }

  get ValueStart() {
    return this.mValueStart;
  }

  get ValueEnd() {
    return this.mValueEnd;
  }

  get Finished() {
    return (this.mTimeStart == this.mTimeEnd) || (this.mValueStart == this.mValueEnd);
  }
}


class ggGauge {

  // public properties
  mBackgroundColor = "transparent";
  mFaceStyle = "white";
  mLabelStyle = "black";
  mIndicatorStyle = "#c00"; // drak red
  mIndicatorWidth = 5;
  mRange = { mMin: -20, mMax: 40 };
  mDecimals = 1;
  mLabel = "°C";
  mLabelFont = "13px Arial";
  mTicksFont = "9px Arial";
  mTicksMargin = 3; // pixel
  mTicksSize = 0.25; // percent of radius
  mTicksMajor = [-20, -10, 0, 10, 20, 30, 40];
  mTicksMajorWidth = 1.5;
  mTicksMinor = 2;
  mTicksMinorWidth = 0.6;
  mTicksHighlights = [
    { mRange: { mMin: -20, mMax: 0 }, mStyle: "#8cf" },
    { mRange: { mMin: 20, mMax: 30 }, mStyle: "#fc8" },
    { mRange: { mMin: 30, mMax: 40 }, mStyle: "#e96" },
  ];

  // "private" members
  mValue;
  mDiameter;
  mContext;

  constructor(aCanvas, aValue) {
    this.mContext = aCanvas.getContext("2d");
    let vX = this.mContext.canvas.width / 2;
    let vY = this.mContext.canvas.height / 2;
    this.mContext.translate(vX, vY);
    let vScaleX = this.mContext.canvas.width / this.mContext.canvas.clientWidth;
    let vScaleY = this.mContext.canvas.height / this.mContext.canvas.clientHeight;
    this.mContext.scale(vScaleX, vScaleY);
    this.mValue = new ggTransition(aValue ? aValue : (this.mRange.mMin + this.mRange.mMax) / 2, 300);
  }

  set Value(aValue) {
    if (this.mValue.ValueEnd != aValue) {
      this.mValue.Value = aValue;
      this.Draw();
    }
  }

  get Value() {
    return this.mValue.Value;
  }

  set Diameter(aDiameter) {
    if (this.mDiameter != aDiameter) {
      this.mDiameter = aDiameter;
      this.Draw();
    }
  }

  get Diameter() {
    let vWidth = this.mContext.canvas.clientWidth;
    let vHeight = this.mContext.canvas.clientHeight;
    if (this.mDiameter) {
      return Math.min(this.mDiameter, vWidth, vHeight);
    }
    else {
      return Math.min(vWidth, vHeight);
    }
  }

  set Radius(aRadius) {
    this.Diameter = 2 * aRadius;
  }

  get Radius() {
    return this.Diameter / 2;
  }

  get Center() {
    return new ggVector(0, 0);
  }

  GetAngle(aValue) {
    const vAngleStart = 0.75 * Math.PI;
    const vAngleEnd = 2.25 * Math.PI;
    const vAngleRange = vAngleEnd - vAngleStart;
    const vValueRange = this.mRange.mMax - this.mRange.mMin;
    return vAngleStart + vAngleRange * (aValue - this.mRange.mMin) / vValueRange;
  }

  GetPosition(aAngle, aRadius) {
    let vCenter = this.Center;
    let vX = vCenter.mX + aRadius * Math.cos(aAngle);
    let vY = vCenter.mY + aRadius * Math.sin(aAngle);
    return new ggVector(vX, vY);
  }

  GetTicksMajor() {
    if (typeof this.mTicksMajor == "number") {
      const vRange = this.mRange.mMax - this.mRange.mMin;
      const vDelta = vRange / (this.mTicksMajor - 1);
      let vTicks = [];      
      for (let vIndex = 0; vIndex < this.mTicksMajor; vIndex++) {
        vTicks[vIndex] = this.mRange.mMin + vIndex * vDelta;
      }
      return vTicks;
    }
    else {
      return this.mTicksMajor;
    }
  }

  GetTicksMinor(aTickMajorA, aTickMajorB) {
    const vRange = aTickMajorB - aTickMajorA;
    const vDelta = vRange / this.mTicksMinor;
    let vTicks = [];
    for (let vIndex = 0; vIndex + 1 < this.mTicksMinor; vIndex++) {
      vTicks[vIndex] = aTickMajorA + (vIndex + 1) * vDelta;
    }
    return vTicks;
  }

  SetTextAlignment(aAngle) {
    const vX = Math.cos(aAngle);
    if (vX <= -0.5) this.mContext.textAlign = "left";
    else if (vX < 0.5) this.mContext.textAlign = "center";
    else this.mContext.textAlign = "right";
    const vY = Math.sin(aAngle);
    if (vY <= -0.5) this.mContext.textBaseline = "hanging";
    else if (vY < 0.5) this.mContext.textBaseline = "middle";
    else this.mContext.textBaseline = "alphabetic";
  }

  Draw() {

    const vCenter = this.Center;
    const vRadius = this.Radius;

    // draw the gauge background
    this.mContext.clearRect(vCenter.mX - vRadius, vCenter.mY - vRadius, 2 * vRadius, 2 * vRadius);
    this.mContext.beginPath();
    this.mContext.arc(vCenter.mX, vCenter.mY, vRadius, 0, 2*Math.PI);
    this.mContext.fillStyle = this.mFaceStyle;
    this.mContext.fill();

    // draw tick highlights
    const vTicksLength = this.mTicksSize * (vRadius - this.mTicksMargin);
    const vHighlightRadius = vRadius - this.mTicksMargin - vTicksLength / 2;
    this.mContext.lineWidth = vTicksLength;
    for (let vHighlight of this.mTicksHighlights) {
      this.mContext.beginPath();
      this.mContext.arc(vCenter.mX, vCenter.mY, vHighlightRadius,
                         this.GetAngle(vHighlight.mRange.mMin),
                         this.GetAngle(vHighlight.mRange.mMax));
      this.mContext.strokeStyle = vHighlight.mStyle;
      this.mContext.stroke();
    }

    // draw major and minor tickmarks
    const vTickRadiusA = vRadius - this.mTicksMargin;
    const vTickRadiusB = vTickRadiusA - vTicksLength;
    const vTickRadiusT = vTickRadiusB - 0.7 * this.mTicksMargin;
    this.mContext.strokeStyle = this.mLabelStyle;
    this.mContext.fillStyle = this.mLabelStyle;
    this.mContext.font = this.mTicksFont;
    let vTicksMajor = this.GetTicksMajor();
    for (let vTickIndexMajor = 0; vTickIndexMajor < vTicksMajor.length; vTickIndexMajor++) {
      let vTickMajor = vTicksMajor[vTickIndexMajor];
      let vAngleMajor = this.GetAngle(vTickMajor);
      let vPositionMajorA = this.GetPosition(vAngleMajor, vTickRadiusA);
      let vPositionMajorB = this.GetPosition(vAngleMajor, vTickRadiusB);
      let vPositionMajorLabel = this.GetPosition(vAngleMajor, vTickRadiusT);
      // draw major tickmark line
      this.mContext.lineWidth = this.mTicksMajorWidth;
      this.mContext.beginPath();
      this.mContext.moveTo(vPositionMajorA.mX, vPositionMajorA.mY);
      this.mContext.lineTo(vPositionMajorB.mX, vPositionMajorB.mY);
      this.mContext.stroke();
      // draw major tickmark label text
      this.SetTextAlignment(vAngleMajor);
      this.mContext.fillText(vTickMajor, vPositionMajorLabel.mX, vPositionMajorLabel.mY);
      // draw minor tickmarks
      if (vTickIndexMajor + 1 < vTicksMajor.length) {
        this.mContext.lineWidth = this.mTicksMinorWidth;
        let vTicksMinor = this.GetTicksMinor(vTicksMajor[vTickIndexMajor], vTicksMajor[vTickIndexMajor + 1]);
        for (let vTickIndexMinor = 0; vTickIndexMinor < vTicksMinor.length; vTickIndexMinor++) {
          let vTickMinor = vTicksMinor[vTickIndexMinor];
          let vAngleMinor = this.GetAngle(vTickMinor);
          let vPositionMinorA = this.GetPosition(vAngleMinor, vTickRadiusA);
          let vPositionMinorB = this.GetPosition(vAngleMinor, vTickRadiusA - vTicksLength / 3);
          this.mContext.beginPath();
          this.mContext.moveTo(vPositionMinorA.mX, vPositionMinorA.mY);
          this.mContext.lineTo(vPositionMinorB.mX, vPositionMinorB.mY);
          this.mContext.stroke();
        }
      }
    }

    // "mValue" has a transition (changes over time)
    let vValue = this.mValue.Value;

    // draw value (and label)
    let vLabelPosition = new ggVector(vCenter.mX, vCenter.mY + 0.75 * vTickRadiusA);
    this.mContext.textAlign = "center";
    this.mContext.textBaseline = "middle";
    this.mContext.font = this.mLabelFont;
    this.mContext.fillText(this.mLabel, vLabelPosition.mX, vLabelPosition.mY - 0.12 * vRadius);
    this.mContext.fillText(vValue.toFixed(this.mDecimals), vLabelPosition.mX, vLabelPosition.mY + 0.12 * vRadius);

    // draw indicator
    this.mContext.strokeStyle = this.mIndicatorStyle;
    this.mContext.lineWidth = this.mIndicatorWidth;
    this.mContext.fillStyle = this.mIndicatorStyle;
    this.mContext.shadowBlur = 0.05 * vRadius;
    this.mContext.shadowOffsetX = this.mIndicatorWidth / 2;
    this.mContext.shadowOffsetY = this.mIndicatorWidth / 2;
    this.mContext.shadowColor = "rgba(0, 0, 0, 0.3)";
    let vAngleIndicator = this.GetAngle(Math.max(this.mRange.mMin, Math.min(vValue, this.mRange.mMax)));
    let vPositionIndicatorA = this.GetPosition(vAngleIndicator, vTickRadiusA - vTicksLength / 3);
    let vPositionIndicatorB = this.GetPosition(vAngleIndicator, -vTicksLength);
    this.mContext.beginPath();
    this.mContext.moveTo(vPositionIndicatorA.mX, vPositionIndicatorA.mY);
    this.mContext.lineTo(vPositionIndicatorB.mX, vPositionIndicatorB.mY);
    this.mContext.stroke();
    this.mContext.beginPath();
    this.mContext.arc(vCenter.mX, vCenter.mY, vTicksLength / 2, 0, 2*Math.PI);
    this.mContext.fill();
    this.mContext.shadowColor = "transparent";

    // redraw, if animation not yet finished
    if (!this.mValue.Finished) {
      let vGauge = this;
      requestAnimationFrame(function() {
        vGauge.Draw();
      });
    }
  }

}
