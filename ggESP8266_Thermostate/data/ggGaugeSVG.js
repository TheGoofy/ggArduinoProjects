"use strict"


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


class ggGaugeSVG {

  // public properties
  mBackgroundColor = "transparent";
  mFaceStyle = "white";
  mLabelStyle = "black";
  mIndicatorStyle = "#c00"; // drak red
  mIndicatorWidth = 5;
  mRange = { mMin: -20, mMax: 40 };
  mDecimals = 1;
  mLabel = "°C";
  mLabelFont = "13px arial"; // "bold 14px Lucida Console";
  mTicksFont = "8px arial";
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
  mValue = null;
  mDiameter = null;
  mSvg = null;
  mSvgNS = "http://www.w3.org/2000/svg";
  mValueSvg = null;
  mIndicatorSvg = null;

  constructor(aSVG, aValue) {
    this.mSvg = aSVG;
    this.mValue = aValue ? aValue : (this.mRange.mMin + this.mRange.mMax) / 2;
  }

  set Value(aValue) {
    let vValueOld = this.mValue;
    if (this.mValue != aValue) {
      this.mValue = aValue;
    }
    if (this.mValueSvg) {
      this.mValueSvg.innerHTML = aValue.toFixed(this.mDecimals);
    }
    if (this.mIndicatorSvg) {
      let vAngleA = vValueOld ? this.GetAngle(vValueOld) : this.GetAngle(this.ValueLimited);
      let vAngleB = this.GetAngle(this.ValueLimited);
      this.AnimateElementRotation(this.mIndicatorSvg, this.Center, vAngleA, vAngleB);
    }
  }

  get Value() {
    return this.mValue;
  }

  get ValueLimited() {
    return Math.max(this.mRange.mMin, Math.min(this.mValue, this.mRange.mMax));
  }

  set Diameter(aDiameter) {
    if (this.mDiameter != aDiameter) {
      this.mDiameter = aDiameter;
      this.Draw();
    }
  }

  get Diameter() {
    let vWidth = this.mSvg.clientWidth;
    let vHeight = this.mSvg.clientHeight;
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
    return new ggVector(this.mSvg.clientLeft + this.mSvg.clientWidth / 2,
                        this.mSvg.clientTop + this.mSvg.clientHeight / 2);
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

  GetTextAnchor(aAngle) {
    const vX = Math.cos(aAngle);
    if (vX <= -0.5) return "start";
    if (vX < 0.5) return "middle";
    return "end";
  }

  GetAlignmentBaseline(aAngle) {
    const vY = Math.sin(aAngle);
    if (vY <= -0.5) return "hanging";
    if (vY < 0.5) return "middle";
    return "alphabetic";
  }

  CreateGroup(aParent) {
    let vGroup = document.createElementNS(this.mSvgNS, "g");
    if (aParent) aParent.appendChild(vGroup);
    return vGroup;
  }

  CreateLine(aPointA, aPointB) {
    let vLine = document.createElementNS(this.mSvgNS, "line");
    vLine.setAttributeNS(null, "x1", aPointA.mX);
    vLine.setAttributeNS(null, "y1", aPointA.mY);
    vLine.setAttributeNS(null, "x2", aPointB.mX);
    vLine.setAttributeNS(null, "y2", aPointB.mY);
    return vLine;
  }

  CreateCircle(aCenter, aRadius) {
    let vCircle = document.createElementNS(this.mSvgNS, "circle");
    vCircle.setAttributeNS(null, "cx", aCenter.mX);
    vCircle.setAttributeNS(null, "cy", aCenter.mY);
    vCircle.setAttributeNS(null, "r", aRadius);
    return vCircle;
  }

  CreateArc(aCenter, aRadius, aAngleA, aAngleB) {
    let vArc = document.createElementNS(this.mSvgNS, "path");
    let vPosAX = aCenter.mX + aRadius * Math.cos(aAngleA);
    let vPosAY = aCenter.mY + aRadius * Math.sin(aAngleA);
    let vPosBX = aCenter.mX + aRadius * Math.cos(aAngleB);
    let vPosBY = aCenter.mY + aRadius * Math.sin(aAngleB);
    let vRotation = "0"; // angle in degree (only matters for elliptic arc)
    let vLargeArcFlag = Math.abs(aAngleB - aAngleA) % (2 * Math.PI) > Math.PI ? "1" : "0";
    let vSweepFlag = aAngleA < aAngleB ? "1" : "0";
    vArc.setAttributeNS(null, "d", `M ${vPosAX} ${vPosAY} ` +
                                   `A ${aRadius} ${aRadius} ${vRotation} ${vLargeArcFlag} ${vSweepFlag} ${vPosBX} ${vPosBY}`);
    return vArc;
  }

  CreateText(aPosition, aText) {
    let vText = document.createElementNS(this.mSvgNS, "text");
    vText.setAttributeNS(null, "x", aPosition.mX);
    vText.setAttributeNS(null, "y", aPosition.mY);
    vText.innerHTML = aText;
    return vText;
  }

  SetStyle(aObject, aStyle) {
    aObject.setAttributeNS(null, "style", aStyle);
  }

  AnimateElementRotation(aElement, aCenter, aAngleA, aAngleB) {
    let vAngleDegA = 180 * aAngleA / Math.PI;
    let vAngleDegB = 180 * aAngleB / Math.PI;
    aElement.setAttributeNS(null, "transform", `rotate(${vAngleDegA}, ${aCenter.mX}, ${aCenter.mY})`);
    let vAnimateTransform = `<animateTransform 
      attributeName='transform' type='rotate' 
      to='${vAngleDegB} ${aCenter.mX} ${aCenter.mY}'
      dur='500ms' fill='freeze'/>`;
    aElement.innerHTML = vAnimateTransform;
    aElement.firstChild.beginElement();
  }

  Draw() {

    // center and radius are relevant for all parts ...
    const vCenter = this.Center;
    const vRadius = this.Radius;

    // main container
    let vGroupMain = this.CreateGroup();

    // shadow-filter (for indicator)
    const vShadowFilterName = "ShadowFilter";
    vGroupMain.innerHTML +=
      `<filter id='${vShadowFilterName}' x='-2' y='-2' width='4' height='4'>
         <feDropShadow dx='0.8' dy='1.2' stdDeviation='1' flood-opacity='0.5' flood-color='black'/>
       </filter>`;

    // gauge background
    let vCircle = this.CreateCircle(vCenter, vRadius);
    this.SetStyle(vCircle, `fill: ${this.mFaceStyle}; stroke-width: 0;`)
    vGroupMain.appendChild(vCircle);

    // tick highlights
    let vGroupTickHighlights = this.CreateGroup();
    vGroupMain.appendChild(vGroupTickHighlights);
    const vTicksLength = this.mTicksSize * (vRadius - this.mTicksMargin);
    const vHighlightRadius = vRadius - this.mTicksMargin - vTicksLength / 2;
    this.SetStyle(vGroupTickHighlights, `stroke-width: ${vTicksLength};`);
    for (let vHighlight of this.mTicksHighlights) {
      let vArc = this.CreateArc(vCenter, vHighlightRadius,
                                this.GetAngle(vHighlight.mRange.mMin),
                                this.GetAngle(vHighlight.mRange.mMax));
      this.SetStyle(vArc, `stroke: ${vHighlight.mStyle}; fill: none;`);
      vGroupTickHighlights.appendChild(vArc);
    }

    // major and minor tickmarks
    let vGroupTickMarksMajor = this.CreateGroup(vGroupMain);
    let vGroupTickMarksMinor = this.CreateGroup(vGroupMain);
    let vGroupTickMarksLabels = this.CreateGroup(vGroupMain);
    this.SetStyle(vGroupTickMarksMajor, `stroke: ${this.mLabelStyle}; stroke-width: ${this.mTicksMajorWidth};`);
    this.SetStyle(vGroupTickMarksMinor, `stroke: ${this.mLabelStyle}; stroke-width: ${this.mTicksMinorWidth};`);
    this.SetStyle(vGroupTickMarksLabels, `font: ${this.mTicksFont}; fill: ${this.mLabelStyle}; stroke: none; stroke-width: 0;`);
    const vTickRadiusA = vRadius - this.mTicksMargin;
    const vTickRadiusB = vTickRadiusA - vTicksLength;
    const vTickRadiusT = vTickRadiusB - 0.7 * this.mTicksMargin;
    let vTicksMajor = this.GetTicksMajor();
    for (let vTickIndexMajor = 0; vTickIndexMajor < vTicksMajor.length; vTickIndexMajor++) {
      let vTickMajor = vTicksMajor[vTickIndexMajor];
      let vAngleMajor = this.GetAngle(vTickMajor);
      let vPositionMajorA = this.GetPosition(vAngleMajor, vTickRadiusA);
      let vPositionMajorB = this.GetPosition(vAngleMajor, vTickRadiusB);
      let vPositionMajorLabel = this.GetPosition(vAngleMajor, vTickRadiusT);
      // major tickmark line
      vGroupTickMarksMajor.appendChild(this.CreateLine(vPositionMajorA, vPositionMajorB));
      // major tickmark label text
      let vTickMajorLabel = this.CreateText(vPositionMajorLabel, vTickMajor);
      let vTextAnchor = this.GetTextAnchor(vAngleMajor);
      let vAlignmentBaseline = this.GetAlignmentBaseline(vAngleMajor);
      this.SetStyle(vTickMajorLabel, `text-anchor: ${vTextAnchor}; alignment-baseline: ${vAlignmentBaseline};`);
      vGroupTickMarksLabels.appendChild(vTickMajorLabel);
      // minor tickmarks
      if (vTickIndexMajor + 1 < vTicksMajor.length) {
        let vTicksMinor = this.GetTicksMinor(vTicksMajor[vTickIndexMajor], vTicksMajor[vTickIndexMajor + 1]);
        for (let vTickIndexMinor = 0; vTickIndexMinor < vTicksMinor.length; vTickIndexMinor++) {
          let vTickMinor = vTicksMinor[vTickIndexMinor];
          let vAngleMinor = this.GetAngle(vTickMinor);
          let vPositionMinorA = this.GetPosition(vAngleMinor, vTickRadiusA);
          let vPositionMinorB = this.GetPosition(vAngleMinor, vTickRadiusA - vTicksLength / 3);
          vGroupTickMarksMinor.appendChild(this.CreateLine(vPositionMinorA, vPositionMinorB));
        }
      }
    }

    // draw value (and label)
    let vGroupLabelAndValue = this.CreateGroup(vGroupMain);
    vGroupLabelAndValue.setAttributeNS(null, "filter", `url(#${vShadowFilterName})`);
    let vLabelAndValueStyle = `font: ${this.mLabelFont}; fill: ${this.mLabelStyle}; stroke: none; stroke-width: 0; ` + 
                              `text-anchor: middle; alignment-baseline: middle;`;
    let vLabelPosition = new ggVector(vCenter.mX, vCenter.mY + 0.75 * vTickRadiusA - 0.12 * vRadius);
    let vMainLabelText = vGroupLabelAndValue.appendChild(this.CreateText(vLabelPosition, this.mLabel));
    this.SetStyle(vMainLabelText, vLabelAndValueStyle);
    let vValuePosition = new ggVector(vCenter.mX, vCenter.mY + 0.75 * vTickRadiusA + 0.12 * vRadius);
    this.mValueSvg = vGroupLabelAndValue.appendChild(this.CreateText(vValuePosition, this.mValue.toFixed(this.mDecimals)));
    this.SetStyle(this.mValueSvg, vLabelAndValueStyle);

    // draw indicator
    let vGroupIndicator = this.CreateGroup(vGroupMain);
    this.SetStyle(vGroupIndicator, `stroke: ${this.mIndicatorStyle}; stroke-width: ${this.mIndicatorWidth}; fill: ${this.mIndicatorStyle};`);
    vGroupIndicator.setAttributeNS(null, "filter", `url(#${vShadowFilterName})`);
    vGroupIndicator.appendChild(this.CreateCircle(vCenter, (vTicksLength - this.mIndicatorWidth) / 2));
    let vPositionIndicatorA = this.GetPosition(0, vTickRadiusA - vTicksLength / 3);
    let vPositionIndicatorB = this.GetPosition(0, -vTicksLength);
    this.mIndicatorSvg = vGroupIndicator.appendChild(this.CreateLine(vPositionIndicatorA, vPositionIndicatorB));
    this.AnimateElementRotation(this.mIndicatorSvg, vCenter, this.GetAngle(this.ValueLimited), this.GetAngle(this.ValueLimited));

    // clear old graphics
    this.mSvg.innerHTML = "";

    // add new graphics
    this.mSvg.appendChild(vGroupMain);
  }
}