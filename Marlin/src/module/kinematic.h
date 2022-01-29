//
// Created by shubin on 28.01.2022.
//

#ifndef MARLIN_KINEMATIC_H
#define MARLIN_KINEMATIC_H

#include "../MarlinCore.h"
#include "planner.h"

#if 0
#if IS_CORE
#if CORE_IS_XY
    #define CORE_AXIS_1 A_AXIS
    #define CORE_AXIS_2 B_AXIS
    #define NORMAL_AXIS Z_AXIS
  #elif CORE_IS_XZ
    #define CORE_AXIS_1 A_AXIS
    #define NORMAL_AXIS Y_AXIS
    #define CORE_AXIS_2 C_AXIS
  #elif CORE_IS_YZ
    #define NORMAL_AXIS X_AXIS
    #define CORE_AXIS_1 B_AXIS
    #define CORE_AXIS_2 C_AXIS
  #endif
  #define CORESIGN(n) (ANY(COREYX, COREZX, COREZY) ? (-(n)) : (n))
#elif ENABLED(MARKFORGED_XY)
// Markforged kinematics
  #define CORE_AXIS_1 A_AXIS
  #define CORE_AXIS_2 B_AXIS
  #define NORMAL_AXIS Z_AXIS
#endif
#endif

typedef struct : abce_float_t {
    struct { float x, y, z; } head;
} DistanceMM;

class Kinematic {
public:
    virtual AxisEnum getXHead() = 0;
    virtual AxisEnum getYHead() = 0;
    virtual float get_axis_steps(const AxisEnum axis) = 0;
    virtual void setToolheadDirection(uint32_t da, uint32_t db, uint32_t dc, axis_bits_t &dm) = 0;
    virtual void setBlockSteps(uint32_t da, uint32_t db, uint32_t dc, block_t * const block) = 0;
    virtual void calcDistance(uint32_t da, uint32_t db, uint32_t dc, DistanceMM &distance) = 0;
    virtual float calcMillimeters(DistanceMM &distance) = 0;
    virtual void enableActiveAxes(block_t * const block) = 0;
    virtual void axisDidMove(block_t * const block, axis_bits_t &dm) = 0;

};

class KinematicXYZ : public Kinematic {
public:
    float get_axis_steps(const AxisEnum axis) override;
    void setToolheadDirection(uint32_t da, uint32_t db, uint32_t dc, axis_bits_t &dm) override;
    void setBlockSteps(uint32_t da, uint32_t db, uint32_t dc, block_t * const block) override;
    void calcDistance(uint32_t da, uint32_t db, uint32_t dc, DistanceMM &distance) override;
    float calcMillimeters(DistanceMM &distance) override;
    void enableActiveAxes(block_t * const block) override;
    void axisDidMove(block_t * const block, axis_bits_t &dm) override;

    AxisEnum getXHead() override {
        return AxisEnum::X_AXIS;
    };
    AxisEnum getYHead() override {
        return AxisEnum::Y_AXIS;
    };
};

class KinematicCore : public Kinematic {
public:
    float get_axis_steps(const AxisEnum axis) override;
    void setToolheadDirection(uint32_t da, uint32_t db, uint32_t dc, axis_bits_t &dm) override;
    void setBlockSteps(uint32_t da, uint32_t db, uint32_t dc, block_t * const block) override;
    void calcDistance(uint32_t da, uint32_t db, uint32_t dc, DistanceMM &distance) override;
    float calcMillimeters(DistanceMM &distance) override;
    void enableActiveAxes(block_t * const block) override;
    void axisDidMove(block_t * const block, axis_bits_t &dm) override;

    AxisEnum getXHead() override {
        return AxisEnum::X_HEAD;
    };
    AxisEnum getYHead() override {
        return AxisEnum::Y_HEAD;
    };
};

class KinematicMarkforged : public Kinematic {
public:
    float get_axis_steps(const AxisEnum axis) override;
    void setToolheadDirection(uint32_t da, uint32_t db, uint32_t dc, axis_bits_t &dm) override;
    void setBlockSteps(uint32_t da, uint32_t db, uint32_t dc, block_t * const block) override;
    void calcDistance(uint32_t da, uint32_t db, uint32_t dc, DistanceMM &distance) override;
    float calcMillimeters(DistanceMM &distance) override;
    void enableActiveAxes(block_t * const block) override;
    void axisDidMove(block_t * const block, axis_bits_t &dm) override;

    AxisEnum getXHead() override {
        return AxisEnum::X_HEAD;
    };
    AxisEnum getYHead() override {
        return AxisEnum::Y_HEAD;
    };
};

extern Kinematic * kinematic;

#endif //MARLIN_KINEMATIC_H
