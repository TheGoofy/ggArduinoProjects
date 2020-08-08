"use strict"


export default class ggVector {

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
