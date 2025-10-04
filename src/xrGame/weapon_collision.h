////////////////////////////////////////////////////////////////////////////
//	Module		: weapon_collision.h
//	Created		: 12/10/2012
//	Modified	: 12/10/2012
//	Author		: lost alpha (SkyLoader)
//	Description	: weapon collision
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../envelope.h"

class CWeaponCollision
{
public:
	CWeaponCollision();
	virtual ~CWeaponCollision();
	void Load();
	void Update(Fmatrix &x, float range, bool is_zoom);
	void CheckState();

private:
	float		fTime;
	float		fReminderDist; //float
	float		fReminderNeedDist;
	float		fReminderDist_x; //float
//	float		fReminderNeedDist_x;
	float		fReminderNeedDist_x;
	float		fReminderDist_z; //float
	float		fReminderNeedDist_z;
//	CEnvelope*	fReminderStrafe;
//	float		fReminderNeedStrafe;
//	float		fNewStrafeTime;
	bool		bFirstUpdate;
	u32		dwMState;
	bool		is_limping;


protected:

	float					r_torso_tgt_roll;
};