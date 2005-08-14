#ifndef _ps_World_H
#define _ps_World_H

#include "Terrain.h"
#include "UnitManager.h"
#include "EntityManager.h"
#include "Projectile.h"

class CGame;
class CGameAttributes;

class CWorld
{
	CGame *m_pGame;
	
	CTerrain m_Terrain;
	// These all point to the respective g_* globals - the plan is to remove
	// the globals and move them into CWorld members as soon as all code has
	// been converted
	CUnitManager &m_UnitManager;
	CEntityManager &m_EntityManager;
	CProjectileManager &m_ProjectileManager;

public:
	inline CWorld(CGame *pGame):
		m_pGame(pGame),
		m_Terrain(),
		m_UnitManager(g_UnitMan),
		m_EntityManager(*(new CEntityManager())),
		m_ProjectileManager( *(new CProjectileManager()))
	{}

	~CWorld();

	void RegisterInit(CGameAttributes *pGameAttributes);
	/*
	Initialize the World - load the map and all objects
	*/
	void Initialize(CGameAttributes *pGameAttributes);

	// provided for JS _rewritemaps function
	void RewriteMap();

	inline CTerrain *GetTerrain()
	{	return &m_Terrain; }
	inline CUnitManager *GetUnitManager()
	{	return &m_UnitManager; }
	inline CProjectileManager *GetProjectileManager()
	{	return &m_ProjectileManager; }

private:
	// squelch "unable to generate" warnings
	CWorld(const CWorld& rhs);
	const CWorld& operator=(const CWorld& rhs);
};

#include "Game.h"
ERROR_SUBGROUP(Game, World);
ERROR_TYPE(Game_World, MapLoadFailed);

// rationale: see definition.
class CLightEnv;
extern CLightEnv g_LightEnv;

#endif
