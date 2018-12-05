typedef enum ModulatorType {
	WAVE,
	SCALARSPRING,
	SCALARGOALFOLLOWER,
	NEWTONIAN,
	SHIFTREGISTER
}ModulatorType;

//
//Modulator types
//

typedef struct ModWave {
	float amplitude;
	float frequency;
	uint64_t time;
	float  value;
	bool enabled;
}ModWave;

typedef struct ModScalarSpring {
	float smooth;
	float undamp;
	float goal;
	float value;
	float vel;
	uint64_t time;
	bool enabled;
}ModScalarSpring;

typedef struct ModScalarGoalFollower {
	float **regions;
	bool random_region;
	float threshold;
	float vel_threshold;
	uint64_t pause_range[2];
	Modulator *follower; 
	size_t current_region;
	uint64_t paused_left;
	uint64_t time;
	bool enabled;
}ModScalarGoalFollower;

typedef struct ModNewtonian {
	float speed_limit[2];
	float acceleration[2];
	float decelleration[2];
	float goal;
	float value;
	uint64_t time;
	bool enabled;
	float s;
	float a;
	float d;
	float f;
	float t[3];
}ModNewtonian;

typedef enum ShiftRegisterInterp{
	LINEAR,
	QUADRATIC,
	NONE
}ShiftRegisterInterp;

typedef struct ModShiftRegister {
	float **buckets; //TODO: array of buckets ->stretch_buf
	uint32_t **ages; //TODO: symmetrical array ->stretchy_buf
	float value_range[2];
	float odds;
	uint32_t age_range[2];
	float period;
	ShiftRegisterInterp interp;
	uint64_t time;
	float value;
	bool enabled;
}ModShiftRegister;

typedef struct Modulator {
	const char *name;
	ModulatorType type;
	union {
		ModWave wave;
		ModScalarSpring scalar_spring;
		ModScalarGoalFollower scalar_goal_follower;
		ModNewtonian newtonian;
		ModShiftRegister shift_register;
	};
}Modulator;

//
//Modulator memory management
//


Arena *modulator_arna = NULL;
Map *modulator_list = NULL;

Modulator *new_modulator(const char *name, ModulatorType type) {
	Modulator *mod = xmalloc(sizeof(Modulator));
	mod->name = name;
	mod->type = type;
	return mod;
}

Modulator *wave_modulator(const char *name, float amplitude, float frequency) {
	Modulator *m = new_modulator(name, WAVE);
	m->wave.amplitude = amplitude;
	m->wave.frequency = frequency;
	m->wave.time = 0;
	m->wave.value = 0.0;
	m->wave.enabled = true;
	return m;
}

Modulator *scalar_spring(const char *name, float smooth, float undamp, float initial) {
	Modulator *m = new_modulator(name, SCALARSPRING);
	m->scalar_spring.smooth = smooth;
	m->scalar_spring.undamp = undamp;
	m->scalar_spring.goal = initial;
	m->scalar_spring.value = initial;
	m->scalar_spring.vel = 0.0;
	m->scalar_spring.time = 0;
	return m;
}

Modulator *scalar_goal_follower(const char *name) {
	Modulator *m = new_modulator(name, SCALARGOALFOLLOWER);
	m->scalar_goal_follower.regions= NULL; //TODO -> array of arrays
	m->scalar_goal_follower.random_region = false;
	m->scalar_goal_follower.threshold = 0.01;
	m->scalar_goal_follower.vel_threshold = 0.0001;
	m->scalar_goal_follower.pause_range[0] = 0;
	m->scalar_goal_follower.pause_range[1] = 0;
	m->scalar_goal_follower.current_region = 0;
	m->scalar_goal_follower.paused_left = 0;
	m->scalar_goal_follower.time = 0;
	m->scalar_goal_follower.enabled = true;
	return m;
}

Modulator *newtonian(const char *name, float speed_limit[2], float acceleration[2], float deceleration[2], float initial) {
	Modulator *m = new_modulator(name, NEWTONIAN);
	m->newtonian.speed_limit[0] = speed_limit[0];
	m->newtonian.speed_limit[1] = speed_limit[1];
	m->newtonian.acceleration[0] = acceleration[0];
	m->newtonian.acceleration[1] = acceleration[1];
	m->newtonian.decelleration[0] = deceleration[0];
	m->newtonian.decelleration[1] = deceleration[1];
	m->newtonian.goal = initial;
	m->newtonian.value = initial;
	m->newtonian.time = 0;
	m->newtonian.enabled = true;
	m->newtonian.s = 0.0;
	m->newtonian.a = 0.0;
	m->newtonian.d = 0.0;
	m->newtonian.f = initial;
	m->newtonian.t[0] = 0.0;
	m->newtonian.t[1] = 0.0;
	m->newtonian.t[2] = 0.0;
	return m;
}

Modulator *shift_register(const char *name, size_t buckets, float value_range[2], float odds, float period, ShiftRegisterInterp interp) {
	Modulator *m = new_modulator(name, SHIFTREGISTER);
	m->shift_register.buckets = 0; //TODO innitialize as a stretchy buf
	float v = 0; //give a value based on the first bucket, or else 0
	m->shift_register.ages = 0; //TODO: create a 0 initialized stretchy buf based on the number of buckets
	m->shift_register.value_range[0] = value_range[0];
	m->shift_register.value_range[1] = value_range[1];
	m->shift_register.odds = odds;
	m->shift_register.age_range[0] = 0; //TODO set to max unsigned int32 
	m->shift_register.age_range[1] = 0; //TODO idem
	m->shift_register.period = period;
	m->shift_register.interp = interp;
	m->shift_register.time = 0;
	m->shift_register.value = v;
}



