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
    aSvgObject.setAttributeNS(null, "transform", `rotate(${vAngleDegA}, ${aCenter.mX}, ${aCenter.mY})`);
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


class ggClockSVG {

  // public properties
  mBackgroundColor = "transparent";
  mFaceStyle = "white";
  mHandSecondStyle = "#d00"; // drak red
  mHandMinuteStyle = "#000";
  mHandHourStyle = "#000";
  // mLabels = ["12", "", "", "3", "", "", "6", "", "", "9", "", ""];
  // mLabels = ["12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"];
  // mLabels = ["XII", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI"];
  mLabelStyle = "#222";
  mLabelFont = "bold 13px arial"; // or "bold 14px Lucida Console" ...
  mTicksMargin = 0.03; // percent of radius
  mTicksHourLength = 0.24; // percent of radius
  mTicksHourWidth = 0.07; // percent of radius
  mTicksMinuteLength = 0.06; // percent of radius
  mTicksMinuteWidth = 0.025; // percent of radius

  // "private" members
  mDiameter = null;
  mSvg = null;

  mTimeSecond = 0;
  mTimeMinute = 0;
  mTimeHour = 0;
  mTimerID = 0;

  mSvgHandSecond = null;
  mSvgHandMinute = null;
  mSvgHandHour = null;

  constructor(aSVG, aGetDateFunc) {
    this.mSvg = aSVG;
    if (aGetDateFunc) this.GetDate = aGetDateFunc;
    this.Update();
  }

  Update() {
    this.CreateGraphics();
    this.CreateTimer();
  }

  GetDate() {
    return new Date();
  }

  static IsDateValid(aDate) {
    return aDate && Object.prototype.toString.call(aDate) === "[object Date]" && !isNaN(aDate);
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

  AnimateHandSecond(aSecondA, aSecondB, aDuration, aMinimizeRotationAngle) {
    ggSvg.AnimateRotation(
      this.mSvgHandSecond,
      this.Center,
      this.GetAngleSecond(aSecondA),
      this.GetAngleSecond(aSecondB),
      aDuration,
      aMinimizeRotationAngle);
  }

  AnimateHandMinute(aMinuteA, aMinuteB, aDuration) {
    ggSvg.AnimateRotation(
      this.mSvgHandMinute,
      this.Center,
      this.GetAngleMinute(aMinuteA),
      this.GetAngleMinute(aMinuteB),
      aDuration,
      true /*aMinimizeRotationAngle*/);
  }

  AnimateHandHour(aHourA, aHourB, aDuration) {
    ggSvg.AnimateRotation(
      this.mSvgHandHour,
      this.Center,
      this.GetAngleHour(aHourA),
      this.GetAngleHour(aHourB),
      aDuration,
      true /*aMinimizeRotationAngle*/);
  }

  CreateGraphics() {

    // center and radius are relevant for all parts ...
    const vCenter = this.Center;
    const vRadius = this.Radius;
    const vTickRadius = (1 - this.mTicksMargin) * vRadius;

    // main container
    let vGroupMain = ggSvg.CreateGroup();

    // shadow-filter for hands (with unique ID)
    const vShadowFilterName = this.mSvg.id + "_ShadowFilter";
    const vShadowDeviation = 0.02 * vTickRadius;
    vGroupMain.innerHTML +=
      `<filter id='${vShadowFilterName}' x='-2' y='-2' width='4' height='4'>
         <feDropShadow dx='${1.0 * vShadowDeviation}' dy='${1.5 * vShadowDeviation}' stdDeviation='${vShadowDeviation}' flood-opacity='0.5' flood-color='black'/>
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
    ggSvg.SetRotation(this.mSvgHandHour, vCenter, this.GetAngleHour(0));

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
    ggSvg.SetRotation(this.mSvgHandMinute, vCenter, this.GetAngleMinute(0));

    // second hand
    const vSecondPosA = ggVector.Add(vCenter, new ggVector(-1.6 * this.mTicksHourLength * vTickRadius, 0));
    const vSecondPosB = ggVector.Add(vCenter, new ggVector(vTickRadiusH - this.mTicksHourWidth * vTickRadius, 0));
    this.mSvgHandSecond = ggSvg.CreateGroup(vGroupHands);
    let vSvgSecondHandNeck = this.mSvgHandSecond.appendChild(ggSvg.CreateLine(vSecondPosA, vSecondPosB));
    let vSvgSecondHandHead = this.mSvgHandSecond.appendChild(ggSvg.CreateCircle(vSecondPosB, 0.8 * this.mTicksHourLength * vTickRadius / 2));
    ggSvg.SetStyle(vSvgSecondHandNeck, `stroke: ${this.mHandSecondStyle}; stroke-width: ${this.mTicksMinuteWidth * vTickRadius};`);
    ggSvg.SetStyle(this.mSvgHandSecond, `fill: ${this.mHandSecondStyle}; stroke-width: 0;`);
    ggSvg.SetRotation(this.mSvgHandSecond, vCenter, this.GetAngleSecond(0));

    // clear old graphics
    this.mSvg.innerHTML = "";

    // add new graphics
    this.mSvg.appendChild(vGroupMain);
  }

  CreateTimer() {

    let vInitializeHands = true;

    let UpdateTime = () => {
      let vDate = this.GetDate();
      let vDateIsValid = ggClockSVG.IsDateValid(vDate);
      if (!vDateIsValid) {
        vDate = {
          getMilliseconds: () => 0,
          getSeconds: () => 0,
          getMinutes: () => 0,
          getHours: () => 0
        };
      }
      let vSecondOld = this.mTimeSecond;
      let vSecondNew = vInitializeHands ? vDate.getSeconds() : vDateIsValid ? 60 : 0;
      let vMinuteOld = this.mTimeMinute;
      let vMinuteNewPlus = vDate.getSeconds() >= 58 ? 1 : 0;
      let vMinuteNew = vDate.getMinutes() + vMinuteNewPlus;
      let vHourOld = this.mTimeHour + vMinuteOld / 60;
      let vHourNewPlus = ((vDate.getSeconds() >= 58) && (vDate.getMinutes() >= 59)) ? 1 : 0;
      let vHourNew = (vDate.getHours() + vHourNewPlus) % 12 + vMinuteNew / 60;
      if (vMinuteOld == 59 && vMinuteNew == 0) {
        vMinuteNew = 60;
        if ((vDate.getHours() + vHourNewPlus) % 12 == 0) {
          vHourNew += 12;
        }
      }
      const vBaseAnimationDuration = 0.3;
      const vFullMinuteStopDuration = 1.0;
      let vTimeoutNextUpdate = vBaseAnimationDuration;
      let vSecondAnimationDuration = vBaseAnimationDuration;
      if (!vInitializeHands) {
        vTimeoutNextUpdate = vSecondNew - vDate.getSeconds() - vDate.getMilliseconds() / 1000;
        if (vTimeoutNextUpdate < vFullMinuteStopDuration + 1) vTimeoutNextUpdate += 60;
        vSecondAnimationDuration = vTimeoutNextUpdate - vFullMinuteStopDuration;
      }
      // console.log(this.mSvg.id + " ==> " + vTimeoutNextUpdate + ", " + vSecondAnimationDuration);
      this.AnimateHandSecond(vSecondOld, vSecondNew, vSecondAnimationDuration, vInitializeHands);
      this.AnimateHandMinute(vMinuteOld, vMinuteNew, vBaseAnimationDuration);
      this.AnimateHandHour(vHourOld, vHourNew, vBaseAnimationDuration);
      this.mTimeSecond = vInitializeHands ? vDate.getSeconds() : 0;
      this.mTimeMinute = vDate.getMinutes() + vMinuteNewPlus;
      this.mTimeHour = (vDate.getHours() + vHourNewPlus) % 12;
      if (this.mTimerID != 0) clearTimeout(this.mTimerID);
      this.mTimerID = setTimeout(UpdateTime, 1000 * vTimeoutNextUpdate);
      vInitializeHands = false;
    }

    UpdateTime();
  }
}
