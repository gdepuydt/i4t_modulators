//todo: use one ValueRange struct definition to generate value ranges for acceleration deceleration, etc. nstead of defining a new struct for basically the same thing.

typedef enum ModulatorType {
	WAVE,
	SCALARSPRING,
	SCALARGOALFOLLOWER,
	NEWTONIAN,
	SHIFTREGISTER
}ModulatorType;

typedef struct ValueRange {
	float min;
	float max;
}ValueRange;

//
//Modulator types
//

typedef struct Modulator Modulator;

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

typedef struct ModScalarGoalFollower{
	ValueRange *regions;
	bool random_region;
	float threshold;
	float vel_threshold;
	ValueRange pause_range;
	Modulator *follower; 
	size_t current_region;
	uint64_t paused_left;
	uint64_t time;
	bool enabled;
}ModScalarGoalFollower;

typedef struct PhaseTime {
	float acceleration;
	float sustain;
	float deceleration;
}PhaseTime;

typedef struct ModNewtonian {
	ValueRange speed_limit_range;
	ValueRange acceleration_range;
	ValueRange deceleration_range;
	float goal;
	float value;
	uint64_t time;
	bool enabled;
	float s;
	float a;
	float d;
	float f;
	PhaseTime phase;
}ModNewtonian;


typedef enum ShiftRegisterInterp{
	LINEAR,
	QUADRATIC,
	NONE
}ShiftRegisterInterp;

typedef struct ModShiftRegister {
	float *buckets; 
	uint32_t *value_ages;
	ValueRange value_range;
	float odds;
	ValueRange age_range;
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
	m->scalar_goal_follower.regions= NULL; //array of arrays
	m->scalar_goal_follower.random_region = false;
	m->scalar_goal_follower.threshold = 0.01;
	m->scalar_goal_follower.vel_threshold = 0.0001;
	m->scalar_goal_follower.pause_range.min = 0;
	m->scalar_goal_follower.pause_range.max = 0;
	m->scalar_goal_follower.current_region = 0;
	m->scalar_goal_follower.paused_left = 0;
	m->scalar_goal_follower.time = 0;
	m->scalar_goal_follower.enabled = true;
	return m;
}

void reset(Modulator *m, float value) {
	if (m->type == NEWTONIAN) {
		m->newtonian.value = value;
		m->newtonian.goal = value;
		m->newtonian.s = 0.0;
		m->newtonian.a = 0.0;
		m->newtonian.d = 0.0;
		m->newtonian.f = value;
		m->newtonian.phase = (PhaseTime) { 0.0, 0.0, 0.0 };
	}
	else exit(1); //TODO: better error handling?
	
}

float gen_value(ValueRange range) {
	return RNDRNG(range.min, range.max);
}

void calculate_events(Modulator *m) {
	if (m->type == NEWTONIAN) {
		float x = m->newtonian.goal - m->newtonian.f;
	}
	if (m->newtonian.a < FLT_EPSILON) {
		//TODO
	}
}

void move_to(Modulator *m, float goal) {
	if (m->type == NEWTONIAN) {
		m->newtonian.time = 0;
		m->newtonian.goal = goal;

		m->newtonian.s = gen_value(m->newtonian.speed_limit_range);
		m->newtonian.a = gen_value(m->newtonian.acceleration_range);
		m->newtonian.d = gen_value(m->newtonian.deceleration_range);
		m->newtonian.f = m->newtonian.value;

		calculate_events(m);
	}
}


Modulator *newtonian(const char *name, ValueRange speed_limit_range, ValueRange acceleration_range, ValueRange deceleration_range, float initial) {
	Modulator *m = new_modulator(name, NEWTONIAN);
	m->newtonian.speed_limit_range = speed_limit_range;
	m->newtonian.acceleration_range = acceleration_range;
	m->newtonian.deceleration_range = deceleration_range;
	m->newtonian.goal = initial;
	m->newtonian.value = initial;
	m->newtonian.time = 0;
	m->newtonian.enabled = true;
	m->newtonian.s = 0.0;
	m->newtonian.a = 0.0;
	m->newtonian.d = 0.0;
	m->newtonian.f = initial;
	m->newtonian.phase = (PhaseTime){0.0, 0.0, 0.0};
	return m;
}

void new_buckets(float *buffer, size_t buckets, ValueRange value_range) {
	for (int current_bucket = 0; current_bucket < buckets; current_bucket++) {
		float x = RNDRNG(value_range.min, value_range.max);
		buf_push(buffer, x); 
	}
}

// Total time of a shift register loop (period) in microseconds
uint64_t total_period(Modulator *m) {
	if (m->type == SHIFTREGISTER) {
		return (uint64_t)(m->shift_register.period * 1000000.0);
	}
	exit(1); //TODO: better error handling
}

// Time spent visiting a bucket, in microseconds
uint64_t bucket_period(Modulator *m) {
	if (m->type == SHIFTREGISTER) {
		size_t n = buf_len(m->shift_register.buckets);
		if (n > 0) {
			return (uint64_t)(total_period(m) / n);
		}
		else return 0;
	}
	exit(1);
}

// Return the bucket index after the one we are given
size_t next_bucket(Modulator *m, size_t index) {
	if (m->type == SHIFTREGISTER) {
		size_t len = buf_len(m->shift_register.buckets);
		if (len > 0) {
			if (index < len - 1) {
				return index + 1;
			}
		}
	}
	exit(1);
}

// Return the bucket index before the one we are given
size_t previous_bucket(Modulator *m, size_t index) {
	if (m->type == SHIFTREGISTER) {
		size_t len = buf_len(m->shift_register.buckets);
		if (len > 0) {
			if (index > 0 && index < len) {
				return index - 1;
			}
			else return len - 1;
		}
		return NULL; //TODO: not sure this is good practice --> maybe do fatal error here?
	}
	exit(1);
}


Modulator *shift_register(const char *name, size_t buckets, ValueRange value_range, float odds, float period, ShiftRegisterInterp interp) {
	Modulator *m = new_modulator(name, SHIFTREGISTER);
	
	m->shift_register.buckets = NULL; 
	new_buckets(m->shift_register.buckets, buckets, value_range);
	
	float v;
	if (buf_len(m->shift_register.buckets) > 0) {
		 v = m->shift_register.buckets[0]; 
	}
	else {
		v = 0.0;
	}
	
	m->shift_register.value_ages = NULL; //uninitialized buffer. will contain a age value for each bucket
	
	m->shift_register.value_range = value_range;
	m->shift_register.odds = odds;
	m->shift_register.age_range = (ValueRange){ UINT32_MAX, UINT32_MAX };  
	m->shift_register.period = period;
	m->shift_register.interp = interp;
	m->shift_register.time = 0;
	m->shift_register.value = v;
}

Modulator *modulator(const char *name, Modulator **env) {
	for (Modulator **it = env; it != buf_end(env); it++) {
		Modulator *mod = *it;
		if (strcmp(mod->name, name) == 0) {
			return mod;
		}
	}
	return NULL;
}



