////////////////////////////////////////////////////////////////////////////
//	Module		: weapon_collision.cpp
//	Created		: 12/10/2012
//	Modified 	: 13/01/2013
//	Author		: lost alpha (SkyLoader)
//	Description	: weapon HUD collision
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "weapon_collision.h"
#include "Actor.h"
#include "actoreffector.h"
#include "static_cast_checked.hpp"


CWeaponCollision::CWeaponCollision()
{
	Load();

	r_torso_tgt_roll = 0;
}

CWeaponCollision::~CWeaponCollision()
{
}

void CWeaponCollision::Load()
{
	is_limping = false;
	fReminderDist = 0;
	fReminderNeedDist = 0;
	fReminderDist_x = 0;
	fReminderNeedDist_x = 0;
	fReminderDist_z = 0;
	fReminderNeedDist_z = 0;
	bFirstUpdate = true;

}

void CWeaponCollision::CheckState()
{
	dwMState = Actor()->MovingState();
//	is_limping = Actor()->IsLimping();
}

static const float SPEED_REMINDER = 1.f;
static const u16 TIME_REMINDER_STRAFE = 900;
static const float STRAFE_ANGLE = 0.1f;

extern Flags32 psActorFlags;


static const float ORIGIN_OFFSET = -0.05f;
static const float TENDTO_SPEED = 5.f;
void CWeaponCollision::Update(Fmatrix &o, float range, bool is_zoom)
{
	CheckState();

	Fvector	xyz = o.c;
	Fvector	xyza = o.c;
	Fvector	dir;
	o.getHPB(dir.x, dir.y, dir.z);

	//////collision of weapon hud:

	if (bFirstUpdate) {
		fReminderDist = xyza.z;
		fReminderNeedDist = xyza.z;
		fReminderDist_x = xyz.x;
		fReminderNeedDist_x = xyz.x;
		fReminderDist_z = xyz.z;
		fReminderNeedDist_z = xyz.z;
		bFirstUpdate = false;

	//	fNewStrafeTime = Device.dwTimeGlobal;
	}





	///////////////////////<< движения в перет назад  по координатом z >>///////////////// Author : lost alpha(SkyLoader);   переделал для 1.0007 (serg)
	
/*	if ((dwMState&ACTOR_DEFS::mcFwd || dwMState&ACTOR_DEFS::mcBack) && !is_zoom)
	{
		float k = ((dwMState & ACTOR_DEFS::mcCrouch) ? 0.5f : 1.f);
		if (dwMState&ACTOR_DEFS::mcFwd)
			k *= -1.f;
		fReminderNeedDist_z = xyz.z + (0.01 * k); } else { fReminderNeedDist_z = xyz.z; }

		if (!fsimilar(fReminderDist_z, fReminderNeedDist_z))
		{
			if (fReminderDist_z < fReminderNeedDist_z)
			{
				fReminderDist_z += 0.06f * Device.fTimeDelta;
				if (fReminderDist_z > fReminderNeedDist_z)
					fReminderDist_z = fReminderNeedDist_z;
			}

			else if (fReminderDist_z > fReminderNeedDist_z)
			{
				fReminderDist_z -= 0.06f * Device.fTimeDelta;
				if (fReminderDist_z < fReminderNeedDist_z)
					fReminderDist_z = fReminderNeedDist_z;
			}
		}

	if (!fsimilar(fReminderDist_z, xyz.z))  { xyz.z = fReminderDist_z;  o.c.set(xyz); }

	////////////////////////////////////////////////////////////////////////////////////
///////////////////////////<< движения влево вправо по координатом х >>///////////////////////
	////////////////////////////////////////////////////////////////////////////////////

/*	float x = 0.f;
	if ((dwMState&ACTOR_DEFS::mcStrafe) && !is_zoom)
	{
		x = (dwMState & ACTOR_DEFS::mcLStrafe) ? -0.5 : 0.5;

		if ((dwMState&ACTOR_DEFS::mcLStrafe) && (dwMState&ACTOR_DEFS::mcRStrafe))
			x = 0.0f;

		fReminderNeedDist_x = xyz.x + (0.015 * x); 
	} else { fReminderNeedDist_x = xyz.x; }
	

	if (!fsimilar(fReminderDist_x,  fReminderNeedDist_x))
	{
	if (fReminderDist_x < fReminderNeedDist_x)
	{
		fReminderDist_x += 0.05f * Device.fTimeDelta;
	if (fReminderDist_x > fReminderNeedDist_x)
	fReminderDist_x = fReminderNeedDist_x; 
	}

	else if (fReminderDist_x > fReminderNeedDist_x) 
	{
		fReminderDist_x -= 0.05f * Device.fTimeDelta;
	if (fReminderDist_x < fReminderNeedDist_x)
	fReminderDist_x = fReminderNeedDist_x; 
	}
	}
	
	if (!fsimilar(fReminderDist_x, xyz.x)) 
	{ 

	xyz.x = fReminderDist_x;  
	o.c.set(xyz);
	}
	
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////


	if (range < 0.8f && !is_zoom)
		fReminderNeedDist = xyza.z - ((1 - range - 0.2) * 0.6);
	else
		fReminderNeedDist = xyza.z;

	if (!fsimilar(fReminderDist, fReminderNeedDist)) {
		if (fReminderDist < fReminderNeedDist) {
			fReminderDist += SPEED_REMINDER * Device.fTimeDelta;
			if (fReminderDist > fReminderNeedDist)
				fReminderDist = fReminderNeedDist;
		}
		else if (fReminderDist > fReminderNeedDist) {
			fReminderDist -= SPEED_REMINDER * Device.fTimeDelta;
			if (fReminderDist < fReminderNeedDist)
				fReminderDist = fReminderNeedDist;
		}
	}

	if (!fsimilar(fReminderDist, xyza.z))
	{
		xyza.z = fReminderDist;
		o.c.set(xyza);
	}









/////////////////////////////////////////////////////////////////////////////////////////////
	//////strafe inertion:
	/*
	if (!psActorFlags.test(AF_STRAFE_INERT)) return;

	float fLastStrafe = fReminderNeedStrafe;
	if ((dwMState&ACTOR_DEFS::mcLStrafe || dwMState&ACTOR_DEFS::mcRStrafe) && !is_zoom)
	{
		float k = ((dwMState & ACTOR_DEFS::mcCrouch) ? 0.5f : 1.f);
		if (dwMState&ACTOR_DEFS::mcLStrafe)
			k *= -1.f;

		fReminderNeedStrafe = dir.z + (STRAFE_ANGLE * k);

		if (dwMState&ACTOR_DEFS::mcFwd || dwMState&ACTOR_DEFS::mcBack)
			fReminderNeedStrafe /= 2.f;

	}
	else

		fReminderNeedStrafe = dir.z;



	float result;
	if (fNewStrafeTime>(float)Device.dwTimeGlobal)
		result = fReminderStrafe->Evaluate(Device.dwTimeGlobal);
	else {
		if (fReminderStrafe->keys.size()>0)
			result = fReminderStrafe->Evaluate(fReminderStrafe->keys.back()->time);
	}

	if (!fsimilar(fLastStrafe, fReminderNeedStrafe))
	{
		float time_new = TIME_REMINDER_STRAFE + Device.dwTimeGlobal;

		fReminderStrafe->DeleteLastKeys(Device.dwTimeGlobal);

		if (!fsimilar(result, 0.f))
			fReminderStrafe->InsertKey(Device.dwTimeGlobal, result);
		else
			fReminderStrafe->InsertKey(Device.dwTimeGlobal, dir.z);
		fReminderStrafe->InsertKey(time_new, fReminderNeedStrafe);

		fNewStrafeTime = time_new;

		result = fReminderStrafe->Evaluate(Device.dwTimeGlobal);
	}

	if (!fsimilar(result, dir.z))
	{
		dir.z = result;
		Fmatrix m;
		m.setHPB(dir.x, dir.y, dir.z);
		Fmatrix tmp;
		tmp.mul_43(o, m);
		o.set(tmp);
	}
	else {
		if (fReminderStrafe->keys.size()>10 && fNewStrafeTime<(float)Device.dwTimeGlobal)
		{
			fReminderStrafe->Clear();	//clear all keys
			fReminderStrafe->InsertKey(Device.dwTimeGlobal, fReminderNeedStrafe);
			fNewStrafeTime = Device.dwTimeGlobal;
		}
	}
	*/
}