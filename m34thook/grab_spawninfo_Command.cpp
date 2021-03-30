#include "game_exe_interface.hpp"
#include "doomoffs.hpp"
#include "meathook.h"
#include "cmdsystem.hpp"
#include "idtypeinfo.hpp"
#include "eventdef.hpp"
#include "scanner_core.hpp"
#include "idLib.hpp"

#include <cmath>
#include <cstdlib>
#include <cstdio>
namespace idMath {
void SinCos(float a, float& s, float& c)
{

	//byte_angle_t by = rad2byte(a);

	// DG: non-MSVC version
	s = sinf(a);
	c = cosf(a);

}
const float	PI				= 3.14159265358979323846f;

const float	M_DEG2RAD		= PI / 180.0f;
const float	M_RAD2DEG		= 180.0f / PI;

}
struct idVec3 {
	float x, y, z;
	void Set(float _x, float _y, float _z){
		x=_x;
		y=_y;
		z=_z;
	}
};

struct idMat3 {
	idVec3			mat[ 3 ];
};
class idAngles
{
public:
	float			pitch;
	float			yaw;
	float			roll;
	idMat3 ToMat3() const;
};

#define DEG2RAD(a)				( (a) * idMath::M_DEG2RAD )

idMat3 idAngles::ToMat3() const
{
	idMat3 mat;
	float sr, sp, sy, cr, cp, cy;



	idMath::SinCos( DEG2RAD( yaw ), sy, cy );
	idMath::SinCos( DEG2RAD( pitch ), sp, cp );
	idMath::SinCos( DEG2RAD( roll ), sr, cr );

	mat.mat[ 0 ].Set( cp * cy, cp * sy, -sp );
	mat.mat[ 1 ].Set( sr * sp * cy + cr * -sy, sr * sp * sy + cr * cy, sr * cp );
	mat.mat[ 2 ].Set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );

	return mat;
}


static char clipboard_buffer[4096];

// Buffer for spawn group recordings
static char spawnGroupRecording[64][128];
static int spawnGroupRecordingCount = -1;

const char* get_clipboard_data() {

	if (!OpenClipboard(NULL))
		return nullptr;
	HANDLE cbhandle = GetClipboardData(CF_TEXT);
	CloseClipboard();
	return (const char*)cbhandle;
}

void set_clipboard_data(const char* dat) {
	if (!OpenClipboard(NULL))
		return;
	size_t datsize=strlen(dat)+1;

	HGLOBAL mem = GlobalAlloc( GMEM_MOVEABLE, datsize );
	char	*str = (char *)GlobalLock( mem );

	memcpy(str, dat,datsize);
	GlobalUnlock( str );
	EmptyClipboard();
	SetClipboardData(CF_TEXT,mem);
	CloseClipboard();
}

void cmd_mh_spawninfo(idCmdArgs* args) {

	idCmd::execute_command_text("getviewpos");

	if (!OpenClipboard(NULL))
		return;
	HANDLE cbhandle = GetClipboardData(CF_TEXT);

	float x, y, z;
	float yaw, pitch;

	sscanf_s((const char*)cbhandle, "%f %f %f %f %f", &x, &y, &z, &yaw, &pitch);

	// Substract 1.7 from the Z coordinate (player's height)
	z -= 1.7;

	idAngles angles{pitch, yaw, .0};
	idMat3 mat = angles.ToMat3();
	const char* fmtstr = "\tspawnOrientation = {\n\t\tmat = {\n\t\t\tmat[0] = {\n\t\t\t\tx = %f;\n\t\t\t\ty = %f;\n\t\t\t\tz = %f;\n\t\t\t}\n\t\t\tmat[1] = {\n\t\t\t\tx = %f;\n\t\t\t\ty = %f;\n\t\t\t\tz = %f;\n\t\t\t}\n\t\t\tmat[2] = {\n\t\t\t\tx = %f;\n\t\t\t\ty = %f;\n\t\t\t\tz = %f;\n\t\t\t}\n\t\t}\n\t}\n\tspawnPosition = {\n\t\tx = %f;\n\t\ty = %f;\n\t\tz = %f;\n\t}";
	auto& m = mat.mat;
	sprintf_s(clipboard_buffer, fmtstr, m[0].x, m[0].y, m[0].z, m[1].x, m[1].y, m[1].z, m[2].x, m[2].y, m[2].z, x, y, z);
	HGLOBAL mem = GlobalAlloc( GMEM_MOVEABLE, 4096);
	char	*str = (char *)GlobalLock( mem );

	memcpy(str, clipboard_buffer, 4096);
	GlobalUnlock( str );
	EmptyClipboard();
	SetClipboardData(CF_TEXT,mem);
	CloseClipboard();
}

void mh_create_spawn(const char* spawnParent, const char* aiStateOverride, const char* spawnAnim, const char* entityDefName) {
	idCmd::execute_command_text("getviewpos");

	if (!OpenClipboard(NULL))
		return;
	HANDLE cbhandle = GetClipboardData(CF_TEXT);

	float x, y, z;
	float yaw, pitch;

	sscanf_s((const char*)cbhandle, "%f %f %f %f %f", &x, &y, &z, &yaw, &pitch);

	// Substract 1.7 from the Z coordinate (player's height)
	z -= 1.7;

	idAngles angles{ pitch, yaw, .0 };
	idMat3 mat = angles.ToMat3();
	const char* fmtstr = "entity {\n\tentityDef %s {\n\tinherit = \"target/spawn\";\n\tclass = \"idTarget_Spawn\";\n\texpandInheritance = false;\n\tpoolCount = 0;\n\tpoolGranularity = 2;\n\tnetworkReplicated = false;\n\tdisableAIPooling = false;\n\tedit = {\n\t\tflags = {\n\t\t\tnoFlood = true;\n\t\t}\n\t\tspawnConditions = {\n\t\t\tmaxCount = 0;\n\t\t\treuseDelaySec = 0.100000001;\n\t\t\tdoBoundsTest = false;\n\t\t\tboundsTestType = \"BOUNDSTEST_TRUE\";\n\t\t\tfovCheck = 0;\n\t\t\tminDistance = 0;\n\t\t\tmaxDistance = 0;\n\t\t\tneighborSpawnerDistance = -1;\n\t\t\tLOS_Test = \"LOS_NONE\";\n\t\t\tplayerToTest = \"PLAYER_SP\";\n\t\t\tconditionProxy = \"\";\n\t\t}\n\t\tspawnEditableShared = {\n\t\t\tgroupName = \"\";\n\t\t\tdeathTrigger = \"\";\n\t\t\tcoverRadius = 0;\n\t\t\tmaxEnemyCoverDistance = 0;\n\t\t}\n\t\tentityDefs = {\n\t\t\tnum = 0;\n\t\t}\n\t\tconductorEntityAIType = \"SPAWN_AI_TYPE_ANY\";\n\t\tinitialEntityDefs = {\n\t\t\tnum = 0;\n\t\t}\n\t\tspawnEditable = {\n\t\t\tspawnAt = \"\";\n\t\t\tcopyTargets = false;\n\t\t\tadditionalTargets = {\n\t\t\t\tnum = 0;\n\t\t\t}\n\t\t\toverwriteTraversalFlags = true;\n\t\t\ttraversalClassFlags = \"CLASS_A\";\n\t\t\tcombatHintClass = \"CLASS_ALL\";\n\t\t\tspawnAnim = \"%s\";\n\t\t\taiStateOverride = \"%s\";\n\t\t\tinitialTargetOverride = \"\";\n\t\t}\n\t\tportal = \"\";\n\t\ttargetSpawnParent = \"%s\";\n\t\tdisablePooling = false;\n\t\tspawnOrientation = {\n\t\t\tmat = {\n\t\t\t\tmat[0] = {\n\t\t\t\t\tx = %f;\n\t\t\t\t\ty = %f;\n\t\t\t\t\tz = %f;\n\t\t\t\t}\n\t\t\t\tmat[1] = {\n\t\t\t\t\tx = %f;\n\t\t\t\t\ty = %f;\n\t\t\t\t\tz = %f;\n\t\t\t\t}\n\t\t\t\tmat[2] = {\n\t\t\t\t\tx = %f;\n\t\t\t\t\ty = %f;\n\t\t\t\t\tz = %f;\n\n\t\t\t\t}\n\t\t\t}\n\t\t}\t\t\n\t\tspawnPosition = {\n\t\t\tx = %f;\n\t\t\ty = %f;\n\t\t\tz = %f;\n\t\t}\n\t}\n}\n}";
	auto& m = mat.mat;
	sprintf_s(clipboard_buffer, fmtstr, entityDefName, spawnAnim, aiStateOverride, spawnParent, m[0].x, m[0].y, m[0].z, m[1].x, m[1].y, m[1].z, m[2].x, m[2].y, m[2].z, x, y, z);
	HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, 4096);
	char* str = (char*)GlobalLock(mem);

	memcpy(str, clipboard_buffer, 4096);
	GlobalUnlock(str);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();

	if (spawnGroupRecordingCount >= 0)
	{
		if (spawnGroupRecordingCount >= 64) {
			idLib::Printf("Warning: reached max of 64 recorded spawns, resetting to 0.\n");
			spawnGroupRecordingCount = 0;
		}

		sprintf_s(spawnGroupRecording[spawnGroupRecordingCount], "%s", entityDefName);
		spawnGroupRecordingCount++;
	}
}

void cmd_mh_create_spawn(idCmdArgs* args) {
	if (args->argc < 4 || args->argc > 5) {
		idLib::Printf("Usage: mh_create_spawn <spawn parent> <ai state override> [spawn anim] <entitydef name>.\n");
		return;
	}

	if (args->argc == 4) {
		mh_create_spawn(args->argv[1], args->argv[2], "", args->argv[3]);
	}
	else if (args->argc == 5) {
		mh_create_spawn(args->argv[1], args->argv[2], args->argv[3], args->argv[4]);
	}
}

void cmd_mh_begin_spawn_group(idCmdArgs* args) {
	if (spawnGroupRecordingCount > 0) {
		idLib::Printf("There's an active recording, end it first with mh_end_spawn_group.\n");
		return;
	}

	spawnGroupRecordingCount = 0;
	idLib::Printf("Spawn recording started.\n");
}

void cmd_mh_end_spawn_group(idCmdArgs* args) {
	if (args->argc < 3) {
		idLib::Printf("Usage: mh_end_spawn_group <spawn parent> <entitydef name>.\n");
		idLib::Printf("Ends the creation of a spawn group. This will record all spawn entitydef names created with mh_create_spawn.\n");
		idLib::Printf("Use mh_end_spawn_group to finish recording the spawn group and copy the spawn group entitydef to your clipboard.\n");
		return;
	}

	if (spawnGroupRecordingCount <= 0) {
		idLib::Printf("No spawns recorded yet.\n");
		return;
	}

	if (!OpenClipboard(NULL))
		return;

	const char* fmtstrstart = "entity {\n\tentityDef %s {\n\tinherit = \"encounter/spawn_group/zone\";\n\tclass = \"idTargetSpawnGroup\";\n\texpandInheritance = false;\n\tpoolCount = 0;\n\tpoolGranularity = 2;\n\tnetworkReplicated = false;\n\tdisableAIPooling = false;\n\tedit = {\n\t\tspawnPosition = {\n\t\t\tx = 0;\n\t\t\ty = 0;\n\t\t\tz = 0;\n\t\t}\n\t\trenderModelInfo = {\n\t\t\tmodel = NULL;\n\t\t}\n\t\tclipModelInfo = {\n\t\t\tclipModelName = NULL;\n\t\t}\n\t\tspawners = {\n\t\t\tnum = %d;\n";
	const char* fmtstrend = "\t\t}\n\t\ttargetSpawnParent = \"%s\";\n\t}\n}\n}";
	char fmtstr[4096];

	sprintf_s(fmtstr, "%s", fmtstrstart);

	for (int i = 0; i < spawnGroupRecordingCount; i++) {
		char buffer[256];

		sprintf_s(buffer, "\t\t\titem[%d] = \"%s\"\n", i, spawnGroupRecording[i]);
		sprintf_s(fmtstr, "%s%s", fmtstr, buffer);
	}

	sprintf_s(fmtstr, "%s%s", fmtstr, fmtstrend);
	sprintf_s(clipboard_buffer, fmtstr, args->argv[2], spawnGroupRecordingCount, args->argv[1]);

	HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, 4096);
	char* str = (char*)GlobalLock(mem);

	memcpy(str, clipboard_buffer, 4096);
	GlobalUnlock(str);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();

	idLib::Printf("Spawn recording ended, copied spawn group entityDef to your clipboard.\n");
	spawnGroupRecordingCount = -1;
}

void cmd_mh_ang2mat(idCmdArgs* args) {
	double inv1 = .0;
	if (args->argc > 1) {
		inv1 = atof(args->argv[1]);
	}
	double inv2 = .0f;
	if (args->argc > 2) {
		inv2 = atof(args->argv[2]);
	}

	double inv3 = .0f;
	if (args->argc > 3) {
		inv3 = atof(args->argv[3]);
	}
	idAngles angles{ inv1, inv2, inv3 };
	idMat3 mat = angles.ToMat3();
	//copypaste
	const char* fmtstr = "{\n\tmat = {\n\t\tmat[0] = {\n\t\t\tx = %f;\n\t\t\ty = %f;\n\t\t\tz = %f;\n\t\t}\n\t\tmat[1] = {\n\t\t\tx = %f;\n\t\t\ty = %f;\n\t\t\tz=%f;\n\t\t}\n\t\tmat[2] = {\n\t\t\tx = %f;\n\t\t\ty = %f;\n\t\t\tz = %f;\n\t\t}\n\t}\n}\n";
	auto& m = mat.mat;
	sprintf_s(clipboard_buffer, fmtstr, m[0].x, m[0].y, m[0].z, m[1].x, m[1].y, m[1].z, m[2].x, m[2].y, m[2].z);
	set_clipboard_data(clipboard_buffer);
}
