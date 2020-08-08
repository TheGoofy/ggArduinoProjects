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

  static AnimateRotation(aSvgObject, aCenter, aAngleA, aAngleB, aDuration = "1s") {
    let vAngleDegA = 180 * aAngleA / Math.PI;
    let vAngleDegB = 180 * aAngleB / Math.PI;
    aSvgObject.setAttributeNS(null, "transform", `rotate(${vAngleDegA}, ${aCenter.mX}, ${aCenter.mY})`);
    let vSvgAnimateTransform = document.createElementNS(this.mSvgNS, "animateTransform");
    vSvgAnimateTransform.setAttributeNS(null, "attributeName", "transform");
    vSvgAnimateTransform.setAttributeNS(null, "type", "rotate");
    vSvgAnimateTransform.setAttributeNS(null, "fill", "freeze");
    vSvgAnimateTransform.setAttributeNS(null, "to", `${vAngleDegB} ${aCenter.mX} ${aCenter.mY}`);
    vSvgAnimateTransform.setAttributeNS(null, "dur", `${aDuration}`);
    //aSvgObject.innerHTML = "";
    //aSvgObject.appendChild(vSvgAnimateTransform);
    vSvgAnimateTransform.beginElement();
    aSvgObject.replaceChild(vSvgAnimateTransform, aSvgObject.getElementById("animateTransform"));
    /*
    let vAnimateTransform = `<animateTransform 
      attributeName='transform' type='rotate' 
      to='${vAngleDegB} ${aCenter.mX} ${aCenter.mY}'
      dur='${aDuration}' fill='freeze'/>`;
    aSvgObject.innerHTML = vAnimateTransform;
    aSvgObject.firstChild.beginElement();
    */
  }
}


class ggClockSVG {

  // public properties
  mBackgroundColor = "transparent";
  mFaceStyle = "white";
  mHandSecondStyle = "#c00"; // drak red
  mHandMinuteStyle = "black";
  mHandHourStyle = "black";
  // mLabels = ["12", "", "", "3", "", "", "6", "", "", "9", "", ""];
  // mLabels = ["12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"];
  // mLabels = ["XII", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI"];
  mLabelStyle = "black";
  mLabelFont = "bold 13px arial"; // or "bold 14px Lucida Console" ...
  mTicksMargin = 0.03; // percent of radius
  mTicksHourLength = 0.24; // percent of radius
  mTicksHourWidth = 0.07; // percent of radius
  mTicksMinuteLength = 0.06; // percent of radius
  mTicksMinuteWidth = 0.025; // percent of radius

  // "private" members
  mDiameter = null;
  mSvg = null;

  mSvgHandSecond = null;
  mSvgHandMinute = null;
  mSvgHandHour = null;

  constructor(aSVG) {
    this.mSvg = aSVG;
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

  GetAngleHour(aHour) {
    return ((aHour - 3) / 12) * 2 * Math.PI;
  }

  GetAngleMinute(aMinute) {
    return ((aMinute - 15) / 60) * 2 * Math.PI;
  }

  GetAngleSecond(aSecond) {
    return ((aSecond - 15) / 60) * 2 * Math.PI;
  }

  GetPosition(aAngle, aRadius) {
    let vCenter = this.Center;
    let vX = vCenter.mX + aRadius * Math.cos(aAngle);
    let vY = vCenter.mY + aRadius * Math.sin(aAngle);
    return new ggVector(vX, vY);
  }

  Draw() {

    // center and radius are relevant for all parts ...
    const vCenter = this.Center;
    const vRadius = this.Radius;
    const vTickRadius = (1 - this.mTicksMargin) * vRadius;

    // main container
    let vGroupMain = ggSvg.CreateGroup();

    // shadow-filter (for indicator)
    const vShadowFilterName = "ShadowFilter";
    vGroupMain.innerHTML +=
      `<filter id='${vShadowFilterName}' x='-2' y='-2' width='4' height='4'>
         <feDropShadow dx='0.8' dy='1.2' stdDeviation='1' flood-opacity='0.5' flood-color='black'/>
       </filter>`;

    // clock face background
    let vCircle = ggSvg.CreateCircle(vCenter, vRadius);
    ggSvg.SetStyle(vCircle, `fill: ${this.mFaceStyle}; stroke-width: 0;`)
    vGroupMain.appendChild(vCircle);

    // tickmarks and labels
    let vGroupTicksMinute = ggSvg.CreateGroup(vGroupMain);
    let vGroupTicksHour = ggSvg.CreateGroup(vGroupMain);
    let vGroupTicksHourLabel = ggSvg.CreateGroup(vGroupMain);
    ggSvg.SetStyle(vGroupTicksMinute, `stroke: ${this.mLabelStyle}; stroke-width: ${this.mTicksMinuteWidth * vTickRadius};`);
    ggSvg.SetStyle(vGroupTicksHour, `stroke: ${this.mLabelStyle}; stroke-width: ${this.mTicksHourWidth * vTickRadius};`);
    ggSvg.SetStyle(vGroupTicksHourLabel, `font: ${this.mLabelFont}; fill: ${this.mLabelStyle}; stroke-width: 0;`);
    const vTickRadiusM = (1 - this.mTicksMinuteLength) * vTickRadius;
    const vTickRadiusH = (1 - this.mTicksHourLength) * vTickRadius;
    for (let vMinute = 0; vMinute < 60; vMinute++) {
      if (vMinute % 5 != 0) {
        let vAngleMinute = this.GetAngleMinute(vMinute);
        let vPosMinute = this.GetPosition(vAngleMinute, vTickRadius);
        let vPosMinuteM = this.GetPosition(vAngleMinute, vTickRadiusM);
        vGroupTicksMinute.appendChild(ggSvg.CreateLine(vPosMinute, vPosMinuteM));
      }
    }
    for (let vHour = 0; vHour < 12; vHour++) {
      let vAngleHour = this.GetAngleHour(vHour);
      let vPosHour = this.GetPosition(vAngleHour, vTickRadius);
      let vPosHourH = this.GetPosition(vAngleHour, vTickRadiusH);
      vGroupTicksHour.appendChild(ggSvg.CreateLine(vPosHour, vPosHourH));
      if (this.mLabels && this.mLabels.length == 12) {
        let vPosLabel = this.GetPosition(vAngleHour, vTickRadiusH - this.mTicksMargin * vTickRadius);
        let vLabel = ggSvg.CreateText(vPosLabel, this.mLabels[vHour]);
        let vTextAnchor = ggSvg.GetTextAnchor(vAngleHour);
        let vAlignmentBaseline = ggSvg.GetAlignmentBaseline(vAngleHour);
        ggSvg.SetStyle(vLabel, `text-anchor: ${vTextAnchor}; alignment-baseline: ${vAlignmentBaseline};`);
        vGroupTicksHourLabel.appendChild(vLabel);
      }
    }

    // hands group
    let vGroupHands = ggSvg.CreateGroup(vGroupMain);
    vGroupHands.setAttributeNS(null, "filter", `url(#${vShadowFilterName})`);

    // hour hand
    const vHourPosA = ggVector.Add(vCenter, new ggVector(-this.mTicksHourLength * vTickRadius, 0));
    const vHourPosB = ggVector.Add(vCenter, new ggVector(vTickRadiusH - 1.5 * this.mTicksHourWidth * vTickRadius, 0));
    this.mSvgHandHour = vGroupHands.appendChild(ggSvg.CreatePolygon([
      ggVector.Add(vHourPosA, new ggVector(0, 1.7 * this.mTicksHourWidth / 2 * vTickRadius)),
      ggVector.Add(vHourPosB, new ggVector(0, 1.3 * this.mTicksHourWidth / 2 * vTickRadius)),
      ggVector.Add(vHourPosB, new ggVector(0, -1.3 * this.mTicksHourWidth / 2 * vTickRadius)),
      ggVector.Add(vHourPosA, new ggVector(0, -1.7 * this.mTicksHourWidth / 2 * vTickRadius))
    ]));
    ggSvg.SetStyle(this.mSvgHandHour, `fill: ${this.mHandHourStyle}; stroke-width: 0;`);
    ggSvg.AnimateRotation(this.mSvgHandHour, vCenter, this.GetAngleHour(12), this.GetAngleHour(13.82));

    // minute hand
    const vMinutePosA = ggVector.Add(vCenter, new ggVector(-this.mTicksHourLength * vTickRadius, 0));
    const vMinutePosB = ggVector.Add(vCenter, new ggVector(vTickRadiusM + this.mTicksMinuteWidth * vTickRadius, 0));
    this.mSvgHandMinute = vGroupHands.appendChild(ggSvg.CreatePolygon([
      ggVector.Add(vMinutePosA, new ggVector(0, 1.3 * this.mTicksHourWidth / 2 * vTickRadius)),
      ggVector.Add(vMinutePosB, new ggVector(0, 1.0 * this.mTicksHourWidth / 2 * vTickRadius)),
      ggVector.Add(vMinutePosB, new ggVector(0, -1.0 * this.mTicksHourWidth / 2 * vTickRadius)),
      ggVector.Add(vMinutePosA, new ggVector(0, -1.3 * this.mTicksHourWidth / 2 * vTickRadius))
    ]));
    ggSvg.SetStyle(this.mSvgHandMinute, `fill: ${this.mHandMinuteStyle}; stroke-width: 0;`);
    ggSvg.AnimateRotation(this.mSvgHandMinute, vCenter, this.GetAngleMinute(60), this.GetAngleMinute(49));

    // second hand
    const vSecondPosA = ggVector.Add(vCenter, new ggVector(-1.6 * this.mTicksHourLength * vTickRadius, 0));
    const vSecondPosB = ggVector.Add(vCenter, new ggVector(vTickRadiusH - this.mTicksHourWidth * vTickRadius, 0));
    this.mSvgHandSecond = ggSvg.CreateGroup(vGroupHands);
    let vSvgSecondHandNeck = this.mSvgHandSecond.appendChild(ggSvg.CreateLine(vSecondPosA, vSecondPosB));
    let vSvgSecondHandHead = this.mSvgHandSecond.appendChild(ggSvg.CreateCircle(vSecondPosB, 0.8 * this.mTicksHourLength * vTickRadius / 2));
    ggSvg.SetStyle(vSvgSecondHandNeck, `stroke: ${this.mHandSecondStyle}; stroke-width: ${this.mTicksMinuteWidth * vTickRadius};`);
    ggSvg.SetStyle(this.mSvgHandSecond, `fill: ${this.mHandSecondStyle}; stroke-width: 0;`);
    ggSvg.AnimateRotation(vSvgSecondHandNeck, vCenter, this.GetAngleSecond(0), this.GetAngleSecond(27));
    ggSvg.AnimateRotation(vSvgSecondHandHead, vCenter, this.GetAngleSecond(0), this.GetAngleSecond(27));

    // clear old graphics
    this.mSvg.innerHTML = "";

    // add new graphics
    this.mSvg.appendChild(vGroupMain);
  }
}
