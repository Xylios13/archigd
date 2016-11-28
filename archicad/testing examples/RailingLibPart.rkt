#lang racket
(require "main.rkt")

(provide lib-part-railing)

#|
(library-part name
                      2D-section
                      3D-section
                      #:master-code [master-code ""]
                      #:parameter-code [parameter-code ""]
                      #:type [type "Object"]
                      #:parent-id [parent-id "ModelElement"]
                      #:properties [properties (list)])

(send (library-part "railing-test"
                    2d
                    3d
                    #:master-code mc
                    #:parameter-code pc
                    #:properties additional-p)
(object "railing-test" (x 2) #:additional-parameters (list (list "lSlo" pi/4))))

(send (object "railing-test" (x 2) #:additional-parameters (list (list "lSlo" pi/4))))

(send (object "railing-test" (x 2) #:additional-parameters (list (list "lSlo" pi/4)))
        (object "railing-test" (xy 2 1.8) #:additional-parameters (list (list "lSlo" pi/4)) #:angle pi))

(send 
        (object "railing-test" (x 2) #:properties (list  "lSlo" (/ pi 4)))
        (object "railing-test" (x 2) #:properties (list  "lSlo" (/ pi 8))))
|#

(define (lib-part-railing)
  (library-part "railing-test"
                    2d
                    3d
                    #:master-code mc
                    #:parameter-code pc
                    #:properties additional-p-new))

(define str #<<END
asd
asdjn
xzc "
adsas"
END
  )

(define mc #<<END
! Balustrade_Sentrel.gsm Object
! Jan 2011
! Master Script


! Variables  ********************

	uid = 1

	aSlo = atn(lSlo / lLen)

	lCblHig = lHig - lBotRaiGap - (0.07 + 0.07 + 0.03 * bHnd) / cos(aSlo)

	if tPnlTyp = `Balustrade` then
		if lLen >= 0.136 and lLen <  0.264 then : tModNam = "BP01" : iPolNum = 1 : iNodNum = 1  : tPolPos = "1"              : endif
		if lLen >= 0.264 and lLen <  0.392 then : tModNam = "BP02" : iPolNum = 2 : iNodNum = 2  : tPolPos = "1 2"            : endif
		if lLen >= 0.392 and lLen <  0.520 then : tModNam = "BP03" : iPolNum = 2 : iNodNum = 5  : tPolPos = "1 5"            : endif
		if lLen >= 0.520 and lLen <  0.648 then : tModNam = "BP04" : iPolNum = 2 : iNodNum = 7  : tPolPos = "1 7"            : endif
		if lLen >= 0.648 and lLen <  0.776 then : tModNam = "BP05" : iPolNum = 2 : iNodNum = 9  : tPolPos = "1 9"            : endif
		if lLen >= 0.776 and lLen <  0.904 then : tModNam = "BP06" : iPolNum = 2 : iNodNum = 11 : tPolPos = "1 11"           : endif
		if lLen >= 0.904 and lLen <  1.032 then : tModNam = "BP07" : iPolNum = 2 : iNodNum = 13 : tPolPos = "1 13"           : endif
		if lLen >= 1.032 and lLen <  1.160 then : tModNam = "BP08" : iPolNum = 3 : iNodNum = 15 : tPolPos = "1 8 15"         : endif
		if lLen >= 1.160 and lLen <  1.288 then : tModNam = "BP09" : iPolNum = 3 : iNodNum = 17 : tPolPos = "1 9 17"         : endif
		if lLen >= 1.288 and lLen <  1.416 then : tModNam = "BP10" : iPolNum = 3 : iNodNum = 19 : tPolPos = "1 10 19"        : endif
		if lLen >= 1.416 and lLen <  1.544 then : tModNam = "BP11" : iPolNum = 3 : iNodNum = 21 : tPolPos = "1 11 21"        : endif
		if lLen >= 1.544 and lLen <  1.672 then : tModNam = "BP12" : iPolNum = 3 : iNodNum = 23 : tPolPos = "1 12 23"        : endif
		if lLen >= 1.672 and lLen <  1.800 then : tModNam = "BP13" : iPolNum = 4 : iNodNum = 25 : tPolPos = "1 9 17 25"      : endif
		if lLen >= 1.800 and lLen <  1.928 then : tModNam = "BP14" : iPolNum = 4 : iNodNum = 28 : tPolPos = "1 10 19 28"     : endif
		if lLen >= 1.928 and lLen <  2.056 then : tModNam = "BP15" : iPolNum = 4 : iNodNum = 29 : tPolPos = "1 10 20 29"     : endif
		if lLen >= 2.056 and lLen <  2.184 then : tModNam = "BP16" : iPolNum = 4 : iNodNum = 31 : tPolPos = "1 11 21 31"     : endif
		if lLen >= 2.184 and lLen <  2.312 then : tModNam = "BP17" : iPolNum = 4 : iNodNum = 33 : tPolPos = "1 12 22 33"     : endif
		if lLen >= 2.312 and lLen <  2.440 then : tModNam = "BP18" : iPolNum = 4 : iNodNum = 35 : tPolPos = "1 12 24 35"     : endif
		if lLen >= 2.440 and lLen <  2.568 then : tModNam = "BP19" : iPolNum = 4 : iNodNum = 37 : tPolPos = "1 13 25 37"     : endif
		if lLen >= 2.568 and lLen <= 2.696 then : tModNam = "BP20" : iPolNum = 5 : iNodNum = 39 : tPolPos = "1 10 20 30 39"  : endif
	endif

	if tPnlTyp = `Pool Fence` then
		if lLen >= 0.314 and lLen <  0.442 then : tModNam = "PF01" : iPolNum = 2 : iNodNum = 4  : tPolPos = "1 4"            : endif
		if lLen >= 0.442 and lLen <  0.570 then : tModNam = "PF02" : iPolNum = 2 : iNodNum = 6  : tPolPos = "1 6"            : endif
		if lLen >= 0.570 and lLen <  0.698 then : tModNam = "PF03" : iPolNum = 2 : iNodNum = 8  : tPolPos = "1 8"            : endif
		if lLen >= 0.698 and lLen <  0.826 then : tModNam = "PF04" : iPolNum = 2 : iNodNum = 10  : tPolPos = "1 10"          : endif
		if lLen >= 0.826 and lLen <  0.915 then : tModNam = "PF05" : iPolNum = 3 : iNodNum = 11  : tPolPos = "1 6 11"        : endif
		if lLen >= 0.915 and lLen <  1.043 then : tModNam = "PF06" : iPolNum = 3 : iNodNum = 13 : tPolPos = "1 7 13"         : endif
		if lLen >= 1.043 and lLen <  1.171 then : tModNam = "PF07" : iPolNum = 3 : iNodNum = 15 : tPolPos = "1 8 15"         : endif
		if lLen >= 1.171 and lLen <  1.299 then : tModNam = "PF08" : iPolNum = 3 : iNodNum = 17 : tPolPos = "1 9 17"         : endif
		if lLen >= 1.299 and lLen <  1.427 then : tModNam = "PF09" : iPolNum = 3 : iNodNum = 19 : tPolPos = "1 10 19"        : endif
		if lLen >= 1.427 and lLen <  1.452 then : tModNam = "PF10" : iPolNum = 4 : iNodNum = 19 : tPolPos = "1 7 13 19"      : endif
		if lLen >= 1.452 and lLen <  1.516 then : tModNam = "PF11" : iPolNum = 4 : iNodNum = 20 : tPolPos = "1 7 14 20"      : endif
		if lLen >= 1.516 and lLen <  1.644 then : tModNam = "PF12" : iPolNum = 4 : iNodNum = 22 : tPolPos = "1 8 15 22"      : endif
		if lLen >= 1.644 and lLen <  1.708 then : tModNam = "PF13" : iPolNum = 4 : iNodNum = 23 : tPolPos = "1 8 16 23"      : endif
		if lLen >= 1.708 and lLen <  1.836 then : tModNam = "PF14" : iPolNum = 4 : iNodNum = 25 : tPolPos = "1 9 17 25"      : endif
		if lLen >= 1.836 and lLen <  1.900 then : tModNam = "PF15" : iPolNum = 4 : iNodNum = 26 : tPolPos = "1 9 18 26"      : endif
		if lLen >= 1.900 and lLen <  2.028 then : tModNam = "PF16" : iPolNum = 4 : iNodNum = 28 : tPolPos = "1 10 19 28"     : endif
		if lLen >= 2.028 and lLen <  2.117 then : tModNam = "PF17" : iPolNum = 5 : iNodNum = 29 : tPolPos = "1 8 15 22 29"   : endif
		if lLen >= 2.117 and lLen <  2.245 then : tModNam = "PF18" : iPolNum = 5 : iNodNum = 31 : tPolPos = "1 8 16 24 31"   : endif
		if lLen >= 2.245 and lLen <  2.373 then : tModNam = "PF19" : iPolNum = 5 : iNodNum = 33 : tPolPos = "1 9 17 25 33"   : endif
		if lLen >= 2.373 and lLen <= 2.501 then : tModNam = "PF20" : iPolNum = 5 : iNodNum = 35 : tPolPos = "1 9 18 27 35"   : endif
	endif

	if tPnlTyp = `Pool Gate` then
		tModNam = "PG" : iPolNum = 1 : iNodNum = 11  : tPolPos = "6"
	endif

	lBalTruLen = 0.064 * (iNodNum - 1) 		! Baluster True Length


! Definitions  ********************

	define empty_fill "Empty Fill"

END
  )

(define 2d #<<END
! Balustrade_Sentrel.gsm Object
! Jan 2011
! 2d Script

	resol iRes


!  HandRail  ****************************

	line_type ltHnd
	drawindex 10
	fill "Empty Fill"

	if bHnd then poly2_b 	5,1+2,SYMB_FILL_PEN,	SYMB_FBGD_PEN,
							lHndWid / 2,		(lHndWid / 2) * tan(aCutAng1),			1,
							lHndWid / 2,		lLen + (lHndWid / 2) * tan(aCutAng2),	1,
							-lHndWid / 2,		lLen - (lHndWid / 2) * tan(aCutAng2),	1,
							-lHndWid / 2,		-(lHndWid / 2) * tan(aCutAng1),			1,
							lHndWid / 2,		(lHndWid / 2) * tan(aCutAng1),			1
			

!  Rails  ****************************

	line_type ltRai
	drawindex 20

	poly2_b 	5,1+2,SYMB_FILL_PEN,	SYMB_FBGD_PEN,

				0.0225, 	0.0225 * tan(aCutAng1), 		1,
				0.0225,  	lLen + 0.0225 * tan(aCutAng2), 	1,
				-0.0225, 	lLen - 0.0225 * tan(aCutAng2), 	1,
				-0.0225, 	-0.0225 * tan(aCutAng1),		1,
				0.0225, 	0.0225 * tan(aCutAng1), 		1

!  Gate Frame Posts  ****************************

	if tPnlTyp = `Pool Gate` then
		line2 0.0225, 	0.07,  			-0.0225, 	0.07
		line2 0.0225, 	lLen - 0.07,  	-0.0225, 	lLen - 0.07
	endif

!  Poles and Cables  ****************************

	if bPolCbl2d then
		add2 0, lLen / 2 - lBalTruLen / 2
	
		for j = 1 to iNodNum
			if strstr (" " + tPolPos + " ", " " + str(j, 1, 0)+ " ")  then
				circle2 0, 	0, 				0.0125
			else
				circle2 0, 	0, 				0.00125
			endif
	
			hotspot2 0, 	0
		add2 0, 0.064
		next j
	
		del iNodNum
	
		del 1
	endif


!  Hotspots  *************************

	aux = 0.0225 + (lHndWid / 2 - 0.0225) * bHnd

	hotspot2 0, 		0
	hotspot2 0, 		lLen
	hotspot2 -aux, 		-aux * tan(aCutAng1)
	hotspot2 aux, 		aux * tan(aCutAng1)
	hotspot2 -aux, 		lLen - aux * tan(aCutAng2)
	hotspot2 aux, 		lLen + aux * tan(aCutAng2)

	hotline2 0, 0, 0, lLen

	if bHotEdt then
		hotspot2 0, 	0, 		uid, lLen, 1+256 	: uid = uid+1    ! base
		hotspot2 0, 	lLen, 	uid, lLen, 2 		: uid = uid+1    ! moving     
		hotspot2 0, 	-1, 	uid, lLen, 3 		: uid = uid+1    ! ref

		hotspot2 -lHndWid / 2, 			lLen / 2, 	uid, lHndWid, 1 	: uid = uid+1    ! base
		hotspot2 lHndWid / 2, 			lLen / 2, 	uid, lHndWid, 2 	: uid = uid+1    ! moving     
		hotspot2 -1 - lHndWid / 2, 		lLen / 2, 	uid, lHndWid, 3 	: uid = uid+1    ! ref
	
		hotspot2 aux, 	0, 						uid, aCutAng1, 4+128 	: uid = uid+1    ! base
		hotspot2 aux, 	aux * tan(aCutAng1), 	uid, aCutAng1, 5 		: uid = uid+1    ! moving     
		hotspot2 0, 	0, 						uid, aCutAng1, 6 		: uid = uid+1    ! center
	
		add2 0, lLen
	
		hotspot2 aux, 	0, 						uid, aCutAng2, 4+128 	: uid = uid+1    ! base
		hotspot2 aux, 	aux * tan(aCutAng2), 	uid, aCutAng2, 5 		: uid = uid+1    ! moving     
		hotspot2 0, 	0, 						uid, aCutAng2, 6 		: uid = uid+1    ! center
	
		del 1
	endif

END
  )


(define 3d #<<END
! Balustrade_Sentrel.gsm Object
! Jan 2011
! 3d Script

	resol iRes

!  HandRail  ****************************

	if bHnd then
		material mHandrail
	
		add 0, 0, lHig
		XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
				0, 	1, 	0, 	0,
				0, 	tan(aSlo), 	1 / cos(aSlo), 	0
		rotx -90
	
		sprism_{2} 	mHandrail, mHandrail, mHandrail,
					9,
					0, 0, 	0, -1, 	lLen, -aCutAng2,
					0, 0, 	0, 1, 	0, aCutAng1,
					-lHndWid / 2,   0.035,      15, mHandrail, 
					-0.023,       	0.035,      15, mHandrail, 
					-0.023,      	0.0295,     15, mHandrail, 
					0.023,      	0.0295,     15, mHandrail, 
					0.023,       	0.035,      15, mHandrail, 
					lHndWid / 2,    0.035,      15, mHandrail, 
					lHndWid / 2,    0,      	15, mHandrail, 
					-lHndWid / 2,   0,      	15, mHandrail, 
					-lHndWid / 2,   0.035,      15, mHandrail
	
		gosub 200:
	
		del 3
	endif


!  Bottom Rail  ****************************

	material mRail

	add 0, 0, lBotRaiGap
	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
		0, 	1, 	0, 	0,
		0, 	0, 	1, 	0
	rotx -90

	sprism_{2} 	mRail, mRail, mRail,
				5,
				0, 0, 	0, -1, 	lLen, -aCutAng2,
				0, 0, 	0, 1, 	0, aCutAng1,
				-0.0225,    -0.07,      15, mRail, 
				0.0225,     -0.07,      15, mRail, 
				0.0225,     0,      	15, mRail, 
				-0.0225,    0,      	15, mRail, 
				-0.0225,    -0.07,      15, mRail

	gosub 100:

	del 3


!  Top Rail  ****************************

	add 0, 0, lHig - (0.030 / cos(aSlo)) * bHnd
	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
			0, 	1, 	0, 	0,
			0, 	tan(aSlo), 	1 / cos(aSlo), 	0
	rotx -90

	sprism_{2} 	mRail, mRail, mRail,
				5,
				0, 0, 	0, -1, 	lLen, -aCutAng2,
				0, 0, 	0, 1, 	0, aCutAng1,
				-0.0225,    0.07,   15, mRail, 
				0.0225,     0.07,  	15, mRail, 
				0.0225,     0,      15, mRail, 
				-0.0225,    0,     	15, mRail, 
				-0.0225,    0.07, 	15, mRail

	gosub 100:

	del 3


!  Gate Frame Posts  ****************************

	if tPnlTyp = `Pool Gate` then
		add 0, 0, lBotRaiGap + 0.07 / cos(aSlo)
		XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
				0, 	1, 	0, 	0,
				0, 	tan(0), 	1, 	0

		prism_	4, lCblHig * cos(aSlo),
				0.0225, 	0.0225 * tan(aCutAng1), 		15,
				0.0225, 	0.07, 	15,
				-0.0225, 	0.07, 	15,
				-0.0225, 	-0.0225 * tan(aCutAng1), 		15

			gosub 300:

		add 0, lLen, 0

		prism_	4, lCblHig * cos(aSlo),
				0.0225, 	0.0225 * tan(aCutAng2), 		15,
				0.0225, 	-0.07, 	15,
				-0.0225, 	-0.07, 	15,
				-0.0225, 	-0.0225 * tan(aCutAng2), 		15

			gosub 300:


		del 3
	endif


!  Poles and Cables  ****************************

	add 0, lLen / 2 - lBalTruLen / 2, lBotRaiGap + 0.07 / cos(0) + (lLen / 2 - lBalTruLen / 2) * tan(0)
	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
		0, 	1, 	0, 	0,
		0, 	tan(0),	1, 	0

	for j = 1 to iNodNum
		if strstr (" " + tPolPos + " ", " " + str(j, 1, 0)+ " ")  then
			material mPole

			cylind (lCblHig + (lLen / 2 - lBalTruLen / 2 + 0.064 * j) * tan(aSlo)), 0.0125

			gosub 300:

		else
			material mCable

			cylind (lCblHig + (lLen / 2 - lBalTruLen / 2 + 0.064 * j) * tan(aSlo)), 0.00125

			gosub 300:

		endif

!		hotspot 0, 	0, 0
!		hotspot 0, 	0, lCblHig
	add 0, 0.064, 0
	next j

	del iNodNum

	del 2



!  Hotspots  *************************

	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
			0, 	1, 	0, 	0,
			0, 	tan(aSlo), 	1, 	0 

	hotspot 0, 					0, 										lHig
	hotspot 0, 					lLen, 									lHig
	hotspot 0, 					lLen / 2, 								lHig
	hotspot 0, 					lLen / 2, 								0
	hotspot 0, 					lLen / 2, 								lBotRaiGap

	if bHnd then
		hotspot -(lHndWid / 2), 	-(lHndWid / 2) * tan(aCutAng1), 		lHig
		hotspot (lHndWid / 2), 		(lHndWid / 2) * tan(aCutAng1), 			lHig
		hotspot -(lHndWid / 2), 	lLen - (lHndWid / 2) * tan(aCutAng2), 	lHig
		hotspot (lHndWid / 2), 		lLen + (lHndWid / 2) * tan(aCutAng2), 	lHig
	
		hotspot -(lHndWid / 2), 	-(lHndWid / 2) * tan(aCutAng1), 		lHig - 0.035 / cos(aSlo)
		hotspot (lHndWid / 2), 		(lHndWid / 2) * tan(aCutAng1), 			lHig - 0.035 / cos(aSlo)
		hotspot -(lHndWid / 2), 	lLen - (lHndWid / 2) * tan(aCutAng2), 	lHig - 0.035 / cos(aSlo)
		hotspot (lHndWid / 2), 		lLen + (lHndWid / 2) * tan(aCutAng2), 	lHig - 0.035 / cos(aSlo)
	endif

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lHig - (0.035 / cos(aSlo)) * bHnd
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lHig - (0.035 / cos(aSlo)) * bHnd
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lHig - (0.035 / cos(aSlo)) * bHnd
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lHig - (0.035 / cos(aSlo)) * bHnd

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lBotRaiGap
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lBotRaiGap
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lBotRaiGap
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lBotRaiGap

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lBotRaiGap + 0.07 / cos(aSlo)
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lBotRaiGap + 0.07 / cos(aSlo)
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lBotRaiGap + 0.07 / cos(aSlo)
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lBotRaiGap + 0.07 / cos(aSlo)

	del 1

	if bHotEdt then
		hotspot 0, 0,    0, uid, lLen, 1+256 	: uid = uid+1    ! base
		hotspot 0, lLen, 0, uid, lLen, 2 		: uid = uid+1    ! moving
		hotspot 0, -1,   0, uid, lLen, 3 		: uid = uid+1    ! ref

		hotspot 0, 0, 0, 	uid, lHig, 1 	: uid = uid+1    ! base
		hotspot 0, 0, lHig, uid, lHig, 2 	: uid = uid+1    ! moving
		hotspot 0, 0, -1, 	uid, lHig, 3 	: uid = uid+1    ! ref

		hotspot 0, lLen, lSlo, 			uid, lHig, 1 	: uid = uid+1    ! base
		hotspot 0, lLen, lSlo + lHig, 	uid, lHig, 2 	: uid = uid+1    ! moving
		hotspot 0, lLen, lSlo - 1, 		uid, lHig, 3 	: uid = uid+1    ! ref

		hotspot 0, 0, 0, 			uid, lBotRaiGap, 1 	: uid = uid+1    ! base
		hotspot 0, 0, lBotRaiGap, 	uid, lBotRaiGap, 2 	: uid = uid+1    ! moving
		hotspot 0, 0, -1, 			uid, lBotRaiGap, 3 	: uid = uid+1    ! ref

		hotspot 0, lLen, 0, 									uid, lSlo, 1 	: uid = uid+1    ! base
		hotspot 0, lLen, lSlo + lBotRaiGap / 2 * not(lSlo), 	uid, lSlo, 2 	: uid = uid+1    ! moving
		hotspot 0, lLen, -1, 									uid, lSlo, 3 	: uid = uid+1    ! ref

	endif


end  !**************************

100:
	addz 0.2 * rnd(5)
	
	base
	vert 0, 0, 0
	vert 1, 0,0
	vert 0, 1, 0
	vert 0, 0, 1
	coor 2+256, -1, -2, -3, -4
	body 1
	
	del 1
return

200:
	addz 0.2 * rnd(5)
	
	base
	vert 0, 0, 0
	vert 0, 1, 0
	vert 1, 0, 0
	vert 0, 0, -1
	coor 2+256, -1, -2, -3, -4
	body 1
	
	del 1
return


300:
	addz 0.2 * rnd(5)
	
	base
	vert 0, 0, 0
	vert 0, 1, 0
	vert 1, 0, 0
	vert 0, 0, -1
	coor 2+256, -1, -2, -3, -4
	body 1
	
	del 1
return

END
  )

(define new-3d #<<END
! Balustrade_Sentrel.gsm Object
! Jan 2011
! 3d Script

	resol iRes

!  HandRail  ****************************

	if bHnd then
		material mHandrail
	
		add 0, 0, lHig
		XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
				0, 	1, 	0, 	0,
				0, 	tan(aSlo), 	1 / cos(aSlo), 	0
		rotx -90
	
		sprism_{2} 	mHandrail, mHandrail, mHandrail,
					9,
					0, 0, 	0, -1, 	lLen, -aCutAng2,
					0, 0, 	0, 1, 	0, aCutAng1,
					-lHndWid / 2,   0.035,      15, mHandrail, 
					-0.023,       	0.035,      15, mHandrail, 
					-0.023,      	0.0295,     15, mHandrail, 
					0.023,      	0.0295,     15, mHandrail, 
					0.023,       	0.035,      15, mHandrail, 
					lHndWid / 2,    0.035,      15, mHandrail, 
					lHndWid / 2,    0,      	15, mHandrail, 
					-lHndWid / 2,   0,      	15, mHandrail, 
					-lHndWid / 2,   0.035,      15, mHandrail
	
		gosub 200:
	
		del 3
	endif

!  Bottom Rail  ****************************

	material mRail

	add 0, 0, lBotRaiGap
	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
		0, 	1, 	0, 	0,
		0, 	0, 	1, 	0
	rotx -90

	sprism_{2} 	mRail, mRail, mRail,
				5,
				0, 0, 	0, -1, 	lLen, -aCutAng2,
				0, 0, 	0, 1, 	0, aCutAng1,
				-0.0225,    -lHig + 0.07,      15, mRail, 
				0.0225,     -lHig + 0.07,      15, mRail, 
				0.0225,     0,      	15, mRail, 
				-0.0225,    0,      	15, mRail, 
				-0.0225,    -0.07,      15, mRail

	gosub 100:

	del 3


!  Top Rail  ****************************

	add 0, 0, lHig - (0.030 / cos(aSlo)) * bHnd
	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
			0, 	1, 	0, 	0,
			0, 	tan(aSlo), 	1 / cos(aSlo), 	0
	rotx -90

	sprism_{2} 	mRail, mRail, mRail,
				5,
				0, 0, 	0, -1, 	lLen, -aCutAng2,
				0, 0, 	0, 1, 	0, aCutAng1,
				-0.0225,    0.07,   15, mRail, 
				0.0225,     0.07,  	15, mRail, 
				0.0225,     0,      15, mRail, 
				-0.0225,    0,     	15, mRail, 
				-0.0225,    0.07, 	15, mRail

	gosub 100:

	del 3


!  Gate Frame Posts  ****************************

	if tPnlTyp = `Pool Gate` then
		add 0, 0, lBotRaiGap + 0.07 / cos(aSlo)
		XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
				0, 	1, 	0, 	0,
				0, 	tan(0), 	1, 	0

		prism_	4, lCblHig * cos(aSlo),
				0.0225, 	0.0225 * tan(aCutAng1), 		15,
				0.0225, 	0.07, 	15,
				-0.0225, 	0.07, 	15,
				-0.0225, 	-0.0225 * tan(aCutAng1), 		15

			gosub 300:

		add 0, lLen, 0

		prism_	4, lCblHig * cos(aSlo),
				0.0225, 	0.0225 * tan(aCutAng2), 		15,
				0.0225, 	-0.07, 	15,
				-0.0225, 	-0.07, 	15,
				-0.0225, 	-0.0225 * tan(aCutAng2), 		15

			gosub 300:


		del 3
	endif


!  Poles and Cables  ****************************

	add 0, lLen / 2 - lBalTruLen / 2, lBotRaiGap + 0.07 / cos(0) + (lLen / 2 - lBalTruLen / 2) * tan(0)
	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
		0, 	1, 	0, 	0,
		0, 	tan(0),	1, 	0

	for j = 1 to iNodNum
		if strstr (" " + tPolPos + " ", " " + str(j, 1, 0)+ " ")  then
			material mPole

			cylind (lCblHig + (lLen / 2 - lBalTruLen / 2 + 0.064 * j) * tan(aSlo)), 0.0125

			gosub 300:

		else
			material mCable

			cylind (lCblHig + (lLen / 2 - lBalTruLen / 2 + 0.064 * j) * tan(aSlo)), 0.00125

			gosub 300:

		endif

!		hotspot 0, 	0, 0
!		hotspot 0, 	0, lCblHig
	add 0, 0.064, 0
	next j

	del iNodNum

	del 2



!  Hotspots  *************************

	XFORM 	1, 	0, 	0, 	0,										!Skew the balustrade for stairs
			0, 	1, 	0, 	0,
			0, 	tan(aSlo), 	1, 	0 

	hotspot 0, 					0, 										lHig
	hotspot 0, 					lLen, 									lHig
	hotspot 0, 					lLen / 2, 								lHig
	hotspot 0, 					lLen / 2, 								0
	hotspot 0, 					lLen / 2, 								lBotRaiGap

	if bHnd then
		hotspot -(lHndWid / 2), 	-(lHndWid / 2) * tan(aCutAng1), 		lHig
		hotspot (lHndWid / 2), 		(lHndWid / 2) * tan(aCutAng1), 			lHig
		hotspot -(lHndWid / 2), 	lLen - (lHndWid / 2) * tan(aCutAng2), 	lHig
		hotspot (lHndWid / 2), 		lLen + (lHndWid / 2) * tan(aCutAng2), 	lHig
	
		hotspot -(lHndWid / 2), 	-(lHndWid / 2) * tan(aCutAng1), 		lHig - 0.035 / cos(aSlo)
		hotspot (lHndWid / 2), 		(lHndWid / 2) * tan(aCutAng1), 			lHig - 0.035 / cos(aSlo)
		hotspot -(lHndWid / 2), 	lLen - (lHndWid / 2) * tan(aCutAng2), 	lHig - 0.035 / cos(aSlo)
		hotspot (lHndWid / 2), 		lLen + (lHndWid / 2) * tan(aCutAng2), 	lHig - 0.035 / cos(aSlo)
	endif

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lHig - (0.035 / cos(aSlo)) * bHnd
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lHig - (0.035 / cos(aSlo)) * bHnd
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lHig - (0.035 / cos(aSlo)) * bHnd
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lHig - (0.035 / cos(aSlo)) * bHnd

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lHig - (0.07 + 0.03 * bHnd) / cos(aSlo)

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lBotRaiGap
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lBotRaiGap
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lBotRaiGap
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lBotRaiGap

	hotspot -0.0175, 	-0.0175 * tan(aCutAng1), 		lBotRaiGap + 0.07 / cos(aSlo)
	hotspot 0.0175, 	0.0175 * tan(aCutAng1), 		lBotRaiGap + 0.07 / cos(aSlo)
	hotspot -0.0175, 	lLen - 0.0175 * tan(aCutAng2), 	lBotRaiGap + 0.07 / cos(aSlo)
	hotspot 0.0175, 	lLen + 0.0175 * tan(aCutAng2), 	lBotRaiGap + 0.07 / cos(aSlo)

	del 1

	if bHotEdt then
		hotspot 0, 0,    0, uid, lLen, 1+256 	: uid = uid+1    ! base
		hotspot 0, lLen, 0, uid, lLen, 2 		: uid = uid+1    ! moving
		hotspot 0, -1,   0, uid, lLen, 3 		: uid = uid+1    ! ref

		hotspot 0, 0, 0, 	uid, lHig, 1 	: uid = uid+1    ! base
		hotspot 0, 0, lHig, uid, lHig, 2 	: uid = uid+1    ! moving
		hotspot 0, 0, -1, 	uid, lHig, 3 	: uid = uid+1    ! ref

		hotspot 0, lLen, lSlo, 			uid, lHig, 1 	: uid = uid+1    ! base
		hotspot 0, lLen, lSlo + lHig, 	uid, lHig, 2 	: uid = uid+1    ! moving
		hotspot 0, lLen, lSlo - 1, 		uid, lHig, 3 	: uid = uid+1    ! ref

		hotspot 0, 0, 0, 			uid, lBotRaiGap, 1 	: uid = uid+1    ! base
		hotspot 0, 0, lBotRaiGap, 	uid, lBotRaiGap, 2 	: uid = uid+1    ! moving
		hotspot 0, 0, -1, 			uid, lBotRaiGap, 3 	: uid = uid+1    ! ref

		hotspot 0, lLen, 0, 									uid, lSlo, 1 	: uid = uid+1    ! base
		hotspot 0, lLen, lSlo + lBotRaiGap / 2 * not(lSlo), 	uid, lSlo, 2 	: uid = uid+1    ! moving
		hotspot 0, lLen, -1, 									uid, lSlo, 3 	: uid = uid+1    ! ref

	endif


end  !**************************

100:
	addz 0.2 * rnd(5)
	
	base
	vert 0, 0, 0
	vert 1, 0,0
	vert 0, 1, 0
	vert 0, 0, 1
	coor 2+256, -1, -2, -3, -4
	body 1
	
	del 1
return

200:
	addz 0.2 * rnd(5)
	
	base
	vert 0, 0, 0
	vert 0, 1, 0
	vert 1, 0, 0
	vert 0, 0, -1
	coor 2+256, -1, -2, -3, -4
	body 1
	
	del 1
return


300:
	addz 0.2 * rnd(5)
	
	base
	vert 0, 0, 0
	vert 0, 1, 0
	vert 1, 0, 0
	vert 0, 0, -1
	coor 2+256, -1, -2, -3, -4
	body 1
	
	del 1
return

END
  )

(define pc #<<END
! Balustrade_Sentrel.gsm Object
! Jan 2011
! Parameter Script

! Values ***************

	values "iRes" 			RANGE[0,]
	if tPnlTyp = `Balustrade` then values "lLen" RANGE[0.136,2.696]
	if tPnlTyp = `Pool Fence` then values "lLen" RANGE[0.314,2.501]
	if tPnlTyp = `Pool Gate`  then values "lLen" 0.9
	values "lHig" 			RANGE[0.6,]
	values "lHndWid" 		0.07, 0.09, 0.11
	values "lBotRaiGap" 	RANGE[0.08,0.1]
	values "tModNam"		tModNam
	values "tPnlTyp" 		`Balustrade`, `Pool Fence`, `Pool Gate`



! Parameters ********************

	parameters tModNam = tModNam


END
  )

(define additional-p
  (list (list "BO_mainmat" "Wood")
        (list "BO_secmat" "Stainless steel")
        (list "BO_bocat" "Railing")
        (list "BO_ifcclas" "Railing")
        ;...
        (list "tPnlTyp" "Balustrade")
        (list "lLen" 0.9)
        (list "lHig" 1)
        (list "lSlo" 0)
        (list "lHndWid" 0.110)
        (list "lBotRaiGap" 0.080)
        (list "aCutAng1" 0)
        (list "aCutAng2" 0)
        (list "bHnd" #t)
        (list "iRes" 16)
        (list "bPolCbl2d" #t)
        (list "bHotEdt" #t)
        (list "ltHnd" "LineType")
        (list "ltRai" "LineType")
        (list "mCable" "MaterialType")
        (list "mPole" "MaterialType")
        (list "mHandrail" (list "MaterialType" "Metal - Gold"))
        (list "mRail" "MaterialType")
        (list "tModNam" "BP06")
        
        ))

(define additional-p-new
  (list "BO_mainmat" "Wood"
        "BO_secmat" "Stainless steel"
        "BO_bocat" "Railing"
        "BO_ifcclas" "Railing"
        ;...
        "tPnlTyp" "Balustrade"
        "lLen" 0.9
        "lHig" 1
        "lSlo" 0
        "lHndWid" 0.110
        "lBotRaiGap" 0.080
        "aCutAng1" 0
        "aCutAng2" 0
        "bHnd" #t
        "iRes" 16
        "bPolCbl2d" #t
        "bHotEdt" #t
        "ltHnd" "LineType"
        "ltRai" "LineType"
        "mCable" "MaterialType"
        "mPole" "MaterialType"
        "mHandrail" (list "MaterialType" "Metal - Gold")
        "mRail" "MaterialType"
        "tModNam" "BP06"))

