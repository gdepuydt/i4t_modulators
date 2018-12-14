
#define _CRT_SECURE_NO_WARNINGS

#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <float.h>



#include "../../i4t_lib/src/common.c"
#include "modulators.c"


void modulator_test() {
	
	//
	//For the time being a dynamic array will store the pointers to the different modulators.
	//Each modifier has a unique name by which you can identify it 
	//

	Modulator *m1 = wave_modulator("wave_1", 1, 0.5);
	Modulator *m2 = scalar_spring("spring_1", 1, 1, 1);
	Modulator *m3 = scalar_goal_follower("follow_1");

	value(m1);
	range(m2);
	goal(m1);
	set_goal(m2, 0.0);
	elapsed_us(m1);
	enabled(m2);
	set_enabled(m1, true);
	advance(m2, 100);
	value(m3);


	ValueRange s_limit_range_1 = { 0.5, 1 };
	ValueRange a_range_1 = { 0.1, 1 };
	ValueRange d_range_1 = { 0.1, 1 };
	Modulator *m4 = newtonian("newtonian_1", s_limit_range_1, a_range_1, d_range_1, 0.5);

	value(m4);
	range(m4);
	goal(m4);
	set_goal(m4, 0.0);
	elapsed_us(m4);
	enabled(m4);
	set_enabled(m4, true);
	advance(m4, 100);


	ValueRange v_range_1 = { 0, 1 };
	Modulator *m5 = shift_register("shift_1", 8, v_range_1, 0.2, 0.5, QUADRATIC);

	value(m5);
	range(m5);
	goal(m5);
	set_goal(m5, 0.0);
	elapsed_us(m5);
	enabled(m5);
	set_enabled(m5, true);
	advance(m5, 100);

	add_modulator("env1", m1);
	add_modulator("env1", m2);
	add_modulator("env1", m3);
	add_modulator("env2", m4);
	add_modulator("env3", m5);

	for (int i = 0; i < env_map.cap; i++) {
		if (env_map.keys[i]) {
			ModulatorEnvironment *env = ((ModulatorEnvironment*)env_map.vals[i]);
			printf("Environment: \"%s\"\n", env->name);
			for (int j = 0; j < env->modulator_map.cap; j++) {
				if (env->modulator_map.keys[j]) {
					Modulator *mod = (Modulator*)env->modulator_map.vals[j];
					printf("	Modulator: \"%s\"\n",mod->name);
				}
			}
		}
	}
}

void main() {
	printf("Hello Modulators!\n");
	modulator_test();
	getchar();
}