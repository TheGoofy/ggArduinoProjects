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

  toString() {
    return `${this.mX},${this.mY}`;
  }

  static Add(aVectorA, aVectorB) {
    if (typeof aVectorA === "undefined") return aVectorB;
    if (typeof aVectorB === "undefined") return aVectorA;
    return new ggVector(aVectorA.mX + aVectorB.mX, aVectorA.mY + aVectorB.mY);
  }
}


class ggSvg {

  static mSvgNS = "http://www.w3.org/2000/svg";

  static CreateGroup(aParent) {
    let vGroup = document.createElementNS(this.mSvgNS, "g");
    if (aParent) aParent.appendChild(vGroup);
    return vGroup;
  }

  static CreateLine(aPointA, aPointB) {
    let vLine = document.createElementNS(this.mSvgNS, "line");
    vLine.setAttributeNS(null, "x1", aPointA.mX);
    vLine.setAttributeNS(null, "y1", aPointA.mY);
    vLine.setAttributeNS(null, "x2", aPointB.mX);
    vLine.setAttributeNS(null, "y2", aPointB.mY);
    return vLine;
  }

  static CreatePolygon(aPoints) {
    let vPolygon = document.createElementNS(this.mSvgNS, "polygon");
    let vPointsString = aPoints.join(" ");
    vPolygon.setAttributeNS(null, "points", vPointsString);
    return vPolygon;
  }

  static CreateCircle(aCenter, aRadius) {
    let vCircle = document.createElementNS(this.mSvgNS, "circle");
    vCircle.setAttributeNS(null, "cx", aCenter.mX);
    vCircle.setAttributeNS(null, "cy", aCenter.mY);
    vCircle.setAttributeNS(null, "r", aRadius);
    return vCircle;
  }

  static CreateArc(aCenter, aRadius, aAngleA, aAngleB) {
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

  static CreateText(aPosition, aText) {
    let vText = document.createElementNS(this.mSvgNS, "text");
    vText.setAttributeNS(null, "x", aPosition.mX);
    vText.setAttributeNS(null, "y", aPosition.mY);
    vText.innerHTML = aText;
    return vText;
  }

  static GetTextAnchor(aAngle) {
    const vX = Math.cos(aAngle);
    if (vX <= -0.5) return "start";
    if (vX < 0.5) return "middle";
    return "end";
  }

  static GetAlignmentBaseline(aAngle) {
    const vY = Math.sin(aAngle);
    if (vY <= -0.5) return "hanging";
    if (vY < 0.5) return "middle";
    return "alphabetic";
  }

  static SetStyle(aSvgObject, aStyle) {
    aSvgObject.setAttributeNS(null, "style", aStyle);
  }

  static SetRotation(aSvgObject, aCenter, aAngle) {
    let vAngleDeg = 180 * aAngle / Math.PI;
    aSvgObject.setAttributeNS(null, "transform", `rotate(${vAngleDeg}, ${aCenter.mX}, ${aCenter.mY})`);
  }

  static AnimateRotation(aSvgObject, aCenter, aAngleA, aAngleB, aDuration = "1s", aMinimizeRotationAngle = false) {
    let vAngleDegA = 180 * aAngleA / Math.PI;
    let vAngleDegB = 180 * aAngleB / Math.PI;
    if (aMinimizeRotationAngle) {
      let vAngleDegDelta = vAngleDegB - vAngleDegA;
      if (vAngleDegDelta > 180) vAngleDegA += 360;
      else if (vAngleDegDelta < -180) vAngleDegA -= 360;
    }
    this.SetRotation(aSvgObject, aCenter, aAngleA);
    let vSvgAnimateTransform = document.createElementNS(this.mSvgNS, "animateTransform");
    vSvgAnimateTransform.setAttributeNS(null, "attributeName", "transform");
    vSvgAnimateTransform.setAttributeNS(null, "type", "rotate");
    vSvgAnimateTransform.setAttributeNS(null, "fill", "freeze");
    vSvgAnimateTransform.setAttributeNS(null, "to", `${vAngleDegB} ${aCenter.mX} ${aCenter.mY}`);
    vSvgAnimateTransform.setAttributeNS(null, "dur", `${aDuration}`);
    let vSvgAnimateTransformsExisting = aSvgObject.getElementsByTagName(vSvgAnimateTransform.nodeName);
    if (vSvgAnimateTransformsExisting.length > 0) vSvgAnimateTransformsExisting[0].replaceWith(vSvgAnimateTransform);
    else aSvgObject.appendChild(vSvgAnimateTransform);
    vSvgAnimateTransform.beginElement();
  }
}


class ggGaugeSVG {

  // public properties
  mFaceStyle = "white";
  mLabelStyle = "black";
  mIndicatorStyle = "#c00"; // drak red
  mIndicatorWidth = 0.1;
  mRange = { mMin: -20, mMax: 40 };
  mDecimals = 1;
  mLabel = "°C";
  mLabelFont = "13px arial"; // "bold 14px Lucida Console";
  mTicksFont = "9.5px arial";
  mTicksMargin = 3; // pixel
  mTicksSize = 0.2; // percent of radius
  mTicksMajor = [-20, -10, 0, 10, 20, 30, 40];
  mTicksMajorWidth = 1.5;
  mTicksMinor = 10;
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
    this.Draw();
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
      ggSvg.AnimateRotation(this.mIndicatorSvg, this.Center, vAngleA, vAngleB, 0.5);
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

  Draw() {

    // center and radius are relevant for all parts ...
    const vCenter = this.Center;
    const vRadius = this.Radius;

    // main container
    let vGroupMain = ggSvg.CreateGroup();

    // shadow-filter (for indicator)
    const vShadowFilterName = this.mSvg.id + "_ShadowFilter";
    const vShadowDeviation = 0.02 * vRadius;
    vGroupMain.innerHTML +=
      `<filter id='${vShadowFilterName}' x='-2' y='-2' width='4' height='4'>
         <feDropShadow dx='${1.0 * vShadowDeviation}' dy='${1.5 * vShadowDeviation}' stdDeviation='${vShadowDeviation}' flood-opacity='0.5' flood-color='black'/>
       </filter>`;

    // gauge background
    let vCircle = ggSvg.CreateCircle(vCenter, vRadius);
    ggSvg.SetStyle(vCircle, `fill: ${this.mFaceStyle}; stroke-width: 0;`)
    vGroupMain.appendChild(vCircle);

    // tick highlights
    let vGroupTickHighlights = ggSvg.CreateGroup();
    vGroupMain.appendChild(vGroupTickHighlights);
    const vTicksLength = this.mTicksSize * (vRadius - this.mTicksMargin);
    const vHighlightRadius = vRadius - this.mTicksMargin - vTicksLength / 2;
    ggSvg.SetStyle(vGroupTickHighlights, `stroke-width: ${vTicksLength};`);
    for (let vHighlight of this.mTicksHighlights) {
      let vArc = ggSvg.CreateArc(vCenter, vHighlightRadius,
                                 this.GetAngle(vHighlight.mRange.mMin),
                                 this.GetAngle(vHighlight.mRange.mMax));
      ggSvg.SetStyle(vArc, `stroke: ${vHighlight.mStyle}; fill: none;`);
      vGroupTickHighlights.appendChild(vArc);
    }

    // major and minor tickmarks
    let vGroupTickMarksMajor = ggSvg.CreateGroup(vGroupMain);
    let vGroupTickMarksMinor = ggSvg.CreateGroup(vGroupMain);
    let vGroupTickMarksLabels = ggSvg.CreateGroup(vGroupMain);
    ggSvg.SetStyle(vGroupTickMarksMajor, `stroke: ${this.mLabelStyle}; stroke-width: ${this.mTicksMajorWidth};`);
    ggSvg.SetStyle(vGroupTickMarksMinor, `stroke: ${this.mLabelStyle}; stroke-width: ${this.mTicksMinorWidth};`);
    ggSvg.SetStyle(vGroupTickMarksLabels, `font: ${this.mTicksFont}; fill: ${this.mLabelStyle}; stroke: none; stroke-width: 0;`);
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
      vGroupTickMarksMajor.appendChild(ggSvg.CreateLine(vPositionMajorA, vPositionMajorB));
      // major tickmark label text
      let vTickMajorLabel = ggSvg.CreateText(vPositionMajorLabel, vTickMajor);
      let vTextAnchor = ggSvg.GetTextAnchor(vAngleMajor);
      let vAlignmentBaseline = ggSvg.GetAlignmentBaseline(vAngleMajor);
      ggSvg.SetStyle(vTickMajorLabel, `text-anchor: ${vTextAnchor}; alignment-baseline: ${vAlignmentBaseline};`);
      vGroupTickMarksLabels.appendChild(vTickMajorLabel);
      // minor tickmarks
      if (vTickIndexMajor + 1 < vTicksMajor.length) {
        let vTicksMinor = this.GetTicksMinor(vTicksMajor[vTickIndexMajor], vTicksMajor[vTickIndexMajor + 1]);
        for (let vTickIndexMinor = 0; vTickIndexMinor < vTicksMinor.length; vTickIndexMinor++) {
          let vTickMinor = vTicksMinor[vTickIndexMinor];
          let vAngleMinor = this.GetAngle(vTickMinor);
          let vPositionMinorA = this.GetPosition(vAngleMinor, vTickRadiusA);
          let vPositionMinorB = this.GetPosition(vAngleMinor, vTickRadiusA - vTicksLength / 3);
          vGroupTickMarksMinor.appendChild(ggSvg.CreateLine(vPositionMinorA, vPositionMinorB));
        }
      }
    }

    // draw value (and label)
    let vGroupLabelAndValue = ggSvg.CreateGroup(vGroupMain);
    // vGroupLabelAndValue.setAttributeNS(null, "filter", `url(#${vShadowFilterName})`);
    let vLabelAndValueStyle = `font: ${this.mLabelFont}; fill: ${this.mLabelStyle}; stroke: none; stroke-width: 0; ` + 
                              `text-anchor: middle; alignment-baseline: middle;`;
    let vLabelPosition = new ggVector(vCenter.mX, vCenter.mY + 0.75 * vTickRadiusA - 0.12 * vRadius);
    let vMainLabelText = vGroupLabelAndValue.appendChild(ggSvg.CreateText(vLabelPosition, this.mLabel));
    ggSvg.SetStyle(vMainLabelText, vLabelAndValueStyle);
    let vValuePosition = new ggVector(vCenter.mX, vCenter.mY + 0.75 * vTickRadiusA + 0.12 * vRadius);
    this.mValueSvg = vGroupLabelAndValue.appendChild(ggSvg.CreateText(vValuePosition, this.mValue.toFixed(this.mDecimals)));
    ggSvg.SetStyle(this.mValueSvg, vLabelAndValueStyle);

    // draw indicator
    let vGroupIndicator = ggSvg.CreateGroup(vGroupMain);
    ggSvg.SetStyle(vGroupIndicator, `stroke: none; stroke-width: 0; fill: ${this.mIndicatorStyle};`);
    vGroupIndicator.setAttributeNS(null, "filter", `url(#${vShadowFilterName})`);
    const vIndicatorWidth2 = this.mIndicatorWidth / 2 * vRadius;
    const vIndicatorPosA = ggVector.Add(vCenter, new ggVector(0.3 * (vTicksLength - vTickRadiusA), 0));
    const vIndicatorPosB = ggVector.Add(vCenter, new ggVector(vTickRadiusA - vIndicatorWidth2 - vTicksLength / 3, 0));
    vGroupIndicator.appendChild(ggSvg.CreateCircle(vCenter, 2.2 * vIndicatorWidth2));
    this.mIndicatorSvg = vGroupIndicator.appendChild(ggSvg.CreatePolygon([
      ggVector.Add(vIndicatorPosA, new ggVector(0, vIndicatorWidth2)),
      ggVector.Add(vIndicatorPosB, new ggVector(0, vIndicatorWidth2)),
      ggVector.Add(vIndicatorPosB, new ggVector(vIndicatorWidth2, 0)),
      ggVector.Add(vIndicatorPosB, new ggVector(0, -vIndicatorWidth2)),
      ggVector.Add(vIndicatorPosA, new ggVector(0, -vIndicatorWidth2))
    ]));
    ggSvg.AnimateRotation(this.mIndicatorSvg, vCenter, this.GetAngle(this.ValueLimited), this.GetAngle(this.ValueLimited), 0.5);

    // clear old graphics
    this.mSvg.innerHTML = "";

    // add new graphics
    this.mSvg.appendChild(vGroupMain);
  }
}