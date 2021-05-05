#include "../cg_local.h"

/* DESCRIPTION
	cg_bbox 1 triggers the display of a bbox on PCs and NPCs.

	Code/gfx courtesy of the Unlagged mod.
*/

void xq_drawbbox(centity_t *cent) {
	polyVert_t verts[4];
	int i;
	vec3_t mins = {-15, -15, -24};
	vec3_t maxs = {15, 15, 32};
	float extx, exty, extz;
	vec4_t corners[8];
	qhandle_t bboxShader, bboxShader_nocull;
    playerState_t *ps = &cg.snap->ps;

	if (!cg_bbox.integer) {
		return;
	}


	// Only draw if currently targeted
	if (cent != &cg_entities[ps->xq_target]) {
		if (cg_bbox.integer == 2) {
			return;
		}
	}

	if (cent->currentState.number != 1) {
		if (cg_bbox.integer == 3) {
			return;
		}
	}
	int my_solid;


	// don't draw it if it's us in first-person
	if ( cent->currentState.number == cg.predictedPlayerState.clientNum &&
			!cg.renderingThirdPerson ) {
		return;
	}

	// don't draw it for dead players
	if ( cent->currentState.eFlags & EF_DEAD ) {
		return;
	}

	// get the shader handles
	bboxShader = trap_R_RegisterShader( "bbox" );
	bboxShader_nocull = trap_R_RegisterShader( "bbox_nocull" );

	// if they don't exist, forget it
	if ( !bboxShader || !bboxShader_nocull ) {
		return;
	}

	int x, zd, zu;
	// get the player's client info



	// if it's us
	if ( cent->currentState.number == cg.predictedPlayerState.clientNum ) {
		my_solid = ps->xq_mysolid;
	} else {
		my_solid = cent->currentState.solid;
	}


	// otherwise grab the encoded bounding box
	x = (my_solid & 255);
	zd = ((my_solid>>8) & 255);
	zu = ((my_solid>>16) & 255) - 32;
	/* xq_clog(COLOR_WHITE, "%s: x: %i, zd: %i, zu: %i, mysolid: %i",
		xq_getname(cent, 0), x, zd, zu, my_solid);
	*/

	mins[0] = mins[1] = -x;
	maxs[0] = maxs[1] = x;
	mins[2] = -zd;
	maxs[2] = zu;


	// get the extents (size)
	extx = maxs[0] - mins[0];
	exty = maxs[1] - mins[1];
	extz = maxs[2] - mins[2];

	if (xq_debugBbox.integer) {
		if (xq_1second()) {
			xq_clog(COLOR_WHITE, "extx: %f, exty: %f, extz: %f", extx, exty, extz);
		}
	}


	// set the polygon's texture coordinates
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;


	// Red: 160, 0, 0, 255
	// Blue: 0, 0, 192, 255
	for ( i = 0; i < 4; i++ ) {
		// green
		verts[i].modulate[0] = 0;
		verts[i].modulate[1] = 128;
		verts[i].modulate[2] = 0;
		verts[i].modulate[3] = 255;
	}

	VectorAdd( cent->lerpOrigin, maxs, corners[3] );

	VectorCopy( corners[3], corners[2] );
	corners[2][0] -= extx;

	VectorCopy( corners[2], corners[1] );
	corners[1][1] -= exty;

	VectorCopy( corners[1], corners[0] );
	corners[0][0] += extx;

	for ( i = 0; i < 4; i++ ) {
		VectorCopy( corners[i], corners[i + 4] );
		corners[i + 4][2] -= extz;
	}

	// top
	VectorCopy( corners[0], verts[0].xyz );
	VectorCopy( corners[1], verts[1].xyz );
	VectorCopy( corners[2], verts[2].xyz );
	VectorCopy( corners[3], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// bottom
	VectorCopy( corners[7], verts[0].xyz );
	VectorCopy( corners[6], verts[1].xyz );
	VectorCopy( corners[5], verts[2].xyz );
	VectorCopy( corners[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// top side
	VectorCopy( corners[3], verts[0].xyz );
	VectorCopy( corners[2], verts[1].xyz );
	VectorCopy( corners[6], verts[2].xyz );
	VectorCopy( corners[7], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// left side
	VectorCopy( corners[2], verts[0].xyz );
	VectorCopy( corners[1], verts[1].xyz );
	VectorCopy( corners[5], verts[2].xyz );
	VectorCopy( corners[6], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// right side
	VectorCopy( corners[0], verts[0].xyz );
	VectorCopy( corners[3], verts[1].xyz );
	VectorCopy( corners[7], verts[2].xyz );
	VectorCopy( corners[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// bottom side
	VectorCopy( corners[1], verts[0].xyz );
	VectorCopy( corners[0], verts[1].xyz );
	VectorCopy( corners[4], verts[2].xyz );
	VectorCopy( corners[5], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );
}
