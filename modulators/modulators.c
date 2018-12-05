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
	float regions[2];
	bool random_region;
	float threshold;
	float vel_threshold;
	uint64_t pause_range[2];
	Modulator *follower; 
	size_t current_region;
	uint64_t paused_left;
	uint64_t time;
}ModScalarGoalFollower;

typedef struct ModNewtonian {
	float speed_limit[2];
	float acceleration[2];
	float decelleration[2];
	float goal;
	float value;
	uint64_t time;
	bool enabled;
}ModNewtonian;

typedef enum ShiftRegisterInterp{
	LINEAR,
	QUADRATIC,
	NONE
}ShiftRegisterInterp;

typedef struct ModShiftRegister {
	float **buckets; //TODO: array of buckets
	uint32_t **ages; //TODO: symmetrical array
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

void create_modulator(const char *name, ModulatorType type) {
	//TODO
}

