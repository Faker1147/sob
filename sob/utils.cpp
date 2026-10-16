// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "eiface.h"
#include "basehandle.h"
#include "IEngineTrace.h"
#include "IStaticPropMgr.h"
#include "bot_plugin_meta.h"
#include "bot.h"
#include "utils.h"

// interfaces of interest

extern ICvar* icvar;
extern IVEngineServer* engine;
extern IPlayerInfoManager* playerinfomanager;
extern IServerPluginHelpers* helpers;
extern IServerGameClients* gameclients;
extern IEngineTrace* enginetrace;
extern IBotManager* g_pBotManager;
extern CGlobalVars* gpGlobals;
extern IServerGameEnts* servergameents;
extern IServerGameDLL* servergamedll;
extern IServerTools* servertools;
extern IStaticPropMgrServer* staticpropmgr;

namespace private_utils
{
	static Vector GetOBBCenter(const ICollideable* collider)
	{
		Vector result(0.0f, 0.0f, 0.0f);
		VectorLerp(collider->OBBMins(), collider->OBBMaxs(), 0.5f, result);
		return result;
	}

	static bool isBoundsDefinedInEntitySpace(const ICollideable* collider)
	{
		return ((collider->GetSolidFlags() & FSOLID_FORCE_WORLD_ALIGNED) == 0 && collider->GetSolid() != SOLID_BBOX && collider->GetSolid() != SOLID_NONE);
	}

	static Vector collisionToWorldSpace(const Vector& in, const ICollideable* collider)
	{
		Vector result(0.0f, 0.0f, 0.0f);

		if (!isBoundsDefinedInEntitySpace(collider) || collider->GetCollisionAngles() == vec3_angle)
		{
			VectorAdd(in, collider->GetCollisionOrigin(), result);
		}
		else
		{
			VectorTransform(in, collider->CollisionToWorldTransform(), result);
		}

		return result;
	}
}

bool sobutils::IsValidEdict(const edict_t* edict)
{
	if (edict == nullptr || edict->IsFree() || edict->GetIServerEntity() == nullptr)
	{
		return false;
	}

	return true;
}

int sobutils::IndexOfEdict(const edict_t* edict)
{
#if SOURCE_ENGINE >= SE_LEFT4DEAD
	return static_cast<int>(edict - engine->PEntityOfEntIndex(0));
#else
	return engine->IndexOfEdict(edict);
#endif // SOURCE_ENGINE >= SE_LEFT4DEAD
}

edict_t* sobutils::EdictOfIndex(const int index)
{
#if SOURCE_ENGINE >= SE_LEFT4DEAD
	if (index >= 0 && index < gpGlobals->maxEntities)
	{
		return engine->PEntityOfEntIndex(index);
	}

	return nullptr;
#else
	return engine->PEntityOfEntIndex(index);
#endif // SOURCE_ENGINE >= SE_LEFT4DEAD
}

edict_t* sobutils::BaseEntityToEdict(CBaseEntity* entity)
{
	return servergameents->BaseEntityToEdict(entity);
}

CBaseEntity* sobutils::EdictToBaseEntity(edict_t* edict)
{
	return servergameents->EdictToBaseEntity(edict);
}

edict_t* sobutils::GetEdictFromHandleEntity(const IHandleEntity* pHandleEntity)
{
	IHandleEntity* pHE = const_cast<IHandleEntity*>(pHandleEntity);

	if (staticpropmgr->IsStaticProp(pHE))
	{
		return nullptr;
	}

	IServerUnknown* pUnk = reinterpret_cast<IServerUnknown*>(pHE);
	CBaseEntity* pBE = pUnk->GetBaseEntity();

	return BaseEntityToEdict(pBE);
}

CBaseEntity* sobutils::GetEntityFromHandleEntity(const IHandleEntity* pHandleEntity)
{
	IHandleEntity* pHE = const_cast<IHandleEntity*>(pHandleEntity);

	if (staticpropmgr->IsStaticProp(pHE))
	{
		return nullptr;
	}

	IServerUnknown* pUnk = reinterpret_cast<IServerUnknown*>(pHE);

	return pUnk->GetBaseEntity();
}

edict_t* sobutils::GetHandleEdict(const CBaseHandle& handle)
{
	if (!handle.IsValid())
	{
		return nullptr;
	}

	const int index = handle.GetEntryIndex();
	edict_t* pEdict = EdictOfIndex(index);

	if (!IsValidEdict(pEdict))
	{
		return nullptr;
	}

	const IServerEntity* pServerEntity = pEdict->GetIServerEntity();

	if (pServerEntity->GetRefEHandle() != handle)
	{
		return nullptr;
	}

	return pEdict;
}

void sobutils::SetHandleEdict(CBaseHandle& handle, const edict_t* edict)
{
	const IServerEntity* pServerEntity = edict->GetIServerEntity();
	handle.Set(pServerEntity);
}

const Vector& sobutils::GetEntityOrigin(edict_t* entity)
{
	return entity->GetCollideable()->GetCollisionOrigin();
}

const Vector& sobutils::GetEntityOrigin(CBaseEntity* entity)
{
	return reinterpret_cast<IServerEntity*>(entity)->GetCollideable()->GetCollisionOrigin();
}

const QAngle& sobutils::GetEntityAngles(edict_t* entity)
{
	return entity->GetCollideable()->GetCollisionAngles();
}

const QAngle& sobutils::GetEntityAngles(CBaseEntity* entity)
{
	return reinterpret_cast<IServerEntity*>(entity)->GetCollideable()->GetCollisionAngles();
}

Vector sobutils::GetWorldSpaceCenter(edict_t* entity)
{
	const ICollideable* collider = entity->GetCollideable();
	Vector result = private_utils::GetOBBCenter(collider);
	result = private_utils::collisionToWorldSpace(result, collider);
	return result;
}

Vector sobutils::GetWorldSpaceCenter(CBaseEntity* entity)
{
	const ICollideable* collider = reinterpret_cast<IServerEntity*>(entity)->GetCollideable();
	Vector result = private_utils::GetOBBCenter(collider);
	result = private_utils::collisionToWorldSpace(result, collider);
	return result;
}

bool sobutils::PointIsWithinTrigger(edict_t* pEntity, const Vector& vPoint)
{
	Ray_t ray;
	trace_t tr;
	ICollideable* pCollide = pEntity->GetCollideable();
	ray.Init(vPoint, vPoint);
	enginetrace->ClipRayToCollideable(ray, MASK_ALL, pCollide, &tr);
	return (tr.startsolid);
}
