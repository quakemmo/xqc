// General QWIN styling
#define QW_WIN_STATUSBAR_HEIGHT			10
#define QW_WIN_STATUSBAR_BGCOLOR		0x333333
#define QW_WIN_CLOSEX_WIDTH				10
#define QW_WIN_BORDER_WIDTH				1
#define QW_WIN_BORDER_COLOR				0x889988
#define QW_WIN_BG_COLOR					0x101321
#define QW_HOVER_ACCENT					20  // how much (out of 255) to add to the background color
											// of stuff that is sensitive to mouse hovering, when hovered over



// Object-specific styling
// -----------------------

// button
#define QW_OBJ_BUTTON_TRADE_AGREE_YES	0x00ff00
#define QW_OBJ_BUTTON_TRADE_AGREE_NO	0xeebbee
#define QW_OBJ_BUTTON_BGCOL				0x440044
#define QW_OBJ_BUTTON_BRDCOL			0xeebbee
#define QW_OBJ_BUTTON_PAD_X				10
#define QW_OBJ_BUTTON_PAD_Y				10
#define QW_OBJ_BUTTON_CAPITALIZE		0
#define QW_OBJ_BUTTON_BRDWIDTH			1

// spell gem
#define QW_OBJ_SPELL_GEM_WIDTH			48
#define QW_OBJ_SPELL_GEM_HEIGHT			48
#define QW_OBJ_SPELL_GEM_BRDWIDTH		3
#define QW_OBJ_SPELL_GEM_BGCOL			0x774477

// spell icon
#define	QW_OBJ_SPELL_ICON_WIDTH			48
#define	QW_OBJ_SPELL_ICON_HEIGHT		48

// autoequip area
#define QW_OBJ_AUTOEQUIP_WIDTH			130
#define QW_OBJ_AUTOEQUIP_HEIGHT			218
#define QW_OBJ_AUTOEQUIP_BRDCOL			0x555555
#define QW_OBJ_AUTOEQUIP_BGCOL			0x222222

// item inspector
#define QW_OBJ_ITEMINSPECTOR_WIDTH		400
#define	QW_OBJ_ITEMINSPECTOR_HEIGHT		200
#define QW_OBJ_ITEMINSPECTOR_CTR		1

// inventory slot
#define QW_OBJ_INVSLOT_HEIGHT			42
#define QW_OBJ_INVSLOT_WIDTH			42
#define QW_OBJ_INVSLOT_BGCOL			0x0c1214
#define QW_OBJ_INVSLOT_BGCOL_HOVER		0
#define QW_OBJ_INVSLOT_UNSELECTED		0x374548
#define QW_OBJ_INVSLOT_SELECTED			0x00aa00
#define QW_OBJ_INVSLOT_SELECTED_WEAP	0x00ffff

// perc bar
#define QW_OBJ_PERCBAR_BRDCOL			0x444444
#define QW_OBJ_PERCBAR_WIDTH			100
#define QW_OBJ_PERCBAR_HEIGHT			7
#define QW_OBJ_PERCBAR_FILLEDCOL		0xffffff
#define QW_OBJ_PERCBAR_EMPTYCOL			0x201b1b
#define QW_OBJ_PERCBAR_NOTCHES			1
#define QW_OBJ_PERCBAR_SMALLNOTCHCOL	0x777777
#define QW_OBJ_PERCBAR_BIGNOTCHCOL		0xffffff
#define QW_OBJ_PERCBAR_FASTBAR			0
#define QW_OBJ_PERCBAR_FASTBARCOL		0x346fab
#define QW_OBJ_PERCBAR_DISABLEDCOL		0x444444
#define QW_OBJ_PERCBAR_CTR				1

// text
#define QW_OBJ_TEXT_CAPITALIZE			0
#define QW_OBJ_TEXT_CTR					1

// tooltip
#define QW_OBJ_TOOLTIP_BRDCOL			0x777777
#define QW_OBJ_TOOLTIP_BGCOL			0x333333


// spell effects
#define QW_SPFX_ROWS					5
#define QW_SPFX_COLS					2
#define QW_SPFX_BLINK_REMAIN			10000   // Start blinking when there's less than that many msec remaining on the spell effect
#define QW_SPFX_BLINK_RATE				50		// Blink every X client frame

