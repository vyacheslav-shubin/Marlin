//
// Created by shubin on 28.01.2022.
//

#include "kinematic.h"
#include "stepper.h"

float KinematicXYZ::get_axis_steps(const AxisEnum axis){
    return stepper.position(axis);
}

float KinematicCore::get_axis_steps(const AxisEnum axis){
    // Requesting one of the "core" axes?
    if (axis == AxisEnum::A_AXIS || axis == AxisEnum::B_AXIS) {

        // Protect the access to the position.
        const bool was_enabled = stepper.suspend();

        const int32_t p1 = stepper.position(AxisEnum::A_AXIS);
        const int32_t p2 = stepper.position(AxisEnum::B_AXIS);

        if (was_enabled) stepper.wake_up();

        // ((a1+a2)+(a1-a2))/2 -> (a1+a2+a1-a2)/2 -> (a1+a1)/2 -> a1
        // ((a1+a2)-(a1-a2))/2 -> (a1+a2-a1+a2)/2 -> (a2+a2)/2 -> a2
        return  (axis == AxisEnum::B_AXIS ? p1 - p2 : p1 + p2) * 0.5f;
    }
    else
        return  stepper.position(axis);
}

float KinematicMarkforged::get_axis_steps(const AxisEnum axis){
    // Requesting one of the joined axes?
    if (axis == AxisEnum::A_AXIS || axis == AxisEnum::B_AXIS) {
        // Protect the access to the position.
        const bool was_enabled = stepper.suspend();

        const int32_t p1 = stepper.position(AxisEnum::A_AXIS);
        const int32_t p2 = stepper.position(AxisEnum::B_AXIS);

        if (was_enabled) stepper.wake_up();

        return axis == AxisEnum::A_AXIS ? p1 + this->b_sign * p2 : p2;
    }
    else
        return stepper.position(axis);

}
void KinematicXYZ::setToolheadDirection(int32_t da, int32_t db, int32_t dc, axis_bits_t &dm) {
    if (da < 0) SBI(dm, X_AXIS);
    if (db < 0) SBI(dm, Y_AXIS);
    if (dc < 0) SBI(dm, Z_AXIS);
}

void KinematicCore::setToolheadDirection(int32_t da, int32_t db, int32_t dc, axis_bits_t &dm) {
    if (da < 0) SBI(dm, X_HEAD);                // Save the toolhead's true direction in X
    if (db < 0) SBI(dm, Y_HEAD);                // ...and Y
    if (dc < 0) SBI(dm, Z_AXIS);
    if (da + db < 0) SBI(dm, A_AXIS);           // Motor A direction
    if (da - db < 0) SBI(dm, B_AXIS); // Motor B direction
}

void KinematicMarkforged::setToolheadDirection(int32_t da, int32_t db, int32_t dc, axis_bits_t &dm) {
    if (da < 0) SBI(dm, X_HEAD);                // Save the toolhead's true direction in X
    if (db < 0) SBI(dm, Y_HEAD);                // ...and Y
    if (dc < 0) SBI(dm, Z_AXIS);
    if (da - this->b_sign * db < 0) SBI(dm, A_AXIS);           // Motor A direction
    if (db < 0) SBI(dm, B_AXIS);                // Motor B direction
}

void KinematicXYZ::setBlockSteps(int32_t da, int32_t db, int32_t dc, block_t * const block) {
    block->steps.set(ABS(da), ABS(db), ABS(dc));
};
void KinematicCore::setBlockSteps(int32_t da, int32_t db, int32_t dc, block_t * const block) {
    block->steps.set(ABS(da + db), ABS(da - db), ABS(dc));
};
void KinematicMarkforged::setBlockSteps(int32_t da, int32_t db, int32_t dc, block_t * const block) {
    block->steps.set(ABS(da - this->b_sign * db), ABS(db), ABS(dc));
};

void KinematicXYZ::calcDistance(int32_t da, int32_t db, int32_t dc, DistanceMM &distance) {
    distance.a = da * Planner::mm_per_step[A_AXIS];
    distance.b = db * Planner::mm_per_step[B_AXIS];
    distance.c = dc * Planner::mm_per_step[C_AXIS];
}

void KinematicCore::calcDistance(int32_t da, int32_t db, int32_t dc, DistanceMM &distance) {
    distance.head.x = da * Planner::mm_per_step[A_AXIS];
    distance.head.y = db * Planner::mm_per_step[B_AXIS];
    distance.z      = dc * Planner::mm_per_step[Z_AXIS];
    distance.a      = (da + db) * Planner::mm_per_step[A_AXIS];
    distance.b      = (da - db) * Planner::mm_per_step[B_AXIS];
}

void KinematicMarkforged::calcDistance(int32_t da, int32_t db, int32_t dc, DistanceMM &distance) {
    distance.head.x = da * Planner::mm_per_step[A_AXIS];
    distance.head.y = db * Planner::mm_per_step[B_AXIS];
    distance.z      = dc * Planner::mm_per_step[Z_AXIS];
    distance.a      = (da + this->b_sign * db) * Planner::mm_per_step[A_AXIS];
    distance.b      = db * Planner::mm_per_step[B_AXIS];
}

float KinematicXYZ::calcMillimeters(DistanceMM &distance) {
    return SQRT(sq(distance.x) + sq(distance.y) + sq(distance.z));
}
float KinematicCore::calcMillimeters(DistanceMM &distance) {
    return SQRT(sq(distance.head.x) + sq(distance.head.y) + sq(distance.z));
}
float KinematicMarkforged::calcMillimeters(DistanceMM &distance) {
    return SQRT(sq(distance.head.x) + sq(distance.head.y) + sq(distance.z));
}

void KinematicXYZ::enableActiveAxes(block_t * const block) {
    if (block->steps.x) stepper.enable_axis(X_AXIS);
    if (block->steps.y) stepper.enable_axis(Y_AXIS);
    if (TERN(Z_LATE_ENABLE, 0, block->steps.z)) stepper.enable_axis(Z_AXIS);
}
void KinematicCore::enableActiveAxes(block_t * const block) {
    if (block->steps.a || block->steps.b) {
        stepper.enable_axis(X_AXIS);
        stepper.enable_axis(Y_AXIS);
    }
    if (TERN(Z_LATE_ENABLE, 0, block->steps.z)) stepper.enable_axis(Z_AXIS);
}
void KinematicMarkforged::enableActiveAxes(block_t * const block) {
    if (block->steps.a || block->steps.b) {
        stepper.enable_axis(X_AXIS);
        stepper.enable_axis(Y_AXIS);
    }
    if (TERN(Z_LATE_ENABLE, 0, block->steps.z)) stepper.enable_axis(Z_AXIS);
}

void KinematicXYZ::axisDidMove(block_t * const block, axis_bits_t &dm){
    if (!!block->steps.a) SBI(dm, A_AXIS);
    if (!!block->steps.b) SBI(dm, B_AXIS);
    if (!!block->steps.c) SBI(dm, C_AXIS);
}

void KinematicCore::axisDidMove(block_t * const block, axis_bits_t &dm){
    if (
            (block->steps[A_AXIS] != block->steps[B_AXIS])
            ||
            (block->steps[A_AXIS] > 0 && (TEST(block->direction_bits, A_AXIS)==TEST(block->direction_bits, B_AXIS)))
    ) SBI(dm, A_AXIS);
    if (
            (block->steps[A_AXIS] != block->steps[B_AXIS])
            || (block->steps[A_AXIS] > 0 && (TEST(block->direction_bits, A_AXIS)!=TEST(block->direction_bits, B_AXIS)))
    ) SBI(dm, B_AXIS);
    if (!!block->steps.c) SBI(dm, C_AXIS);
}
void KinematicMarkforged::axisDidMove(block_t * const block, axis_bits_t &dm){
    if (block->steps.a != block->steps.b) SBI(dm, A_AXIS);
    if (!!block->steps.b) SBI(dm, B_AXIS);
    if (!!block->steps.c) SBI(dm, C_AXIS);
}

void KinematicXYZ::set_position(xyze_long_t &count_position, const abce_long_t &spos) {
    count_position = spos;
}
void KinematicCore::set_position(xyze_long_t &count_position, const abce_long_t &spos) {
    count_position.set(spos.a + spos.b, spos.a - spos.b, spos.c);
    count_position.e = spos.e;
}
void KinematicMarkforged::set_position(xyze_long_t &count_position, const abce_long_t &spos) {
    count_position.set(spos.a + this->b_sign * spos.b, spos.b, spos.c);
    count_position.e = spos.e;
}
int32_t KinematicXYZ::endstop_triggered(xyze_long_t &count_position, const AxisEnum axis) {
    return count_position[axis];
}
int32_t KinematicCore::endstop_triggered(xyze_long_t &count_position, const AxisEnum axis) {
    return ((axis == B_AXIS) ? count_position[A_AXIS] - count_position[B_AXIS] : count_position[A_AXIS] + count_position[B_AXIS]) * double(0.5);
}
int32_t KinematicMarkforged::endstop_triggered(xyze_long_t &count_position, const AxisEnum axis) {
    return (axis == A_AXIS)? count_position[A_AXIS] + this->b_sign * count_position[B_AXIS] : count_position[B_AXIS];
}

void KinematicXYZ::report_position(const xyz_long_t &pos) {
    SERIAL_ECHOLNPGM_P(PSTR(STR_COUNT_X), pos.x, SP_Y_LBL, pos.y, SP_Z_LBL, pos.z);
};
void KinematicCore::report_position(const xyz_long_t &pos) {
    SERIAL_ECHOLNPGM_P(PSTR(STR_COUNT_A), pos.x, PSTR("B:"), pos.y, SP_Z_LBL, pos.z);
};
void KinematicMarkforged::report_position(const xyz_long_t &pos) {
    SERIAL_ECHOLNPGM_P(PSTR(STR_COUNT_A), pos.x, PSTR("B:"), pos.y, SP_Z_LBL, pos.z);
};
