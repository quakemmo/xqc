#include "tr_local.h"

void RB_XQName( void ) {
	vec3_t		left, up;

	// calculate the xyz locations for the four corners
	float radius = backEnd.currentEntity->e.radius;
	if (backEnd.currentEntity->e.rotation == 0) {
		VectorScale(backEnd.viewParms.or.axis[1], radius * 5, left);
		VectorScale(backEnd.viewParms.or.axis[2], radius / 1, up);
	} else {
		float	s, c;
		float	ang;
		
		ang = M_PI * backEnd.currentEntity->e.rotation / 180;
		s = sin(ang);
		c = cos(ang);

		VectorScale(backEnd.viewParms.or.axis[1], c * radius, left);
		VectorMA(left, -s * radius, backEnd.viewParms.or.axis[2], left);

		VectorScale(backEnd.viewParms.or.axis[2], c * radius, up);
		VectorMA(up, s * radius, backEnd.viewParms.or.axis[1], up);
	}
	if (backEnd.viewParms.isMirror) {
		VectorSubtract(vec3_origin, left, left);
	}

	RB_AddQuadStamp(backEnd.currentEntity->e.origin, left, up, backEnd.currentEntity->e.shaderRGBA);
}
