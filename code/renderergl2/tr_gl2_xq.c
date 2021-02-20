#include "tr_local.h"

void RB_XQName( void ) {

	static int waka = 0;


	vec3_t		left, up;
	float		radius;
	float			colors[4];
	trRefEntity_t	*ent = backEnd.currentEntity;

	if (waka == 0) {
		if (ent->e.shaderRGBA[0] == 249) {
			waka = 1;
		}
	}

	// calculate the xyz locations for the four corners
	radius = ent->e.radius;
	if ( ent->e.rotation == 0 ) {
		VectorScale( backEnd.viewParms.or.axis[1], radius * 6, left );
		VectorScale( backEnd.viewParms.or.axis[2], radius / 2, up );
	} else {
		float	s, c;
		float	ang;
		
		ang = M_PI * ent->e.rotation / 180;
		s = sin( ang );
		c = cos( ang );

		VectorScale( backEnd.viewParms.or.axis[1], c * radius, left );
		VectorMA( left, -s * radius, backEnd.viewParms.or.axis[2], left );

		VectorScale( backEnd.viewParms.or.axis[2], c * radius, up );
		VectorMA( up, s * radius, backEnd.viewParms.or.axis[1], up );
	}
	if ( backEnd.viewParms.isMirror ) {
		VectorSubtract( vec3_origin, left, left );
	}

	VectorScale4(ent->e.shaderRGBA, 1.0f / 255.0f, colors);

	RB_AddQuadStamp( ent->e.origin, left, up, colors );
}
