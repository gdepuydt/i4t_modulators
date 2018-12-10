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

typedef struct PauseRange {
	float min;
	float max;
}PauseRange;

typedef struct Region {
	float min;
	float max;
}Region;


typedef struct ModScalarGoalFollower{
	Region *regions;
	bool random_region;
	float threshold;
	float vel_threshold;
	PauseRange pause_range;
	Modulator *follower; 
	size_t current_region;
	uint64_t paused_left;
	uint64_t time;
	bool enabled;
}ModScalarGoalFollower;

typedef struct SpeedLimitRange {
	float min;
	float max;
}SpeedLimitRange;

typedef struct AccelerationRange {
	float min;
	float max;
}AccelerationRange;

typedef struct DecelerationRange {
	float min;
	float max;
}DecelerationRange;

typedef struct PhaseTime {
	float acceleration;
	float sustain;
	float deceleration;
}PhaseTime;

typedef struct ModNewtonian {
	SpeedLimitRange speed_limit_range;
	AccelerationRange acceleration_range;
	DecelerationRange deceleration_range;
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

typedef struct ValueRange {
	float min;
	float max;
}ValueRange;

typedef struct AgeRange {
	uint32_t min;
	uint32_t max;
}AgeRange;

typedef struct ModShiftRegister {
	float *buckets; //TODO: array of buckets ->stretch_buf
	uint32_t *value_ages; //TODO: symmetrical array ->stretchy_buf
	ValueRange value_range;
	float odds;
	AgeRange age_range;
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
	m->scalar_goal_follower.regions= NULL; //TODO -> array of arrays
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

Modulator *newtonian(const char *name, SpeedLimitRange speed_limit_range, AccelerationRange acceleration_range, DecelerationRange deceleration_range, float initial) {
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
		float range = value_range.max - value_range.min;
		float x = ((float)rand() / (float)(RAND_MAX / range)) + value_range.min; //TODO: test this!
		buf_push(buffer, x); 
	}
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
	
	m->shift_register.value_ages = 0; //TODO: create a 0 initialized stretchy buf based on the number of buckets
	
	m->shift_register.value_range = value_range;
	m->shift_register.odds = odds;
	m->shift_register.age_range = (AgeRange){ UINT32_MAX, UINT32_MAX };  
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



