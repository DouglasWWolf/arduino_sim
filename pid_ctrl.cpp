//=========================================================================================================
// pid_ctrl.cpp - Implements a classic PID controller
//=========================================================================================================
#include "pid_ctrl.h"


//=========================================================================================================
// set_output_limits() Define the minimum and maximum legal values for the output
//=========================================================================================================
void CPIDController::set_output_limits(pid_t lower_limit, pid_t upper_limit)
{
    m_lower_limit = lower_limit;
    m_upper_limit = upper_limit;
}
//=========================================================================================================


//=========================================================================================================
// new_setpoint() - Starts a new setpoint.
//=========================================================================================================
void CPIDController::new_setpoint(pid_t setpoint)
{
    m_setpoint = setpoint;
}
//=========================================================================================================


//=========================================================================================================
// reset() - Resets the integral term
//=========================================================================================================
void CPIDController::reset()
{
    m_integral = m_previous_error = 0;
}
//=========================================================================================================




//=========================================================================================================
// set_constants() - Stores new constants and resets the integral term
//=========================================================================================================
void CPIDController::set_constants(pid_t kp, pid_t ki, pid_t kd)
{
    m_kp = kp;
    m_ki = ki;
    m_kd = kd;
    reset();
}
//=========================================================================================================


//=========================================================================================================
// compute() - Compute a new output value
// 
// Passed: pv = Present value of the thing being controlled (temperature, etc)
//         dt = Amount of time that has elapsed since the last time this was called
//
// Do NOT pass 0 (or even very small numbers) for dt!!
//=========================================================================================================
pid_t CPIDController::compute(pid_t pv, pid_t dt)
{
    // How far away is the present value from the desired setpoint?
    pid_t error = m_setpoint - pv;

    // Compute the proportional term
    pid_t P = m_kp * error;

    // Accumuate error
    m_integral += error * dt;

    // Compute the integral term
    pid_t I = m_ki * m_integral;

    // Compute the rate of change of the error
    pid_t rate_of_change = (error - m_previous_error) / dt;
    
    // Comute the derivative term
    pid_t D = m_kd * rate_of_change;

    // Compute the new output value
    pid_t output = P + I + D;

    // Make sure we keep coloring inside the lines
    if (output > m_upper_limit)
        output = m_upper_limit;
    else if (output < m_lower_limit)
        output = m_lower_limit;

    // Save the error so we can compute the rate of change on the next pass
    m_previous_error = error;

    // Hand the caller the new output setting
    return output;
}
//=========================================================================================================
