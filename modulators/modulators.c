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
	return m->wave.value;
}
ValueRange wave_range(Modulator *m) {
	ValueRange r = { -m->wave.amplitude, m->wave.amplitude};
	return r;
}

float wave_goal(Modulator *m) {
	return m->wave.value;
}

void wave_set_goal(Modulator *m, float f) {
}

uint64_t wave_elapsed_us(Modulator *m) {
	return m->wave.time;
}

bool wave_enabled(Modulator *m) {
	return m->wave.enabled;
}

void wave_set_enabled(Modulator *m, bool enabled) {
	m->wave.enabled = enabled;
}

void wave_advance(Modulator *m, uint64_t dt) {
	m->wave.time += dt;
	//TODO: m->wave.value = ?? (self.wave)(self, ModulatorEnv::<f32>::micros_to_secs(self.time));
}

//--ScalarSpring


//Update the target the spring is moving to
void spring_to(Modulator *m, float goal) {
	assert(m->type == SCALARSPRING);
	m->scalar_spring.value = goal;
}

//Jump immediately to the given goal, zero velocity
void jump_to(Modulator *m, float goal) {
	assert(m->type == SCALARSPRING);
	m->scalar_spring.goal = goal;
	m->scalar_spring.value = goal;
	m->scalar_spring.vel = 0.0;
}

float scalar_spring_val(Modulator *m) {
	return m->scalar_spring.value;
}
ValueRange scalar_spring_range(Modulator *m) {
	ValueRange r = {0.0, 0.0};
	return r;
}

float scalar_spring_goal(Modulator *m) {
	return m->scalar_spring.goal;
}

void scalar_spring_set_goal(Modulator *m, float f) {
	spring_to(m, f);
}

uint64_t scalar_spring_elapsed_us(Modulator *m) {
	return m->scalar_spring.time;
}

bool scalar_spring_enabled(Modulator *m) {
	return m->scalar_spring.enabled;
}

void scalar_spring_set_enabled(Modulator *m, bool enabled) {
	m->scalar_spring.enabled = enabled;
}

void scalar_spring_advance(Modulator *m, uint64_t dt) {
	m->scalar_spring.time += dt;
	if (m->scalar_spring.smooth < 0.0001) {
		m->scalar_spring.value = m->scalar_spring.goal;
		m->scalar_spring.vel = 0.0;
	}
	else {
		float _dt = micros_to_secs(dt);
		float omega = 2.0 / m->scalar_spring.smooth;
		float x = omega * _dt;
		float ex = 1.0 / expf(x);
		float ud = _dt * m->scalar_spring.undamp;

		float d = m->scalar_spring.value - m->scalar_spring.goal;
		float v = m->scalar_spring.vel;
		float t = (v + omega * d) * _dt;

		m->scalar_spring.vel = (v - omega * t) * ex + v * ud;
		m->scalar_spring.value = m->scalar_spring.goal + (d + t) * ex;
	}
}

//--ScalarGoalFollower

void set_new_goal(Modulator *m) {
	assert(m->type == SCALARGOALFOLLOWER);
	size_t n = buf_len(m->scalar_goal_follower.regions);
	if (n > 0) {
		if (m->scalar_goal_follower.random_region) {
			m->scalar_goal_follower.current_region = RNDRNG(0, n);
		}
		else if (m->scalar_goal_follower.current_region + 1 < n) {
			m->scalar_goal_follower.current_region += 1;
		}
		else {
			m->scalar_goal_follower.current_region = 0;
		}

		ValueRange *region = &m->scalar_goal_follower.regions[m->scalar_goal_follower.current_region];
		float goal = 0.0;
		if (region->max > region->min) {
			goal = RNDRNG(region->min, region->max);
		}
		else {
			goal = region->min;
		}
		set_goal(m->scalar_goal_follower.follower, goal);
	}
}



float scalar_goal_follower_val(Modulator *m) {
	return value(m->scalar_goal_follower.follower);
}

ValueRange scalar_goal_follower_range(Modulator *m) {
	size_t n = buf_len(m->scalar_goal_follower.regions);
	ValueRange r = { 0.0, 0.0 };
	if (n > 0) {
		r = m->scalar_goal_follower.regions[0];
	}

	for (ValueRange *it = &m->scalar_goal_follower.regions[1]; it != buf_end(m->scalar_goal_follower.regions); it++) {
		if (it->min < r.min) {
			r.min = it->min;
		}
		if (it->max > r.max) {
			r.max = it->max;
		}
	}
	return r;
}

float scalar_goal_follower_goal(Modulator *m) {
	return goal(m->scalar_goal_follower.follower);
}

void scalar_goal_follower_set_goal(Modulator *m, float goal) {
	set_goal(m->scalar_goal_follower.follower, goal);
}

uint64_t scalar_goal_follower_elapsed_us(Modulator *m) {
	return m->scalar_goal_follower.time;
}

bool scalar_goal_follower_enabled(Modulator *m) {
	return m->scalar_goal_follower.enabled;
}

void scalar_goal_follower_set_enabled(Modulator *m, bool enabled) {
	m->scalar_goal_follower.enabled = enabled;
}

void scalar_goal_follower_advance(Modulator *m, uint64_t dt) {
	m->scalar_goal_follower.time += dt;

	if (m->scalar_goal_follower.paused_left > 0) {
		m->scalar_goal_follower.paused_left -= (uint64_t)MIN((m->scalar_goal_follower.paused_left), dt);
	}
	else {
		float p0 = value(m->scalar_goal_follower.follower);
		advance(m->scalar_goal_follower.follower, dt);
		float p1 = value(m->scalar_goal_follower.follower);
		float secs = micros_to_secs(dt);
		float vel = 0.0;
		if (secs > FLT_MIN) {
			vel = (p1 - p0) / secs;
		}
		else {
			vel = 0.0;
		}

		if (p1 - fabs(goal(m->scalar_goal_follower.follower)) > m->scalar_goal_follower.threshold || fabs(vel) > m->scalar_goal_follower.vel_threshold) {
			return; //Still moving towards goal
		}
		if (m->scalar_goal_follower.pause_range.max > m->scalar_goal_follower.pause_range.min) {
			m->scalar_goal_follower.paused_left = RNDRNG(m->scalar_goal_follower.pause_range.min, m->scalar_goal_follower.pause_range.max);
		}
		else {
			m->scalar_goal_follower.paused_left = m->scalar_goal_follower.pause_range.min;
		}
	}

	if (m->scalar_goal_follower.paused_left == 0) {
		set_new_goal(m); //done pausing, resume following
	}
}

//--Newtonian

void reset(Modulator *m, float value) {
	assert(m->type == NEWTONIAN);
	m->newtonian.value = value;
	m->newtonian.goal = value;
	m->newtonian.s = 0.0;
	m->newtonian.a = 0.0;
	m->newtonian.d = 0.0;
	m->newtonian.f = value;
	m->newtonian.phase = (PhaseTime) { 0.0, 0.0, 0.0 };
}

float gen_value(ValueRange range) {
	return RNDRNG(range.min, range.max);
}

void calculate_events(Modulator *m) {
	assert(m->type == NEWTONIAN);
	float x = abs(m->newtonian.goal - m->newtonian.f);

	float a = 0.0;
	if (m->newtonian.a < FLT_EPSILON) {
		a = m->newtonian.a;
	}
	else {
		a = 1000000.0;
	}
	float d = 0.0;
	if (m->newtonian.d > FLT_EPSILON) {
		d = m->newtonian.d;
	}
	else {
		d = 1000000.0;
	}
	float r = a / d;

	m->newtonian.phase.acceleration = sqrtf((x * 2.0 / (a * (1.0 + r))));

	float v = a * m->newtonian.phase.acceleration;
	if (v > m->newtonian.s) {
		v = m->newtonian.s;
		m->newtonian.phase.acceleration = m->newtonian.s / a;
	}
	else {
		m->newtonian.s = v;
	}

	m->newtonian.phase.deceleration = m->newtonian.phase.acceleration * r;

	float d0 = m->newtonian.phase.acceleration *  m->newtonian.phase.acceleration * a * 0.5;
	float d2 = m->newtonian.phase.deceleration *  m->newtonian.phase.deceleration * d * 0.5;

	m->newtonian.phase.sustain = (x - d0 - d2) / v;

	if (m->newtonian.goal > m->newtonian.f) {
		m->newtonian.a = a;
		m->newtonian.d = -d;
	}
	else {
		m->newtonian.s = -m->newtonian.s;
		m->newtonian.a = -a;
		m->newtonian.d = d;
	}

	m->newtonian.phase.sustain = m->newtonian.phase.acceleration + m->newtonian.phase.sustain;
	m->newtonian.phase.deceleration = m->newtonian.phase.sustain + m->newtonian.phase.deceleration;
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

float accelerate(float a, float t) {
	return a * t *t *0.5;
}

float forward(float s, float t) {
	return s * t;
}

float newtonian_val(Modulator *m) {
	return m->newtonian.value;
}
ValueRange newtonian_range(Modulator *m) {
	ValueRange r = { 0.0, 0.0 };
	return r;
}

float newtonian_goal(Modulator *m) {
	return m->newtonian.goal;
}

void newtonian_set_goal(Modulator *m, float goal) {
	move_to(m, goal);
}

uint64_t newtonian_elapsed_us(Modulator *m) {
	return m->newtonian.time;
}

bool newtonian_enabled(Modulator *m) {
	return m->newtonian.enabled;
}

void newtonian_set_enabled(Modulator *m, bool enabled) {
	m->newtonian.enabled = enabled;
}

void newtonian_advance(Modulator *m, uint64_t dt) {
	
	m->newtonian.time += dt;
	float t = micros_to_secs(m->newtonian.time); //time to goal
	float a = m->newtonian.phase.acceleration;
	float d = m->newtonian.phase.deceleration;
	float s = m->newtonian.phase.sustain;

	m->newtonian.value = m->newtonian.f + accelerate(m->newtonian.a, MIN(t, a));
	if (t > a) {
		m->newtonian.value = m->newtonian.value + forward(m->newtonian.s, MIN(t, d) - a);
		if (t > s) {
			m->newtonian.value = m->newtonian.value + accelerate(m->newtonian.d, MIN(t, d) - s);
		}
	}
}

//--ShiftRegister

void new_buckets(float *buffer, size_t buckets, ValueRange value_range) {
	for (int current_bucket = 0; current_bucket < buckets; current_bucket++) {
		float x = RNDRNG(value_range.min, value_range.max);
		buf_push(buffer, x);
	}
}

// Total time of a shift register loop (period) in microseconds
uint64_t total_period(Modulator *m) {
	assert(m->type == SHIFTREGISTER);
	return (uint64_t)(m->shift_register.period * 1000000.0);
}

// Time spent visiting a bucket, in microseconds
uint64_t bucket_period(Modulator *m) {
	assert(m->type == SHIFTREGISTER);
	size_t n = buf_len(m->shift_register.buckets);
	if (n > 0) {
		return (uint64_t)(total_period(m) / n);
	}
	else return 0;
}

// Return the bucket index after the one we are given
size_t next_bucket(Modulator *m, size_t index) {
	assert(m->type == SHIFTREGISTER);
	size_t len = buf_len(m->shift_register.buckets);
	if (len > 0) {
		if (index < len - 1) {
			return index + 1;
		}
	}
}

// Return the bucket index before the one we are given
size_t previous_bucket(Modulator *m, size_t index, size_t n) {
	assert(m->type == SHIFTREGISTER);
	assert(n > 0);
	if (index > 0 && index < n) {
		return index - 1;
	}
	else return n - 1;
}


float shiftregister_val(Modulator *m) {
	return m->shift_register.value;
}
ValueRange shiftregister_range(Modulator *m) {
	return m->shift_register.value_range;
}

float shiftregister_goal(Modulator *m) {
	return m->shift_register.value;
}

void shiftregister_set_goal(Modulator *m, float f) {
}

uint64_t shiftregister_elapsed_us(Modulator *m) {
	return m->shift_register.time;
}

bool shiftregister_enabled(Modulator *m) {
	return m->shift_register.enabled;
}

void shiftregister_set_enabled(Modulator *m, bool enabled) {
	m->shift_register.enabled = enabled;
}

void shiftregister_advance(Modulator *m, uint64_t dt) {
	size_t n = buf_len(m->shift_register.buckets);
	uint64_t p = total_period(m);
	uint64_t bp = bucket_period(m);
	if (n == 0 || p == 0 || bp == 0) {
		return;
	}
	
	uint64_t pt = m->shift_register.time % p; //convert accumulated time into period time
	size_t bi = (size_t)(MIN((size_t)(pt / bp), n - 1)); //current bucket in period

	uint64_t bt = pt - bp * bi; //time aready spent visiting the current bucket
	uint64_t r = (bt + dt) / bp; //number of buckets we are going to visit

	for (int i = 0; i < r; i++) {
		size_t bh = previous_bucket(m, bi, n);
		float odds = (float)(MIN(MAX(0.0, m->shift_register.odds), 1.0));

		if (m->shift_register.value_ages[bh] >= m->shift_register.age_range.min && m->shift_register.age_range.min < m->shift_register.age_range.max) {
			float t = MIN((float)(m->shift_register.value_ages[bh] - m->shift_register.age_range.min) / (m->shift_register.age_range.max - m->shift_register.age_range.min),  1.0);
			odds = odds + (1.0 - odds) * t;
		}
		
		if (RND() < odds) {
			m->shift_register.buckets[bh] = RNDRNG(m->shift_register.age_range.min, m->shift_register.age_range.max);
			m->shift_register.value_ages[bh] = 0;
		}
		else {
			m->shift_register.value_ages[bh] += 1;
		}

		bi = next_bucket(m, bi, n);
	}

	m->shift_register.time += dt;
	switch (m->shift_register.interp) {
	case(QUADRATIC): {
		size_t bh = previous_bucket(m, bi, n);
		size_t bj = next_bucket(m, bi, n);

		float v1 = m->shift_register.buckets[bi];
		float v0 = (m->shift_register.buckets[bh] + v1) * 0.5;
		float v2 = (m->shift_register.buckets[bj] + v1) * 0.5;

		uint64_t bt = (uint64_t)(m->shift_register.time % p - bp * bi);
		float tt = (float)bt / (float)bp;
		
		float a0 = v0 + (v1 - v0) * tt;
		float a1 = v1 + (v2 + v1) * tt;

		m->shift_register.value = a0 + (a1 - a0) * tt;
		break;
	}
	case(LINEAR): {
		float v0 = m->shift_register.buckets[bi];
		float v1 = next_bucket(m, bi, n);
		uint64_t bt = (uint64_t)(m->shift_register.time % p - bp * bi);
		m->shift_register.value = v0 + (v1 - v0) * ((float)bt / (float)bp);
		break;
	}
		
	case(NONE):
	default:
		m->shift_register.value = m->shift_register.buckets[bi];
		break;
	}
}


//
//Modulator constructors
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
