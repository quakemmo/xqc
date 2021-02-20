/* DESCRIPTION

	Camera cycling and mouse wheel zooming.

*/
#include "../cg_local.h"

#define ZOOM_STEP	15		// mouse wheel button changes zoom by that much
#define ZOOM_MAX	200		// cannot zoom out more than that

void xq_camera_CycleView(qboolean reset) {
	// Cycle the camera view point around the player

	if (reset) {
		trap_Cvar_Set("cg_thirdPerson", "0");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRangeGoal", "50");
		return;	
	}

	int curmode = cg_thirdPerson.integer + 1;

	if (curmode == 9) {
		curmode = 0;
		trap_Cvar_Set("cg_thirdPerson", "0");
		xq_clog(COLOR_WHITE, "Setting camera to first person view.");
	} else {
		trap_Cvar_Set("cg_thirdPerson", va("%i", curmode));
		trap_Cvar_Set("cg_thirdPersonAngle", va("%i", (curmode-1) * 45));
		if (curmode == 1) {
			xq_clog(COLOR_WHITE, "Setting camera to third person view.");
		}
	}
}
void xq_camera_MouseZoom(qboolean dir) {
	// Handles the mouse wheel zoom in mouse mode

	int curmode = cg_thirdPerson.integer;
	int curdist = cg_thirdPersonRangeGoal.integer;



	// Apply the requested action (zoom in or zoom out) to curdist
	if (dir) {
		// zooming in
		if (curmode != 0) {
			// we are zoomed out somewhat, and we're asked to zoom in (we might have to switch to 1st person view)
			curdist -= ZOOM_STEP;
		} else {
			// we are fully zoomed in and we're asked to zoom in some more
			return;
		}
	} else {
		// zooming out
		if (curmode != 0) {
			// we are already zoomed out somewhat, and we're asked to further zoom out
			curdist += ZOOM_STEP;
		} else {
			// we are fully zoomed in and we're asked to zoom out (we will swtich to 3rd person view then)
			curdist = ZOOM_STEP;
		}
	}


	// Is the camera getting too close or too far?
	if (curdist < ZOOM_STEP) {
		// we are too close - just switch back to 1st person view
		xq_camera_CycleView(qtrue);
		return;
	} else if (curdist > ZOOM_MAX) {
		// do not zoom out more than this
		curdist = ZOOM_MAX;
	}


	// Do we need to switch from 1st to 3rd person view?
	if (curmode == 0) {
		// we need to switch to 3rd person view - default is view from behind
		trap_Cvar_Set("cg_thirdPerson", "1");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRange", "0");
	}

	// apply the computed distance
	trap_Cvar_Set("cg_thirdPersonRangeGoal", va("%i", curdist));
}
void xq_camera_frame() {
	// This is called every client frame and makes mouse zoom smooth in mouse mode
	// instead of snapping to desired values.

	int goal = cg_thirdPersonRangeGoal.integer;
	int cur = cg_thirdPersonRange.integer;


	if (goal == cur) {
		// Don't need to do anything
		return;
	}


	// Maybe we are very close to desired range and should just snap to it
	int step = (goal - cur);
	if (step > -3 && step < 3) {
		// Snap to goal range
		trap_Cvar_Set("cg_thirdPersonRange", va("%i", goal));
		return;
	}


	// We need to move the current range toward the goal.
	// Don't make it too fast/choppy.
	if (step > 0) {
		if (step > 5) {
			step = 2;
		} else {
			step = 1;
		}
	} else {
		if (step < -5) {
			step = -2;
		} else {
			step = -1;
		}
	}

	trap_Cvar_Set("cg_thirdPersonRange", va("%i", cur + step));
}
void xq_Camera_ZoomIn() {
	xq_camera_MouseZoom(qtrue);
}
void xq_Camera_ZoomOut() {
	xq_camera_MouseZoom(qfalse);
}
