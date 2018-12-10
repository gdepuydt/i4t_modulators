
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

	Modulator **env1 = NULL; //List of pointers to modulators 

	Modulator *m1 = wave_modulator("wave_1", 1, 0.5);
	Modulator *m2 = scalar_spring("spring_1", 1, 1, 1);
	Modulator *m3 = scalar_goal_follower("follow_1");

	ValueRange s_limit_range_1 = { 0.5, 1 };
	ValueRange a_range_1 = { 0.1, 1 };
	ValueRange d_range_1 = { 0.1, 1 };
	Modulator *m4 = newtonian("newtonian_1", s_limit_range_1, a_range_1, d_range_1, 0.5);


	ValueRange v_range_1 = { 0, 1 };
	Modulator *m5 = shift_register("shift_1", 8, v_range_1, 0.2, 0.5, QUADRATIC);
	buf_push(env1, m1);
	buf_push(env1, m2);
	buf_push(env1, m3);
	buf_push(env1, m4);
	buf_push(env1, m5);


	for (Modulator **it = env1; it != buf_end(env1); it++) {
		Modulator *mod = *it;
		printf("%s\n", mod->name);
	}

	printf("print name of modulator follow_1:\n%s", modulator("follow_1", env1)->name);
}

void main() {
	printf("Hello Modulators!\n");
	modulator_test();
	getchar();
}