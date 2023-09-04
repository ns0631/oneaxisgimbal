#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include "servos.h"

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const struct pwm_dt_spec continuous_servo = PWM_DT_SPEC_GET(DT_NODELABEL(continuous_servo));

static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse) / 1000;
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse) / 1000;

static const uint32_t min_pulse_continuous = 1000;//DT_PROP(DT_NODELABEL(continuous_servo), min_pulse) / 1000;
static const uint32_t max_pulse_continuous = 2000;//DT_PROP(DT_NODELABEL(continuous_servo), max_pulse) / 1000;

static float prev_angle = 0;
static float current_angle = 0;

void move_servo(float angle){
	int pwm_range = max_pulse - min_pulse;
	int half_range = pwm_range / 2;

	int middle_pulse = (max_pulse + min_pulse) / 2;

	//uint32_t pulse_width = min_pulse + pwm_range * angle / 120;
	uint32_t pulse_width = middle_pulse + half_range * angle / 85;

	if(pulse_width > max_pulse){
		pulse_width = max_pulse;
		current_angle = prev_angle;
	} else if (pulse_width < min_pulse){
		pulse_width = min_pulse;
		current_angle = prev_angle;
	} else{
		prev_angle = current_angle;
	}

	angle = current_angle;

	printk("Desired angle: %f; pulse width: %d\n", angle, pulse_width);
	
	int ret = pwm_set_pulse_dt(&servo, PWM_USEC(pulse_width));
	if (ret < 0) {
		printk("Error %d: failed to set pulse width\n", ret);
	}
}

void change_servo_angle(float angle){
	current_angle += angle;
	move_servo(current_angle);
}

float move_continuous_servo(float PID_output){
	int pwm_range = max_pulse_continuous - min_pulse_continuous;
	int half_range = pwm_range / 2;

	int middle_pulse = (max_pulse_continuous + min_pulse_continuous) / 2;

	//uint32_t pulse_width = min_pulse + pwm_range * angle / 120;
	uint32_t pulse_width = middle_pulse + half_range * PID_output / 5;

	if(pulse_width > max_pulse_continuous){
		pulse_width = max_pulse_continuous;
	} else if (pulse_width < min_pulse_continuous){
		pulse_width = min_pulse_continuous;
	}

	//printk("Continuous servo pulse width: %d\n", pulse_width);

	//pwm_set_pulse_dt(&continuous_servo, PWM_USEC(1600));
	//return 1400;
	
	int ret = pwm_set_pulse_dt(&continuous_servo, PWM_USEC(pulse_width));
	if (ret < 0) {
		printk("Error %d: failed to set pulse width\n", ret);
	}

	return pulse_width;
}

void setup_continuous_servo(){
	if (!device_is_ready(continuous_servo.dev)) {
		printk("Error: PWM device %s is not ready\n", continuous_servo.dev->name);
	}
	int ret = pwm_set_pulse_dt(&continuous_servo, PWM_USEC(1500));
}

void setup_servo(){
	if (!device_is_ready(servo.dev)) {
		printk("Error: PWM device %s is not ready\n", servo.dev->name);
	}
	move_servo(0);
}