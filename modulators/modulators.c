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
//Modulator structs
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

typedef struct ModulatorFunctions {
	float(*value)(Modulator *);
	ValueRange(*range)(Modulator *);
	float(*goal)(Modulator *);
	void(*set_goal)(Modulator *, float);
	uint64_t(*elapsed_us)(Modulator *);
	bool(*enabled)(Modulator *);
	void(*set_enabled)(Modulator *, bool);
	void(*advance)(Modulator *, uint64_t);
} ModulatorFunctions;

typedef struct Modulator {
	const ModulatorFunctions * const modulator_functions;
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
//public API functions that need to be implemented by all Modulator types
//

inline float value(Modulator *m) { return m->modulator_functions->value(m); }
inline ValueRange range(Modulator *m) { return m->modulator_functions->range(m); }
inline float goal(Modulator *m) { return m->modulator_functions->goal(m); }
inline void set_goal(Modulator *m, float f) { m->modulator_functions->set_goal(m, f); }
inline uint64_t elapsed_us(Modulator *m) { return m->modulator_functions->elapsed_us(m); }
inline bool enabled(Modulator *m) { return m->modulator_functions->enabled(m); }
inline void set_enabled(Modulator *m, bool enabled) { m->modulator_functions->set_enabled(m, enabled); }
inline void advance(Modulator *m, uint64_t dt) { m->modulator_functions->advance(m, dt); }

//
//private implementations of the different ModulatorFunctions
//

//--wave modulator--

float wave_val(Modulator *m) {
	printf("This is the wave_val function!\n");
	return 0;
}
ValueRange wave_range(Modulator *m) {
	printf("This is the wave_range function!\n");
	ValueRange r = { 0.0, 1.0 };
	return r;
}

float wave_goal(Modulator *m) {
	printf("This is the wave_goal function!\n");
	return 0;
}

void wave_set_goal(Modulator *m, float f) {
	printf("This is the wave_set_goal function!\n");
}

uint64_t wave_elapsed_us(Modulator *m) {
	printf("This is the wave_elapsed_us function!\n");
	return 0;
}

bool wave_enabled(Modulator *m) {
	printf("This is the wave_enabled function!\n");
	return 0;
}

void wave_set_enabled(Modulator *m, bool enabled) {
	printf("This is the wave_set_enabled function!\n");
}

void wave_advance(Modulator *m, uint64_t dt) {
	printf("This is the wave_advanced function!\n");
}

//--ScalarSpring

float scalar_spring_val(Modulator *m) {
	printf("This is the scalar_spring_val function!\n");
	return 0;
}
ValueRange scalar_spring_range(Modulator *m) {
	printf("This is the scalar_spring_range function!\n");
	ValueRange r = { 0.0, 1.0 };
	return r;
}

float scalar_spring_goal(Modulator *m) {
	printf("This is the scalar_spring_goal function!\n");
	return 0;
}

void scalar_spring_set_goal(Modulator *m, float f) {
	printf("This is the scalar_spring_set_goal function!\n");
}

uint64_t scalar_spring_elapsed_us(Modulator *m) {
	printf("This is the scalar_spring_elapsed_us function!\n");
	return 0;
}

bool scalar_spring_enabled(Modulator *m) {
	printf("This is the scalar_spring_enabled function!\n");
	return 0;
}

void scalar_spring_set_enabled(Modulator *m, bool enabled) {
	printf("This is the scalar_spring_set_enabled function!\n");
}

void scalar_spring_advance(Modulator *m, uint64_t dt) {
	printf("This is the scalar_spring_advanced function!\n");
}

//--ScalarGoalFollower

float scalar_goal_follower_val(Modulator *m) {
	printf("This is the scalar_goal_follower_val function!\n");
	return 0;
}
ValueRange scalar_goal_follower_range(Modulator *m) {
	printf("This is the scalar_goal_follower_range function!\n");
	ValueRange r = { 0.0, 1.0 };
	return r;
}

float scalar_goal_follower_goal(Modulator *m) {
	printf("This is the scalar_goal_follower_goal function!\n");
	return 0;
}

void scalar_goal_follower_set_goal(Modulator *m, float f) {
	printf("This is the scalar_goal_follower_set_goal function!\n");
}

uint64_t scalar_goal_follower_elapsed_us(Modulator *m) {
	printf("This is the scalar_goal_follower_elapsed_us function!\n");
	return 0;
}

bool scalar_goal_follower_enabled(Modulator *m) {
	printf("This is the scalar_goal_follower_enabled function!\n");
	return 0;
}

void scalar_goal_follower_set_enabled(Modulator *m, bool enabled) {
	printf("This is the scalar_goal_follower_set_enabled function!\n");
}

void scalar_goal_follower_advance(Modulator *m, uint64_t dt) {
	printf("This is the scalar_goal_follower_advanced function!\n");
}

//--Newtonian

float newtonian_val(Modulator *m) {
	printf("This is the newtonian_val function!\n");
	return 0;
}
ValueRange newtonian_range(Modulator *m) {
	printf("This is the newtonian_range function!\n");
	ValueRange r = { 0.0, 1.0 };
	return r;
}

float newtonian_goal(Modulator *m) {
	printf("This is the newtonian_goal function!\n");
	return 0;
}

void newtonian_set_goal(Modulator *m, float f) {
	printf("This is the newtonian_set_goal function!\n");
}

uint64_t newtonian_elapsed_us(Modulator *m) {
	printf("This is the newtonian_elapsed_us function!\n");
	return 0;
}

bool newtonian_enabled(Modulator *m) {
	printf("This is the newtonian_enabled function!\n");
	return 0;
}

void newtonian_set_enabled(Modulator *m, bool enabled) {
	printf("This is the newtonian_set_enabled function!\n");
}

void newtonian_advance(Modulator *m, uint64_t dt) {
	printf("This is the newtonian_advanced function!\n");
}

//--ShiftRegister

float shiftregister_val(Modulator *m) {
	printf("This is the shiftregister_val function!\n");
	return 0;
}
ValueRange shiftregister_range(Modulator *m) {
	printf("This is the shiftregister_range function!\n");
	ValueRange r = { 0.0, 1.0 };
	return r;
}

float shiftregister_goal(Modulator *m) {
	printf("This is the shiftregister_goal function!\n");
	return 0;
}

void shiftregister_set_goal(Modulator *m, float f) {
	printf("This is the shiftregister_set_goal function!\n");
}

uint64_t shiftregister_elapsed_us(Modulator *m) {
	printf("This is the shiftregister_elapsed_us function!\n");
	return 0;
}

bool shiftregister_enabled(Modulator *m) {
	printf("This is the shiftregister_enabled function!\n");
	return 0;
}

void shiftregister_set_enabled(Modulator *m, bool enabled) {
	printf("This is the shiftregister_set_enabled function!\n");
}

void shiftregister_advance(Modulator *m, uint64_t dt) {
	printf("This is the shiftregister_advanced function!\n");
}


//
//Modulator constructor and helper functions
//


Modulator *new_modulator(const char *name, ModulatorType type, ModulatorFunctions *wave_functions) {
	Modulator *mod = xmalloc(sizeof(Modulator));
	memcpy(&mod->modulator_functions, &wave_functions, sizeof(wave_functions));
	mod->name = name;
	mod->type = type;
	return mod;
}

Modulator *wave_modulator(const char *name, float amplitude, float frequency) {
	static const ModulatorFunctions wave_functions = {
		wave_val, wave_range, wave_goal, wave_set_goal, wave_elapsed_us, wave_enabled, wave_set_enabled, wave_advance
	};
	Modulator *m = new_modulator(name, WAVE, &wave_functions);
	m->wave.amplitude = amplitude;
	m->wave.frequency = frequency;
	m->wave.time = 0;
	m->wave.value = 0.0;
	m->wave.enabled = true;
	return m;
}

Modulator *scalar_spring(const char *name, float smooth, float undamp, float initial) {
	static const ModulatorFunctions scalar_spring_functions = {
	scalar_spring_val, scalar_spring_range, scalar_spring_goal, scalar_spring_set_goal, scalar_spring_elapsed_us, scalar_spring_enabled, scalar_spring_set_enabled, scalar_spring_advance
	};
	Modulator *m = new_modulator(name, SCALARSPRING, &scalar_spring_functions);
	m->scalar_spring.smooth = smooth;
	m->scalar_spring.undamp = undamp;
	m->scalar_spring.goal = initial;
	m->scalar_spring.value = initial;
	m->scalar_spring.vel = 0.0;
	m->scalar_spring.time = 0;
	return m;
}

Modulator *scalar_goal_follower(const char *name) {
	static const ModulatorFunctions scalar_goal_follower_functions = {
	scalar_goal_follower_val, scalar_goal_follower_range, scalar_goal_follower_goal, scalar_goal_follower_set_goal, scalar_goal_follower_elapsed_us, scalar_goal_follower_enabled, scalar_goal_follower_set_enabled, scalar_goal_follower_advance
	};
	Modulator *m = new_modulator(name, SCALARGOALFOLLOWER, &scalar_goal_follower_functions);
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
	static const ModulatorFunctions newtonian_functions = {
	newtonian_val, newtonian_range, newtonian_goal, newtonian_set_goal, newtonian_elapsed_us, newtonian_enabled, newtonian_set_enabled, newtonian_advance
	};
	Modulator *m = new_modulator(name, NEWTONIAN, &newtonian_functions);
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
	static const ModulatorFunctions shift_register_functions = {
	shiftregister_val, shiftregister_range, shiftregister_goal, shiftregister_set_goal, shiftregister_elapsed_us, shiftregister_enabled, shiftregister_set_enabled, shiftregister_advance
	};
	Modulator *m = new_modulator(name, SHIFTREGISTER, &shift_register_functions);
	
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

//
//ModulatorEnvirnment HashMap for managing Modulators
//

typedef struct ModulatorEnvironment {
	const char* name;
	Map modulator_map;
} ModulatorEnvironment;

Map env_map;

ModulatorEnvironment *create_environment(const char *environment_name) {
	ModulatorEnvironment *new_env = xcalloc(1, sizeof(ModulatorEnvironment));
	new_env->name = environment_name;
	return new_env;
}

void add_modulator(const char *environment_name, Modulator *modulator) {
	ModulatorEnvironment *env = map_get(&env_map, environment_name);
	if (env) {
		map_put(&env->modulator_map, modulator->name, modulator);
	}
	else {
		ModulatorEnvironment *new_env = create_environment(environment_name);
		map_put(&env_map, new_env->name, new_env);
		map_put(&new_env->modulator_map, modulator->name, modulator);
	}
}
