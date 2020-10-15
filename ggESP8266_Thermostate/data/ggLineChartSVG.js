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
    vLine.setAttribute("x1", aPointA.mX);
    vLine.setAttribute("y1", aPointA.mY);
    vLine.setAttribute("x2", aPointB.mX);
    vLine.setAttribute("y2", aPointB.mY);
    return vLine;
  }

  static CreatePolyline(aPoints) {
    let vPolyline = document.createElementNS(this.mSvgNS, "polyline");
    let vPointsString = aPoints.join(" ");
    vPolyline.setAttribute("points", vPointsString);
    return vPolyline;
  }

  static CreatePolygon(aPoints) {
    let vPolygon = document.createElementNS(this.mSvgNS, "polygon");
    let vPointsString = aPoints.join(" ");
    vPolygon.setAttribute("points", vPointsString);
    return vPolygon;
  }

  static CreateRect(aPointA, aPointB) {
    let vRect = document.createElementNS(this.mSvgNS, "rect");
    vRect.setAttribute("x", aPointA.mX);
    vRect.setAttribute("y", aPointA.mY);
    vRect.setAttribute("width", aPointB.mX - aPointA.mX);
    vRect.setAttribute("height", aPointB.mY - aPointA.mY);
    return vRect;
  }

  static CreateCircle(aCenter, aRadius) {
    let vCircle = document.createElementNS(this.mSvgNS, "circle");
    vCircle.setAttribute("cx", aCenter.mX);
    vCircle.setAttribute("cy", aCenter.mY);
    vCircle.setAttribute("r", aRadius);
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
    vArc.setAttribute("d", `M ${vPosAX} ${vPosAY} ` +
                           `A ${aRadius} ${aRadius} ${vRotation} ${vLargeArcFlag} ${vSweepFlag} ${vPosBX} ${vPosBY}`);
    return vArc;
  }

  static CreateSpan(aPositionX, aPositionY, aDeltaX, aDeltaY, aText) {
    let vSpan = document.createElementNS(this.mSvgNS, "tspan");
    vSpan.setAttribute("x", aPositionX);
    vSpan.setAttribute("y", aPositionY);
    vSpan.setAttribute("dx", aDeltaX);
    vSpan.setAttribute("dy", aDeltaY);
    vSpan.innerHTML = aText;
    return vSpan;
  }

  static CreateText(aPosition, aText) {
    let vText = document.createElementNS(this.mSvgNS, "text");
    vText.setAttribute("x", aPosition.mX);
    vText.setAttribute("y", aPosition.mY);
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

  static tAlign = {
    eLeft: "start",
    eCenter: "middle",
    eRight: "end",
    eTop: "hanging",
    eMiddle: "middle",
    eBottom: "alphabetic"
  }

  static SetTextAlign(aSvgText, aAlignHorizontal, aAlignVertical) {
    ggSvg.SetStyle(aSvgText, `text-anchor: ${aAlignHorizontal}; alignment-baseline: ${aAlignVertical};`);
  }

  static SetStyle(aSvgObject, aStyle) {
    aSvgObject.setAttribute("style", aStyle);
  }

  static SetRotation(aSvgObject, aCenter, aAngle) {
    let vAngleDeg = 180 * aAngle / Math.PI;
    aSvgObject.setAttribute("transform", `rotate(${vAngleDeg}, ${aCenter.mX}, ${aCenter.mY})`);
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
    vSvgAnimateTransform.setAttribute("attributeName", "transform");
    vSvgAnimateTransform.setAttribute("type", "rotate");
    vSvgAnimateTransform.setAttribute("fill", "freeze");
    vSvgAnimateTransform.setAttribute("to", `${vAngleDegB} ${aCenter.mX} ${aCenter.mY}`);
    vSvgAnimateTransform.setAttribute("dur", `${aDuration}`);
    let vSvgAnimateTransformsExisting = aSvgObject.getElementsByTagName(vSvgAnimateTransform.nodeName);
    if (vSvgAnimateTransformsExisting.length > 0) vSvgAnimateTransformsExisting[0].replaceWith(vSvgAnimateTransform);
    else aSvgObject.appendChild(vSvgAnimateTransform);
    vSvgAnimateTransform.beginElement();
  }
}


const mRoundFactors20 = [
  -10.0, -5.0, -2.0, -1.0,
    1.0,  2.0,  5.0, 10.0
];

const mRoundFactors15 = [
  -10.0, -7.0, -5.0, -3.0, -2.0, -1.5, -1.0,
    1.0,  1.5,  2.0,  3.0,  5.0,  7.0, 10.0
]

function RoundToNearest(aValue, aValues) {
  let vResult = aValue;
  let vErrorMin = Number.MAX_VALUE;
  for (let vValue of aValues) {
    if (vErrorMin > Math.abs(vValue - aValue)) {
      vErrorMin = Math.abs(vValue - aValue);
      vResult = vValue;
    }
  }
  return vResult;
}

function RoundToMultipleOfBase10(aValue, aValues) {
  // nothing to do, if value is 0
  if (aValue == 0.0) return aValue;
  // calculate the order of magnitude (negative if value below 1)
  let vDecimals = Math.floor(Math.log10(Math.abs(aValue)));
  // search the value, which is as close as possible to the value
  const vFactor = Math.pow(10.0, vDecimals);
  return vFactor * RoundToNearest(aValue / vFactor, aValues);
}


class ggTime {
  mSeconds = 0;
  mMinutes = 0;
  mHours = 0;
  mDays = 0;
  mMonths = 0;
  mYears = 0;
}


function RoundToTime(aMilliSeconds) {

  // 1-2-5-10 rounding scheme for time:
  // - each step is roughly a factor of 2
  // - align with minutes, hours, days, months, years

  let vTime = new ggTime;

  const vSeconds = aMilliSeconds / 1000.0;
  const vMinutes = vSeconds / 60.0;
  const vHours = vMinutes / 60.0;
  const vDays = vHours / 24.0;
  const vMonths = vDays / (365.25 / 12.0);
  const vYears = vDays / 365.25;

  if (aMilliSeconds <= 1000.0) {
    vTime.mSeconds = 0.001 * RoundToMultipleOfBase10(aMilliSeconds, [1, 2, 5, 10]);
    if (vTime.mSeconds < 0.001) vTime.mSeconds = 0.001;
  }
  else if (vSeconds <= 30.0) {
    vTime.mSeconds = RoundToNearest(vSeconds, [1, 2, 3, 5, 10, 15, 20, 30]);
  }
  else if (vMinutes <= 30.0) {
    vTime.mMinutes =  RoundToNearest(vMinutes, [1, 2, 3, 5, 10, 15, 20, 30]);
  }
  else if (vHours <= 12.0) {
    vTime.mHours = RoundToNearest(vHours, [1, 2, 3, 4, 6, 12]);
  }
  else if (vDays <= 15.0) {
    vTime.mDays = RoundToNearest(vDays, [1, 2, 4, 7, 14]);
  }
  else if (vMonths <= 6.0) {
    vTime.mMonths = RoundToNearest(vMonths, [1, 2, 3, 6]);
  }
  else {
    vTime.mYears = RoundToMultipleOfBase10(vYears, [1, 2, 5, 10]);
  }

  return vTime;
}


function TimeCeil(aMilliSeconds, aTimeTick) {
  let vDate = new Date(aMilliSeconds);
  if (aTimeTick.mSeconds != 0) {
    let vMilliSeconds = vDate.getMilliseconds() + 1000.0 * vDate.getSeconds();
    vDate.setSeconds(0);
    vDate.setMilliseconds(1000.0 * aTimeTick.mSeconds * Math.ceil(0.001 * vMilliSeconds / aTimeTick.mSeconds));
    return vDate.getTime();
  }
  vDate.setMilliseconds(0);
  vDate.setSeconds(0);
  if (aTimeTick.mMinutes != 0) {
    vDate.setMinutes(aTimeTick.mMinutes * Math.ceil(vDate.getMinutes() / aTimeTick.mMinutes));
    return vDate.getTime();
  }
  vDate.setMinutes(0);
  if (aTimeTick.mHours != 0) {
    vDate.setHours(aTimeTick.mHours * Math.ceil(vDate.getHours() / aTimeTick.mHours));
    return vDate.getTime();
  }
  vDate.setHours(0);
  if (aTimeTick.mDays != 0) {
    vDate.setDate(aTimeTick.mDays * Math.ceil((vDate.getDate() - 1) / aTimeTick.mDays) + 1);
    return vDate.getTime();
  }
  vDate.setDate(1);
  if (aTimeTick.mMonths != 0) {
    vDate.setMonth(aTimeTick.mMonths * Math.ceil(vDate.getMonth() / aTimeTick.mMonths));
    return vDate.getTime();
  }
  vDate.setMonth(0);
  if (aTimeTick.mYears != 0) {
    vDate.setFullYear(aTimeTick.mYears * Math.ceil(vDate.getFullYear() / aTimeTick.mYears));
    return vDate.getTime();
  }
  return vDate.getTime();
}


function TimeAdd(aMilliSeconds, aTime) {
  let vDate = new Date(aMilliSeconds);
  vDate.setMilliseconds(vDate.getMilliseconds() + 1000.0 * aTime.mSeconds);
  vDate.setMinutes(vDate.getMinutes() + aTime.mMinutes);
  vDate.setHours(vDate.getHours() + aTime.mHours);
  vDate.setDate(vDate.getDate() + aTime.mDays);
  vDate.setMonth(vDate.getMonth() + aTime.mMonths);
  vDate.setFullYear(vDate.getFullYear() + aTime.mYears);
  return vDate.getTime();
}


class ggLineChartSVG {

  // public properties
  mPlotBackgroundColor = "transparent";
  mPlotLineColor = "red";
  mPlotLineWidth = 1.5;
  mPlotRangeOpacity = 0.25;
  mDisplayRange = true;
  mAxisX = {
    mLabel: "",
    mLabelFont: "14px arial",
    mLabelColor: "white",
    mTicksSpacing: 100,
    mTicksLineColor: "rgba(127,127,127,0.3)",
    mTicksLineWidth: 1.0,
    mTicksLabelFont: "10px arial",
    mTicksLabelColor: "white",
  };
  mAxisY = {
    mLabel: "",
    mLabelFont: "14px arial",
    mLabelColor: "white",
    mTicksSpacing: 30,
    mTicksLineColor: "rgba(127,127,127,0.4)",
    mTicksLineWidth: 1.0,
    mTicksLabelFont: "10px arial",
    mTicksLabelColor: "white",
  };
  
  // "private" members
  mValues = [];
  mSvg = null;
  mSvgNS = "http://www.w3.org/2000/svg";

  constructor(aSVG, aValues = [], aDraw = true) {
    this.mSvg = aSVG;
    this.mValues = aValues;
    this.mValues.sort((aValueA, aValueB) => aValueA[0] - aValueB[0]);
    if (aDraw) this.Draw();
  }

  set Values(aValues) {
    this.mValues = aValues;
    this.mValues.sort((aValueA, aValueB) => aValueA[0] - aValueB[0]);
    this.Draw();
  }

  get Values() {
    return this.mValues;
  }

  get Center() {
    return new ggVector(this.mSvg.clientLeft + this.mSvg.clientWidth / 2,
                        this.mSvg.clientTop + this.mSvg.clientHeight / 2);
  }

  get LeftTop() {
    return new ggVector(this.mSvg.clientLeft,
                        this.mSvg.clientTop);
  }

  get RightTop() {
    return new ggVector(this.mSvg.clientLeft + this.mSvg.clientWidth,
                        this.mSvg.clientTop);
  }

  get LeftBottom() {
    return new ggVector(this.mSvg.clientLeft,
                        this.mSvg.clientTop + this.mSvg.clientHeight);
  }
  get RightBottom() {
    return new ggVector(this.mSvg.clientLeft + this.mSvg.clientWidth,
                        this.mSvg.clientTop + this.mSvg.clientHeight);
  }

  Draw() {

    // main container
    let vSgvGroupMain = ggSvg.CreateGroup();

    // y-axis: main label
    let vAxisYLabelSizeX = 0;
    if (this.mAxisY.mLabel != "") {
      const vAxisYLabelPosition = new ggVector(this.mSvg.clientLeft, this.mSvg.clientTop + this.mSvg.clientHeight / 2);
      const vSvgAxisYLabel = ggSvg.CreateText(vAxisYLabelPosition, this.mAxisY.mLabel);
      const vAxisYLabelStyle =
        `font: ${this.mAxisY.mLabelFont};` + 
        `fill: ${this.mAxisY.mLabelColor};` +
        `stroke: none;` +
        `stroke-width: 0;` + 
        `text-anchor: ${ggSvg.tAlign.eCenter};` +
        `alignment-baseline: ${ggSvg.tAlign.eTop};`;
      ggSvg.SetStyle(vSvgAxisYLabel, vAxisYLabelStyle);
      ggSvg.SetRotation(vSvgAxisYLabel, vAxisYLabelPosition, -Math.PI/2.0);
      vSgvGroupMain.appendChild(vSvgAxisYLabel);
      vAxisYLabelSizeX = 16;
    }

    // plot area background
    let vSvgPlotBackground = ggSvg.CreateRect(new ggVector(this.mSvg.clientLeft + vAxisYLabelSizeX, this.mSvg.clientTop), this.RightBottom);
    ggSvg.SetStyle(vSvgPlotBackground, `fill: ${this.mPlotBackgroundColor}; stroke-width: 0;`)

    // data shortcut
    let vValues = this.mValues;
    if (vValues.length < 2) return;

    // find range
    function Range(aRange, aValue) {
      if (!aRange) return {
        mMin: aValue.slice(),
        mMax: aValue.slice()
      };
      for (let vIndex = 0; vIndex < aValue.length; vIndex++) {
        aRange.mMin[vIndex] = Math.min(aRange.mMin[vIndex], aValue[vIndex]);
        aRange.mMax[vIndex] = Math.max(aRange.mMax[vIndex], aValue[vIndex]);
      }
      return aRange;
    }
    const vValuesRange = vValues.reduce(Range, null);

    // scale factors x: left-right (=normal) / y: bottom-top (flipped)
    const vValueMinX = vValuesRange.mMin[0];
    const vValueMaxX = vValuesRange.mMax[0];
    const vValueRangeX = vValueMaxX - vValueMinX;
    const vTickDeltaX = RoundToTime(vValueRangeX * this.mAxisX.mTicksSpacing / this.mSvg.clientWidth);
    const vScaleX = vValueRangeX != 0 ? (this.RightBottom.mX - this.LeftTop.mX - vAxisYLabelSizeX) / vValueRangeX : 1;
    const vOffsetX = vAxisYLabelSizeX + this.LeftTop.mX - vScaleX * vValueMinX;
    const GetPlotPointX = aValueX => vScaleX * aValueX + vOffsetX;
    const vValueMinY = Math.min(...vValuesRange.mMin.slice(1));
    const vValueMaxY = Math.max(...vValuesRange.mMax.slice(1));
    const vValueRangeY = vValueMinY != vValueMaxY ? vValueMaxY - vValueMinY : 1.0;
    const vTickDeltaY = RoundToMultipleOfBase10(vValueRangeY * this.mAxisY.mTicksSpacing / this.mSvg.clientHeight, mRoundFactors20);
    const vPlotMinY = vTickDeltaY * Math.floor(vValueMinY / vTickDeltaY);
    const vPlotMaxY = vTickDeltaY * Math.ceil(vValueMaxY / vTickDeltaY);
    const vPlotRangeY = vPlotMaxY - vPlotMinY;
    const vNumTicksY = Math.floor((vPlotRangeY / vTickDeltaY) + 0.5);
    const vScaleY = vPlotRangeY != 0 ? (this.LeftTop.mY - this.RightBottom.mY) / vPlotRangeY : 1;
    const vOffsetY = this.RightBottom.mY - vScaleY * vPlotMinY;
    const GetPlotPointY = aValueY => vScaleY * aValueY + vOffsetY;

    // x-axis: labels and gridlines
    let vSvgLinesX = ggSvg.CreateGroup();
    let vSvgLabelsX = ggSvg.CreateText(new ggVector(0.0, 0.0), "");
    ggSvg.SetStyle(vSvgLinesX, `stroke:${this.mAxisX.mTicksLineColor}; stroke-width:${this.mAxisX.mTicksLineWidth}; stroke-linejoin:bevel; fill:none;`);
    ggSvg.SetStyle(vSvgLabelsX, `text-anchor: ${ggSvg.tAlign.eRight}; alignment-baseline: ${ggSvg.tAlign.eBottom}; font: ${this.mAxisX.mTicksLabelFont}; fill: ${this.mAxisX.mTicksLabelColor}; stroke: none; stroke-width: 0;`);
    let vIsFirstTickX = true;
    let vTickX = TimeCeil(vValueMinX, vTickDeltaX);
    while (vTickX < vValueMaxX) {
      if (vTickX >= vValueMinX) {
        const vPointA = new ggVector(GetPlotPointX(vTickX), GetPlotPointY(vPlotMinY));
        const vPointB = new ggVector(GetPlotPointX(vTickX), GetPlotPointY(vPlotMaxY));
        vSvgLinesX.appendChild(ggSvg.CreateLine(vPointA, vPointB));
        if (!vIsFirstTickX) {
          const vDate = new Date(vTickX);
          const vRightBottom = this.RightBottom;
          const vLabelPositionX = vPointA.mX - this.mAxisX.mTicksLineWidth / 2.0 - 1.0
          vSvgLabelsX.appendChild(ggSvg.CreateSpan(vLabelPositionX, vRightBottom.mY, 0, "-1em", vDate.toLocaleTimeString()));
          vSvgLabelsX.appendChild(ggSvg.CreateSpan(vLabelPositionX, vRightBottom.mY, 0, "0em", vDate.toLocaleDateString()));
        }
          vIsFirstTickX = false;
      }
      vTickX = TimeAdd(vTickX, vTickDeltaX);
    }

    // y-axis: labels and gridlines
    let vSvgLinesY = ggSvg.CreateGroup();
    let vSvgLabelsY = ggSvg.CreateText(new ggVector(0.0, 0.0), "");
    ggSvg.SetStyle(vSvgLinesY, `stroke:${this.mAxisY.mTicksLineColor}; stroke-width:${this.mAxisY.mTicksLineWidth}; stroke-linejoin:bevel; fill:none;`);
    ggSvg.SetStyle(vSvgLabelsY, `text-anchor: ${ggSvg.tAlign.eLeft}; alignment-baseline: ${ggSvg.tAlign.eBottom}; font: ${this.mAxisY.mTicksLabelFont}; fill: ${this.mAxisY.mTicksLabelColor}; stroke: none; stroke-width: 0;`);
    let vTickDecimalsY = Math.floor(Math.log10(Math.abs(vTickDeltaY)));
    vTickDecimalsY = vTickDecimalsY < 0 ? -vTickDecimalsY : 0;
    for (let vTickIndexY = 1; vTickIndexY <= vNumTicksY; vTickIndexY++) {
      let vTickY = vPlotMinY + vTickIndexY * vTickDeltaY;
      const vPointA = new ggVector(GetPlotPointX(vValueMinX), GetPlotPointY(vTickY));
      const vPointB = new ggVector(GetPlotPointX(vValueMaxX), GetPlotPointY(vTickY));
      vSvgLinesY.appendChild(ggSvg.CreateLine(vPointA, vPointB));
      const vLabelPositionY = vPointA.mY + this.mAxisY.mTicksLineWidth / 2.0 + 1.0;
      vSvgLabelsY.appendChild(ggSvg.CreateSpan(vPointA.mX, vLabelPositionY, 0, "0.7em", vTickY.toFixed(vTickDecimalsY)));
    }

    // data range x and y
    if (this.mDisplayRange) {
      const vRightTop = this.RightTop;
      const vDateMin = new Date(vValueMinX);
      const vDateMax = new Date(vValueMaxX);
      vSvgLabelsX.appendChild(ggSvg.CreateSpan(vRightTop.mX, vRightTop.mY, 0, "1em", "x-min: " + vDateMin.toLocaleString()));
      vSvgLabelsX.appendChild(ggSvg.CreateSpan(vRightTop.mX, vRightTop.mY, 0, "2em", "x-max: " + vDateMax.toLocaleString()));
      vSvgLabelsX.appendChild(ggSvg.CreateSpan(vRightTop.mX, vRightTop.mY, 0, "3em", "y-min: " + vValueMinY));
      vSvgLabelsX.appendChild(ggSvg.CreateSpan(vRightTop.mX, vRightTop.mY, 0, "4em", "y-max: " + vValueMaxY));  
    }

    // data points
    const vPoints = vValues.map(vValue => [GetPlotPointX(vValue[0]), GetPlotPointY(vValue[1])]);
    const vPointsMin = vValues.map(vValue => [GetPlotPointX(vValue[0]), GetPlotPointY(vValue[2])]);
    const vPointsMax = vValues.map(vValue => [GetPlotPointX(vValue[0]), GetPlotPointY(vValue[3])]);
    const vPointsRange = vPointsMin.concat(vPointsMax.reverse());
    const vSvgPlotRange = ggSvg.CreatePolygon(vPointsRange);
    ggSvg.SetStyle(vSvgPlotRange, `fill:${this.mPlotLineColor}; fill-opacity:${this.mPlotRangeOpacity};  stroke-width:0;`);
    const vSvgPlotLine = ggSvg.CreatePolyline(vPoints);
    ggSvg.SetStyle(vSvgPlotLine, `stroke:${this.mPlotLineColor}; stroke-width:${this.mPlotLineWidth}; stroke-linejoin:bevel; fill:none;`);

    // compile SVG-objects (z-order)
    vSgvGroupMain.appendChild(vSvgPlotBackground);
    vSgvGroupMain.appendChild(vSvgLinesX);
    vSgvGroupMain.appendChild(vSvgLinesY);
    vSgvGroupMain.appendChild(vSvgPlotRange);
    vSgvGroupMain.appendChild(vSvgPlotLine);
    vSgvGroupMain.appendChild(vSvgLabelsX);
    vSgvGroupMain.appendChild(vSvgLabelsY);

    // clear old graphics
    this.mSvg.innerHTML = "";

    // add new graphics
    this.mSvg.appendChild(vSgvGroupMain);
  }
}
