#lang racket
(provide (except-out (all-defined-out)
                     ))

(require "install.rkt")
(require rosetta/revit)
(require "rosetta/protobuf1/protobuf.rkt")
(require "rosetta/protobuf1/syntax.rkt")
(require "rosetta/protobuf1/encoding.rkt")
(require srfi/26)
(require "Messages.rkt")
(require racket/date)

(define do-not-install #f)

(unless do-not-install
    (move-addon))

(define current-level (make-parameter #f))
(define default-level-to-level-height (make-parameter 3))

(define DEGRAD (/ pi 180.0))

(define input #f)
(define output #f)
(define server-addr "localhost")

(define (connect)
  (ensure-connection))

(define (ensure-connection)
  (let ((time-out-tries 10))
    (let rec ((n time-out-tries))
      (with-handlers ([exn:fail?
                       (lambda (e)
                         (if (> n 0)
                             (begin
                               (displayln (string-append "[" (number->string (+ 1 (- time-out-tries n))) "/" (number->string time-out-tries) "] "
                                                         "You must first use the connect button on ArchiCAD.\n Try in the top bar, Addon->Racket->Connect")) 
                               (sleep 2)
                               (rec (- n 1)))
                             (raise e)))])
        (start-connection)))))

(define (start-connection)
  (begin
    (call-with-values(lambda () (tcp-connect server-addr 53800))
                     (lambda (a b)
                       (set! input a)
                       (set! output b)
                       (file-stream-buffer-mode input 'none)
                       (file-stream-buffer-mode output 'none)
                       (set! current-level (make-parameter (check-level)))))))

;;Usage: (send (create-...) (create-...) ... )
(define-syntax-rule (send expr ...)
  (begin
    (connect)
    (parameterize ((current-level (check-level)))
      expr ...)
    (disconnect)))

;;Auxiliar Functions;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;Function to send name 
(define (write-msg-name name)
  (write-sized serialize (namemessage* #:name name) output))

;;Function to call 
;;a function with a 'name' and 'strct'
(define (write-msg name strct)
  (write-msg-name name)
  (write-sized serialize strct output))

;;Function to send a double
(define (send-double d)
  (write-sized serialize (doublemessage* #:d d) output))

;;Function to send list of points
(define (send-list-points lst)
  (for-each (lambda (point)
                (write-sized serialize (pointmessage* #:p0x (car point) 
                                                      #:p0y (cdr point)) output)) 
              lst))

;;Function to send list of points using repeated fields
(define (send-points-old lst)
  (let ((lst-x (list))
        (lst-y (list))
        (lst-z (list)))
    (for-each (lambda (point)
               (set! lst-x (append lst-x (list (car point))))
               (set! lst-y (append lst-y (list (cdr point)))))
              lst)
    (write-sized serialize (pointsmessage* #:px lst-x 
                                           #:py lst-y 
                                           #:pz lst-z) output)))

;;Function to send list of points using repeated fields with XYZ Rosetta implementation
(define (send-points lst)
  (let ((lst-x (list))
        (lst-y (list))
        (lst-z (list)))
    (for-each (lambda (point)
               (set! lst-x (append lst-x (list (cx point))))
               (set! lst-y (append lst-y (list (cy point))))
               (set! lst-z (append lst-z (list (cz point)))))
              lst)
    (write-sized serialize (pointsmessage* #:px lst-x 
                                           #:py lst-y 
                                           #:pz lst-z) output)))

;;Function to send list of arcs
(define (send-list-arcs lst)
  (for-each (lambda (arc)
                (write-sized serialize (polyarcmessage* #:begindex (car arc) 
                                                        #:endindex (car (cdr arc)) 
                                                        #:arcangle (car (cdr (cdr arc)))) output)) 
              lst))

;;Function to send list of arcs using repeated fields
(define (send-arcs-complex lst)
  (let ((lst-beg-index (list))
        (lst-end-index (list))
        (lst-arc-angle (list)))
    (for-each (lambda (arc)
               (set! lst-beg-index (append lst-beg-index (list (car arc))))
               (set! lst-end-index (append lst-end-index (list (car (cdr arc)))))
               (set! lst-arc-angle (append lst-arc-angle (list (car (cdr (cdr arc)))))))
              lst)
    (write-sized serialize (polyarcsmessage* #:begindex lst-beg-index 
                                           #:endindex lst-end-index 
                                           #:arcangle lst-arc-angle) output)))

#|
 Function to send list of arcs with assumed order
 The first arc will be applied to the line that is formed 
 by the first and second point. The logic is applied to
 the following arcs.
 Ex.: List with two arcs, will apply the first arc to
      line(p1, p2), and the second arc to line(p2, p3)
|#
(define (send-arcs lst)
  (let ((lst-beg-index (list))
        (lst-end-index (list))
        (lst-arc-angle (list))
        (index 1))
    (for-each (lambda (arc)
               (set! lst-beg-index (append lst-beg-index (list index)))
               (set! lst-end-index (append lst-end-index (list (+ index 1))))
               (set! lst-arc-angle (append lst-arc-angle (list  arc)))
               (set! index (+ index 1)))
              lst)
    (write-sized serialize (polyarcsmessage* #:begindex lst-beg-index 
                                           #:endindex lst-end-index 
                                           #:arcangle lst-arc-angle) output)))

#|
Function to update file materials
The file is merely used for consulting
(send (update-material-file))
|#
(define (update-material-file)
  (write-msg-name "WriteMaterialsFile"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define crash-on-no-material? (make-parameter #t))
(define crash-on-no-name? (make-parameter #t))
;;Functions to create objects;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

#|
Function used internally to create-walls
No need to use this function
To create walls use the two below.
|#

(define default-wall-alignment (make-parameter "Center"))
(define default-wall-type-of-material (make-parameter "Basic"))
(define default-wall-thickness (make-parameter 0.3))
#;(define default-wall-material  
    (cond [(eq? (default-wall-type-of-material) "Basic") (make-parameter "GENERIC - STRUCTURAL")]
          [(eq? (default-wall-type-of-material) "Composite") (make-parameter "Generic Wall/Shell")]))
#|
Can be:
Normal |
Slanted - alpha angle needed \
DoubleSlanted - alpha and beta angle needed /\
|#
(define default-wall-profile (make-parameter "Normal"))

#|
Function used to create a wall
 
 TODO information
 returns: id of the created wall

Example of usage:
(send (create-wall (list (xy 0 0)(xy 10 0))))
(send (create-wall (list (xy 0 0)(xy 10 0)) #:type-of-profile "Slanted" #:alpha-angle (* 80 DEGRAD)))
(send (create-wall (list (xy 0 0)(xy 10 0)) #:type-of-profile "DoubleSlanted" #:alpha-angle (* 100 DEGRAD) #:beta-angle (* 80 DEGRAD)))


|#
(define (create-wall guide 
              #:alignment [alignment (default-wall-alignment)]
              #:bottom-level [bottom-level (current-level)]
              #:top-level [top-level (upper-level #:level bottom-level)]
              ;;ArchiCAD ONLY --------------------------------------------------------------
              #:thickness [thickness (default-wall-thickness)]
              #:angle [angle 0]
              
              #:type-of-material [type-of-material (default-wall-type-of-material)]
              #:material [material 
                          (cond [(eq? type-of-material "Basic") "GENERIC - STRUCTURAL"]
                                [(eq? type-of-material "Composite") "Generic Wall/Shell"])]
              #:alpha-angle [alpha-angle (/ pi 2)]
              #:beta-angle [beta-angle (/ pi 2)]
              #:type-of-profile [type-of-profile (default-wall-profile)])
  (let* ((p0 (x 0))
         (p1 (x 0))
         (msg (wallmsg* #:p0x (cx p0)
                        #:p0y (cy p0)
                        #:p1x (cx p1)
                        #:p1y (cy p1)
                        #:bottomindex (storyinfo-index bottom-level)
                        #:upperindex (storyinfo-index top-level)
                        #:thickness thickness
                        #:angle angle
                        #:material material
                        #:type type-of-material
                        #:referenceline alignment
                        #:alphaangle alpha-angle
                        #:betaangle beta-angle
                        #:typeprofile type-of-profile)))
    (write-msg "NewWall" msg)
    (send-points guide)
    (let ((result (read-sized (cut deserialize (elementidlist*) <>)input)))
      (if (and (elementidlist-crashmaterial result) 
               (crash-on-no-material?))
          (begin 
            (disconnect)
            (error (string-append "The material does not exist - " material)))
          (if (null? (cdr (elementidlist-guid result)))
              (car (elementidlist-guid result))
              (elementidlist-guid result))))))

#|
Function used to create a door into an existing wall

 guid: id of wall, this value is returned by the functions that create walls
       (create-wall ...)
 objloc: object location in the wall
 zpos: z of the door

 returns: door id 

Example of usage:
(send (create-door wallId 1.0 0.0))
|#

(define (create-door guid objloc [width -10000] [bottom 0] [height -10000])
  (let ((door-to-create (doormessage* #:guid guid
                                      #:objloc objloc
                                      #:zpos bottom
                                      #:height height
                                      #:width width
                                      #:hole #f
                                      )))
    (write-msg "Door" door-to-create)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

(define (create-hole-in-wall guid objloc [width -10000] [bottom 0] [height -10000])
  (let ((door-to-create (doormessage* #:guid guid
                                      #:objloc objloc
                                      #:zpos bottom
                                      #:height height
                                      #:width width
                                      #:hole #t
                                      )))
    (write-msg "Door" door-to-create)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

(define (create-hole-in-wall-test guid list-points [list-arcs (list)])
  (let ((msg (holemsg* #:guid guid )))
    (write-msg "HoleTest" msg)
    (send-points list-points)
    (send-arcs list-arcs)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#|
Function used to create a window into a existing wall
 
 guid: id of wall, this value is returned by the functions that create walls
       (create-wall ...)
 objloc: object location in the wall
 zpos: z of the window

 returns: window id 

Example of usage:
(send (create-window wallId 1.0 1.0))
|#
(define (create-window guid objloc zpos)
  (let ((window-to-create (windowmessage* #:guid guid
                                          #:objloc objloc
                                          #:zpos zpos)))
    (write-msg "Window" window-to-create)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#|
Function used to create a circle
 
 p: center of circle
 radius: radius of circle
 
 returns: circle id

Example of usage: 
(send (create-circle (xy 0 0) 1))
|#
(define (create-circle p radius)
  (let ((circle-to-create (circlemessage* #:p0x (cx p)
                                          #:p0y (cy p)
                                          #:radius radius)))
    (write-msg "Circle" circle-to-create)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#| 
Function used to create an arc

Examples of usage:
create an arc with center (0,0), radius 1,
 with an amplitude of 90º, on the 1º quadrant (x+, y+)
(send (create-arc (xy 0 0) 1 0 0 (* 90 DEGRAD)))

by using begang instead of endang, we get the opposite result,
the arc will be the opposite of the previous result. 
(send (create-arc (xy 0 0) 1 0 (* 90 DEGRAD) 0))

|#
(define (create-arc p radius angle begang endang)
  (let ((arc-to-create (arcmessage* #:p0x (cx p)
                                    #:p0y (cy p)
                                    #:radius radius
                                    #:angle angle
                                    #:begang begang
                                    #:endang endang)))
    (write-msg "Arc" arc-to-create)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#| 
Function used to create a sphere

Example of usage: 
(send (create-sphere (xy 0 0) 1))
|#
(define (create-sphere p radius #:level [level (current-level)])
  (let ((sphere-to-create (spheremessage* #:c0x (cx p)
                                          #:c0y (cy p)
                                          #:c0z (cz p)
                                          #:radius radius
                                          #:level (storyinfo-index level))))
    (write-msg "Sphere" sphere-to-create)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))


#|
Function to create a Complex Shell
Example of usage: (create-complex-shell tmat lstpoints lstarcs 1 hpoints harcs hheight htmat 1.0 0.0)
|#
(define (create-complex-shell transmat listpoints listarcs numholes holepoints holearcs holeheight holetransmat reflectx reflecty)
  (let ((shell-to-create (shellcomplexmessage* #:numpoints (length listpoints)
                                               #:numarcs (length listarcs)
                                               #:numholes numholes
                                               #:numhpoints (length holepoints)
                                               #:numharcs (length holearcs)
                                               #:holeheight holeheight
                                               #:reflectx reflectx
                                               #:reflecty reflecty)))
    (write-msg "ComplexShell" shell-to-create)
    (for-each (lambda (point)
                (write-sized serialize (doublemessage* #:d point) output)) 
              transmat)
    
    (for-each (lambda (point)
                (write-sized serialize (pointmessage* #:p0x (car point) #:p0y (cdr point)) output)) 
              listpoints)
    
    (for-each (lambda (arc)
                (write-sized serialize (polyarcmessage* #:begindex (car arc) #:endindex (car (cdr arc)) #:arcangle (car (cdr (cdr arc)))) output)) 
              listarcs)
    
    (for-each (lambda (point)
                (write-sized serialize (pointmessage* #:p0x (car point) #:p0y (cdr point)) output)) 
              holepoints)
    
    (for-each (lambda (arc)
                (write-sized serialize (polyarcmessage* #:begindex (car arc) #:endindex (car (cdr arc)) #:arcangle (car (cdr (cdr arc)))) output)) 
              holearcs)
    
    (for-each (lambda (point)
                (write-sized serialize (doublemessage* #:d point) output)) 
              holetransmat)
    
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#|
Function to create a Simple Shell
Example of usage: (create-simple-shell lstpoints)
|#
(define (create-simple-shell listpoints)
  (create-shell listpoints (list)))

#|
Function to create a Shell
This is the most primitive shell we can create 
Example of usage: (create-shell lstpoints lstarcs)
|#
(define (create-shell listpoints listarcs)
  (let ((shell-to-create (shellmessage* #:numpoints (length listpoints)
                                        #:numarcs (length listarcs))))
    (write-msg "Shell" shell-to-create)
    (send-points listpoints)
    (send-arcs-complex listarcs)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))
#|
Function to rotate a Shell
Receives the axis in which the shell will rotate, the angle and the shellId
Example of usage: (rotate-shell "x" 90 shellId)
|#
(define (rotate-shell axis angle shellId)
  (let ((rot-shell-msg (rotshellmessage* #:axis axis
                                         #:angle angle
                                         #:guid shellId)))
    (write-msg "RotateShell" rot-shell-msg)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#|
Function to translate a Shell
Receives a point that represents the translation and the shell ID
Example of usage: (translate-shell (list 0 5 0) shellId)
|#
(define (translate-shell point shellId)
  (let ((t-shell-msg (tshellmessage* #:tx (car point)
                                     #:ty (car (cdr point))
                                     #:tz (car (cdr (cdr point)))
                                     #:guid shellId)))
    (write-msg "TranslateShell" t-shell-msg)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#|
Function to create a hole on a Shell
Receives a point that represents the translation and the shell ID
Example of usage: (create-hole-on-shell hpoints harcs hheight shellId)
|#
(define (create-hole-on-shell listpoints listarcs height shellId)
  (let ((hole-msg (oldholemessage* #:height height
                                #:guid shellId)))
    (write-msg "Hole" hole-msg)
    (send-points listpoints)
    (send-arcs-complex listarcs)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    ))

#|
Function to create a Curtain Wall
 
 TODO information
 
 returns: curtain-wall id

Example of usage: 
(send (delete-stories)(create-curtain-wall (list (xy 0 0)(xy 10 0))))
|#
(define (create-curtain-wall guide
                             #:listarcs [listarcs (list)]
                             #:bottom-level [bottom-level (current-level)]
                             #:top-level [top-level (upper-level #:level bottom-level)])
  (let ((c-wall-msg (curtainwallmsg* #:numpoints (length guide)
                                     #:numarcs (length listarcs)
                                     #:bottomindex (storyinfo-index bottom-level)
                                     #:upperindex (storyinfo-index top-level))))
    (write-msg "CurtainWall" c-wall-msg)
    (send-points guide)
    (send-arcs listarcs)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input)))
  )

#| NOT WORKING
Function to add arcs to an Element

Example of usage: 
(send (add-arcs elementId cArcs))
|#
(define (add-arcs id listarcs)
  (let ((ele-id-msg (elementid* #:guid id
                                #:crashmaterial #f)))
    (write-msg "AddArcs" ele-id-msg)
    (send-arcs listarcs)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
   ))
#|
Function to create a Slab
 TODO information
 returns: slab id
Example of usage: 
(send (slab cPoints))

|#
(define default-slab-alignment (make-parameter "Center"))
(define default-slab-type-of-material (make-parameter "Composite"))
#;(define default-slab-material  
  (make-parameter (cond [(eq? (default-slab-type-of-material) "Basic") "GENERIC - INTERNAL CLADDING"]
                        [(eq? (default-slab-type-of-material) "Composite") "Generic Slab/Roof"])))
(define (create-slab guide
              #:bottom-level [bottom-level (current-level)]
              ;;ArchiCAD ONLY --------------------------------------------------------------
              #:thickness [thickness 0.3]
              #:bottom [bottom 0]
              #:type-of-material [type-of-material (default-slab-type-of-material)]
              #:material [material (cond [(eq? type-of-material "Basic") "GENERIC - INTERNAL CLADDING"]
                                         [(eq? type-of-material "Composite") "Generic Slab/Roof"])]
              #:sub-polygons [sub-polygons (list (length guide))])
  (let ((slab-msg (slabmessage* #:level bottom
                                #:material material
                                #:thickness thickness
                                #:type type-of-material
                                #:bottomlevel (storyinfo-index bottom-level)
                                #:subpolygons sub-polygons)))
    (write-msg "NewSlab" slab-msg)  
    (send-points guide)
    ;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    (let ((result (read-sized (cut deserialize (elementid*) <>)input)))
    (if (and (elementid-crashmaterial result) 
             (crash-on-no-material?))
        (begin 
          (disconnect)
          (error "The material does not exist"))
        (elementid-guid result)))))

#|
Function to create a hole on a slab
 listpoints: list of points that define the hole
             IMPORTANT: the list must end on the point that it began 
                        so it is a closed slab
 listarcs: list of eventual angles that will be applied to the hole 
           can be empty
 returns: slab id
Example of usage: 
(send (create-hole-slab (create-slab slabPoints) hole-points))
|#
(define (create-hole-slab slab-id listpoints [listarcs (list)])
  (let ((slab-msg (holemsg* #:guid slab-id)))
    (write-msg "HoleSlab" slab-msg)  
    (send-points listpoints)
    (send-arcs listarcs)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))


#|
Function to create walls from a Slab
 slab-id: id of the slab from where the walls will be created
 height: height of the walls that will be created
 material: material of the walls that will be created
 returns: a list with all the id's of the wall that were created
Example of usage: 
(send (create-walls-from-slab slabId 5.0))
|#

(define (internal-create-walls-from-slab slab-id height thickness material reference-line type)
  (let ((ele-id-msg (wallsfromslab* #:guid slab-id #:height height #:thickness thickness #:material material #:type type #:referenceline reference-line)))
    (write-msg "WallsSlab" ele-id-msg)
    ;(elementidlist-guid (read-sized (cut deserialize (elementidlist*) <>)input))
    (let ((result (read-sized (cut deserialize (elementidlist*) <>)input)))
    (if (and (elementidlist-crashmaterial result) 
             (crash-on-no-material?))
        (begin 
          (disconnect)
          (error "The material does not exist"))
        (elementidlist-guid result)))))

(define create-walls-from-slab-material-default (make-parameter "GENERIC - STRUCTURAL"))
(define create-walls-from-slab-reference-line-default (make-parameter "Center"))
(define (create-walls-from-slab slab-id [height (default-level-to-level-height)] [thickness 0.3] [material (create-walls-from-slab-material-default)] [reference-line (create-walls-from-slab-reference-line-default)])
  (internal-create-walls-from-slab slab-id height thickness material reference-line "BasicStructure"))

#|
Function to create walls from a Slab, using composite materials
 slab-id: id of the slab from where the walls will be created
 height: height of the walls that will be created
 material: material of the walls that will be created
 returns: a list with all the id's of the wall that were created
Example of usage: 
(send (create-walls-from-slab slabId 5.0))
|#
(define create-walls-from-slab-composite-material-default (make-parameter "Generic Wall/Shell"))
(define create-walls-from-slab-composite-reference-line-default (make-parameter "Center"))
(define (create-walls-from-slab-composite slab-id [height (default-level-to-level-height)] [thickness 0.3] [material (create-walls-from-slab-composite-material-default)] [reference-line (create-walls-from-slab-composite-reference-line-default)])
  (internal-create-walls-from-slab slab-id height thickness material reference-line "CompositeStructure"))

#|HEIGHT NOT WORKING
Function to create curtain walls from a Slab
 slab-id: id of the slab from where the curtain walls will be created
 height: height of the curtain walls that will be created
 returns: a list with all the id's of the curtain wall that were created
Example of usage: 
(send (create-walls-from-slab slabId))
|#
(define (create-cwalls-from-slab slabId height)
  (let ((ele-id-msg (elementid* #:guid slabId
                                #:crashmaterial #f)))
    (write-msg "CWallsSlab" ele-id-msg)
    (send-double height)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))
  
#|NOT WORKING
Function to translate an element
Receives a point that represents the translation and the object ID
Example of usage: 
(send (translate-element id (xyz 0 0 10))
|#
(define (translate-element ID point)
  (let ((t-msg (translatemsg* #:tx (cx point)
                              #:ty (cy point)
                              #:tz (cz point)
                              #:guid ID)))
    (write-msg "Translate" t-msg)
    ;;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    ))
#|
Function to rotate an element on the z-axis
 id: id of the element to rotate
     IMPORTANT: Working with slabs and columns at the moment.
 angle: angle of the rotation in radians
Example of usage: 
(send (rotate-element-z (create-slab slabPoints (list)) (* 45 DEGRAD)))
|#
(define (rotate-element-z ID angle [copy #f])
  (define eleList (if (list? ID) ID (list ID)))
  (let ((r-msg (rotatemsg* #:guid eleList
                           #:axis "z"
                           #:angle angle
                           #:copy copy)))
    (write-msg "RotateZ" r-msg)
    (define return-list (elementidlist-guid (read-sized (cut deserialize (elementidlist*) <>)input)))
    (if (equal? (length return-list) 1) (car return-list) return-list)
    ;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    ))

#|
Function to mirror an element on the x-axis
|#
(define (mirror-element-x ID [copy #t])
  (let ((msg (mirrormsg* #:guid ID
                         #:axis "x"
                         #:copy copy)))
    (write-msg "Mirror" msg)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))
#|
Function to mirror an element on the y-axis
|#
(define (mirror-element-y ID [copy #t])
  (let ((msg (mirrormsg* #:guid ID
                         #:axis "y"
                         #:copy copy)))
    (write-msg "Mirror" msg)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#|NOT WORKING
Function to trim an element
Receives the ID of two elements to trim 
Example of usage: (trim-element idWall idSlab)
|#
(define (trim-elements ID1 ID2)
  (let ((t-msg (trimmsg* #:guid1 ID1
                         #:guid2 ID2)))
    (write-msg "Trim" t-msg)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

#|
Function to intersect a wall with an element
 ID1: id of the element that will suffer the changes (wall)
 ID2: id of the element that may suffer changes (depends on destructive)
 destructive: If #t, will destroy both elements, and the result of the operation will be the intersection
              If #f, the second element will remain intact. Useful for the construction of Absolute Towers
Example of usage: 
(send (intersect-wall (create-wall (xy -15 0) (xy 15 0) 3.0) (create-slab slabPoints (list))))

|#
(define (intersect-wall ID1 ID2 [destructive #f])
  (let ((i-msg (intersectmsg* #:guid1 ID1
                              #:guid2 ID2)))
    (if destructive (write-msg "DestructiveIntersectWall" i-msg)(write-msg "IntersectWall" i-msg))
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    ))

#|
Function to create a column
 orig-pos: origin of column
 circle-based?: circle column or not
 angle: angle of the column
 depth: size of y-axis
 width: size of x-axis
Example of usage: 
(send (create-column (xy 0 0)))
(send (create-column (xy 0 0) #:slant-angle (/ pi 4)))
(send (create-column (xy 0 0) #:slant-angle (/ pi 4) #:slant-direction (/ pi 2)))
|#
(define (create-column orig-pos
              #:bottom-level [bottom-level (current-level)]
              #:top-level [top-level (upper-level #:level bottom-level)]
              ;;ArchiCAD ONLY --------------------------------------------------------------
              #:circle-based? [circle-based? #t]
              #:angle [angle 0]
              #:depth [depth 0.15]
              #:width [width 0.15]
              #:slant-angle [slant-angle (/ pi 2)]
              #:slant-direction [slant-direction 0])
  (let ((msg (columnmsg*  #:posx (cx orig-pos)
                          #:posy (cy orig-pos)
                          #:bottom 0
                          #:height 0
                          #:circlebased circle-based?
                          #:angle angle
                          #:depth depth
                          #:width width
                          #:slantangle slant-angle
                          #:slantdirection slant-direction
                          #:bottomindex (storyinfo-index bottom-level)
                          #:upperindex (storyinfo-index top-level))))
    (write-msg "NewColumn" msg)
    ;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    (let ((result (read-sized (cut deserialize (elementid*) <>)input)))
    (if (and (elementid-crashmaterial result) 
             (crash-on-no-material?))
        (begin 
          (disconnect)
          (error "The material does not exist"))
        (elementid-guid result)))))

#|
Function to create a object
 index: index that indentifies what object will be used (needs better documentation)
 orig-pos: position of the object
Example of usage: 
(send (create-object 1324 (xy 0.0 0.0)))
|#
(define (create-object index orig-pos)
  (let ((msg (objectmsg* #:index index
                         #:posx (cx orig-pos)
                         #:posy (cy orig-pos))))
    (write-msg "Object" msg)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    ))

#|
Function to create stairs
 index: index that indentifies what stairs will be used (needs better documentation)
 orig-pos: position of the stairs
|#
(define (create-stairs #:name name 
                #:orig-pos orig-pos 
                #:angle [angle 0] 
                #:x-ratio [x-ratio 1] 
                #:y-ratio [y-ratio 1]
                #:bottom-offset [bottom-offset 0] 
                #:bottom-level [bottom-level (current-level)]
                #:use-xy-fix-size [use-xy-fix-size #f])
  (let ((msg (stairsmsg* #:name name
                         #:posx (cx orig-pos)
                         #:posy (cy orig-pos)
                         #:bottom bottom-offset
                         #:xratio x-ratio
                         #:yratio y-ratio
                         #:angle angle
                         #:bottomindex (storyinfo-index bottom-level)
                         #:usexyfixsize use-xy-fix-size)))
    (write-msg "Stairs" msg)
    ;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    (let ((result (read-sized (cut deserialize (elementid*) <>)input)))
      (if (and (elementid-crashmaterial result)
               (crash-on-no-name?))
          (begin 
            (disconnect)
            (error "The name does not exist"))
          (elementid-guid result)))      
    ))
#|
Function to create a plane roof
 listpoints: list with the points that define the roof shape
 height: height of the roof
 listarcs: list with angles between two consecutive points
           ex: (list (* 90 DEGRAD) (* 45 DEGRAD)), this means
               between the first and second points of the roof there is an angle of 90º
               and between the second and third points one of 45º
Example of usage: 
(send (create-roof slabPoints 3))
|#
(define default-roof-alignment (make-parameter "Center"))
(define default-roof-type-of-material (make-parameter "Composite"))
#;(define default-roof-material  
  (make-parameter (cond [(eq? (default-roof-type-of-material) "Basic") "GENERIC - STRUCTURAL"]
                        [(eq? (default-roof-type-of-material) "Composite") "Generic Roof/Shell"])))
(define (create-roof guide
              #:bottom-level [bottom-level (current-level)]
              ;;ArchiCAD ONLY --------------------------------------------------------------
              #:thickness [thickness 0.3]
              #:height [height 0]
              #:type-of-material [type-of-material (default-roof-type-of-material)]
              #:material [material (cond [(eq? type-of-material "Basic") "GENERIC - STRUCTURAL"]
                                         [(eq? type-of-material "Composite") "Generic Roof/Shell"])])
  (let ((roof-msg (roofmsg* #:height height
                            #:material material
                            #:thickness thickness
                            #:type type-of-material
                            #:bottomlevel (storyinfo-index bottom-level))))
    (write-msg "NewRoof" roof-msg)  
    (send-points guide)
    ;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    (let ((result (read-sized (cut deserialize (elementid*) <>)input)))
    (if (and (elementid-crashmaterial result) 
             (crash-on-no-material?))
        (begin 
          (disconnect)
          (error "The material does not exist"))
        (elementid-guid result)))))
#|
Function to create a poly roof
|#
;;Auxiliar Function
(define (get-sub-polys sub-poly-list)
  (cond 
   [(null? sub-poly-list) (list)]
   [(list? (car sub-poly-list)) (append (list (length (car sub-poly-list))) (get-sub-polys (cdr sub-poly-list))) ]
   [else (list (length sub-poly-list))]))

(define (internal-create-poly-roof listpoints height listarcs thickness levels-angle levels-height material type)
  (let* ((msg (roofmsg* #:height height
                        #:material material
                        #:thickness thickness
                        #:type type))
         (roof-levels-msg (rooflevelsmsg* #:angle levels-angle
                                          #:height levels-height))
         (sub-poly-list (get-sub-polys listpoints))
         (sub-poly-msg (intlistmsg* #:ilist sub-poly-list)))
    
    (write-msg "PolyRoof" msg)
    (send-points (flatten listpoints))
    (send-arcs listarcs)
    (write-sized serialize sub-poly-msg output)
    (write-sized serialize roof-levels-msg output)
    ;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    (let ((result (read-sized (cut deserialize (elementid*) <>)input)))
    (if (and (elementid-crashmaterial result) 
             (crash-on-no-material?))
        (begin 
          (disconnect)
          (error "The material does not exist"))
        (elementid-guid result)))
    ))

(define create-poly-roof-material-default (make-parameter "GENERIC - STRUCTURAL"))
(define create-roof-material-default (make-parameter "GENERIC - STRUCTURAL"))
(define (create-poly-roof listpoints height [listarcs (list)] [thickness 0.3] [levels-angle (list)] [levels-height (list)] [material (create-roof-material-default)])
  (internal-create-poly-roof listpoints height listarcs thickness levels-angle levels-height material "BasicStructure"))

(define create-poly-roof-composite-material-default (make-parameter "Generic Roof/Shell"))
(define create-roof-composite-material-default (make-parameter "Generic Roof/Shell"))
(define (create-poly-roof-composite listpoints height [listarcs (list)] [thickness 0.3] [levels-angle (list)] [levels-height (list)] [material (create-roof-composite-material-default)])
  (internal-create-poly-roof listpoints height listarcs thickness levels-angle levels-height material "CompositeStructure"))

;(send (create-poly-roof (list (list (xy 10 10)(xy 10 -10)(xy -10 -10)(xy -10 10)(xy 10 10))(list (xy 5 0)(xy -5 0)(xy 5 0))) 0 (list 0 0 0 0 0 pi pi)))


(define default-mesh-material 
  (make-parameter "GENERIC - ENVIRONMENT"))
#|
(send (create-mesh (list (xyz 0 0 0)(xyz 5 0 5)(xyz 5 5 0) (xyz 0 5 0)(xyz 0 0 0))))
(send (create-mesh (list (xyz 0 0 0)(xyz 10 0 0)(xyz 10 10 0)(xyz 0 10 0)(xyz 0 0 0))))
(send (create-mesh (list (xyz 0 0 0)(xyz 5 0 0)(xyz 10 0 0)(xyz 10 5 0)(xyz 10 10 0)(xyz 5 10 0)(xyz 0 10 0)(xyz 0 5 0)(xyz 0 0 0))))
(send (create-mesh (list (xyz 0 0 0)(xyz 10 0 0)(xyz 10 10 0)(xyz 0 10 0)(xyz 0 0 0))
                     #:level-lines (list (xyz 2 2 2)(xyz 8 2 2)(xyz 8 8 2)(xyz 2 8 2))))
|#
(define (create-mesh guide
              #:bottom-level [bottom-level (current-level)]
              ;;ArchiCAD ONLY --------------------------------------------------------------
              #:bottom [bottom 0]
              #:material [material (default-mesh-material)]
              #:level-lines [level-lines (list)])
  (let ((slab-msg (meshmessage* #:level bottom
                                #:material material
                                #:bottomlevel (storyinfo-index bottom-level))))
    (write-msg "Mesh" slab-msg)  
    (send-points guide)
    (send-points level-lines)
    ;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    (let ((result (read-sized (cut deserialize (elementid*) <>)input)))
    (if (and (elementid-crashmaterial result) 
             (crash-on-no-material?))
        (begin 
          (disconnect)
          (error "The material does not exist"))
        (elementid-guid result)))))

;TODO
;dx = 3, dy = 2, dz = 1
(define (create-morph reference-point coords edges polygons)
  (let* ((msg (morphmsg* #:refx (cx reference-point)
                         #:refy (cy reference-point)
                         #:refz (cz reference-point)))
        (sub-poly-sizes (get-sub-polys polygons))
        (msg-poly-sizes (intlistmsg* #:ilist sub-poly-sizes)))
    (write-msg "Morph" msg)
    (send-points coords)
    (send-points edges)
    (send-points (flatten polygons))
    (write-sized serialize msg-poly-sizes output)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    ))

(define (create-box-2points point1 point2 #:bottom-level [bottom-level (current-level)])
  (let ((x1 (cx point1))
        (y1 (cy point1))
        (z1 (cz point1))
        (x2 (cx point2))
        (y2 (cy point2))
        (z2 (cz point2)))
    (create-morph
     (xyz 0 0 0)
     (list (xyz x1 y1 z1)
           (xyz x2 y1 z1)
           (xyz x2 y2 z1)
           (xyz x1 y2 z1)
           (xyz x1 y1 z2)
           (xyz x2 y1 z2)
           (xyz x2 y2 z2)
           (xyz x1 y2 z2))
     (list (xy 0 1)(xy 1 2)(xy 2 3)(xy 3 0)
           (xy 4 5)(xy 5 6)(xy 6 7)(xy 7 4)
           (xy 0 4)(xy 1 5)(xy 2 6)(xy 3 7))
     
     #;(list (list (xy 0 0)(xy 1 0) (xy 2 0)(xy 3 0))
             (list (xy 4 0)(xy 5 0) (xy 6 0)(xy 7 0))
             (list (xy 0 0)(xy 9 0) (xy 4 1)(xy 8 1))
             (list (xy 1 0)(xy 10 0)(xy 5 1)(xy 9 1))
             (list (xy 2 1)(xy 10 0)(xy 6 0)(xy 11 1))
             (list (xy 3 0)(xy 8 0) (xy 7 1)(xy 11 1)))
     
     (list (list (xy 0 0)(xy 1 0) (xy 2 0)(xy 3 0))
           (list (xy 4 0)(xy 5 0) (xy 6 0)(xy 7 0))
           (list (xy 0 0)(xy 9 0) (xy 4 1)(xy 8 1))
           (list (xy 1 0)(xy 10 0)(xy 5 1)(xy 9 1))
           (list (xy 2 1)(xy 10 0)(xy 6 0)(xy 11 1))
           (list (xy 3 1)(xy 11 0) (xy 7 0)(xy 8 1)))
     )))

(define (create-box point1 length width height #:bottom-level [bottom-level (current-level)])
  (create-box-2points point1 (+xyz point1 length width height) #:bottom-level bottom-level))

(define (points-regular-pyramid number-sides center radius rotation height)
  (let ((lst (list))
        (step (/ (* 2 pi) number-sides)))
    (for ([n number-sides])
      (set! lst (append lst (list (pol radius (+ rotation (* n step)))))))
    (append lst (list (xyz (cx center) (cy center) height)))))

(define (edges-regular-pyramid number-sides points)
  (let ((lst (list)))
    (for ([n number-sides])
      (if (= n (- number-sides 1))
          (set! lst (append lst (list (xy n 0))))
          (set! lst (append lst (list (xy n (+ n 1)))))))
    (for ([n number-sides])
      (set! lst (append lst (list (xy n (length points))))))
    lst
    ))

(define (polygon-regular-pyramid number-sides edges)
  (let ((lst (list))
        (base-polygon (list)))
    (for ([n number-sides])
      (set! base-polygon (append base-polygon (list (xy n 0)))))
    (set! lst (append lst (list base-polygon)))
    (for ([n number-sides])
      (if (= n (- number-sides 1))
          (set! lst (append lst (list (list (xy n 1)
                                            (xy number-sides 0)
                                            (xy (+ n number-sides) 1)))))
          (set! lst (append lst (list (list (xy n 1)
                                            (xy (+ n 1 number-sides) 0)
                                            (xy (+ n number-sides) 1)))))))
    lst))

(define (create-regular-pyramid number-sides center radius rotation height)
  (let* ((points (points-regular-pyramid number-sides center radius rotation height))
         (edges (edges-regular-pyramid number-sides points))
         (polygons (polygon-regular-pyramid number-sides edges)))
    (create-morph center 
                  points
                  edges
                  (list (list (xy 0 0)
                              (xy 1 0)
                              (xy 2 0)
                              (xy 3 0))
                        (list (xy 0 0)
                              (xy 4 0)
                              (xy 5 1))
                        (list (xy 1 0)
                              (xy 5 0)
                              (xy 6 1))
                        (list (xy 2 0)
                              (xy 6 0)
                              (xy 7 1))
                        (list (xy 3 0)
                              (xy 7 0)
                              (xy 4 1)))
                  )))

(define (create-box-cylinder p0 p1 radius height)
    (list))

(define (create-cylinder p0 radius p1 #:level [level (current-level)])
  (let ((sphere-to-create (cylindermsg* #:p0x (cx p0)
                                            #:p0y (cy p0)
                                            #:p0z (cz p0)
                                            #:p1x (cx p1)
                                            #:p1y (cy p1)
                                            #:p1z (cz p1)
                                            #:radius radius
                                            #:level (storyinfo-index level))))
    (write-msg "Cylinder" sphere-to-create)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))))

(define (check-level)
  (write-msg-name "CheckStory")
  (read-sized (cut deserialize (storyinfo*) <>)input))

#|
Function to create a level given a height, this uses absolute height
Will create a level with 10 height, if there isn't one
 height: height of the level
Example of usage: 
(send (create-level 10.0))
|#
(define (create-level #:height [height 0])
  (let ((msg (storymsg* #:height height
                        #:name "Story")))
    (write-msg "Story" msg)
    (read-sized (cut deserialize (storyinfo*) <>)input)))

(define (upper-level #:level [level (current-level)]
                     #:height [height (default-level-to-level-height)])
  (let ((msg (upperlevelmsg* #:height height
                             #:index (storyinfo-index level))))
    (write-msg "UpperLevel" msg)
    (read-sized (cut deserialize (storyinfo*) <>)input)))

(define (current-level-elevation)
  (storyinfo-level (current-level)))
(define (current-level-index)
  (storyinfo-index (current-level)))

#|
Function to delete all stories
Example of usage: 
(send (delete-stories))
|#
(define (delete-stories)
  (write-msg-name "DeleteStories"))
(define (delete-levels) (delete-stories))
#|
Function to delete list of elements
 elem-id: id of the element to be deleted, or list of ids to be deleted
Example of usage: 
(send (delete-elements elemID))                  
(send (delete-elements elemListID))

TODO: delete a door that was deleted before because the wall was deleted
|#
(define (delete-elements elem-id)
  (define eleList (if (list? elem-id) elem-id (list elem-id)))
  (let ((msg (elementidlist* #:guid eleList
                             #:crashmaterial #f)))
    (write-msg "Delete" msg)))

(define (extract-list-of-points points-x points-y points-z)
  (if (null? points-x)
      (list)
      (cons (xyz (car points-x)(car points-y)(car points-z))
            (extract-list-of-points (cdr points-x)(cdr points-y)(cdr points-z)))))

(define (get-levels)
  (write-msg-name "GetLevels")
  (read-sized (cut deserialize (levelrepeated*) <>) input))

(define (get-walls)
  (write-msg-name "GetWalls")
  (read-sized (cut deserialize (wallrepeated*) <>)input))

(define (get-slabs)
  (write-msg-name "GetSlabs")
  (read-sized (cut deserialize (slabrepeated*) <>)input))

(define (get-columns)
  (write-msg-name "GetColumns")
  (read-sized (cut deserialize (columnrepeated*) <>)input))

(define (get-objects)
  (write-msg-name "GetObjects")
  (read-sized (cut deserialize (objectrepeated*) <>)input))

(define (get-roofs)
  (write-msg-name "GetRoofs")
  (read-sized (cut deserialize (roofrepeated*) <>)input))

(define (recreate-levels [level-list (get-levels)])
  (for ([n (length (levelrepeated-levels level-list))])
       (create-level #:height (storyinfo-level (list-ref (levelrepeated-levels level-list) n))))
  )

(define (recreate-walls [wall-list (get-walls)])
  (delete-elements (wallrepeated-guid wall-list))
  (for ([n (length (wallrepeated-guid wall-list))])
       ;(displayln (list-ref (wallrepeated-material wall-list) n))
       ;(displayln (list-ref (wallrepeated-type wall-list) n))
       (create-wall (list (xy (list-ref (wallrepeated-p0x wall-list) n)
                              (list-ref (wallrepeated-p0y wall-list) n))
                          (xy (list-ref (wallrepeated-p1x wall-list) n)
                              (list-ref (wallrepeated-p1y wall-list) n)))
                    #:alignment (list-ref (wallrepeated-referenceline wall-list) n)
                    #:bottom-level (list-ref (wallrepeated-bottomlevel wall-list) n)
                    #:thickness (list-ref (wallrepeated-thickness wall-list) n)
                    #:angle (list-ref (wallrepeated-angle wall-list) n)
                    #:top-level (list-ref (wallrepeated-toplevel wall-list) n)
                    #:type-of-material (list-ref (wallrepeated-type wall-list) n)
                    #:material (list-ref (wallrepeated-material wall-list) n)
                    #:alpha-angle (list-ref (wallrepeated-alphaangle wall-list) n)
                    #:beta-angle (list-ref (wallrepeated-betaangle wall-list) n)
                    #:type-of-profile (list-ref (wallrepeated-typeprofile wall-list) n))))

(define (recreate-slabs [slab-list (get-slabs)])
  (delete-elements (slabrepeated-guid slab-list))
  (for ([n (length (slabrepeated-guid slab-list))])
       (let ((points (list-ref (slabrepeated-points slab-list) n)))
         ;(displayln (list-ref (slabrepeated-type slab-list) n))
         ;(displayln (list-ref (slabrepeated-material slab-list) n))
         (create-slab (extract-list-of-points (pointsmessage-px points)(pointsmessage-py points)(pointsmessage-pz points))
                      #:bottom-level (list-ref (slabrepeated-bottomlevel slab-list) n)
                      #:thickness (list-ref (slabrepeated-thickness slab-list) n)
                      #:type-of-material (list-ref (slabrepeated-type slab-list) n)
                      #:material (list-ref (slabrepeated-material slab-list) n)
                      #:sub-polygons (intlistmsg-ilist (list-ref (slabrepeated-subpolygons slab-list) n))))))

(define (recreate-columns [column-list (get-columns)])
  (delete-elements (columnrepeated-guid column-list))
  (for ([n (length (columnrepeated-guid column-list))])
       (create-column (xy (list-ref (columnrepeated-px column-list) n)(list-ref (columnrepeated-py column-list) n))
                    #:bottom-level (list-ref (columnrepeated-bottomlevel column-list) n)
                    #:top-level (list-ref (columnrepeated-toplevel column-list) n)
                    #:circle-based? (list-ref (columnrepeated-circular column-list) n)
                    #:angle (list-ref (columnrepeated-angle column-list) n)
                    #:depth (list-ref (columnrepeated-depth column-list) n)
                    #:width (list-ref (columnrepeated-width column-list) n)
                    #:slant-angle (list-ref (columnrepeated-slantangle column-list) n)
                    #:slant-direction (list-ref (columnrepeated-slantdirection column-list) n))))

(define (recreate-objects [object-list (get-objects)])
  (delete-elements (objectrepeated-guid object-list))
  (for ([n (length (objectrepeated-guid object-list))])
       (if (list-ref (objectrepeated-stairs object-list) n)
       (create-stairs #:name (list-ref (objectrepeated-name object-list) n)
                      #:orig-pos (xy (list-ref (objectrepeated-px object-list) n)(list-ref (objectrepeated-py object-list) n))
                      #:angle (list-ref (objectrepeated-angle object-list) n)
                      #:x-ratio (list-ref (objectrepeated-xratio object-list) n)
                      #:y-ratio (list-ref (objectrepeated-yratio object-list) n)
                      #:bottom-offset (list-ref (objectrepeated-bottomoffset object-list) n)
                      #:bottom-level (list-ref (objectrepeated-bottomlevel object-list) n)
                      #:use-xy-fix-size (list-ref (objectrepeated-usexyfixsize object-list) n))
       ;(create-object index orig-pos)
       (list))))

(define (recreate-roofs [roof-list (get-roofs)])
  (delete-elements (roofrepeated-guid roof-list))
  (for ([n (length (roofrepeated-guid roof-list))])
       (let ((points (list-ref (roofrepeated-points roof-list) n)))
         ;(displayln (list-ref (roofrepeated-type roof-list) n))
         ;(displayln (list-ref (roofrepeated-material roof-list) n))
         (create-roof (extract-list-of-points (pointsmessage-px points)(pointsmessage-py points)(pointsmessage-pz points))
                      #:bottom-level (list-ref (roofrepeated-bottomlevel roof-list) n)
                      #:thickness (list-ref (roofrepeated-thickness roof-list) n)
                      #:type-of-material (list-ref (roofrepeated-type roof-list) n)
                      #:material (list-ref (roofrepeated-material roof-list) n)
                      ;Currently roofs don't have sub-polygons, i.e. holes. In the future it may be added
                      ;#:sub-polygons (intlistmsg-ilist (list-ref (roofrepeated-subpolygons roof-list) n))
                      #:height (list-ref (roofrepeated-height roof-list) n)))))
#|
Had to do change the function ClickAnElem provided by the API
It wasn't working correctly, the action of selecting an element was being
cancelled without anything occuring, I suspect that it cancelled due to
ArchiCAD not being selected.
|#
(define (select-element)
  (write-msg-name "SelectElement")
  (elementid-guid (read-sized (cut deserialize (elementid*) <>)input)))

(define (highlight-element elem-id)
  (let* ((eleList (if (list? elem-id)
                      elem-id
                      (list elem-id)))
         (msg (elementidlist* #:guid eleList
                              #:crashmaterial #f)))
    (write-msg "Highlight" msg)))

;;Function to quit
(define (disconnect)
  (write-msg-name "quit")
  "quit successful")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;Function to create a Lemon
(define (create-lemon)
  (write-msg-name "Lemon"))

;;Function to create a Chapel no communication
(define (create-chapel-no-com)
  (write-msg-name "ChapelNoCom"))

;(provide (all-defined-out))


;;TEST FUNCTION
(define (test-function)
    (write-msg-name "Test")
    ;;(elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
  )
(define (test-function-msg msg)
  (write-msg "Test" msg))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;Defines to help with Demos
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define tmat (list 1 0 0
                   0 0 0 
                   -1 0 0 
                   1 0 0))
(define lstpoints (list (xy 0.0 0.0) 
                        (xy 0.4 5.0) 
                        (xy 1.0 5.0)
                        (xy 1.0 6.0)
                        (xy 1.7 6.0)
                        (xy 1.7 7.0)
                        (xy 2.4 7.0)
                        (xy 2.4 7.4)
                        (xy 0.0 7.7)
                        (xy 0.0 8.0)
                        (xy 8.0 10.0)
                        (xy 12.0 0.0)
                        (xy 0.0 0.0)))
(define lstarcs (list (list 1 2 -0.143099565651258 )
                      (list 10 11 0.566476134070805)
                      (list 11 12 0.385936923743763)))
(define hpoints (list (xy -1.5 -0.3) 
                        (xy -1.5 3.1) 
                        (xy 1.5 3.1)
                        (xy 1.5 -0.3)
                        (xy -1.5 -0.3)))
(define harcs (list (list 2 3 (* -240 DEGRAD))))
(define hheight -5.2)
(define htmat (list 1 0 0 0 
                    0 1 0 0
                    0 0 1 10))

(define cPoints (list (xy 0.0 0.0)
                      (xy 0.0 10.0)
                      ;;(xy 10.0 10.0)
                      ;;(xy 10.0 0.0)
                      (xy 0.0 0.0)
                      ))

(define cArcs (list (* 90 DEGRAD)
                    (* 90 DEGRAD)
                    ))

(define (eight-spheres)
  (create-sphere 0.0 0.0 0.0001 0.0 0.0)
  (create-sphere 0.0 0.0 0.0001 0.0 1.0)
  (create-sphere 0.0 0.0 0.0001 0.0 -1.0)
  
  (create-sphere 0.0 0.0 0.0001 1.0 1.0)
  (create-sphere 0.0 0.0 0.0001 1.0 -1.0)
  
  (create-sphere 0.0 0.0 0.0001 -1.0 0.0)
  (create-sphere 0.0 0.0 0.0001 -1.0 1.0)
  (create-sphere 0.0 0.0 0.0001 -1.0 -1.0)
)

(define slabPoints (list (xy 0.0 0.0)
                         (xy 10.0 0.0)
                         (xy 10.0 10.0)
                         (xy 0.0 10.0)
                         (xy 0.0 0.0)))

(define hole-points (list (xy 2.0 2.0)
                          (xy 8.0 2.0)
                          (xy 8.0 8.0)
                          (xy 2.0 8.0)
                          (xy 2.0 2.0)))

;;Park of trees:
;;(send (park 1324 (xy -10 -10) 10 10 8))
;;other objects id's: 1366 1362
(define (park object-index orig-pos rows columns distance-between-trees)
  (for ([c columns])
    (for ([r rows])
      (create-object object-index (xy (+ (cx orig-pos) (* r distance-between-trees)) (+ (cy orig-pos)(* c distance-between-trees)))))))

(define (see-object n until step)
  (create-object n (xy (+ (- until n) step) 0))
  (if (eq? n until)
      (list)
      (see-object (+ n 1) until (+ step 10))))

(define (create-default-object orig-pos)
  (let ((msg (objectmsg* #:index 0
                         #:posx (cx orig-pos)
                         #:posy (cy orig-pos))))
    (write-msg "DefaultObject" msg)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input))
    ))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;Examples:
;;
;; (send (write-msg-name "Test"))
;;
;; (send (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0) (create-wall (xy 1.0 1.0) (xy 3.0 1.0) 3.0))
;;

;using IDs
;; (send (define wallId (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0)) (define wallId2 (create-wall (xy 1.0 1.0) (xy 3.0 1.0) 3.0)))
;; (send (define sphereId (create-sphere 0.0 0.0 0.0001 0.0 0.0)) (define sphereId2 (create-sphere 0.0 0.0 0.0001 1.0 1.0)))

;Wall
;; (send (create-wall (xy 0.0 0.0) (xy 10.0 0.0) 10.0))
;; (send (create-wall(xy 0.0 0.0) (xy 0.0 10.0) 3.0 0.3 (* 90 DEGRAD)) (create-wall(xy 0.0 10.0) (xy 0.0 0.0) 3.0 0.3 (* 90 DEGRAD)))

;Doors
;; (send (define wallId (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0) )(create-door wallId 1.0 0.0) (define wallId2 (create-wall (xy 1.0 1.0) (xy 3.0 1.0) 3.0)) (create-door wallId2 1.0 1.0))
;;= (send (create-door (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0) 1.0 0.0) (create-story-above 10) (create-door (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0) 1.0 0.0))

;Window and Door
;; (send (define wallId (create-wall (xy 0.0 0.0) (xy 5.0 0.0) 3.0)) (create-window wallId 4.0 0.635) (create-door wallId 1.0 0.0))

;creating a chapel
;; (send (create-chapel))

;Shell
;; (send (create-complex-shell tmat lstpoints lstarcs 1 hpoints harcs hheight htmat 0.0 0.0))
;; (send (create-simple-shell lstpoints))
;; (send (create-shell lstpoints lstarcs))
;; (send (rotate-shell "x" 90 (create-shell lstpoints lstarcs)))
;; (send (let ((shell-id (create-shell lstpoints lstarcs)))(rotate-shell "x" 90 shell-id)(rotate-shell "y" 90 shell-id)))
;; (send (rotate-shell "y" 90 (rotate-shell "x" 90 (create-shell lstpoints lstarcs))))
;; (send (translate-shell (list 0 5 0) (create-shell lstpoints lstarcs)))
;; (send (translate-shell (list 11 11 0) (rotate-shell "x" 90 (create-shell lstpoints lstarcs))))

;Shell Hole
;; (send (create-hole-on-slab hpoints harcs hheight (translate-shell (list 11 11 0) (rotate-shell "x" 90 (create-shell lstpoints lstarcs)))))
;; (send (create-hole-on-slab hpoints harcs hheight (create-shell lstpoints lstarcs)))
;; (send (create-hole-on-slab hpoints harcs hheight (create-hole-on-slab hpoints harcs hheight (create-shell lstpoints lstarcs))))
;; (send (rotate-shell "y" 180 (create-hole-on-slab hpoints harcs hheight (create-shell lstpoints lstarcs))))
;; (send (create-hole-on-slab hpoints harcs hheight (rotate-shell "y" 180 (create-hole-on-slab hpoints harcs hheight (create-shell lstpoints lstarcs)))))

;Curtain Walls
;; (send (create-curtain-wall cPoints cArcs 5))
;;= (send (create-curtain-wall cPoints cArcs 5) (create-story-above 10) (create-curtain-wall cPoints cArcs 5))
;; (send (add-arcs (create-curtain-wall cPoints (list) 5) cArcs))
;; (send (add-arcs (add-arcs (create-curtain-wall cPoints (list) 5) cArcs) cArcs))

;Translate Curtain Wall
;; (send (translate-element (xyz 0 0 10) (create-curtain-wall cPoints cArcs 5)))

;Slab
;; (send (create-slab slabPoints (list) 0.0))
;; (send (rotate-element (create-slab slabPoints (list) 0.0) "z" (* 45 DEGRAD)))
;; (send (create-slab cPoints cArcs 0.0))

;Create walls or curtain walls on a regular slab
;; (send (create-walls-from-slab (create-slab slabPoints (list) 0.0) 5.0))
;;= (send (create-walls-from-slab (create-slab slabPoints (list) 0.0) 5.0) (create-story-above 5) (create-walls-from-slab (create-slab slabPoints (list)) 5.0))
;; (send (create-cwalls-from-slab (create-slab slabPoints (list) 0.0) 5.0))

;Create walls or curtain walls on an irregular slab
;; (send (create-walls-from-slab (create-slab slabPoints cArcs 0.0) 5.0))
;; (send (create-cwalls-from-slab (create-slab slabPoints cArcs 0.0) 5.0))

;Create Slab For Absolute Tower
;; (send (create-slab ATslab1 (list) 0.0))
;; (send (rotate-element (create-slab ATslab1 (list) 0.0) "z" (* 90 DEGRAD)))

;Trim Elements
;; (send (trim-elements (create-wall (xy -5.0 5.0) (xy 15.0 5.0) 3.0) (create-slab slabPoints (list)) ))

;PolyWall
;;(send (create-multi-wall (list (xy 0.0 0.0) (xy -10.0 0.0) (xy -10.0 10.0) (xy 0.0 10.0)) (list) 3.0 0.0 1.0))
;;;;create door into polywall
;;;; (send (define laux1 (create-multi-wall (list (xy 0.0 0.0) (xy -10.0 0.0) (xy -10.0 10.0) (xy 0.0 10.0)) (list) 3.0 0.0 1.0)))
;;;; followed by (send (create-door (car laux1) 1.0 0.0))

;True PolyWall - Problems with windows and orientation
;;(send (wallTest (list (xy 0.0 0.0) (xy -10.0 0.0) (xy -10.0 10.0) (xy 0.0 10.0)) (list) 3.0 0.0 1.0))
;;(send (wallTest (list (xy 0.0 0.0) (xy 10.0 0.0) (xy 10.0 10.0) (xy 0.0 10.0)) (list) 3.0 0.0 1.0))
;;(send (wallTest (list (xy 10.0 0.0) (xy 0.0 0.0) (xy 0.0 10.0) (xy 10.0 10.0)) (list) 3.0 0.0 1.0))

;Intersect Wall
;; (send (intersect-wall (create-wall (xy 5.0 0.0) (xy 15.0 0.0) 3.0) (create-slab slabPoints (list)) ))
;; (send (intersect-wall (create-wall (xy -5.0 5.0) (xy 15.0 5.0) 3.0) (create-slab slabPoints (list)) ))
;; (send (intersect-wall (create-wall (xy 40.0 0.0) (xy 40.0 50.0) 3.0) (create-slab slabPoints (list)) #t))

;Column
;; (send (create-column (xyz 0.0 0.0 0.0) 0.0 10.0 true 360 5.0 5.0))

;Story
;; (send (create-story-below 10.0) (create-slab slabPoints (list)) (create-story-below 10.0) (create-slab slabPoints (list)) (create-story-below 10.0) (create-slab slabPoints (list)))
;; (send (create-story-above 10.0) (create-story-above 10.0) (define current-story-information (check-story)))
;; (send (create-story-below 10.0) (create-story-below 10.0) (define current-story-information (check-story)))
;; (send (create-story 10) (create-story 20) (create-story 10))

;Delete
;; (send (delete-elements (create-wall (xy -5.0 5.0) (xy 15.0 5.0) 3.0)))
;;= (send (define wallIDAux (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0)) (create-door wallIDAux 1.0 0.0) (delete-elements wallIDAux))


;Mixed Tests
;; (send (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0) (create-curtain-wall cPoints cArcs 5) (create-story-above 10) (create-wall (xy 0.0 0.0) (xy 3.0 0.0) 3.0)(create-curtain-wall cPoints cArcs 5))



#|
(define (superellipse p a b n t)
  (+xy p (* a (expt (expt (cos t) 2) (/ 1 n)) (sgn (cos t)))
       (* b (expt (expt (sin t) 2) (/ 1 n)) (sgn (sin t)))))

(define (points-superellipse p a b n n-points)
  (map (lambda (t) (superellipse p a b n t))
       (division -pi pi n-points #f)))

(define p (xyz 0 0 0))
(define d 6.3)
(define l-corridor 2.2)
(define t 0.3)
(define r-small 15)
(define daux6 (- 6.3 l-corridor))
(define central-hole (list (+xy p (- (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))
                           (+xy p (+ (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))
                           (+xy p (+ (+ daux6 (/ t 2))) (+ (+ daux6 (/ t 2))))
                           (+xy p (- (+ daux6 (/ t 2))) (+ (+ daux6 (/ t 2))))
                           (+xy p (- (+ daux6 (/ t 2))) (- (+ daux6 (/ t 2))))))


(define (create-several-stories number)
  (for ([i number])
    (let* ((slab-points-aux (points-superellipse (xy 0 0) 26 21 1.75 50))
           (slab-points (append slab-points-aux (list (car slab-points-aux))))
           ;(slab-id (create-slab slab-points (list)))
           ;(slab-id (create-slab cPoints cArcs))
           (slab-id (create-slab slabPoints))
           )
      (create-walls-from-slab slab-id 10)
      ;(create-hole-slab slab-id central-hole)
      (create-hole-slab slab-id hole-points)
      ;(rotate-element-z slab-id (* i 10))
      
      (checked-create-story-above 10))))

(define (test-slab)
  (define slab-id (create-slab slabPoints))
  (create-walls-from-slab slab-id 10)
  (create-hole-slab slab-id hole-points)
  (checked-create-story-above 10)
  (set! slab-id (create-slab slabPoints))
  (create-walls-from-slab slab-id 10)
  (create-hole-slab slab-id hole-points)
  ;(delete-elements slab-id)
  )
|#

#| DIFFERENT WAY OF DOING create-slab FUNCTION - USING KEYWORDS
(define create-slab 
  (lambda (listpoints #:listarcs [listarcs (list)] #:bottomOffset [bottomOffset 0] #:material [material 60] #:thickness [thickness 0.0])
    (let ((slab-msg (slabmessage* #:level bottomOffset
                                  #:material material
                                  #:thickness thickness)))
    (write-msg "Slab" slab-msg)  
    (send-points listpoints)
    (send-arcs listarcs)
    (elementid-guid (read-sized (cut deserialize (elementid*) <>)input)))))
|#



