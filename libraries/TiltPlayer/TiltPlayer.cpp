#include "TiltPlayer.h"
// How to get the values from the MPU6050 is based on this tutorial:
// https://projecthub.arduino.cc/Nicholas_N/how-to-use-the-accelerometer-gyroscope-gy-521-647e65

TiltPlayer::TiltPlayer(uint8_t address)
    : m_address(address) {}

void TiltPlayer::begin()
{
    Wire.beginTransmission(m_address);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);    // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
}

TiltPlayer::Tilt TiltPlayer::getTilt()

{
    Wire.beginTransmission(m_address);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(m_address, (uint8_t)6, (uint8_t) true); // request a total of 14 registers
    int AcX = Wire.read() << 8 | Wire.read();                // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    int AcY = Wire.read() << 8 | Wire.read();                // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    int AcZ = Wire.read() << 8 | Wire.read();                // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

    // The following values were analyzed by printing the values of AcX, AcY and AcZ
    // Whith these values we can quite good determine the tilt of the box
    if (AcY > 2500 || AcY < -2500)
    {
        return Tilt::None;
    }

    if (AcX < 18000 && AcX > 14000)
    {
        return Tilt::Left;
    }

    if (AcX > -18000 && AcX < -14000)
    {
        return Tilt::Right;
    }

    if (AcZ < 18000 && AcZ > 14000)
    {
        return Tilt::Forward;
    }

    if (AcZ > -18000 && AcZ < -14000)
    {
        return Tilt::Backward;
    }

    return Tilt::None;
}