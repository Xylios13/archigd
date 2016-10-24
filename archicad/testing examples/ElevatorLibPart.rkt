#lang racket
(require "main.rkt")

#|
(send (create-library-part "My Elevator"
                           2d-code-elevator
                           3d-code-elevator
                           #:master-code master-code-elevator
                           #:parameter-code parameter-code-elevator
                           #:additional-parameters additional-parameters-elevator))



(send (create-library-part "My Elevator"
                           2d-code-elevator
                           3d-code-elevator
                           #:master-code master-code-elevator
                           #:parameter-code parameter-code-elevator
                           #:additional-parameters (list (list "teste" (list 1 2 3)))))

|#

(define 2d-code-elevator
"unID = 1
current_story_index = 0
n = REQUEST (\"Story\", \"\", current_story_index, current_story_name)

if isFrom14 and GLOB_CONTEXT <> 5 then
	if bSplitLevelStories then
		if current_story_index > connected_stories_id[1]/2 or current_story_index < connected_stories_id[num_connected_stories]/2 then end
	else
		if current_story_index > connected_stories_id[1] or current_story_index < connected_stories_id[num_connected_stories] then end
	endif
endif

if gs_detlevel_2D_m = 1 then	! Scale Sensitive
	if GLOB_SCALE <= 75 then
		gs_detlevel_2D_m = 2
	else
		if GLOB_SCALE <= 150 then
			gs_detlevel_2D_m = 3
		else
			gs_detlevel_2D_m = 4
		endif
	endif
endif

SOLID_WALL		= 1
POCKETCUT_WALL	= 2
SIDECUT_WALL	= 3

! ==============================================================================
! Rectangular
! ==============================================================================
if elevator_form_m = FORM_RECT then

	if bShowElevatorShaft then
		hotspot2 0, 0, unID                             : unID = unID + 1
		hotspot2 totalShaftWidth, 0, unID               : unID = unID + 1
		hotspot2 totalShaftWidth, totalShaftDepth, unID : unID = unID + 1
		hotspot2 0, totalShaftDepth, unID               : unID = unID + 1
	else
		unID = unID + 4
	endif

	hotspot2 elev_shaft_thick, elev_shaft_thick, unID                                 : unID = unID + 1
	hotspot2 totalShaftWidth-elev_shaft_thick, elev_shaft_thick, unID                 : unID = unID + 1
	hotspot2 totalShaftWidth-elev_shaft_thick, totalShaftDepth-elev_shaft_thick, unID : unID = unID + 1
	hotspot2 elev_shaft_thick, totalShaftDepth-elev_shaft_thick, unID                 : unID = unID + 1

	if gs_detlevel_2D_m = 6 then	! Symbolic 2 = Big X

		if bShowElevatorShaft then
			wBigX = totalShaftWidth
			hBigX = totalShaftDepth
		else
			add2 elev_shaft_thick+carSide1, elev_shaft_thick+carSpaceFront
			wBigX = car_width
			hBigX = car_depth
		endif
		pen gs_cont_pen
		fill gs_fill_type
		poly2_b 5, 7, gs_fill_pen, gs_back_pen,
				0,     0, 1,
				wBigX, 0, 1,
				wBigX, hBigX, 1,
				0,     hBigX, 1,
				0,     0, -1
		line2 0, 0, wBigX, hBigX
		line2 0, hBigX, wBigX, 0
		if not(bShowElevatorShaft) then del 1
		end
	endif

	! --------------------------------------------------------------------------
	! Elevator Shaft
	! --------------------------------------------------------------------------

	if bShowElevatorShaft then
		if bSplitLevelStories then
			storyID = current_story_index*2
			if storyID < 0 then
				elevShaftDoorPosID = SL_below_story_door_pos[abs(storyID)]
			else
				elevShaftDoorPosID = SL_above_story_door_pos[storyID+1]
			endif
		else
			storyID = current_story_index
			if storyID < 0 then
				elevShaftDoorPosID = below_story_door_pos[abs(storyID)]
			else
				elevShaftDoorPosID = above_story_door_pos[storyID+1]
			endif
		endif


		! --- Walls and Doors

		elev_wall_thick = elev_shaft_thick
		pen gs_wall_cont_pen
		fill gs_gap_fill_type
		if gs_detlevel_2D_m <> 4 then
			add2 elev_shaft_thick, elev_shaft_thick
			poly2_b 5, 6, gs_gap_fill_pen, gs_gap_back_pen,
				0, 0, 1,
				totalShaftWidth-elev_shaft_thick*2, 0, 1,
				totalShaftWidth-elev_shaft_thick*2, totalShaftDepth-elev_shaft_thick*2, 1,
				0, totalShaftDepth-elev_shaft_thick*2, 1,
				0, 0, -1
			del 1
			fill gs_wall_fill_type

			if dir_second_opening_m = SECOND_OPENING_SIDE1 then
				pl=elev_shaft_thick+carSide1+car_width/2
			else
				if dir_second_opening_m = SECOND_OPENING_SIDE2 then
					pl=totalShaftWidth-car_width-elev_shaft_thick-carSide2+car_width/2
				else
					pl=carSide1+elev_shaft_thick+car_width/2
				endif
			endif
			if elevShaftDoorPosID = 1  or elevShaftDoorPosID = 3 then				! --- Front wall with door
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					pl-opening_width/2+door_pos, 0, 1,
					pl-opening_width/2+door_pos, elev_shaft_thick, 1,
					elev_shaft_thick, elev_shaft_thick, 0,
					0, 0, -1

				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					totalShaftWidth, 0, 1,
					pl+opening_width/2+door_pos, 0, 1,
					pl+opening_width/2+door_pos, elev_shaft_thick, 1,
					totalShaftWidth-elev_shaft_thick, elev_shaft_thick, 0,
					totalShaftWidth, 0, -1

				add2 pl+door_pos, 0
				frame_thk = shaft_inner_depth/2-car_depth/2 !-0.06
				gosub \"shaft door\"
				del 1
			else													! --- Front wall
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					totalShaftWidth, 0, 0,
					totalShaftWidth-elev_shaft_thick, elev_shaft_thick, 1,
					elev_shaft_thick, elev_shaft_thick, 0,
					0, 0, -1
				unID = unID + 4
			endif

			if (elevShaftDoorPosID = 2 or elevShaftDoorPosID = 3) and dir_second_opening_m = SECOND_OPENING_REAR then		! --- Back wall with second door
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					0, totalShaftDepth, 1,
					pl-opening_width/2+second_door_pos, totalShaftDepth, 1,
					pl-opening_width/2+second_door_pos, totalShaftDepth-elev_shaft_thick, 1,
					elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 0,
					0, totalShaftDepth, -1

				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					totalShaftWidth, totalShaftDepth, 1,
					pl+opening_width/2+second_door_pos, totalShaftDepth, 1,
					pl+opening_width/2+second_door_pos, totalShaftDepth-elev_shaft_thick, 1,
					totalShaftWidth-elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 0,
					totalShaftWidth, totalShaftDepth, -1

				add2 pl+second_door_pos, totalShaftDepth
				rot2 180
				frame_thk = shaft_inner_depth/2-car_depth/2 !-0.06
				gosub \"shaft door\"
				del 2
			else													! --- Back wall
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					0, totalShaftDepth, 1,
					totalShaftWidth, totalShaftDepth, 0,
					totalShaftWidth-elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 1,
					elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 0,
					0, totalShaftDepth, -1
				unID = unID + 4
			endif

			if (elevShaftDoorPosID = 2 or elevShaftDoorPosID = 3) and dir_second_opening_m = SECOND_OPENING_SIDE1 then		! --- Wall on Side 1 with second door
				pl=car_depth/2+elev_shaft_thick+openingFromSide
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					0, pl-opening_width/2+second_door_pos, 1,
					elev_shaft_thick, pl-opening_width/2+second_door_pos, 1,
					elev_shaft_thick, elev_shaft_thick, 0,
					0, 0, -1

				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					0, totalShaftDepth, 1,
					0, pl+opening_width/2+second_door_pos, 1,
					elev_shaft_thick, pl+opening_width/2+second_door_pos, 1,
					elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 0,
					0, totalShaftDepth, -1

				add2 0, pl+second_door_pos
				rot2 -90
				frame_thk = shaft_inner_width/2-car_width/2 !-0.06
				gosub \"shaft door\"
				del 2
			else													! --- Wall on Side 1
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					0, totalShaftDepth, 0,
					elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 1,
					elev_shaft_thick, elev_shaft_thick, 0,
					0, 0, -1
				unID = unID + 4
			endif

			if (elevShaftDoorPosID = 2 or elevShaftDoorPosID = 3) and dir_second_opening_m = SECOND_OPENING_SIDE2 then		! --- Wall on Side 2 with second door
				pl=openingFromSide+car_depth/2+elev_shaft_thick
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					totalShaftWidth, 0, 1,
					totalShaftWidth, pl-opening_width/2+second_door_pos, 1,
					totalShaftWidth-elev_shaft_thick, pl-opening_width/2+second_door_pos, 1,
					totalShaftWidth-elev_shaft_thick, elev_shaft_thick, 0,
					totalShaftWidth, 0, -1

				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					totalShaftWidth, totalShaftDepth, 1,
					totalShaftWidth, pl+opening_width/2+second_door_pos, 1,
					totalShaftWidth-elev_shaft_thick, pl+opening_width/2+second_door_pos, 1,
					totalShaftWidth-elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 0,
					totalShaftWidth, totalShaftDepth, -1

				add2 totalShaftWidth, pl+second_door_pos
				rot2 90
				frame_thk = shaft_inner_width/2-car_width/2 !-0.06
				gosub \"shaft door\"
				del 2
			else													! --- Wall on Side 2
				poly2_b 5, 7+32, gs_wall_fill_pen, gs_wall_back_pen,
					totalShaftWidth, 0, 1,
					totalShaftWidth, totalShaftDepth, 0,
					totalShaftWidth-elev_shaft_thick, totalShaftDepth-elev_shaft_thick, 1,
					totalShaftWidth-elev_shaft_thick, elev_shaft_thick, 0,
					totalShaftWidth, 0, -1
				unID = unID + 4
			endif
		else	! gs_detlevel_2D_m = 4
			poly2_b 5, 7, gs_gap_fill_pen, gs_gap_back_pen,
				0,               0, 1,
				totalShaftWidth, 0, 1,
				totalShaftWidth, totalShaftDepth, 1,
				0,               totalShaftDepth, 1,
				0,               0, -1
			unID = unID + 16
		endif
	else	! show elevator shaft
		unID = unID + 16
	endif


	! --------------------------------------------------------------------------
	! Elevator Car
	! --------------------------------------------------------------------------

	! --- where are doors in elevator car?
	! --- the doors are always available by design independently from shaft(wall)
	bFrontDoorPresent		= 1			! always displayed
	bSecondaryDoorPresent	= (dir_second_opening_m <> SECOND_OPENING_NONE)

	add2 elev_shaft_thick+carSide1, elev_shaft_thick+carSpaceFront

	hotspot2 0, 0, unID                     : unID = unID + 1
	hotspot2 car_width, 0, unID             : unID = unID + 1
	hotspot2 car_width, car_depth, unID     : unID = unID + 1
	hotspot2 0, car_depth, unID             : unID = unID + 1
	hotspot2 car_width/2, car_depth/2, unID : unID = unID + 1

	pen gs_cont_pen
	fill gs_fill_type
	poly2_b 5, 7+64, gs_fill_pen, gs_back_pen,
		0, 0, 1,
		car_width, 0, 1,
		car_width, car_depth, 1,
		0, car_depth, 1,
		0, 0, -1

	if current_story_index*(1+bSplitLevelStories) = car_pos_story_m then
		if gs_detlevel_2D_m = 5 then
			! --- symbolic car -------------------------------------------------
			line2 0, 0, car_width, car_depth
			line2 car_width, 0, 0, car_depth

			if bFrontDoorPresent then
				add2 car_width/2+door_pos, frontGap
				gosub \"elev car door\"
				del 1
			endif

			if bSecondaryDoorPresent then
				if dir_second_opening_m = SECOND_OPENING_REAR then
					add2 car_width/2+second_door_pos, car_depth-frontGap
					rot2 180
					gosub \"elev car door\"
					del 2
				endif
				if dir_second_opening_m = SECOND_OPENING_SIDE1 then
					add2 frontGap/2, car_depth/2+second_door_pos
					rot2 -90
					gosub \"elev car door\"
					del 2
				endif
				if dir_second_opening_m = SECOND_OPENING_SIDE2 then
					add2 car_width-frontGap/2, car_depth/2+second_door_pos
					rot2 90
					gosub \"elev car door\"
					del 2
				endif
			endif
		else
			if gs_detlevel_2D_m < 4 then
				! --- detailed car ---------------------------------------------
				! --- front wall ---
				drawWall_detlevel		= gs_detlevel_2D_m
				drawWall_length			= car_width
				drawWall_bDoorPresent	= bFrontDoorPresent
				drawWall_doorStyle		= door_style_m
				drawWall_doorPos		= door_pos
				gosub \"drawOneCarWall\"


				! --- rear wall ---
				add2 0, car_depth
				mul2 1, -1
				drawWall_detlevel		= gs_detlevel_2D_m
				drawWall_length			= car_width
				drawWall_bDoorPresent	= (bSecondaryDoorPresent and dir_second_opening_m = SECOND_OPENING_REAR)
				drawWall_doorStyle		= door_style_m
				drawWall_doorPos		= second_door_pos
				gosub \"drawOneCarWall\"
				del 2


				! --- side 1 wall ---
				rot2 90
				mul2 1, -1
				drawWall_detlevel		= gs_detlevel_2D_m
				drawWall_length			= car_depth
				drawWall_bDoorPresent	= (bSecondaryDoorPresent and dir_second_opening_m = SECOND_OPENING_SIDE1)
				drawWall_doorStyle		= door_style_m
				drawWall_doorPos		= second_door_pos
				gosub \"drawOneCarWall\"
				del 2

				! --- side 2 wall ---
				add2 car_width, 0
				rot2 90
				drawWall_detlevel		= gs_detlevel_2D_m
				drawWall_length			= car_depth
				drawWall_bDoorPresent	= (bSecondaryDoorPresent and dir_second_opening_m = SECOND_OPENING_SIDE2)
				drawWall_doorStyle		= door_style_m
				drawWall_doorPos		= second_door_pos
				gosub \"drawOneCarWall\"
				del 2
			endif
		endif
	endif

	! --------------------------------------------------------------------------
	! Mechanics
	! --------------------------------------------------------------------------

	if gs_detlevel_2D_m = 2 and bShowMechin2D then
		if elevator_type_m = 1 then					! --- Mechanical
			del ntr()
			add2 elev_shaft_thick, elev_shaft_thick
			del_num = 0
			if cweight_pos_m = CW_POS_NORMAL then
				add2 shaft_inner_width/2, car_depth+carSpaceRear+carSpaceFront
				del_num = 1
			endif
			if cweight_pos_m = CW_POS_SIDE1 then
				add2 carSpaceSide1+cweight_depth, shaft_inner_depth/2
				rot2 90
				del_num = 2
			endif
			if cweight_pos_m = CW_POS_SIDE2 then
				add2 carSide1+car_width+carSpaceSide2, shaft_inner_depth/2
				rot2 -90
				del_num = 2
			endif

			add2 -cweight_width/2, 0
			hotspot2 0, cweight_depth/2, unID: unID = unID + 1
			hotspot2 cweight_width, cweight_depth/2, unID: unID = unID + 1

			line2 cweight_depth/6, 0, cweight_width-cweight_depth/6, 0
			line2 cweight_depth/6, cweight_depth, cweight_width-cweight_depth/6, cweight_depth
			line2 cweight_depth/2, 0, cweight_depth/2, cweight_depth
			line2 cweight_width-cweight_depth/2, 0, cweight_width-cweight_depth/2, cweight_depth
			line2 0, 0, 0, cweight_depth
			line2 cweight_width, 0, cweight_width, cweight_depth
			line2 0, cweight_depth/2, cweight_depth/3, cweight_depth/2
			line2 cweight_width, cweight_depth/2, cweight_width-cweight_depth/3, cweight_depth/2
			del del_num+2
		else										! --- Hydraulic
			add2 car_width/2, car_depth/2
			line_type 21
			rect2 -car_width/6, -car_depth/6, car_width/6, car_depth/6
			circle2 0, 0, shaft_inner_width/15
			del 2
			unID = unID + 2
		endif
	else
		unID = unID + 2
		del 1
	endif
endif


! ==============================================================================
! Segmented
! ==============================================================================
if elevator_form_m = FORM_SEGMENTED then

	if bShowElevatorWall then
		hotspot2 0, 0, unID: unID = unID + 1
		hotspot2 A, 0, unID: unID = unID + 1
	else
		add2 elev_wall_overhang, 0
		unID = unID + 2
	endif
	hotspot2 A, elev_wall_thick, unID: unID = unID + 1
	hotspot2 0, elev_wall_thick, unID: unID = unID + 1

	if gs_detlevel_2D_m = 2 or gs_detlevel_2D_m = 3 or gs_detlevel_2D_m = 5 then
		
		! --- Elevator Wall ----------------------------------------------------

		if bShowElevatorWall then
			storyID = current_story_index
			if storyID < 0 then
				elevShaftDoorPosID = S_below_story_door_pos[abs(storyID)]
			else
				elevShaftDoorPosID = S_above_story_door_pos[storyID+1]
			endif

			poly2_b 5, 6, 0, 0,
				0, 0, 1,
				A, 0, 1,
				A, elev_wall_thick, 1,
				0, elev_wall_thick, 1,
				0, 0, -1

			if elevShaftDoorPosID = 1 then				! --- Front wall with door
				pen gs_wall_cont_pen
				fill gs_wall_fill_type

				poly2_b 5, 7, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					A/2-opening_width/2+door_pos, 0, 1,
					A/2-opening_width/2+door_pos, elev_wall_thick, 1,
					0, elev_wall_thick, 1,
					0, 0, -1

				poly2_b 5, 7, gs_wall_fill_pen, gs_wall_back_pen,
					A, 0, 1,
					A/2+opening_width/2+door_pos, 0, 1,
					A/2+opening_width/2+door_pos, elev_wall_thick, 1,
					A, elev_wall_thick, 1,
					A, 0, -1

				add2 A/2+door_pos, 0
				gosub \"shaft door\"
				del 1
			else													! --- Front wall
				pen gs_wall_cont_pen
				fill gs_wall_fill_type

				poly2_b 5, 7, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					A, 0, 1,
					A, elev_wall_thick, 1,
					0, elev_wall_thick, 1,
					0, 0, -1
				unID = unID + 4
			endif
		else
			unID = unID + 4
		endif
	endif


	! --- Elevator Car ---------------------------------------------------------

	alpha = 180/segment_num
	if segment_num/2=int(segment_num/2) then
		dy = car_width/2
	else
		dy = car_width/2*sin(int(segment_num/2)*alpha)
	endif
	add2 0, segmentedFront
	pen gs_cont_pen
	fill gs_fill_type

	add2 A/2, elev_wall_thick

	hotspot2 -car_width/2, carWallThk, unID: unID = unID + 1
	hotspot2 car_width/2, carWallThk, unID: unID = unID + 1
	hotspot2 -car_width/2, 0, unID: unID = unID + 1
	hotspot2 car_width/2, 0, unID: unID = unID + 1

	alpha = 180/segment_num
	for i = 1 to segment_num
		_tX = cos(alpha*i)*car_width/2
		_tY	= carWallThk+(sin(alpha*i)*car_width/2)*((car_depth-carWallThk)/dy)
		put _tX, _tY, 1
		hotspot2 _tX, _tY, unID: unID = unID + 1
	next i

	if gs_detlevel_2D_m = 4 or gs_detlevel_2D_m = 5 or gs_detlevel_2D_m = 6 then					! --- 1:200, Symbolic 2
		pen gs_cont_pen
		fill gs_fill_type
		poly2_b 4+segment_num, 7, gs_fill_pen, gs_back_pen,
			-car_width/2, carWallThk, 1,
			-car_width/2, 0, 1,
			car_width/2, 0, 1,
			car_width/2, carWallThk, 1,
			get(nsp)

		if gs_detlevel_2D_m = 5 or gs_detlevel_2D_m = 6 then			! --- Symbolic 2
			line2 -car_width/2, 0, cos(alpha*int(segment_num/3))*car_width/2, carWallThk+(sin(alpha*int(segment_num/3))*car_width/2)*((car_depth-carWallThk)/dy)
			line2 car_width/2, 0, -cos(alpha*int(segment_num/3))*car_width/2, carWallThk+(sin(alpha*int(segment_num/3))*car_width/2)*((car_depth-carWallThk)/dy)
		endif

		del 1
		pen gs_wall_cont_pen
		fill gs_wall_fill_type
		if (gs_detlevel_2D_m = 4 or gs_detlevel_2D_m = 6) & bShowElevatorWall then
			add2 0, -segmentedFront
			poly2_b 5, 7, gs_wall_fill_pen, gs_wall_back_pen,
				0, 0, 1,
				A, 0, 1,
				A, elev_wall_thick, 1,
				0, elev_wall_thick, 1,
				0, 0, -1
			del 1
		endif

	else											! --- 1:50, 1:100

		poly2_b 4+segment_num, 7, gs_fill_pen, gs_back_pen,
			-car_width/2, carWallThk, 1,
			-car_width/2, 0, 1,
			car_width/2, 0, 1,
			car_width/2, carWallThk, 1,
			get (nsp)

		if current_story_index = car_pos_story_m then
			for i = 1 to segment_num
				put cos(alpha*(i-1))*(car_width/2-0.02), carWallThk+(sin(alpha*(i-1))*(car_width/2-0.02))*((car_depth-carWallThk)/dy), 1
			next i

			poly2_b 4+segment_num, 5, 0, 0,
				-car_width/2+0.02, carWallThk, 1,
				-door_width/2+door_pos, carWallThk, 0,
				door_width/2+door_pos, carWallThk, 1,
				car_width/2-0.02, carWallThk, 1,
				get(nsp)

			wallStatus_detlevel		= gs_detlevel_2D_m
			wallStatus_doorStyle	= door_style_m
			wallStatus_doorPos		= door_pos
			gosub \"determineWallStatusAroundDoor\"

			if leftWallStatus = SOLID_WALL then
				line2 -door_width/2+door_pos, carWallThk, -door_width/2+door_pos, 0

			else
				if leftWallStatus = SIDECUT_WALL then
					line2 -door_width+door_pos, 0, -door_width+door_pos, doorThk
					line2 -door_width+door_pos, doorThk, -door_width/2+door_pos, doorThk
					line2 -door_width/2+door_pos, doorThk, -door_width/2+door_pos, carWallThk
				else
					line2 -door_width/2+door_pos, 0, -door_width/2+door_pos, elevCarOffset
					line2 -door_width/2+door_pos, elevCarOffset, -door_width+door_pos, elevCarOffset
					line2 -door_width+door_pos, elevCarOffset, -door_width+door_pos, elevCarOffset+doorThk 
					line2 -door_width+door_pos, elevCarOffset+doorThk, -door_width/2+door_pos, elevCarOffset+doorThk
					line2 -door_width/2+door_pos, elevCarOffset+doorThk, -door_width/2+door_pos, carWallThk
				endif
			endif

			if rightWallStatus= SOLID_WALL then
				line2 door_width/2+door_pos, carWallThk, door_width/2+door_pos, 0
			else
				if rightWallStatus= SIDECUT_WALL then
					line2 door_width+door_pos, 0, door_width+door_pos, doorThk
					line2 door_width+door_pos, doorThk, door_width/2+door_pos, doorThk
					line2 door_width/2+door_pos, doorThk, door_width/2+door_pos, carWallThk
				else
					line2 door_width/2+door_pos, 0, door_width/2+door_pos, elevCarOffset
					line2 door_width/2+door_pos, elevCarOffset, door_width+door_pos, elevCarOffset
					line2 door_width+door_pos, elevCarOffset, door_width+door_pos, elevCarOffset+doorThk 
					line2 door_width+door_pos, elevCarOffset+doorThk, door_width/2+door_pos, elevCarOffset+doorThk
					line2 door_width/2+door_pos, elevCarOffset+doorThk, door_width/2+door_pos, carWallThk
				endif
			endif

			if gs_detlevel_2D_m = 2 then
				add2 door_pos, elevCarOffset
				gosub \"elev car door\"
				del 1
			endif
		endif
		del 1
	endif
	del 1 + not(bShowElevatorWall)
endif


end

! ==============================================================================
! Subroutines
! ==============================================================================

! ------------------------------------------------------------------------------
! determineWallStatusAroundDoor
! input:
!  wallStatus_detlevel
!  wallStatus_doorStyle
!  wallStatus_doorPos
! output:
!  leftWallStatus
!  rightWallStatus
!  doorThk
! ------------------------------------------------------------------------------
\"determineWallStatusAroundDoor\":
	doorThk	 = elevCarDoorThk
	if wallStatus_detlevel = 2 then
		if wallStatus_doorStyle = 1 then
			! sliding to one side
			doorThk	= elevCarDoorThk*2
			if wallStatus_doorPos < 0 then
				leftWallStatus	= SOLID_WALL
				rightWallStatus	= SIDECUT_WALL
			else
				leftWallStatus	= SIDECUT_WALL
				rightWallStatus	= SOLID_WALL
			endif
		else
			if wallStatus_doorStyle = 5 then
				! sliding to one side as pocket
				if wallStatus_doorPos < 0 then
					leftWallStatus	= SOLID_WALL
					rightWallStatus	= POCKETCUT_WALL
				else
					leftWallStatus	= POCKETCUT_WALL
					rightWallStatus	= SOLID_WALL
				endif
			else
				if wallStatus_doorStyle = 2 or wallStatus_doorStyle = 4 then
					! sliding to both sides as pocket
					leftWallStatus	= POCKETCUT_WALL
					rightWallStatus	= POCKETCUT_WALL
				else
					!Style 3
					leftWallStatus	= SOLID_WALL
					rightWallStatus	= SOLID_WALL
				endif
			endif
		endif
	else
		leftWallStatus	= SOLID_WALL
		rightWallStatus	= SOLID_WALL
	endif
return


! ------------------------------------------------------------------------------
! drawOneCarWall
! input:
!  drawWall_detlevel
!  drawWall_doorStyle
!  drawWall_bDoorPresent
!  drawWall_length
!  drawWall_doorPos
!  doorThk
!  door_width
!  gs_wall_fill_pen, gs_wall_back_pen
!  carWallThk
! ------------------------------------------------------------------------------
\"drawOneCarWall\":
	if drawWall_bDoorPresent then
		wallStatus_detlevel		= drawWall_detlevel
		wallStatus_doorStyle	= drawWall_doorStyle
		wallStatus_doorPos		= drawWall_doorPos
		gosub \"determineWallStatusAroundDoor\"

		if leftWallStatus = SOLID_WALL then
			poly2_b 5, 5, gs_wall_fill_pen, gs_wall_back_pen,
				0, 0, 1,
				drawWall_length/2-door_width/2+drawWall_doorPos, 0, 1,
				drawWall_length/2-door_width/2+drawWall_doorPos, carWallThk, 1,
				carWallThk,                                      carWallThk, 0,
				0, 0, -1
		else
			if leftWallStatus = SIDECUT_WALL then
				poly2_b 7, 5, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					drawWall_length/2-door_width+drawWall_doorPos,   0, 1,
					drawWall_length/2-door_width+drawWall_doorPos,   doorThk, 1,
					drawWall_length/2-door_width/2+drawWall_doorPos, doorThk, 1,
					drawWall_length/2-door_width/2+drawWall_doorPos, carWallThk, 1,
					carWallThk,                                      carWallThk, 0,
					0, 0, -1
			else
				poly2_b 9, 5, gs_wall_fill_pen, gs_wall_back_pen,
					0, 0, 1,
					drawWall_length/2-door_width/2+drawWall_doorPos, 0, 1,
					drawWall_length/2-door_width/2+drawWall_doorPos, elevCarOffset, 1,
					drawWall_length/2-door_width+drawWall_doorPos,   elevCarOffset, 1,
					drawWall_length/2-door_width+drawWall_doorPos,   elevCarOffset+doorThk, 1,
					drawWall_length/2-door_width/2+drawWall_doorPos, elevCarOffset+doorThk, 1,
					drawWall_length/2-door_width/2+drawWall_doorPos, carWallThk, 1,
					carWallThk,                                      carWallThk, 0,
					0, 0, -1
			endif
		endif

		if rightWallStatus = SOLID_WALL then
			poly2_b 5, 5, gs_wall_fill_pen, gs_wall_back_pen,
				drawWall_length,                                 0, 1,
				drawWall_length/2+door_width/2+drawWall_doorPos, 0, 1,
				drawWall_length/2+door_width/2+drawWall_doorPos, carWallThk, 1,
				drawWall_length-carWallThk,                      carWallThk, 0,
				drawWall_length,                                 0, -1
		else
			if rightWallStatus = SIDECUT_WALL then
				poly2_b 7, 5, gs_wall_fill_pen, gs_wall_back_pen,
					drawWall_length,                                 0, 1,
					drawWall_length/2+door_width+drawWall_doorPos,   0, 1,
					drawWall_length/2+door_width+drawWall_doorPos,   doorThk, 1,
					drawWall_length/2+door_width/2+drawWall_doorPos, doorThk, 1,
					drawWall_length/2+door_width/2+drawWall_doorPos, carWallThk, 1,
					drawWall_length-carWallThk,                      carWallThk, 0,
					drawWall_length,                                 0, -1
			else
				poly2_b 9, 5, gs_wall_fill_pen, gs_wall_back_pen,
					drawWall_length,                                 0, 1,
					drawWall_length/2+door_width/2+drawWall_doorPos, 0, 1,
					drawWall_length/2+door_width/2+drawWall_doorPos, elevCarOffset, 1,
					drawWall_length/2+door_width+drawWall_doorPos,   elevCarOffset, 1,
					drawWall_length/2+door_width+drawWall_doorPos,   elevCarOffset+doorThk, 1,
					drawWall_length/2+door_width/2+drawWall_doorPos, elevCarOffset+doorThk, 1,
					drawWall_length/2+door_width/2+drawWall_doorPos, carWallThk, 1,
					drawWall_length-carWallThk,                      carWallThk, 0,
					drawWall_length,                                 0, -1
			endif
		endif

		add2 drawWall_length/2+drawWall_doorPos, elevCarOffset
		if drawWall_doorPos < 0 then mul2 -1, 1
		gosub \"elev car door\"
		if drawWall_doorPos < 0 then del 1
		del 1
	else
		poly2_b 5, 5, gs_wall_fill_pen, gs_wall_back_pen,
			0,                          0, 1,
			drawWall_length,            0, 0,
			drawWall_length-carWallThk, carWallThk, 1,
			carWallThk, carWallThk,     0,
			0,                          0, -1
	endif
return

\"shaft door\":
	if gs_detlevel_2D_m = 2 then
		pen gs_wall_cont_pen
		fill gs_opening_fill_type

		add2 -opening_width/2, elev_wall_thick
		hotspot2 0, 0, unID: unID = unID + 1
		poly2_b 5, 7+32, gs_opening_fill_pen, gs_opening_back_pen,
			0, 0, 1,
			0.18, 0, 1,
			0.18, -0.06, 1,
			0, -0.06, 1,
			0, 0, -1

		hotspot2 opening_width, 0, unID: unID = unID + 1
		poly2_b 5, 7+32, gs_opening_fill_pen, gs_opening_back_pen,
			opening_width, 0, 1,
			opening_width-0.18, 0, 1,
			opening_width-0.18, -0.06, 1,
			opening_width, -0.06, 1,
			opening_width, 0, -1

		if bOpeningFrame then
			hotspot2 0.02, -0.06, unID: unID = unID + 1
			hotspot2 opening_width-0.02, -0.06, unID: unID = unID + 1

			for i = -1 to 1 step 2
				add2 opening_width*(i < 0), 0
				mul2 i, 1
				poly2_b 7, 7+32, gs_opening_fill_pen, gs_opening_back_pen,
					0, -0.06, 1,
					0.02, -0.06, 1,
					0.02, -elev_wall_thick-0.02, 1,
					-0.04, -elev_wall_thick-0.02, 1,
					-0.04, -elev_wall_thick, 1,
					0, -elev_wall_thick, 1,
					0, -0.06, -1
				del 2
			next i
		else
			hotspot2 0, -0.06, unID: unID = unID + 1
			hotspot2 opening_width, -0.06, unID: unID = unID + 1
		endif

		opening_in2D_in_car_pos = 0
		if elev_door_style_m = 1 then		! --- Sliding door
			if (gs_detlevel_2D_m <> 5 or gs_detlevel_2D_m <> 4) and (car_width/2-door_width/2+door_pos)<0 then mul2 1, -1
			add2 0.18-(door_width/2)*(opening_in2D_in_car_pos/100), 0
			rect2 0, 0, door_width/2, 0.02
			del 1

			add2 0.18+door_width/2-door_width*(opening_in2D_in_car_pos/100), 0.02
			rect2 0, 0, door_width/2, 0.02
			del 1
			if (gs_detlevel_2D_m <> 5 or gs_detlevel_2D_m <> 4) and (car_width/2-door_width/2+door_pos)<0 then del 1
		endif

		if elev_door_style_m = 2 then		! --- Opening door

			add2 0.18, 0
			rot2 -90*(opening_in2D_in_car_pos/100)
			poly2_b 5, 5, 0, 0,
				0, 0, 1,
				door_width/2, 0, 1,
				door_width/2, 0.02, 1,
				0, 0.02, 1,
				0, 0, -1
			del 2

			add2 door_width+0.18, 0
			rot2 90*(opening_in2D_in_car_pos/100)
			add2 -door_width/2, 0
			poly2_b 5, 5, 0, 0,
				0, 0, 1,
				door_width/2, 0, 1,
				door_width/2, 0.02, 1,
				0, 0.02, 1,
				0, 0, -1
			del 3
		endif
		del 1

		pen gs_wall_cont_pen
		fill gs_wall_fill_type
	else
		pen gs_wall_cont_pen
		fill gs_wall_fill_type
		line2 -opening_width/2, elev_wall_thick, opening_width/2, elev_wall_thick
		unID = unID + 4
	endif
return


\"elev car door\":
	if door_style_m and gs_detlevel_2D_m = 2 then
		if door_style_m = 1 then
			add2 -door_width/2, 0
			rect2 0, 0, door_width/2, elevCarDoorThk
			del 1
			add2 0, elevCarDoorThk
			rect2 0, 0, door_width/2, elevCarDoorThk
			del 1
		endif
		if door_style_m = 2 then
			add2 -door_width/2, 0
			rect2 0, 0, door_width/2, elevCarDoorThk
			del 1
			rect2 0, 0, door_width/2, elevCarDoorThk
		endif
		if door_style_m = 3 then
			add2 -door_width/2, 0
			rect2 0, 0, door_width/2, elevCarDoorThk
			line2 0.05, 0, 0.05, elevCarDoorThk
			line2 door_width/2-0.05, 0, door_width/2-0.05, elevCarDoorThk
			line2 0.05, elevCarDoorThk/2, door_width/2-0.05, elevCarDoorThk/2
			del 1
			rect2 0, 0, door_width/2, elevCarDoorThk
			line2 0.05, 0, 0.05, elevCarDoorThk
			line2 door_width/2-0.05, 0, door_width/2-0.05, elevCarDoorThk
			line2 0.05, elevCarDoorThk/2, door_width/2-0.05, elevCarDoorThk/2
		endif
		if door_style_m = 4 then
			add2 -door_width/2, 0
			rect2 0, 0, door_width/2, elevCarDoorThk
			line2 0.03, 0, 0.03, elevCarDoorThk
			line2 door_width/2-0.03, 0, door_width/2-0.03, elevCarDoorThk
			line2 0.03, elevCarDoorThk/2, door_width/2-0.03, elevCarDoorThk/2
			del 1
			rect2 0, 0, door_width/2, elevCarDoorThk
			line2 0.03, 0, 0.03, elevCarDoorThk
			line2 door_width/2-0.03, 0, door_width/2-0.03, elevCarDoorThk
			line2 0.03, elevCarDoorThk/2, door_width/2-0.03, elevCarDoorThk/2
		endif
		if door_style_m = 5 then
			add2 -door_width/2, 0
			rect2 0, 0, door_width/2, elevCarDoorThk
			del 1
			add2 0, elevCarDoorThk/4
			rect2 0, 0, door_width/2, elevCarDoorThk/2
			del 1
		endif
	endif

	if gs_detlevel_2D_m = 5 then
		line2 0, -0.20, 0, 0.20
	endif
return")

(define parameter-code-elevator
"call \"FM_types\" parameters all

! =============================================================================
! Detlevel 3D
! =============================================================================
values \"gs_detlevel_3D\" stDetail
values \"gs_detlevel_3D_m\" 2, 1, 0

if GLOB_MODPAR_NAME = \"gs_detlevel_3D\" then
	gs_detlevel_3D_m = 2
	if gs_detlevel_3D = stDetail[2] then gs_detlevel_3D_m = 1	! Simple
	if gs_detlevel_3D = stDetail[3] then gs_detlevel_3D_m = 0	! Off
	parameters gs_detlevel_3D_m = gs_detlevel_3D_m
else
	gs_detlevel_3D = stDetail[1]
	if gs_detlevel_3D_m = 1 then gs_detlevel_3D = stDetail[2]
	if gs_detlevel_3D_m = 0 then gs_detlevel_3D = stDetail[3]
	parameters gs_detlevel_3D = gs_detlevel_3D
endif


! =============================================================================
! Detlevel 2D
! =============================================================================
values \"gs_detlevel_2D\" stDetail2D
values \"gs_detlevel_2D_m\" 1, 2, 3, 4, 5, 6

if GLOB_MODPAR_NAME = \"gs_detlevel_2D\" then
	for i = 1 to vardim1(stDetail2D)
		if gs_detlevel_2D = stDetail2D[i] then
			gs_detlevel_2D_m = i
			i = 30
		endif
	next i
	parameters gs_detlevel_2D_m = gs_detlevel_2D_m
else
	gs_detlevel_2D = stDetail2D[gs_detlevel_2D_m]
	parameters gs_detlevel_2D = gs_detlevel_2D
endif


! =============================================================================
! Elevator Type
! =============================================================================
values \"elevator_type\" stElevType
values \"elevator_type_m\" 1, 2

if GLOB_MODPAR_NAME = \"elevator_type\" then
	elevator_type_m = 1
	if elevator_type = stElevType[2] then elevator_type_m = 2
	parameters elevator_type_m = elevator_type_m
else
	elevator_type = stElevType[elevator_type_m]
	parameters elevator_type = elevator_type
endif


! =============================================================================
! Second Opening
! =============================================================================
values \"dir_second_opening\" stDirection
values \"dir_second_opening_m\" 1, 2, 3, 4

if GLOB_MODPAR_NAME = \"dir_second_opening\" then
	for i = 1 to vardim1(stDirection)
		if dir_second_opening = stDirection[i] then
			dir_second_opening_m = i
			i = 99
		endif
	next i
	parameters dir_second_opening_m = dir_second_opening_m
else
	dir_second_opening = stDirection[dir_second_opening_m]
	parameters dir_second_opening = dir_second_opening
endif

if GLOB_MODPAR_NAME = \"bSplitLevelStories\" and bSplitLevelStories then
	parameters dir_second_opening = stDirection[3]
endif

if dir_second_opening_m = SECOND_OPENING_REAR then
	lock \"car_depth\"
endif
if dir_second_opening_m = SECOND_OPENING_NONE then
	hideparameter	\"second_door_pos\"
	lock			\"second_door_pos\"
endif

dim stAvaliablePosition[]
dim iAvaliablePosition[]
j = 1
for i = 1 to 3
	if dir_second_opening_m-1 <> i then
		stAvaliablePosition[j]	= stPosition[i]
		iAvaliablePosition[j]	= i
		j = j + 1
	endif
next i

values \"cweight_pos\"	stAvaliablePosition
values \"cweight_pos_m\"	iAvaliablePosition

if GLOB_MODPAR_NAME = \"cweight_pos\" then
	for i = 1 to 3
		if cweight_pos = stPosition[i] then
			cweight_pos_m = i
			i = 3
		endif
	next i
	parameters cweight_pos_m = cweight_pos_m
else
	cweight_pos_m = max(min(cweight_pos_m, vardim1(stPosition)), 1)
	parameters cweight_pos = stPosition[cweight_pos_m]
endif


! =============================================================================
! Door Style
! =============================================================================
values \"elev_door_style\" stElevDoorStyle
values \"elev_door_style_m\" 1, 2

if GLOB_MODPAR_NAME = \"elev_door_style\" then
	elev_door_style_m = 1
	if elev_door_style = stElevDoorStyle[2] then elev_door_style_m = 2
	parameters elev_door_style_m = elev_door_style_m
else
	elev_door_style = stElevDoorStyle[elev_door_style_m]
	parameters elev_door_style = elev_door_style
endif


values \"door_style\" stDoorStyle
values \"door_style_m\" 0, 1, 2, 3, 4, 5

if GLOB_MODPAR_NAME = \"door_style\" then
	for i = 1 to 6
		if door_style = stDoorStyle[i] then
			door_style_m = i-1
			i = 6
		endif
	next i
	parameters door_style_m = door_style_m
else
	door_style = stDoorStyle[door_style_m+1]
	parameters door_style = door_style
endif


! =============================================================================
! Form
! =============================================================================
values \"elevator_form\" stElevForm
values \"elevator_form_m\" 1, 2
values \"segment_num\" range [3, )

if GLOB_MODPAR_NAME = \"elevator_form\" then
	elevator_form_m = 1
	if elevator_form = stElevForm[2] then elevator_form_m = 2
	parameters elevator_form_m = elevator_form_m
else
	elevator_form = stElevForm[elevator_form_m]
	parameters elevator_form = elevator_form
endif

if elevator_form_m <> elevator_form_m_prev then
	if elevator_form_m = FORM_RECT then

		elev_shaft_thick	= elev_wall_thick
		bShowElevatorShaft	= bShowElevatorWall
		shaft_inner_width	= A - 2*elev_shaft_thick*bShowElevatorShaft
		shaft_inner_depth	= B - 2*elev_shaft_thick*bShowElevatorShaft

		parameters	shaft_inner_width	= shaft_inner_width,
					shaft_inner_depth	= shaft_inner_depth,
					elev_shaft_thick	= elev_shaft_thick,
					bShowElevatorShaft	= bShowElevatorShaft

	else

		elev_wall_thick		= elev_shaft_thick
		elev_wall_overhang	= 0
		bShowElevatorWall	= bShowElevatorShaft
		car_width			= A - 2*elev_wall_overhang*bShowElevatorWall
		car_depth			= B - segmentedFront - bShowElevatorWall * elev_wall_thick
		parameters	elev_wall_thick		= elev_wall_thick,
					elev_wall_overhang	= elev_wall_overhang,
					bShowElevatorWall	= bShowElevatorWall,
					car_width			= car_width,
					car_depth			= car_depth
	endif
endif

parameters elevator_form_m_prev = elevator_form_m

values \"penthouse_height\" range(0, )
if not(bPenthouse) then
	hideparameter	\"penthouse_height\"
	lock			\"penthouse_height\"
endif

values \"pit_depth\" range(0, )
if not(bPith) then
	hideparameter	\"pit_depth\"
	lock			\"pit_depth\"
endif

values \"top_slab_thick\" range (0, )
if not(bTopSlab) then
	hideparameter	\"top_slab_thick\",
					\"top_slab_ext_mat\",
					\"top_slab_int_mat\",
					\"top_slab_edge_mat\"

	lock			\"top_slab_thick\",
					\"top_slab_ext_mat\",
					\"top_slab_int_mat\",
					\"top_slab_edge_mat\"
endif

values \"pit_slab_thick\" range (0, )
if not(bPitSlab) then
	hideparameter	\"pit_slab_thick\",
					\"pit_slab_ext_mat\",
					\"pit_slab_int_mat\",
					\"pit_slab_edge_mat\"

	lock			\"pit_slab_thick\",
					\"pit_slab_ext_mat\",
					\"pit_slab_int_mat\",
					\"pit_slab_edge_mat\"
endif

lock \"edited_story_height\"

if elevator_form_m = FORM_RECT then
	lock \"segment_num\"
	if elevator_type_m = 2 then
		hideparameter	\"cweight_pos\",
						\"cweight_pos_m\",
						\"cweight_width\",
						\"cweight_depth\"

		lock 			\"cweight_pos\",
						\"cweight_pos_m\",
						\"cweight_width\",
						\"cweight_depth\"
	endif

	if not(bSplitLevelStories) then
		lock 	\"num_SL_stories_below_HS\",
				\"num_SL_stories_above_HS\",
				\"dist_SL_story_to_normal_story\"

		hideparameter 	\"num_SL_stories_below_HS\",
						\"num_SL_stories_above_HS\",
						\"dist_SL_story_to_normal_story\"
	endif

	if not(bShowElevatorShaft) then
		lock	\"bPenthouse\",
				\"penthouse_height\",
				\"bPith\",
				\"pit_depth\",
				\"elev_shaft_thick\",
				\"elev_shaft_ext_mat\",
				\"elev_shaft_int_mat\",
				\"elev_shaft_edge_mat\",
				\"bTopSlab\",
				\"top_slab_thick\",
				\"top_slab_ext_mat\",
				\"top_slab_int_mat\",
				\"top_slab_edge_mat\",
				\"bPitSlab\",
				\"pit_slab_thick\",
				\"pit_slab_ext_mat\",
				\"pit_slab_int_mat\",
				\"pit_slab_edge_mat\",
				\"gs_wall_cont_pen\",
				\"gs_wall_fill_type\",
				\"gs_wall_fill_pen\",
				\"gs_wall_back_pen\",
				\"gs_opening_fill_type\",
				\"gs_opening_fill_pen\",
				\"gs_opening_back_pen\"

		hideparameter	\"bPenthouse\",
						\"penthouse_height\",
						\"bPith\",
						\"pit_depth\",
						\"elev_shaft_thick\",
						\"elev_shaft_ext_mat\",
						\"elev_shaft_int_mat\",
						\"elev_shaft_edge_mat\",
						\"bTopSlab\",
						\"top_slab_thick\",
						\"top_slab_ext_mat\",
						\"top_slab_int_mat\",
						\"top_slab_edge_mat\",
						\"bPitSlab\",
						\"pit_slab_thick\",
						\"pit_slab_ext_mat\",
						\"pit_slab_int_mat\",
						\"pit_slab_edge_mat\",
						\"gs_wall_cont_pen\",
						\"gs_wall_fill_type\",
						\"gs_wall_fill_pen\",
						\"gs_wall_back_pen\",
						\"gs_opening_fill_type\",
						\"gs_opening_fill_pen\",
						\"gs_opening_back_pen\"
	endif

	lock	\"car_front_mat\",
			\"car_back_mat\",
			\"car_mullion_mat\",
			\"bShowElevatorWall\",
			\"elev_wall_thick\",
			\"elev_wall_overhang\",
			\"elev_wall_ext_mat\",
			\"elev_wall_int_mat\",
			\"elev_wall_edge_mat\",
			\"bShowMullion\"

	hideparameter	\"car_front_mat\",
					\"car_back_mat\",
					\"car_mullion_mat\",
					\"bShowElevatorWall\",
					\"elev_wall_thick\",
					\"elev_wall_overhang\",
					\"elev_wall_ext_mat\",
					\"elev_wall_int_mat\",
					\"elev_wall_edge_mat\",
					\"bShowMullion\"

	if not(bShowElevatorShaft) then
		lock 	\"elev_door_style\",
				\"elev_shaft_thick\",
				\"elev_shaft_ext_mat\",
				\"elev_shaft_int_mat\",
				\"elev_shaft_edge_mat\"

		hideparameter 	\"elev_door_style\",
						\"elev_shaft_thick\",
						\"elev_shaft_ext_mat\",
						\"elev_shaft_int_mat\",
						\"elev_shaft_edge_mat\"
	endif

	if not(gs_detlevel_2D_m = 1 | gs_detlevel_2D_m = 2) then
		lock \"bShowMechin2D\"
		hideparameter \"bShowMechin2D\"
	endif
else
!\"B\",
	lock 	\"elevator_type\",
			\"elevator_type_m\",
			\"dir_second_opening\",
			\"dir_second_opening_m\",
			\"cweight_pos\",
			\"cweight_pos_m\",
			\"cweight_width\",
			\"cweight_depth\",
			\"bSplitLevelStories\",
			\"num_SL_stories_below_HS\",
			\"num_SL_stories_above_HS\",
			\"dist_SL_story_to_normal_story\",
			\"second_door_pos\",
			\"bShowElevatorShaft\",
			\"elev_shaft_thick\",
			\"elev_shaft_ext_mat\",
			\"elev_shaft_int_mat\",
			\"elev_shaft_edge_mat\",
			\"bShowMechin2D\",
			
			\"shaft_inner_width\",
			\"shaft_inner_depth\"

!\"B\",
	hideparameter	\"elevator_type\",
					\"elevator_type_m\",
					\"dir_second_opening\",
					\"dir_second_opening_m\",
					\"cweight_pos\",
					\"cweight_pos_m\",
					\"cweight_width\",
					\"cweight_depth\",
					\"bSplitLevelStories\",
					\"num_SL_stories_below_HS\",
					\"num_SL_stories_above_HS\",
					\"dist_SL_story_to_normal_story\",
					\"second_door_pos\",
					\"bShowElevatorShaft\",
					\"elev_shaft_thick\",
					\"elev_shaft_ext_mat\",
					\"elev_shaft_int_mat\",
					\"elev_shaft_edge_mat\",
					\"bShowMechin2D\",
					
					\"shaft_inner_width\",
					\"shaft_inner_depth\"

	if not(bShowMullion) then
		lock \"car_mullion_mat\"
		hideparameter \"car_mullion_mat\"
	endif

	if not(bShowElevatorWall) then
		lock 	\"elev_door_style\",
				\"elev_wall_thick\",
				\"elev_wall_overhang\",
				\"elev_wall_ext_mat\",
				\"elev_wall_int_mat\",
				\"elev_wall_edge_mat\",

				\"bTopSlab\",
				\"top_slab_thick\",
				\"top_slab_ext_mat\",
				\"top_slab_int_mat\",
				\"top_slab_edge_mat\",
				\"bPitSlab\",
				\"pit_slab_thick\",
				\"pit_slab_ext_mat\",
				\"pit_slab_int_mat\",
				\"pit_slab_edge_mat\",

				\"gs_wall_cont_pen\",
				\"gs_wall_fill_type\",
				\"gs_wall_fill_pen\",
				\"gs_wall_back_pen\",
				\"gs_opening_fill_type\",
				\"gs_opening_fill_pen\",
				\"gs_opening_back_pen\"

		hideparameter	\"elev_door_style\",
						\"elev_wall_thick\",
						\"elev_wall_overhang\",
						\"elev_wall_ext_mat\",
						\"elev_wall_int_mat\",
						\"elev_wall_edge_mat\",

						\"bTopSlab\",
						\"top_slab_thick\",
						\"top_slab_ext_mat\",
						\"top_slab_int_mat\",
						\"top_slab_edge_mat\",
						\"bPitSlab\",
						\"pit_slab_thick\",
						\"pit_slab_ext_mat\",
						\"pit_slab_int_mat\",
						\"pit_slab_edge_mat\",

						\"gs_wall_cont_pen\",
						\"gs_wall_fill_type\",
						\"gs_wall_fill_pen\",
						\"gs_wall_back_pen\",
						\"gs_opening_fill_type\",
						\"gs_opening_fill_pen\",
						\"gs_opening_back_pen\"
	endif
endif

if	not(elevator_form_m = FORM_RECT			& bShowElevatorShaft |\
		elevator_form_m = FORM_SEGMENTED	& bShowElevatorWall) then

	lock \"opening_frame_mat\", \"bControlPanel\", \"bOpeningFrame\",
		\"edited_story\", \"edited_story_m\",
		\"edited_story_height\",
		\"edited_story_door_pos\", \"edited_story_door_pos_m\",
		\"cpanel_mat\", \"button_mat\",
		\"num_stories_above_HS\", \"gs_max_StrFl_height\",
		\"bSplitLevelStories\", \"stories\"
	hideparameter \"opening_frame_mat\",  \"bControlPanel\", \"bOpeningFrame\",
		\"edited_story\", \"edited_story_m\",
		\"edited_story_height\",
		\"edited_story_door_pos\", \"edited_story_door_pos_m\",
		\"cpanel_mat\", \"button_mat\",
		\"num_stories_above_HS\", \"gs_max_StrFl_height\",
		\"bSplitLevelStories\", \"stories\"
endif

if elevator_form_m = FORM_SEGMENTED then 			! --- Segmented
	lock \"gs_gap_fill_type\"
	hideparameter \"gs_gap_fill_type\"
	lock \"gs_gap_fill_pen\"
	hideparameter \"gs_gap_fill_pen\"
	lock \"gs_gap_back_pen\"
	hideparameter \"gs_gap_back_pen\"
endif


! =============================================================================
! Sizes
! =============================================================================
values \"elev_shaft_thick\"	range (0, )
values \"elev_wall_thick\"	range (0, )

values \"car_height\"	range (0, )

if elevator_form_m = FORM_RECT then
	if cweight_pos_m = CW_POS_NORMAL then
		values \"cweight_width\" range (0, shaft_inner_width - 2*cweightEPS]
		values \"cweight_depth\" range [minCwDepth, max(minCwDepth, shaft_inner_depth - 2*carWallThk - car_inner_depth - frontGap - 2*carEPS)]
	else
		values \"cweight_width\" range (0, shaft_inner_depth - 2*cweightEPS]
		values \"cweight_depth\" range [minCwDepth, max( minCwDepth, shaft_inner_width - 2*carWallThk - car_inner_width - \
				(2+(dir_second_opening_m = SECOND_OPENING_NONE | dir_second_opening_m = SECOND_OPENING_REAR))*carEPS - \
				(dir_second_opening_m = SECOND_OPENING_SIDE1 | dir_second_opening_m = SECOND_OPENING_SIDE2)*frontGap)]
	endif

	if dir_second_opening_m = SECOND_OPENING_SIDE1 then
		minSide1	= frontGap
	else
		minSide1	= (1+(cweight_pos_m = CW_POS_SIDE1) * (elevator_type_m = 1))*carEPS + minCwDepth*(cweight_pos_m = CW_POS_SIDE1) * (elevator_type_m = 1)
	endif

	if dir_second_opening_m = SECOND_OPENING_SIDE2 then
		minSide2	= frontGap
	else
		minSide2	= (1+(cweight_pos_m = CW_POS_SIDE2) * (elevator_type_m = 1))*carEPS + minCwDepth*(cweight_pos_m = CW_POS_SIDE2) * (elevator_type_m = 1)
	endif

	minFront = frontGap

	if dir_second_opening_m = SECOND_OPENING_REAR then
		minRear			= frontGap
		car_inner_depth	= shaft_inner_depth - 2*carWallThk - minRear - minFront
		parameters car_inner_depth = car_inner_depth
	else
		minRear		= (1+(cweight_pos_m = CW_POS_NORMAL) * (elevator_type_m = 1))*carEPS + minCwDepth*(cweight_pos_m = CW_POS_NORMAL) * (elevator_type_m = 1)
	endif

	values \"shaft_inner_width\"	range [minSide1 + minSide2 + minCarWidth + 2*carWallThk, ]
	values \"shaft_inner_depth\"	range [minFront + minRear  + minCarDepth + 2*carWallThk, ]
	values \"A\"					range [minSide1 + minSide2 + minCarWidth + 2*carWallThk + 2*elev_shaft_thick*bShowElevatorShaft, ]
	values \"B\"					range [minFront + minRear  + minCarDepth + 2*carWallThk + 2*elev_shaft_thick*bShowElevatorShaft, ]
	values \"car_inner_width\"	range [minCarWidth, shaft_inner_width - 2*carWallThk - minSide1 - minSide2]
	values \"car_inner_depth\"	range [minCarDepth, shaft_inner_depth - 2*carWallThk - minFront - minRear ]

	if GLOB_MODPAR_NAME = \"A\" or GLOB_MODPAR_NAME = \"B\" then
		shaft_inner_width = A - 2*elev_shaft_thick*bShowElevatorShaft
		shaft_inner_depth = B - 2*elev_shaft_thick*bShowElevatorShaft

		parameters	shaft_inner_width	= shaft_inner_width,
					shaft_inner_depth	= shaft_inner_depth
	else
			A = shaft_inner_width + 2*elev_shaft_thick*bShowElevatorShaft
			B = shaft_inner_depth + 2*elev_shaft_thick*bShowElevatorShaft
			parameters	A			= A,
						B			= B
	endif

	hideparameter	\"car_width\", \"car_depth\"
	lock			\"car_width\", \"car_depth\"

else

	hideparameter	\"car_inner_width\", \"car_inner_depth\"
	lock			\"car_inner_width\", \"car_inner_depth\"

	values \"car_width\"	range [minCarWidth, )
	values \"car_depth\"	range [minCarDepth, )
	values \"A\"			range [minCarWidth + 2*elev_wall_overhang*bShowElevatorWall, )
	values \"B\"			range [minCarDepth + segmentedFront + bShowElevatorWall * elev_wall_thick, )
	values \"elev_wall_overhang\" range [0,]

	if GLOB_MODPAR_NAME = \"A\" or GLOB_MODPAR_NAME = \"B\" then
		car_width = A - 2*elev_wall_overhang*bShowElevatorWall
		car_depth = B - segmentedFront - bShowElevatorWall * elev_wall_thick
		parameters car_width = car_width, car_depth = car_depth
	else
		A = car_width + 2*elev_wall_overhang*bShowElevatorWall
		B = car_depth + segmentedFront + bShowElevatorWall * elev_wall_thick
		parameters A = A, B = B
	endif
endif


! =============================================================================
! Doors
! =============================================================================
doorDivider	= 0.5
if door_style_m = 0 or door_style_m = 3 then
	nSlidingDiv 	= 1
else
	if door_style_m = 1 | door_style_m = 5 then
		nSlidingDiv	= 1.5
		doorDivider	= 1/3
	else
		nSlidingDiv = 2
	endif
endif

if dir_second_opening_m = 3 or dir_second_opening_m = 4 then
	!!values \"door_width\" range(0, (car_depth-2*(carWallThk+minCarWallFlap)-abs(second_door_pos))/nSlidingDiv]
	values \"door_width\" range(0, (car_depth-2*(carWallThk+minCarWallFlap))/nSlidingDiv]
else
	!!values \"door_width\" range(0, (car_width-2*(carWallThk+minCarWallFlap)-abs(door_pos))/nSlidingDiv]
	values \"door_width\" range(0, (car_width-2*(carWallThk+minCarWallFlap))/nSlidingDiv]
endif
values \"door_height\" range(0, )
values \"door_pos\" range[-(car_width/2-carWallThk-door_width*nSlidingDiv*doorDivider-minCarWallFlap), car_width/2-carWallThk-door_width*nSlidingDiv*doorDivider-minCarWallFlap]
if dir_second_opening_m = 3 or dir_second_opening_m = 4 then
	values \"second_door_pos\" range[-(car_depth/2-carWallThk-door_width*nSlidingDiv*doorDivider-minCarWallFlap), car_depth/2-carWallThk-door_width*nSlidingDiv*doorDivider-minCarWallFlap]
else
	values \"second_door_pos\" range[-(car_width/2-carWallThk-door_width*nSlidingDiv*doorDivider-minCarWallFlap), car_width/2-carWallThk-door_width*nSlidingDiv*doorDivider-minCarWallFlap]
endif
values \"gs_max_StrFl_height\" range(0, )
values \"opening_in3D\" range [0, 100]

if not(bControlPanel) then
	lock 	\"cpanel_mat\",
			\"button_mat\"

	hideparameter	\"cpanel_mat\",
					\"button_mat\"
endif


! =============================================================================
! Levels
! =============================================================================
ac_bottomlevel = connected_stories_elev[num_connected_stories]-pit_depth*bPith-pit_slab_thick*bPitSlab
parameters ac_bottomlevel = ac_bottomlevel
ac_toplevel = connected_stories_elev[1] + connected_stories_height[1]+penthouse_height*bPenthouse+top_slab_thick*bTopSlab
parameters ac_toplevel = ac_toplevel
ZZYZX = ac_toplevel - ac_bottomlevel
parameters ZZYZX = ZZYZX
lock \"ZZYZX\"

! =============================================================================
! Onorm list Settings
! =============================================================================
if LibraryLangCode = \"AUT\" or LibraryLangCode = \"CHE\" or LibraryLangCode = \"GER\" then
	rrr = REQUEST (\"Name_of_macro\", \"\", LPName)
	call \"Onorm_Elevator\" parameters all sLibpartName = strsub(LPName,0,strlen(LPName)-4)
else
	hideparameter \"gs_onorm_Title\"
endif


! =============================================================================
! IFC Parameters
! =============================================================================

ifc_CapacityByWeight	= FM_LiftCapacityNumber
ifc_CapacityByNumber	= FM_TransportablePersons
parameters	ifc_CapacityByWeight	= ifc_CapacityByWeight,
			ifc_CapacityByNumber	= ifc_CapacityByNumber

FM_LiftCapacity = str(\"%~m\", FM_LiftCapacityNumber)
parameters FM_LiftCapacity = FM_LiftCapacity

parameters ifc_optype = 1

values \"FM_LiftCapacityNumber\" range[0, )

! =============================================================================
! UI
! =============================================================================
values \"gs_ui_current_page\" 1,2,3,4,5,6,7,8

")

(define master-code-elevator
"EPS = 0.0001

stStory			= `Story`
stSplitLevel	= `Split-Level`
stSplitStory	= `Split-Level Story`

dim stDoorPosition[4]
	stDoorPosition[1] = `Front`
	stDoorPosition[2] = `Second Opening`
	stDoorPosition[3] = `Both`
	stDoorPosition[4] = `None`

dim stDetail[3]
	stDetail[1] = `Detailed`
	stDetail[2] = `Simple`
	stDetail[3] = `Off`

dim stDetail2D[]
	stDetail2D[1] = `Scale Sensitive`
	stDetail2D[2] = `1:50`
	stDetail2D[3] = `1:100`
	stDetail2D[4] = `1:200`
	stDetail2D[5] = `Symbolic 1`
	stDetail2D[6] = `Symbolic 2`

dim stElevType[2]
	stElevType[1] = `Mechanical`
	stElevType[2] = `Hydraulic`

dim stDirection[4]
	stDirection[1] = `None`
	stDirection[2] = `Rear`
	stDirection[3] = `Side 1`
	stDirection[4] = `Side 2`

! dir_second_opening_m
	SECOND_OPENING_NONE		= 1
	SECOND_OPENING_REAR		= 2
	SECOND_OPENING_SIDE1	= 3
	SECOND_OPENING_SIDE2	= 4

dim stPosition[3]
	stPosition[1] = `Normal`
	stPosition[2] = `Side 1`
	stPosition[3] = `Side 2`

! cweight_pos_m
	CW_POS_NORMAL	= 1
	CW_POS_SIDE1	= 2
	CW_POS_SIDE2	= 3
	
dim stElevDoorStyle[2]
	stElevDoorStyle[1] = `Sliding`
	stElevDoorStyle[2] = `Opening`

dim stDoorStyle[6]
	stDoorStyle[1] = `None`
	stDoorStyle[2] = `Style 1`
	stDoorStyle[3] = `Style 2`
	stDoorStyle[4] = `Style 3`
	stDoorStyle[5] = `Style 4`
	stDoorStyle[6] = `Style 5`

dim stElevForm[2]
	stElevForm[1] = `Rectangular`
	stElevForm[2] = `Segmented`

! elevator_form_m
	FORM_RECT		= 1
	FORM_SEGMENTED	= 2

carWallThk		= 0.08
carEPS			= 0.001
cweightEPS		= 0.001
gap_width		= 0.012
minCarWidth		= 0.2
minCarDepth		= 0.2
minCwDepth		= 0.02
minCarWallFlap	= 0.02
frontGap		= 0.04
segmentedFront	= 0.05
openingFromSide	= 0.04
opening_width	= door_width + 0.36
opening_height	= door_height + 0.18

dim doorThickness[5]
	doorThickness[1] = 0.02
	doorThickness[2] = 0.03
	doorThickness[3] = 0.03
	doorThickness[4] = 0.03
	doorThickness[5] = 0.04

dim doorOffset[5]
	doorOffset[1] = 0
	doorOffset[2] = 0.02
	doorOffset[3] = 0.02
	doorOffset[4] = 0.02
	doorOffset[5] = 0.02

elevCarOffset	= 0
elevCarDoorThk	= 0
elevCarGlassThk	= 0.01
if door_style_m > 0 then
	elevCarDoorThk	= doorThickness[door_style_m]
	elevCarOffset	= doorOffset[door_style_m]
endif


if elevator_form_m = FORM_SEGMENTED then bSplitLevelStories = 0

if GLOB_SCRIPT_TYPE = 7 | GLOB_SCRIPT_TYPE = 8 then goto \"endMasterScript\"

! ==============================================================================
! Story
! ==============================================================================
home_story_index = 0
storyInfo = REQUEST (\"Home_story\", \"\", home_story_index, home_story_name)

if storyInfo then
	num_above_story = 0
	num_below_story = 0

	dim t[]
	n = REQUEST (\"STORY_INFO\", \"\", numStories, t)

	parameters home_story_index = home_story_index

	for i = 1 to numStories
		story_index = t [4 * (i - 1) + 1]
		if story_index > home_story_index then
			num_above_story = num_above_story + 1
			story_name_above[num_above_story] = t[4 * (i - 1) + 2]
			story_elev_above[num_above_story] = t[4 * (i - 1) + 3]
			story_height_above[num_above_story] = t[4 * (i - 1) + 4]
		endif
		if story_index = home_story_index then
			home_story_name = t[4 * (i - 1) + 2]
			home_story_elev = t[4 * (i - 1) + 3]
			home_story_height = t[4 * (i - 1) + 4]
		endif
		if story_index < home_story_index then
			num_below_story = num_below_story + 1
			story_name_below[num_below_story] = t[4 * (i - 1) + 2]
			story_elev_below[num_below_story] = t[4 * (i - 1) + 3]
			story_height_below[num_below_story] = t[4 * (i - 1) + 4]
		endif
	next i
	parameters num_above_story = num_above_story
	parameters story_name_above = story_name_above
	parameters story_elev_above = story_elev_above
	parameters story_height_above = story_height_above

	parameters home_story_name = home_story_name
	parameters home_story_elev = home_story_elev
	parameters home_story_height = home_story_height

	parameters num_below_story = num_below_story
	parameters story_name_below = story_name_below
	parameters story_elev_below = story_elev_below
	parameters story_height_below = story_height_below
endif


dim connected_stories_name[]
dim connected_stories_elev[]
dim connected_stories_height[]
dim connected_stories_id[]
dim available_connected_stories_boolean[]
dim available_connected_stories_name[]
dim available_connected_stories_id[]

ii = 0
if num_above_story < num_stories_above_HS then
	num_stories_above_HS = num_above_story
	parameters num_stories_above_HS = num_stories_above_HS

	if num_stories_above_HS < num_SL_stories_above_HS then
		num_SL_stories_above_HS = num_stories_above_HS
		parameters num_SL_stories_above_HS = num_SL_stories_above_HS
	endif
endif
if num_above_story then
	values \"num_stories_above_HS\" range[0, num_above_story]
	values \"num_SL_stories_above_HS\" range[0, num_stories_above_HS]

	if num_stories_above_HS then
		for i = num_stories_above_HS to 1 step -1
			ii = ii + 1
			if story_name_above[i] = \"\" then
				storyName = stStory
			else
				storyName = story_name_above[i]
			endif
			connected_stories_name[ii] = str(i+home_story_index,1,0)+\". \"+storyName
			connected_stories_elev[ii] = story_elev_above[i]-home_story_elev
			connected_stories_height[ii] = story_height_above[i]

			if bSplitLevelStories then
				connected_stories_id[ii] = i*2+home_story_index*2
				available_connected_stories_boolean[ii] = 1

				ii = ii + 1
				if story_name_above[i] = \"\" then
					storyName = stSplitStory
				else
					storyName = stSplitLevel+\" \"+story_name_above[i]
				endif
				connected_stories_name[ii] = str(i+home_story_index-1,1,0)+\". \"+storyName
				if i > 1 then
					connected_stories_elev[ii] = story_elev_above[i-1]-home_story_elev+dist_SL_story_to_normal_story
					connected_stories_height[ii] = story_height_above[i-1]
				else
					connected_stories_elev[ii] = dist_SL_story_to_normal_story
					connected_stories_height[ii] = home_story_height
				endif
				connected_stories_id[ii] = i*2+home_story_index*2-1
				if num_SL_stories_above_HS+eps > i then
					available_connected_stories_boolean[ii] = 1
				else
					available_connected_stories_boolean[ii] = 0
				endif
			else
				connected_stories_id[ii] = i+home_story_index
			endif
		next i
	else
		lock \"num_SL_stories_above_HS\"
	endif
else
	lock \"num_stories_above_HS\"
endif

ii = ii + 1
if home_story_name = \"\" then home_story_name = stStory
connected_stories_name[ii] = str(home_story_index,1,0)+\". \"+home_story_name
connected_stories_elev[ii] = 0
connected_stories_height[ii] = home_story_height
connected_stories_id[ii] = home_story_index*(1+bSplitLevelStories)
available_connected_stories_boolean[ii] = 1

if num_below_story < num_stories_below_HS then
	num_stories_below_HS = num_below_story
	parameters num_stories_below_HS = num_stories_below_HS

	if num_stories_below_HS < num_SL_stories_below_HS then
		num_SL_stories_below_HS = num_stories_below_HS
		parameters num_SL_stories_below_HS = num_SL_stories_below_HS
	endif
endif
if num_below_story then
	values \"num_stories_below_HS\" range[0, num_below_story]
	values \"num_SL_stories_below_HS\" range[0, num_stories_below_HS]

	if num_stories_below_HS then
		for i = 1 to num_stories_below_HS
			ii = ii + 1
			j = num_below_story+1-i
			if bSplitLevelStories then
				if story_name_below[i] = \"\" then
					storyName = stSplitStory
				else
					storyName = stSplitLevel+\" \"+story_name_below[i]
				endif
				connected_stories_name[ii] = str(-i+home_story_index,1,0)+\". \"+storyName
				connected_stories_elev[ii] = story_elev_below[j]-home_story_elev+dist_SL_story_to_normal_story
				connected_stories_height[ii] = story_height_below[j]
				connected_stories_id[ii] = -i*2+home_story_index*2+1
				if num_SL_stories_below_HS+eps > i then
					available_connected_stories_boolean[ii] = 1
				else
					available_connected_stories_boolean[ii] = 0
				endif

				ii = ii + 1
				connected_stories_id[ii] = -i*2+home_story_index*2
				available_connected_stories_boolean[ii] = 1
			else
				connected_stories_id[ii] = -i+home_story_index
			endif

			if story_name_below[i] = \"\" then
				storyName = stStory
			else
				storyName = story_name_above[i]
			endif
			connected_stories_name[ii] = str(-i+home_story_index,1,0)+\". \"+storyName
			connected_stories_elev[ii] = story_elev_below[j]-home_story_elev
			connected_stories_height[ii] = story_height_below[j]
		next i
	else
		lock \"num_SL_stories_below_HS\"
	endif
else
	lock \"num_stories_below_HS\"
endif
num_connected_stories = ii

bLockedDoorEditedStory = 0
if bSplitLevelStories then
	ii = 0
	for i = 1 to num_connected_stories
		if available_connected_stories_boolean[i] = 1 then
			ii = ii + 1
			available_connected_stories_name[ii] = connected_stories_name[i]
			available_connected_stories_id[ii] = connected_stories_id[i]
		endif
	next i
	num_available_connected_stories = ii

	values \"edited_story\" available_connected_stories_name
	values \"edited_story_m\" available_connected_stories_id

	if GLOB_MODPAR_NAME = \"edited_story\" then
		for i = 1 to num_connected_stories
			if edited_story = connected_stories_name[i] then
				edited_story_m = -i+1+num_stories_above_HS*2+home_story_index*2
				i = num_connected_stories
			endif
		next i
		parameters edited_story_m = edited_story_m
	else
		if edited_story_m < home_story_index*2 - num_stories_below_HS*2 then edited_story_m = home_story_index*2 - num_stories_below_HS*2
		if edited_story_m > home_story_index*2 + num_stories_above_HS*2 then edited_story_m = home_story_index*2 + num_stories_above_HS*2
		edited_story = connected_stories_name[-edited_story_m+1+num_stories_above_HS*2+home_story_index*2]
		parameters edited_story = edited_story
	endif
	editedStoryID = -edited_story_m+1+num_stories_above_HS*2+home_story_index*2
	parameters edited_story_height	= connected_stories_height[editedStoryID]
	parameters edited_story_elev  	= connected_stories_elev[editedStoryID]


	values \"car_pos_story\" available_connected_stories_name
	values \"car_pos_story_m\" available_connected_stories_id

	if GLOB_MODPAR_NAME = \"car_pos_story\" then
		for i = 1 to num_connected_stories
			if car_pos_story = connected_stories_name[i] then
				car_pos_story_m = -i+1+num_stories_above_HS*2+home_story_index*2
				i = num_connected_stories
			endif
		next i
		parameters car_pos_story_m = car_pos_story_m
	else
		if car_pos_story_m < home_story_index*2 - num_stories_below_HS*2 then car_pos_story_m = home_story_index*2 - num_stories_below_HS*2
		if car_pos_story_m > home_story_index*2 + num_stories_above_HS*2 then car_pos_story_m = home_story_index*2 + num_stories_above_HS*2
		car_pos_story = connected_stories_name[-car_pos_story_m+1+num_stories_above_HS*2+home_story_index*2]
		parameters car_pos_story = car_pos_story
	endif


	if abs(INT(connected_stories_id[editedStoryID]/2)-(connected_stories_id[editedStoryID]/2)) < eps then
		if edited_story_door_pos_m = 2 | edited_story_door_pos_m = 3 then
			edited_story_door_pos_m = 1
			parameters edited_story_door_pos_m = edited_story_door_pos_m
		endif
		values \"edited_story_door_pos\" stDoorPosition[1], stDoorPosition[4]
		values \"edited_story_door_pos_m\" 1, 4
	else
		if edited_story_door_pos_m = 1 | edited_story_door_pos_m = 3 then
			edited_story_door_pos_m = 2
			parameters edited_story_door_pos_m = edited_story_door_pos_m
		endif
		if dir_second_opening_m = SECOND_OPENING_NONE then
			bLockedDoorEditedStory	= 1
			edited_story_door_pos	= stDoorPosition[4]
			edited_story_door_pos_m	= 4
			lock \"edited_story_door_pos\", \"edited_story_door_pos_m\"
		endif
		stDoorPosition[2] = dir_second_opening
		values \"edited_story_door_pos\" stDoorPosition[2], stDoorPosition[4]
		values \"edited_story_door_pos_m\" 2, 4
	endif

	if connected_stories_id[1] > -eps then
		for i = 1 to connected_stories_id[1]+1
			if SL_above_story_door_pos[i] = 0 then
				if abs(INT(i/2)-(i/2)) < eps then
					SL_above_story_door_pos[i] = 2
				else
					SL_above_story_door_pos[i] = 1
				endif
				parameters SL_above_story_door_pos[i] = SL_above_story_door_pos[i]
			endif
		next i

		if dir_second_opening_m = SECOND_OPENING_NONE then
			for i = 1 to connected_stories_id[1]+1
				if abs(INT(i/2)-(i/2)) < eps then
					SL_above_story_door_pos[i] = 4
				endif	
			next i
		endif
	endif

	if connected_stories_id[num_connected_stories] < 0 then	
		for i = 1 to abs(connected_stories_id[num_connected_stories])
			if SL_below_story_door_pos[i] = 0 then
				if abs(INT(i/2)-(i/2)) < eps then
					SL_below_story_door_pos[i] = 1
				else
					SL_below_story_door_pos[i] = 2
				endif
				parameters SL_below_story_door_pos[i] = SL_below_story_door_pos[i]
			endif
		next i

		if dir_second_opening_m = SECOND_OPENING_NONE then
			for i = 1 to abs(connected_stories_id[num_connected_stories])
				if abs(INT(i/2)-(i/2)) > eps then
					SL_below_story_door_pos[i] = 4
				endif
			next i
		endif
	endif

	if GLOB_MODPAR_NAME = \"edited_story\" or GLOB_MODPAR_NAME = \"bSplitLevelStories\" then
		if edited_story_m < 0 then
			edited_story_door_pos_m = SL_below_story_door_pos[abs(edited_story_m)]
		else
			edited_story_door_pos_m = SL_above_story_door_pos[edited_story_m+1]
		endif
		parameters edited_story_door_pos_m = edited_story_door_pos_m

		edited_story_door_pos = stDoorPosition[edited_story_door_pos_m]
		parameters edited_story_door_pos = edited_story_door_pos
	else
		if GLOB_MODPAR_NAME = \"edited_story_door_pos\" then
			for i = 1 to 4
				if edited_story_door_pos = stDoorPosition[i] then
					edited_story_door_pos_m = i
					i = 4
				endif
			next i
			parameters edited_story_door_pos_m = edited_story_door_pos_m
		else
			edited_story_door_pos = stDoorPosition[edited_story_door_pos_m]
			parameters edited_story_door_pos = edited_story_door_pos
		endif

		if edited_story_m < 0 then
			SL_below_story_door_pos[abs(edited_story_m)] = edited_story_door_pos_m
			parameters SL_below_story_door_pos[abs(edited_story_m)] = SL_below_story_door_pos[abs(edited_story_m)]
		else
			SL_above_story_door_pos[edited_story_m+1] = edited_story_door_pos_m
			parameters SL_above_story_door_pos[edited_story_m+1] = SL_above_story_door_pos[edited_story_m+1]
		endif
	endif
else
	values \"edited_story\" connected_stories_name
	values \"edited_story_m\" connected_stories_id

	if GLOB_MODPAR_NAME = \"edited_story\" then
		for i = 1 to num_connected_stories
			if edited_story = connected_stories_name[i] then
				edited_story_m = -i+1+num_stories_above_HS+home_story_index
				i = num_connected_stories
			endif
		next i
		parameters edited_story_m = edited_story_m
	else
		if edited_story_m < home_story_index - num_stories_below_HS then edited_story_m = home_story_index - num_stories_below_HS
		if edited_story_m > home_story_index + num_stories_above_HS then edited_story_m = home_story_index + num_stories_above_HS
		edited_story = connected_stories_name[-edited_story_m+1+num_stories_above_HS+home_story_index]
		parameters edited_story = edited_story
	endif
	editedStoryID = -edited_story_m+1+num_stories_above_HS+ home_story_index
	parameters edited_story_height = connected_stories_height[editedStoryID]
	parameters edited_story_elev  = connected_stories_elev[editedStoryID]


	values \"car_pos_story\" connected_stories_name
	values \"car_pos_story_m\" connected_stories_id

	if GLOB_MODPAR_NAME = \"car_pos_story\" then
		for i = 1 to num_connected_stories
			if car_pos_story = connected_stories_name[i] then
				car_pos_story_m = -i+1+num_stories_above_HS+home_story_index
				i = num_connected_stories
			endif
		next i
		parameters car_pos_story_m = car_pos_story_m
	else
		if car_pos_story_m < home_story_index - num_stories_below_HS then car_pos_story_m = home_story_index - num_stories_below_HS
		if car_pos_story_m > home_story_index + num_stories_above_HS then car_pos_story_m = home_story_index + num_stories_above_HS
		car_pos_story = connected_stories_name[-car_pos_story_m+1+num_stories_above_HS+home_story_index]
		parameters car_pos_story = car_pos_story
	endif


	if elevator_form_m = FORM_RECT then
		if dir_second_opening_m = 1 then
			if edited_story_door_pos_m = 2 | edited_story_door_pos_m = 3 then
				edited_story_door_pos_m = 1
				parameters edited_story_door_pos_m = edited_story_door_pos_m
			endif
			values \"edited_story_door_pos\" stDoorPosition[1], stDoorPosition[4]
			values \"edited_story_door_pos_m\" 1, 4
		else
			values \"edited_story_door_pos\" stDoorPosition
			values \"edited_story_door_pos_m\" 1, 2, 3, 4
		endif

		if connected_stories_id[1] > -eps then
			for i = 1 to connected_stories_id[1]+1
				if above_story_door_pos[i] = 0 then
					above_story_door_pos[i] = 1
					parameters above_story_door_pos[i] = above_story_door_pos[i]
				endif
			next i
		endif

		if connected_stories_id[num_connected_stories] < 0 then
			for i = 1 to abs(connected_stories_id[num_connected_stories])
				if below_story_door_pos[i] = 0 then
					below_story_door_pos[i] = 1
					parameters below_story_door_pos[i] = below_story_door_pos[i]
				endif
			next i
		endif

		if GLOB_MODPAR_NAME = \"edited_story\" or GLOB_MODPAR_NAME = \"bSplitLevelStories\" or GLOB_MODPAR_NAME = \"elevator_form\" then
			if edited_story_m < 0 then
				edited_story_door_pos_m = below_story_door_pos[abs(edited_story_m)]
			else
				edited_story_door_pos_m = above_story_door_pos[edited_story_m+1]
			endif
			parameters edited_story_door_pos_m = edited_story_door_pos_m

			edited_story_door_pos = stDoorPosition[edited_story_door_pos_m]
			parameters edited_story_door_pos = edited_story_door_pos
		else
			if GLOB_MODPAR_NAME = \"edited_story_door_pos\" then
				for i = 1 to 4
					if edited_story_door_pos = stDoorPosition[i] then
						edited_story_door_pos_m = i
						i = 4
					endif
				next i
				parameters edited_story_door_pos_m = edited_story_door_pos_m
			else
				edited_story_door_pos = stDoorPosition[edited_story_door_pos_m]
				parameters edited_story_door_pos = edited_story_door_pos
			endif

			if edited_story_m < 0 then
				below_story_door_pos[abs(edited_story_m)] = edited_story_door_pos_m
				parameters below_story_door_pos[abs(edited_story_m)] = below_story_door_pos[abs(edited_story_m)]
			else
				above_story_door_pos[edited_story_m+1] = edited_story_door_pos_m
				parameters above_story_door_pos[edited_story_m+1] = above_story_door_pos[edited_story_m+1]
			endif
		endif
	else 									! --- Segmented
		if edited_story_door_pos_m = 2 | edited_story_door_pos_m = 3 then
			edited_story_door_pos_m = 1
			parameters edited_story_door_pos_m = edited_story_door_pos_m
		endif
		values \"edited_story_door_pos\" stDoorPosition[1], stDoorPosition[4]
		values \"edited_story_door_pos_m\" 1, 4

		if connected_stories_id[1] > -eps then
			for i = 1 to connected_stories_id[1]+1
				if S_above_story_door_pos[i] = 0 then
					S_above_story_door_pos[i] = 1
					parameters S_above_story_door_pos[i] = S_above_story_door_pos[i]
				endif
			next i
		endif

		if connected_stories_id[num_connected_stories] < 0 then
			for i = 1 to abs(connected_stories_id[num_connected_stories])
				if S_below_story_door_pos[i] = 0 then
					S_below_story_door_pos[i] = 1
					parameters S_below_story_door_pos[i] = S_below_story_door_pos[i]
				endif
			next i
		endif

		if GLOB_MODPAR_NAME = \"edited_story\" or GLOB_MODPAR_NAME = \"bSplitLevelStories\" or GLOB_MODPAR_NAME = \"elevator_form\" then
			if edited_story_m < 0 then
				edited_story_door_pos_m = S_below_story_door_pos[abs(edited_story_m)]
			else
				edited_story_door_pos_m = S_above_story_door_pos[edited_story_m+1]
			endif
			parameters edited_story_door_pos_m = edited_story_door_pos_m

			edited_story_door_pos = stDoorPosition[edited_story_door_pos_m]
			parameters edited_story_door_pos = edited_story_door_pos
		else
			if GLOB_MODPAR_NAME = \"edited_story_door_pos\" then
				for i = 1 to 4
					if edited_story_door_pos = stDoorPosition[i] then
						edited_story_door_pos_m = i
						i = 4
					endif
				next i
				parameters edited_story_door_pos_m = edited_story_door_pos_m
			else
				edited_story_door_pos = stDoorPosition[edited_story_door_pos_m]
				parameters edited_story_door_pos = edited_story_door_pos
			endif

			if edited_story_m < 0 then
				S_below_story_door_pos[abs(edited_story_m)] = edited_story_door_pos_m
				parameters S_below_story_door_pos[abs(edited_story_m)] = S_below_story_door_pos[abs(edited_story_m)]
			else
				S_above_story_door_pos[edited_story_m+1] = edited_story_door_pos_m
				parameters S_above_story_door_pos[edited_story_m+1] = S_above_story_door_pos[edited_story_m+1]
			endif
		endif
	endif
endif


! ==============================================================================
! Shaft Sizes
! ==============================================================================
if elevator_form_m = FORM_RECT then
	totalShaftWidth = shaft_inner_width + elev_shaft_thick*2
	totalShaftDepth = shaft_inner_depth + elev_shaft_thick*2
	car_width		= car_inner_width + 2*carWallThk
	car_depth		= car_inner_depth + 2*carWallThk
	bMechanic		= (elevator_type_m = 1)

	if dir_second_opening_m = SECOND_OPENING_SIDE1 then
		carSpaceSide1	= frontGap
	else
		carSpaceSide1	= (shaft_inner_width - car_width - frontGap*(dir_second_opening_m = SECOND_OPENING_SIDE2)- cweight_depth*(cweight_pos_m <> CW_POS_NORMAL) * bMechanic) /\\
						  (1+(dir_second_opening_m <> SECOND_OPENING_SIDE2)+(cweight_pos_m <> CW_POS_NORMAL) * bMechanic)
	endif
	carSide1 = carSpaceSide1 *  (1+(cweight_pos_m = CW_POS_SIDE1) * bMechanic) + cweight_depth*(cweight_pos_m = CW_POS_SIDE1) * bMechanic

	if dir_second_opening_m = SECOND_OPENING_SIDE2 then
		carSpaceSide2	= frontGap
	else
		carSpaceSide2	= (shaft_inner_width - car_width - frontGap*(dir_second_opening_m = SECOND_OPENING_SIDE1) - cweight_depth*(cweight_pos_m <> CW_POS_NORMAL) * bMechanic) /\\
						  (1+(dir_second_opening_m <> SECOND_OPENING_SIDE1)+(cweight_pos_m <> CW_POS_NORMAL) * bMechanic)
	endif
	carSide2 = carSpaceSide2 *  (1+(cweight_pos_m = CW_POS_SIDE2) * bMechanic) + cweight_depth*(cweight_pos_m = CW_POS_SIDE2) * bMechanic

	carSpaceFront = frontGap

	if dir_second_opening_m = SECOND_OPENING_REAR then
		carSpaceRear		= frontGap
	else
		carSpaceRear		= (shaft_inner_depth - car_depth - cweight_depth*(cweight_pos_m = CW_POS_NORMAL) * bMechanic - carSpaceFront) / (1+(cweight_pos_m = CW_POS_NORMAL) * bMechanic)
	endif
endif

! ==============================================================================
\"endMasterScript\":
! ==============================================================================

")

(define 3d-code-elevator "if gs_detlevel_3D_m = 0 then end
if gs_shadow = 0 then shadow off
unID			= 1
num_cutend		= 0


penthouse_height	= penthouse_height * bPenthouse
pit_depth			= pit_depth * bPith
top_slab_thick		= top_slab_thick * bTopSlab
pit_slab_thick		= pit_slab_thick * bPitSlab

! ==============================================================================
! Rectangular
! ==============================================================================
if elevator_form_m = FORM_RECT then
	elev_wall_thick = elev_shaft_thick

	! --------------------------------------------------------------------------
	! Elevator Car
	! --------------------------------------------------------------------------

	! --- where are doors in elevator car?
	bFrontDoorPresent = 0
	bSecondaryDoorPresent = 0
	if bSplitLevelStories then
		for i = 1 to num_connected_stories
			storyID = connected_stories_id[i]
			if connected_stories_height[i] > gs_max_StrFl_height and available_connected_stories_boolean[i] then
				if storyID < 0 then
					if SL_below_story_door_pos[abs(storyID)] = 1 then bFrontDoorPresent = 1
					if SL_below_story_door_pos[abs(storyID)] = 2 then bSecondaryDoorPresent = 1
					if SL_below_story_door_pos[abs(storyID)] = 3 then
						bFrontDoorPresent = 1
						bSecondaryDoorPresent = 1
					endif
				else
					if SL_above_story_door_pos[storyID+1] = 1 then bFrontDoorPresent = 1
					if SL_above_story_door_pos[storyID+1] = 2 then bSecondaryDoorPresent = 1
					if SL_above_story_door_pos[storyID+1] = 3 then
						bFrontDoorPresent = 1
						bSecondaryDoorPresent = 1
					endif
				endif
			endif
		next i
	else
		for i = 1 to num_connected_stories
			storyID = connected_stories_id[i]
			if connected_stories_height[i] > gs_max_StrFl_height then
				if storyID < 0 then
					if below_story_door_pos[abs(storyID)] = 1 then bFrontDoorPresent = 1
					if below_story_door_pos[abs(storyID)] = 2 then bSecondaryDoorPresent = 1
					if below_story_door_pos[abs(storyID)] = 3 then
						bFrontDoorPresent = 1
						bSecondaryDoorPresent = 1
					endif
				else
					if above_story_door_pos[storyID+1] = 1 then bFrontDoorPresent = 1
					if above_story_door_pos[storyID+1] = 2 then bSecondaryDoorPresent = 1
					if above_story_door_pos[storyID+1] = 3 then
						bFrontDoorPresent = 1
						bSecondaryDoorPresent = 1
					endif
				endif
			endif
		next i
	endif

	if bSplitLevelStories then
		carPosStoryID = -car_pos_story_m+1+num_stories_above_HS*2+home_story_index*2
	else
		carPosStoryID = -car_pos_story_m+1+num_stories_above_HS+home_story_index
	endif

	pen gs_cont_pen

	add elev_shaft_thick+carSide1, elev_shaft_thick+carSpaceFront, connected_stories_elev[carPosStoryID]

	hotspot 0,			0,			-0.05, unID: unID = unID + 1
	hotspot car_width,	0,			-0.05, unID: unID = unID + 1
	hotspot car_width,	car_depth,	-0.05, unID: unID = unID + 1
	hotspot 0,			car_depth,	-0.05, unID: unID = unID + 1

	hotspot 0,			0,			car_height+0.05, unID: unID = unID + 1
	hotspot car_width,	0,			car_height+0.05, unID: unID = unID + 1
	hotspot car_width,	car_depth,	car_height+0.05, unID: unID = unID + 1
	hotspot 0,			car_depth,	car_height+0.05, unID: unID = unID + 1

	if gs_detlevel_3D_m = 2 then				! --- Detailed
																								! --- Front door
		addx car_width/2+door_pos+carSide1
		rotx 90
		addx -carSide1
		if door_pos < 0 then
			RightDoor = 1
		else
			RightDoor = 0
		endif

		gosub \"elev car door\"
		del 3

		if second_door_pos<0 then
			RightDoor = 1
		else
			RightDoor = 0
		endif

		if dir_second_opening_m = SECOND_OPENING_REAR then			! --- Second door on Back
			add car_width/2+second_door_pos, car_depth, 0
			rotz 180
			rotx 90
			gosub \"elev car door\"
			del 3
		endif
		if dir_second_opening_m = SECOND_OPENING_SIDE1 then		! --- Second door on Side 1
			add 0, car_depth/2+second_door_pos, 0
			rotz -90
			rotx 90
			gosub \"elev car door\"
			del 3
		endif
		if dir_second_opening_m = SECOND_OPENING_SIDE2 then		! --- Second door on Side 2
			add car_width, car_depth/2+second_door_pos, 0
			rotz 90
			rotx 90
			gosub \"elev car door\"
			del 3
		endif

		material car_int_mat
		cutpolya 5, 2, 0,
			carWallThk, carWallThk, 15,
			car_width-carWallThk, carWallThk, 15,
			car_width-carWallThk, car_depth-carWallThk, 15,
			carWallThk, car_depth-carWallThk, 15,
			carWallThk, carWallThk, -1

		material car_ext_mat
		prism_  5, car_height,
				0, 0, 15,
				car_width, 0, 15,
				car_width, car_depth, 15,
				0, car_depth, 15,
				0, 0, -1
		cutend

		addz -0.05
		cprism_ car_pave_mat, car_ext_mat, car_ext_mat,
				5, 0.05,
				0, 0, 15,
				car_width, 0, 15,
				car_width, car_depth, 15,
				0, car_depth, 15,
				0, 0, -1
		del 1

		addz car_height
		cprism_ car_ext_mat, car_ceiling_mat, car_ext_mat,
				5, 0.05,
				0, 0, 15,
				car_width, 0, 15,
				car_width, car_depth, 15,
				0, car_depth, 15,
				0, 0, -1
		del 1

		for j = 1 to num_cutend
			cutend
		next j
		num_cutend=0

		if bControlPanel then
			if dir_second_opening_m = SECOND_OPENING_SIDE2 then	! Side 2
				add carWallThk, carWallThk+0.10, 1
				rotz 90
			else
				add car_width-carWallThk, carWallThk+0.20, 1
				rotz -90
			endif
			gosub \"inside control panel\"
			del 2
		endif
	else
		material car_ext_mat
		addz -0.05
		prism_  5, car_height+0.10,
				0, 0, 15,
				car_width, 0, 15,
				car_width, car_depth, 15,
				0, car_depth, 15,
				0, 0, -1
		del 1
	endif
	del 1


	! --------------------------------------------------------------------------
	! Elevator Shaft
	! --------------------------------------------------------------------------

	if bShowElevatorShaft then
		if bSplitLevelStories then
			for i = 1 to num_connected_stories						! --- Openings/Doors
				addz connected_stories_elev[i]
				storyID = connected_stories_id[i]
				if connected_stories_height[i] > gs_max_StrFl_height and available_connected_stories_boolean[i] then
					if storyID < 0 then
						elevShaftDoorPosID = SL_below_story_door_pos[abs(storyID)]
					else
						elevShaftDoorPosID = SL_above_story_door_pos[storyID+1]
					endif
				else
					elevShaftDoorPosID = 4
				endif

				opening_in3D_in_car_pos = 0
				if car_pos_story_m = storyID then opening_in3D_in_car_pos = opening_in3D
				gosub \"openings to shaft\"
				del 1
			next i

			pen gs_wall_cont_pen
			material elev_shaft_int_mat
			add elev_shaft_thick, elev_shaft_thick, 0
			cutpolya 5, 2, 0,
				0, 0, 15,
				shaft_inner_width, 0, 15,
				shaft_inner_width, shaft_inner_depth, 15,
				0, shaft_inner_depth, 15,
				0, 0, -1
			del 1
			num_cutend = num_cutend+1

			for i = 1 to num_connected_stories						! --- Stories
				if abs(INT(connected_stories_id[i]/2)-(connected_stories_id[i]/2)) < eps then
					addz connected_stories_elev[i]
					cprism_ elev_shaft_edge_mat, elev_shaft_edge_mat, elev_shaft_ext_mat,
							5, connected_stories_height[i],
							0, 0, 15,
							totalShaftWidth, 0, 15,
							totalShaftWidth, totalShaftDepth, 15,
							0, totalShaftDepth, 15,
							0, 0, -1

					hotspot 0, 0, 0, unID: unID = unID + 1
					hotspot totalShaftWidth, 0, 0, unID: unID = unID + 1
					hotspot totalShaftWidth, totalShaftDepth, 0, unID: unID = unID + 1
					hotspot 0, totalShaftDepth, 0, unID: unID = unID + 1
					del 1
				endif
			next i

			for j = 1 to num_cutend
				cutend
			next j
			num_cutend = 0
		else
			for i = 1 to num_connected_stories						! --- Stories and doors
				addz connected_stories_elev[i]

				storyID = connected_stories_id[i]
				if connected_stories_height[i] > gs_max_StrFl_height then
					if storyID < 0 then
						elevShaftDoorPosID = below_story_door_pos[abs(storyID)]
					else
						elevShaftDoorPosID = above_story_door_pos[storyID+1]
					endif
				else
					elevShaftDoorPosID = 4
				endif

				opening_in3D_in_car_pos = 0
				if car_pos_story_m = storyID then opening_in3D_in_car_pos = opening_in3D
				gosub \"openings to shaft\"

				pen gs_wall_cont_pen
				material elev_shaft_int_mat
				add elev_shaft_thick, elev_shaft_thick, 0
				cutpolya 5, 2, 0,
					0, 0, 15,
					shaft_inner_width, 0, 15,
					shaft_inner_width, shaft_inner_depth, 15,
					0, shaft_inner_depth, 15,
					0, 0, -1
				del 1
				num_cutend = num_cutend+1

				cprism_ elev_shaft_edge_mat, elev_shaft_edge_mat, elev_shaft_ext_mat,
						5, connected_stories_height[i],
						0, 0, 15,
						totalShaftWidth, 0, 15,
						totalShaftWidth, totalShaftDepth, 15,
						0, totalShaftDepth, 15,
						0, 0, -1

				hotspot 0, 0, 0, unID: unID = unID + 1
				hotspot totalShaftWidth, 0, 0, unID: unID = unID + 1
				hotspot totalShaftWidth, totalShaftDepth, 0, unID: unID = unID + 1
				hotspot 0, totalShaftDepth, 0, unID: unID = unID + 1

				for j = 1 to num_cutend
					cutend
				next j
				num_cutend=0
				del 1
			next i
		endif


		pen gs_wall_cont_pen
		material elev_shaft_int_mat
		add elev_shaft_thick, elev_shaft_thick, 0
		cutpolya 5, 2, 0,
			0, 0, 15,
			shaft_inner_width, 0, 15,
			shaft_inner_width, shaft_inner_depth, 15,
			0, shaft_inner_depth, 15,
			0, 0, -1
		del 1

		if bPenthouse then				! --- penthouse
			addz connected_stories_elev[1]+connected_stories_height[1]
			cprism_ elev_shaft_edge_mat, elev_shaft_edge_mat, elev_shaft_ext_mat,
					5, penthouse_height,
					0, 0, 15,
					totalShaftWidth, 0, 15,
					totalShaftWidth, totalShaftDepth, 15,
					0, totalShaftDepth, 15,
					0, 0, -1

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, totalShaftDepth, 0, unID: unID = unID + 1
			hotspot 0, totalShaftDepth, 0, unID: unID = unID + 1
			del 1
		endif

		if bPith then						! --- pit
			addz connected_stories_elev[num_connected_stories]-pit_depth
			cprism_ elev_shaft_edge_mat, elev_shaft_edge_mat, elev_shaft_ext_mat,
					5, pit_depth,
					0, 0, 15,
					totalShaftWidth, 0, 15,
					totalShaftWidth, totalShaftDepth, 15,
					0, totalShaftDepth, 15,
					0, 0, -1

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, totalShaftDepth, 0, unID: unID = unID + 1
			hotspot 0, totalShaftDepth, 0, unID: unID = unID + 1
			del 1
		endif
		cutend

		if bTopSlab then						! --- top slab
			addz connected_stories_elev[1]+connected_stories_height[1]+penthouse_height
			cprism_ top_slab_ext_mat, top_slab_int_mat, top_slab_edge_mat,
					5, top_slab_thick,
					0, 0, 15,
					totalShaftWidth, 0, 15,
					totalShaftWidth, totalShaftDepth, 15,
					0, totalShaftDepth, 15,
					0, 0, -1

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, totalShaftDepth, 0, unID: unID = unID + 1
			hotspot 0, totalShaftDepth, 0, unID: unID = unID + 1
			del 1
		endif

		addz connected_stories_elev[1]+connected_stories_height[1]+penthouse_height+top_slab_thick
		hotspot 0, 0, 0, unID: unID = unID + 1
		hotspot totalShaftWidth, 0, 0, unID: unID = unID + 1
		hotspot totalShaftWidth, totalShaftDepth, 0, unID: unID = unID + 1
		hotspot 0, totalShaftDepth, 0, unID: unID = unID + 1
		del 1

		if bPitSlab then						! --- pit slab
			addz connected_stories_elev[num_connected_stories]-pit_depth-pit_slab_thick
			cprism_ pit_slab_ext_mat, pit_slab_int_mat, pit_slab_edge_mat,
					5, pit_slab_thick,
					0, 0, 15,
					totalShaftWidth, 0, 15,
					totalShaftWidth, totalShaftDepth, 15,
					0, totalShaftDepth, 15,
					0, 0, -1

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, 0, 0, unID: unID = unID + 1
			hotspot totalShaftWidth, totalShaftDepth, 0, unID: unID = unID + 1
			hotspot 0, totalShaftDepth, 0, unID: unID = unID + 1
			del 1
		endif
	endif
endif


! ==============================================================================
! Segmented
! ==============================================================================
if elevator_form_m = FORM_SEGMENTED then

	if door_pos < 0 then
		RightDoor = 1
	else
		RightDoor = 0
	endif
	! --------------------------------------------------------------------------
	! Elevator Wall
	! --------------------------------------------------------------------------
	if bShowElevatorWall then
		for i = 1 to num_connected_stories						! --- Stories and doors
			pen gs_wall_cont_pen

			addz connected_stories_elev[i]

			storyID = connected_stories_id[i]
			if connected_stories_height[i] > gs_max_StrFl_height then
				if storyID < 0 then
					elevShaftDoorPosID = S_below_story_door_pos[abs(storyID)]
				else
					elevShaftDoorPosID = S_above_story_door_pos[storyID+1]
				endif
			else
				elevShaftDoorPosID = 4
			endif

			if elevShaftDoorPosID = 1 then
				addx door_pos
				rotx 90
				addx -opening_width/2

				pen gs_cont_pen
				opening_in3D_in_car_pos = 0
				if car_pos_story_m = storyID then opening_in3D_in_car_pos = opening_in3D
				neg = -0.10-0.08
				pos = opening_width+0.10
				addx A/2

				gosub \"wall door\"

				pen gs_wall_cont_pen
				material elev_wall_edge_mat
				cutpolya 5, 2, 0,
					0, 0, 15,
					opening_width, 0, 15,
					opening_width, opening_height, 15,
					0, opening_height, 15,
					0, 0, -1
				del 4
				num_cutend = num_cutend+1
			endif

			addy elev_wall_thick
			rotx 90
			cprism_ elev_wall_int_mat, elev_wall_ext_mat, elev_wall_edge_mat,
					5, elev_wall_thick,
					0, 0, 15,
					A, 0, 15,
					A, connected_stories_height[i], 15,
					0, connected_stories_height[i], 15,
					0, 0, -1
			del 2

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot A, 0, 0, unID: unID = unID + 1
			hotspot A, elev_wall_thick, 0, unID: unID = unID + 1
			hotspot 0, elev_wall_thick, 0, unID: unID = unID + 1

			for j = 1 to num_cutend
				cutend
			next j
			num_cutend=0

			del 1
		next i

		if bPenthouse then				! --- penthouse
			addz connected_stories_elev[1]+connected_stories_height[1]
			addy elev_wall_thick
			rotx 90
			cprism_ elev_wall_int_mat, elev_wall_ext_mat, elev_wall_edge_mat,
					5, elev_wall_thick,
					0, 0, 15,
					A, 0, 15,
					A, penthouse_height, 15,
					0, penthouse_height, 15,
					0, 0, -1
			del 2

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot A, 0, 0, unID: unID = unID + 1
			hotspot A, elev_wall_thick, 0, unID: unID = unID + 1
			hotspot 0, elev_wall_thick, 0, unID: unID = unID + 1
			del 1
		endif

		if bPith then						! --- pit
			addz connected_stories_elev[num_connected_stories]-pit_depth
			addy elev_wall_thick
			rotx 90
			cprism_ elev_wall_int_mat, elev_wall_ext_mat, elev_wall_edge_mat,
					5, elev_wall_thick,
					0, 0, 15,
					A, 0, 15,
					A, pit_depth, 15,
					0, pit_depth, 15,
					0, 0, -1
			del 2

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot A, 0, 0, unID: unID = unID + 1
			hotspot A, elev_wall_thick, 0, unID: unID = unID + 1
			hotspot 0, elev_wall_thick, 0, unID: unID = unID + 1
			del 1
		endif

		if bTopSlab then						! --- top slab
			addz connected_stories_elev[1]+connected_stories_height[1]+penthouse_height
			addy elev_wall_thick
			rotx 90
			cprism_ top_slab_int_mat, top_slab_ext_mat, top_slab_edge_mat,
					5, elev_wall_thick,
					0, 0, 15,
					A, 0, 15,
					A, top_slab_thick, 15,
					0, top_slab_thick, 15,
					0, 0, -1
			del 2

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot A, 0, 0, unID: unID = unID + 1
			hotspot A, elev_wall_thick, 0, unID: unID = unID + 1
			hotspot 0, elev_wall_thick, 0, unID: unID = unID + 1
			del 1
		endif

		addz connected_stories_elev[1]+connected_stories_height[1]+penthouse_height+top_slab_thick
		hotspot 0, 0, 0, unID: unID = unID + 1
		hotspot A, 0, 0, unID: unID = unID + 1
		hotspot A, elev_wall_thick, 0, unID: unID = unID + 1
		hotspot 0, elev_wall_thick, 0, unID: unID = unID + 1
		del 1

		if bPitSlab then						! --- pit slab
			addz connected_stories_elev[num_connected_stories]-pit_depth-pit_slab_thick
			addy elev_wall_thick
			rotx 90
			cprism_ pit_slab_int_mat, pit_slab_ext_mat, pit_slab_edge_mat,
					5, elev_wall_thick,
					0, 0, 15,
					A, 0, 15,
					A, pit_slab_thick, 15,
					0, pit_slab_thick, 15,
					0, 0, -1
			del 2

			hotspot 0, 0, 0, unID: unID = unID + 1
			hotspot A, 0, 0, unID: unID = unID + 1
			hotspot A, elev_wall_thick, 0, unID: unID = unID + 1
			hotspot 0, elev_wall_thick, 0, unID: unID = unID + 1
			del 1
		endif
	else
		addx elev_wall_overhang
	endif


	! --------------------------------------------------------------------------
	! Elevator Car
	! --------------------------------------------------------------------------

	pen gs_cont_pen
	alpha = 180/segment_num
	if segment_num/2=int(segment_num/2) then
		dy = car_width/2
	else
		dy = car_width/2*sin(int(segment_num/2)*alpha)
	endif

	carPosStoryID = -car_pos_story_m+1+num_stories_above_HS+home_story_index
	add (A-car_width)/2, elev_wall_thick+segmentedFront,  connected_stories_elev[carPosStoryID]

	hotspot car_width/2,  0,			-0.2, unID, car_depth, 1+128:		unID=unID+1
	hotspot car_width/2, -1,			-0.2, unID, car_depth, 3:			unID=unID+1
	hotspot car_width/2,  car_depth,	-0.2, unID, car_depth, 2:			unID=unID+1

	hotspot car_width/2,  0,			-0.2+car_height+0.50, unID, car_depth, 1+128:		unID=unID+1
	hotspot car_width/2, -1,			-0.2+car_height+0.50, unID, car_depth, 3:			unID=unID+1
	hotspot car_width/2,  car_depth,	-0.2+car_height+0.50, unID, car_depth, 2:			unID=unID+1

	hotspot 0,			0, -0.2,					unID : unID = unID + 1
	hotspot car_width,	0, -0.2,					unID : unID = unID + 1
	hotspot car_width,	0, -0.2+car_height+0.50,	unID : unID = unID + 1
	hotspot 0,			0, -0.2+car_height+0.50,	unID : unID = unID + 1

	if GLOB_CONTEXT <> 4 & GLOB_CONTEXT <> 24 & GLOB_CONTEXT <> 44 then
		for i = 0 to segment_num
			hotspot car_width/2 + cos(alpha*i)*car_width/2, carWallThk+sin(alpha*i)*(car_width/2)*((car_depth-carWallThk)/dy), -0.2,			unID : unID = unID + 1
			hotspot car_width/2 + cos(alpha*i)*car_width/2, carWallThk+sin(alpha*i)*(car_width/2)*((car_depth-carWallThk)/dy), -0.2+car_height+0.50,	unID : unID = unID + 1
		next i
	else
		unID = unID + (segment_num+1)*2
	endif

	if gs_detlevel_3D_m = 2 then		! --- Detailed ---
		if door_pos < 0 then
			RightDoor = 1
		else
			RightDoor = 0
		endif

		add car_width/2+door_pos,0, 0
		rotx 90
		gosub \"elev car door\"
		del 2

		material car_back_mat				! --- Back Side ----
		rotx 90
		addz -carWallThk
		cprism_{2} car_back_mat, car_int_mat, car_back_mat,
				9, 0.08,
				0, 0, 0, 15, car_back_mat, 
				car_width/2-door_width/2+door_pos, 0, 0, 15, car_int_mat, 
				car_width/2-door_width/2+door_pos, door_height, 0, 15, car_int_mat, 
				car_width/2+door_width/2+door_pos, door_height, 0, 15, car_int_mat,
				car_width/2+door_width/2+door_pos, 0, 0, 15, car_back_mat, 
				car_width, 0, 0, 15, car_back_mat,
				car_width, car_height, 0, 15, car_back_mat,
				0, car_height, 0, 15, car_back_mat,
				0, 0, 0, -1, car_back_mat
		del 2
		for j = 1 to num_cutend
			cutend
		next j
		num_cutend=0
									! --- Bottom Side ----
		if bShowMullion then ss = 15 else ss = 79

		for i = 1 to segment_num
			put cos(alpha*i)*car_width/2, carWallThk+(sin(alpha*i)*car_width/2)*((car_depth-carWallThk)/dy), ss
		next i

		addx car_width/2
		addz -0.2
		cprism_ car_pave_mat, car_ext_mat, car_ext_mat,
			4+segment_num, 0.20,
			-car_width/2, carWallThk, ss,
			-car_width/2, 0, ss,
			car_width/2, 0, ss,
			car_width/2, carWallThk, ss,
			use(nsp)
		del 1

		addz car_height
		cprism_ car_ext_mat, car_ceiling_mat, car_ext_mat,
			4+segment_num, 0.30,
			-car_width/2, carWallThk, ss,
			-car_width/2, 0, ss,
			car_width/2, 0, ss,
			car_width/2, carWallThk, ss,
			get(nsp)
		del 1

		if bShowMullion then
			material car_mullion_mat
			resol 4

			for i = 1 to segment_num-1
				add cos(alpha*(i))*(car_width/2-0.02),carWallThk+sin(alpha*(i))*(car_width/2-0.02)*((car_depth-carWallThk)/dy), 0
				rotz i*alpha+45
				prism_ 2, car_height,
					0, 0, 979,
					0.02, 360, 4079
				del 2
			next i
		endif

		material car_front_mat
		for i = 1 to segment_num
			put cos(alpha*(i))*(car_width/2-0.01),carWallThk+sin(alpha*(i))*(car_width/2-0.01)*((car_depth-carWallThk)/dy),79
		next i

		for q = segment_num to 1 step -1
			put cos(alpha*(q-1))*(car_width/2-0.02),carWallThk+sin(alpha*(q-1))*(car_width/2-0.02)*((car_depth-carWallThk)/dy),79
		next q

		prism_ 2*segment_num+2, car_height,
			car_width/2-0.01,carWallThk,79,
			get(nsp/2),
			-car_width/2+0.02,carWallThk,79,
			get(nsp)

		if bControlPanel then
			add door_width/2+door_pos+0.20, carWallThk, 1
			rotz 180
			gosub \"inside control panel\"
			del 2
		endif
		del 1

		for j = 1 to num_cutend
			cutend
		next j
		num_cutend=0
	else
		! --- Simple -----------------------------------------------------------

		if bShowMullion then ss = 15 else ss = 79
		for i = 1 to segment_num
			put cos(alpha*i)*car_width/2, carWallThk+sin(alpha*i)*(car_width/2)*((car_depth-carWallThk)/dy), ss
		next i

		addx car_width/2
		addz -0.2
		cprism_ car_ext_mat, car_ext_mat, car_ext_mat,
			4+segment_num, car_height+0.50,
			-car_width/2, carWallThk, ss,
			-car_width/2, 0, ss,
			car_width/2, 0, ss,
			car_width/2, carWallThk, ss,
			get(nsp)
		del 2
	endif
	del 1 + not(bShowElevatorWall)
endif


end

! ==============================================================================
! Subroutines
! ==============================================================================

\"openings to shaft\":
	if dir_second_opening_m = SECOND_OPENING_SIDE1 then
		pl=elev_shaft_thick+carSide1+car_width/2
	else
		if dir_second_opening_m = SECOND_OPENING_SIDE2 then
			pl=totalShaftWidth-car_width-elev_shaft_thick-carSide2+car_width/2
		else
			pl=carSide1+elev_shaft_thick+car_width/2
		endif
	endif

	if elevShaftDoorPosID = 1 or elevShaftDoorPosID = 3 then				! --- Front door

		addx pl+door_pos
		if door_pos < 0 then
			mulx -1
		else
			mulx 1
		endif
		rotx 90
		addx -opening_width/2
		frame_thk = shaft_inner_depth/2-car_depth/2 !-0.06
		neg = -0.10-0.08
		pos = opening_width+0.10
		gosub \"shaft door\"
		del 3
		rotx 90
		addx -opening_width/2
		material elev_shaft_edge_mat
		cutform 5, 1, 2,
			0, 0, 1, -elev_shaft_thick*2,
			0, 0, 15,
			opening_width, 0, 15,
			opening_width, opening_height, 15,
			0, opening_height, 15,
			0, 0, -1
		num_cutend = num_cutend+1
		del 3
	endif
	if (elevShaftDoorPosID = 2 or elevShaftDoorPosID = 3) and (dir_second_opening_m = SECOND_OPENING_NONE or dir_second_opening_m = SECOND_OPENING_REAR) then			! --- Second door on Back

		add pl+second_door_pos, totalShaftDepth, 0
		rotz 180
		if second_door_pos < 0 then
			mulx -1
		else
			mulx 1
		endif

		rotx 90
		addx -opening_width/2
		frame_thk = shaft_inner_depth/2-car_depth/2 !-0.06
		neg = -0.10-0.08
		pos = opening_width+0.10
		gosub \"shaft door\"
		del 3

		rotx 90
		addx -opening_width/2
		material elev_shaft_edge_mat

		cutform 5, 1, 2,
			0, 0, 1, -elev_shaft_thick*2,
			0, 0, 15,
			opening_width, 0, 15,
			opening_width, opening_height, 15,
			0, opening_height, 15,
			0, 0, -1
		num_cutend = num_cutend+1
		del 4
	endif
	if (elevShaftDoorPosID = 2 or elevShaftDoorPosID = 3) and dir_second_opening_m = SECOND_OPENING_SIDE1 then		! --- Second door on Side 1
		add 0, elev_shaft_thick+0.04+car_depth/2+second_door_pos, 0
		rotz -90
		if second_door_pos < 0 then
			mulx -1
		else
			mulx 1
		endif
		rotx 90
		addx -opening_width/2
		frame_thk = shaft_inner_width/2-car_width/2 !-0.06
		neg = -0.10-0.08
		pos = opening_width+0.10
		gosub \"shaft door\"
		del 3

		rotx 90
		addx -opening_width/2
		material elev_shaft_edge_mat

		cutform 5, 1, 2,
			0, 0, 1, -elev_shaft_thick*2,
			0, 0, 15,
			opening_width, 0, 15,
			opening_width, opening_height, 15,
			0, opening_height, 15,
			0, 0, -1
		num_cutend = num_cutend+1
		del 4
	endif
	if (elevShaftDoorPosID = 2 or elevShaftDoorPosID = 3) and dir_second_opening_m = SECOND_OPENING_SIDE2 then		! --- Second door on Side 2
		add totalShaftWidth, 0.04+car_depth/2+elev_shaft_thick+second_door_pos, 0
		rotz 90
		if second_door_pos < 0 then
			mulx -1
		else
			mulx 1
		endif
		rotx 90
		addx -opening_width/2
		frame_thk = shaft_inner_width/2-car_width/2 !-0.06
		pos	= -0.10-0.08
		neg = opening_width+0.10
		gosub \"shaft door\"
		del 3

		rotx 90
		addx -opening_width/2
		material elev_shaft_edge_mat
		cutform 5, 1, 2,
			0, 0, 1, -elev_shaft_thick*2,
			0, 0, 15,
			opening_width, 0, 15,
			opening_width, opening_height, 15,
			0, opening_height, 15,
			0, 0, -1
		del 4
		num_cutend = num_cutend+1
	endif
return

\"shaft door\":
	pen gs_wall_cont_pen
	if gs_detlevel_3D_m = 2 then				! --- Detailed
		material opening_frame_mat
		addz -elev_wall_thick
		prism_ 9, 0.06,
			0, 0, 15,
			0.18, 0, 15,
			0.18, door_height, 15,
			0.18+door_width, door_height, 15,
			0.18+door_width, 0, 15,
			opening_width, 0, 15,
			opening_width, opening_height, 15,
			0, opening_height, 15,
			0, 0, -1
		del 1

		if bOpeningFrame then
			addz 0.06-elev_wall_thick
			prism_ 9, elev_wall_thick-0.06,
				0, 0, 15,
				0.02, 0, 15,
				0.02, opening_height-0.02, 15,
				opening_width-0.02, opening_height-0.02, 15,
				opening_width-0.02, 0, 15,
				opening_width, 0, 15,
				opening_width, opening_height, 15,
				0, opening_height, 15,
				0, 0, -1
			del 1

			prism_ 9, 0.02,
				-0.04, 0, 15,
				0.02, 0, 15,
				0.02, opening_height-0.02, 15,
				opening_width-0.02, opening_height-0.02, 15,
				opening_width-0.02, 0, 15,
				opening_width+0.04, 0, 15,
				opening_width+0.04, opening_height+0.04, 15,
				-0.04, opening_height+0.04, 15,
				-0.04, 0, -1
		endif

		if elev_door_style_m = 1 then		! --- Sliding door
			addz -elev_wall_thick
			material door_panel_mat
			add 0.18-(door_width/2)*(opening_in3D_in_car_pos/100), 0, -0.02
			block door_width/2, door_height, 0.02
			del 1
			add 0.18+door_width/2-door_width*(opening_in3D_in_car_pos/100), 0, -0.04
			block door_width/2, door_height, 0.02
			del 2
		endif

		if elev_door_style_m = 2 then		! --- Opening door
			addz -elev_wall_thick
			material door_panel_mat
			addx 0.18
			roty -90*(opening_in3D_in_car_pos/100)
			addz -0.02
			block door_width/2, door_height, 0.02
			del 3
			addx 0.18+door_width
			roty 90*(opening_in3D_in_car_pos/100)
			add -door_width/2, 0, -0.02
			block door_width/2, door_height, 0.02
			del 3
			del 1
		endif

		if bControlPanel then
			if carSide1+car_width/2-door_pos < shaft_inner_width/2 then
				add neg, 1, 0
			else
				add pos, 1, 0
			endif
			gosub \"outside control panel\"
			del 1
		endif
	else										! --- Simple
		material door_panel_mat
		addz -elev_wall_thick
		block opening_width, opening_height, elev_wall_thick
		del 1
	endif
return


\"wall door\":
	pen gs_wall_cont_pen
	if gs_detlevel_3D_m = 2 then				! --- Detailed
		material opening_frame_mat
		addz -elev_wall_thick
		prism_ 9, 0.06,
			0, 0, 15,
			0.18, 0, 15,
			0.18, door_height, 15,
			0.18+door_width, door_height, 15,
			0.18+door_width, 0, 15,
			opening_width, 0, 15,
			opening_width, opening_height, 15,
			0, opening_height, 15,
			0, 0, -1
		del 1

		if bOpeningFrame then
			addz 0.06-elev_wall_thick
			prism_ 9, elev_wall_thick-0.06,
				0, 0, 15,
				0.02, 0, 15,
				0.02, opening_height-0.02, 15,
				opening_width-0.02, opening_height-0.02, 15,
				opening_width-0.02, 0, 15,
				opening_width, 0, 15,
				opening_width, opening_height, 15,
				0, opening_height, 15,
				0, 0, -1
			del 1

			prism_ 9, 0.02,
				-0.04, 0, 15,
				0.02, 0, 15,
				0.02, opening_height-0.02, 15,
				opening_width-0.02, opening_height-0.02, 15,
				opening_width-0.02, 0, 15,
				opening_width+0.04, 0, 15,
				opening_width+0.04, opening_height+0.04, 15,
				-0.04, opening_height+0.04, 15,
				-0.04, 0, -1
		endif

		if elev_door_style_m = 1 then		! --- Sliding door
			if RightDoor then
				mulx -1
				addx -shaft_inner_width/2
			else
				mulx 1
				addx 0
			endif

			addx 0.17
			rotz -90
			cutplane 90
			del 2

			addz -elev_wall_thick
			material door_panel_mat
			add 0.17-(door_width/2)*(opening_in3D_in_car_pos/100), 0, 0.03
			block door_width/2, door_height, 0.02
			del 1
			add 0.17+door_width/2-door_width*(opening_in3D_in_car_pos/100), 0, 0.01
			block door_width/2, door_height, 0.02
			del 2+2

			cutend
		endif

		if elev_door_style_m = 2 then		! --- Opening door
			addz -elev_wall_thick+0.04
			material door_panel_mat
			addx 0.18
			roty -90*(opening_in3D_in_car_pos/100)
			addz -0.02
			block door_width/2, door_height, 0.02
			del 3
			addx 0.18+door_width
			roty 90*(opening_in3D_in_car_pos/100)
			add -door_width/2, 0, -0.02
			block door_width/2, door_height, 0.02
			del 3
			del 1
		endif

		if bControlPanel then
			if carSide1+car_width/2-door_pos < shaft_inner_width/2 then
				add neg, 1, 0
			else
				add pos, 1, 0
			endif
			gosub \"outside control panel\"
			del 1
		endif
	else										! --- Simple
		material door_panel_mat
		addz -elev_wall_thick
		block opening_width, opening_height, elev_wall_thick
		del 1
	endif
return

\"elev car door\":
	pen gs_cont_pen
	addz -carWallThk+elevCarOffset
	if door_style_m = 1 then
		material door_panel_mat

		if RightDoor then
			mulx -1
		else
			mulx 1
		endif
		addx -door_width/2
		add -(door_width/2)*(opening_in3D/100), 0, carWallThk-elevCarDoorThk
		block door_width/2, door_height, elevCarDoorThk
		del 1
		add door_width/2-door_width*(opening_in3D/100), 0, carWallThk-elevCarDoorThk*2
		block door_width/2, door_height, elevCarDoorThk
		del 3

		addz carWallThk

		if RightDoor then
			addx door_width
		else
			addx 0
		endif

		if SYMB_MIRRORED then MULX -1  ! Rotated or mirrored window
		cutpolya 5, 2, -elevCarDoorThk*2,
			0, 0, 15,
			0, door_height, 15,
			-door_width, door_height, 15,
			-door_width, 0, 15,
			0, 0, -1
		num_cutend = num_cutend+1
		if SYMB_MIRRORED then del 1  ! Rotated or mirrored window
		del 2
	endif

	if door_style_m = 2 then
		addx -door_width/2

		material door_panel_mat
		addx -(door_width/2)*(opening_in3D/100)
		block door_width/2, door_height, elevCarDoorThk
		del 1
		addx door_width/2+(door_width/2)*(opening_in3D/100)
		block door_width/2, door_height, elevCarDoorThk
		del 1
		! one transformation left for the cutform!
	endif

	if door_style_m = 3 then
		addx -door_width/2

		for i = -1 to 1 step 2
			addx door_width*(i < 0)
			mulx i
			material door_panel_mat
			roty 90*(opening_in3D/100)
			prism_ 10, elevCarDoorThk,
				0, 0, 15,
				door_width/2, 0, 15,
				door_width/2, door_height, 15,
				0, door_height, 15,
				0, 0, -1,
				0.05, door_height/2, 15,
				door_width/2-0.05, door_height/2, 15,
				door_width/2-0.05, door_height-0.05, 15,
				0.05, door_height-0.05, 15,
				0.05, door_height/2, -1
			addz (elevCarDoorThk-elevCarGlassThk)/2
			material door_glass_mat
			prism_ 5, elevCarGlassThk,
				0.05, door_height/2, 15,
				door_width/2-0.05, door_height/2, 15,
				door_width/2-0.05, door_height-0.05, 15,
				0.05, door_height-0.05, 15,
				0.05, door_height/2, -1
			del 4
		next i
		del 1
	endif

	if door_style_m = 4 then
		addx -door_width/2

		for i = -1 to 1 step 2
			addx door_width*(i < 0)
			mulx i
			material door_panel_mat
			addx -(door_width/2)*(opening_in3D/100)
			prism_ 10, elevCarDoorThk,
				0, 0, 15,
				door_width/2, 0, 15,
				door_width/2, door_height, 15,
				0, door_height, 15,
				0, 0, -1,
				0.03, 0.15, 15,
				door_width/2-0.03, 0.15, 15,
				door_width/2-0.03, door_height-0.05, 15,
				0.03, door_height-0.05, 15,
				0.03,  0.15, -1
			addz (elevCarDoorThk-elevCarGlassThk)/2
			material door_glass_mat
			prism_ 5, elevCarGlassThk,
				0.03, 0.15, 15,
				door_width/2-0.03, 0.15, 15,
				door_width/2-0.03, door_height-0.05, 15,
				0.03, door_height-0.05, 15,
				0.03, 0.15, -1
			del 4
		next i
		! one transformation left for the cutform!
	endif

	if  door_style_m = 5 then
		if RightDoor then
			mulx -1
		else
			mulx 1
		endif

		addx -door_width/2

		material door_panel_mat
		addx -(door_width/2)*(opening_in3D/100)
		block (door_width/2), door_height, elevCarDoorThk
		add (door_width/2)*(opening_in3D/100), 0, elevCarDoorThk/4
		block (door_width/2), door_height, elevCarDoorThk/2
		del 2

		cutform 5, 1, 2,
			0, 0, 1, elevCarDoorThk,
			0, 0, 15,
			-door_width/2, 0, 15,
			-door_width/2, door_height, 15,
			0, door_height, 15,
			0, 0, -1
		num_cutend = num_cutend+1
		del 2
	endif

	if door_style_m = 2 or door_style_m = 4 then
		addz carWallThk-elevCarOffset
		for i=1 to 2
			cutform 5, 1, 2,
				0, 0, 1, elevCarDoorThk,
				0, 0, 15,
				-door_width/2, 0, 15,
				-door_width/2, door_height, 15,
				0, door_height, 15,
				0, 0, -1
			num_cutend = num_cutend+1

			addx door_width+door_width/2
		next i
		del 3
		del 1	! for previous transformation!
	endif

	if SYMB_MIRRORED then MULX -1           ! Mirrored object
	material car_int_mat
	cutpolya 5, 2, -0.10,
		-door_width/2, 0, 15,
		door_width/2, 0, 15,
		door_width/2, door_height, 15,
		-door_width/2, door_height, 15,
		-door_width/2, 0, -1
	num_cutend = num_cutend+1
	if SYMB_MIRRORED then del 1             ! Mirrored object
	del 1
return

\"inside control panel\":
	numStory = 1+num_stories_above_HS+num_stories_below_HS
	if bSplitLevelStories then numStory = numStory+num_SL_stories_above_HS+num_SL_stories_below_HS
	distButton = 0.10/4

	rowIntNumStory = 15
	columnNumStory  = 1
	if numStory > rowIntNumStory then
		columnNumStory = INT(numStory/rowIntNumStory)
		remainStory = numStory mod rowIntNumStory
		if remainStory then columnNumStory = columnNumStory + 1
	else
		remainStory = 0
		rowIntNumStory = numStory
	endif

	cp_height = distButton*(rowIntNumStory+2)

	rotx 90
	material cpanel_mat
	block 0.10, cp_height, 0.01

	material button_mat
	for i = 0 to 3
		add distButton/2+distButton*(i), distButton/2, 0.01
		resol 4
		rotz 45
		cone 0.005, 0.01, 0.008, 90, 90
		del 2
	next i

	for j = 1 to columnNumStory
		if j = columnNumStory and remainStory then
			currentNumStory = remainStory
		else
			currentNumStory = rowIntNumStory
		endif
		for i = 1 to currentNumStory
			add distButton/2+distButton*(j-1), cp_height-distButton*(i-0.5), 0.01
			resol 4
			rotz 45
			cone 0.005, 0.01, 0.008, 90, 90
			del 2
		next i
	next j
	del 1
return

\"outside control panel\":
	material cpanel_mat
	if i > 1 and i < num_connected_stories then
		block 0.08, 0.20, 0.01
		add 0.04, 0.10, 0.01
	else
		if i = 1 then
			block 0.08, 0.10, 0.01
			add 0.04, 0.095, 0.01
		else
			block 0.08, 0.10, 0.01
			add 0.04, 0.005, 0.01
		endif
	endif

	material button_mat
	resol 12
	if i > 1 then
		addy 0.015
		prism_ 8, 0.002,
			-0.005, 0, 15,
			-0.005, 0.012, 15,
			-0.014, 0.012, 15,
			0, 0.027, 15,
			0.014, 0.012, 15,
			0.005, 0.012, 15,
			0.005, 0, 15,
			-0.005, 0, -1
		del 1

		addy 0.065
		cone 0.006, 0.012, 0.01, 90, 90
		del 1
	endif

	if i < num_connected_stories then
		addy -0.015
		muly -1
		prism_ 8, 0.002,
			-0.005, 0, 15,
			-0.005, 0.012, 15,
			-0.014, 0.012, 15,
			0, 0.027, 15,
			0.014, 0.012, 15,
			0.005, 0.012, 15,
			0.005, 0, 15,
			-0.005, 0, -1
		del 2

		addy -0.065
		cone 0.006, 0.012, 0.01, 90, 90
		del 1
	endif
	del 1
return

")

(define additional-parameters-elevator
  (list ;(list "A" 2500)
        ;(list "B" 2000)
        ;(list "ZZYZX" 5700)
        ;(list "AC_show2DHotspotsIn3D" 0)
        ;(list "ac_bottomlevel" -1300)
        ;(list "ac_toplevel" 4400)
        (list "gs_detlevel_3D" "Detailed")
        (list "gs_detlevel_3D_m" 2)
        (list "elevator_form" "Rectangular")
        (list "elevator_form_m" 1)
        (list "elevator_form_m_prev" 1)
        (list "segment_num" 7)
        (list "elevator_type" "Mechanical")
        (list "elevator_type_m" 1)
        (list "dir_second_opening" "None")
        (list "dir_second_opening_m" 1)
        (list "cweight_pos" "Normal")
        (list "cweight_pos_m" 1)
        (list "cweight_width" 900)
        (list "cweight_depth" 100)
        (list "shaft_inner_width" 2500)
        (list "shaft_inner_depth" 2000)
        (list "num_stories_above_HS" 0)
        (list "num_stories_below_HS" 0)
        (list "gs_max_StrFl_height" 1800)
        (list "bSplitLevelStories" #f)
        (list "num_SL_stories_above_HS" 0)
        (list "num_SL_stories_below_HS" 0)
        (list "dist_SL_story_to_normal_story" 1550)
        (list "bPenthouse" #t)
        (list "penthouse_height" 1000)
        (list "bPith" #t)
        (list "pit_depth" 1000)
        (list "door_width" 900)
        (list "door_height" 2100)
        (list "door_pos" 0)
        (list "second_door_pos" 0)
        (list "elev_door_style" "Sliding")
        (list "elev_door_style_m" 1)
        (list "door_style" "Style 1")
        (list "door_style_m" 1)
        (list "bOpeningFrame" #t)
        (list "edited_story" "0. Story")
        (list "edited_story_m" 0)
        (list "edited_story_height" 3100)
        (list "edited_story_elev" 0)
        (list "edited_story_door_pos" "Front")
        (list "edited_story_door_pos_m" 1)
        (list "bControlPanel" #t)
        (list "door_panel_mat" 13)
        (list "door_glass_mat" 24)
        (list "opening_frame_mat" 13)
        (list "opanel_mat" 11)
        (list "button_mat" 40)
        (list "car_width" 2260)
        (list "car_depth" 1740)
        (list "car_inner_width" 2100)
        (list "car_inner_depth" 1580)
        (list "car_height" 2400)
        (list "car_pos_story" "0. Story")
        (list "car_pos_story_m" 0)
        (list "car_ext_mat" 23)
        (list "car_int_mat" 23)
        (list "car_pave_mat" 47)
        (list "car_ceiling_mat" 78)
        (list "car_front_mat" 24)
        (list "car_back_mat" 23)
        (list "car_mullion_mat" 11)
        (list "bShowElevatorShaft" #f)
        (list "elev_shaft_thick" 150)
        (list "elev_shaft_ext_mat" 18)
        (list "elev_shaft_int_mat" 18)
        (list "elev_shaft_edge_mat" 18)
        (list "bShowElevatorWall" #f)
        (list "elev_wall_thick" 150)
        (list "elev_wall_overhang" 120)
        (list "elev_wall_ext_mat" 18)
        (list "elev_wall_int_mat" 18)
        (list "elev_wall_edge_mat" 18)
        (list "bTopSlab" #t)
        (list "top_slab_thick" 300)
        (list "top_slab_ext_mat" 18)
        (list "top_slab_int_mat" 18)
        (list "top_slab_edge_mat" 18)
        (list "bPitSlab" #t)
        (list "pit_slab_thick" 300)
        (list "pit_slab_ext_mat" 18)
        (list "pit_slab_int_mat" 18)
        (list "pit_slab_edge_mat" 18)
        (list "gs_3D_representation" "")
        (list "opening_in3D" 30)
        (list "gs_shadow" #t)
        (list "gs_resol" 6)
        (list "bShowMullion" #t)
        (list "gs_2D_representation" "")
        (list "gs_detlevel_2D" "Scale Sensitive")
        (list "gs_detlevel_2D_m" 1)
        (list "bShowMechin2D" #t)
        (list "gs_cont_pen" 15)
        (list "gs_fill_type" 65)
        (list "gs_fill_pen" 19)
        (list "gs_back_pen" 19)
        (list "gs_wall_cont_pen" 27)
        (list "gs_wall_fill_type" 65)
        (list "gs_wall_fill_pen" 107)
        (list "gs_wall_back_pen" 19)
        (list "gs_opening_fill_type" 65)
        (list "gs_opening_fill_pen" 107)
        (list "gs_opening_back_pen" 19)
        (list "gs_gap_fill_type" 65)
        (list "gs_gap_fill_pen" 19)
        (list "gs_gap_back_pen" 19)
        (list "_sp0" "")
        (list "num_above_story" 0)
        (list "story_name_above" (list "" "" "" "" ""))
        (list "story_elev_above" (list 3100 6200 9300 6200 9300))
        (list "story_height_above" (list 3100 3100 3100 3100 3100))
        (list "num_below_story" 0)
        (list "story_name_below" (list "" "" "" "" "First Floor" "Second Floor"))
        (list "story_elev_below" (list -12300 -9200 -6100 -2900 3100 6200))
        (list "story_height_below" (list 3100 3100 3100 2900 3100 3100))
        (list "home_story_name" "")
        (list "home_story_elev" 0)
        (list "home_story_height" 3100)
        (list "home_story_index" 0)
        (list "above_story_door_pos" (list 1))
        (list "below_story_door_pos" (list 0))
        (list "SL_above_story_door_pos" (list 0))
        (list "SL_below_story_door_pos" (list 0))
        (list "S_above_story_door_pos" (list 0))
        (list "S_below_story_door_pos" (list 0))
        (list "isFrom14" #f)
        (list "sLibpartName" "")
        (list "LibraryLangCode" "INT")
        ))