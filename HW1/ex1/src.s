						EXPORT SystemInit
            EXPORT __main

						AREA VALUES, DATA, READONLY
NUMELEMS    EQU 10
INITELEMS   DCD 9,3,5,0,7,4,8,1,2,6

						AREA ARRAY, DATA, READWRITE
ELEMS       DCD 0,0,0,0,0,0,0,0,0,0
	
						AREA PROG,CODE,READONLY 

SystemInit FUNCTION
						; initialization code
						LDR R0, =INITELEMS              ;R0 = ADR OF INITELEMS
            LDR R1, =NUMELEMS               ;R1 = NUMELEMS
						LDR R2, =ELEMS                  ;R2 = ADR OF ELEMS
            MOV R3, #0                      ;i = R3 = 0
            BL COPY                         ;COPY ELEMNTS OF VALUES TO ELEMS 
            B INSSORT                       ;INSERTION SORT (ELEMS)                
 ENDFUNC


__main FUNCTION
COPY
            PUSH{R4}                        ;ABI RULES
COPY_LOOP                                   ;COPY INITELEMS TO ELEMS 
            CMP R3, R1                      ;i < len(NUMELEMS)
            BGE OUT_COPY
            LDR R4, [R0, R3, LSL #2]        ;R4 = INITELEMS[i]
            STR R4, [R2, R3, LSL #2]        ;ELEMS[I] = R4
            ADD R3, #1                      ;i++
            B COPY_LOOP
OUT_COPY
            POP{R4}                         ;ABI RULES
            BX LR

INSSORT
            PUSH{R4, R5}                     ;ABI RULES
            MOV R3, #1                       ;i = R0 = 0
FOR_LOOP      
            CMP R3, R1                       ;i < len(NUMELEMS)
            BGE OUT_INSSORT
            LDR R4, [R2, R3, LSL #2]         ;KEY = R4 = ELEMS[I]
            MOV R12, R3                     
            SUB R12, #1                      ;J = I - 1
WHILE_LOOP
            CMP R12 , #0                     ;J >= 0
            BLT OUT_WHILE_LOOP
            LDR R5, [R2, R12, LSL #2]        ;R5 = ELEMS[J]
            CMP R4, R5                       ;KEY < ELEMS[J]
            BGT OUT_WHILE_LOOP
            ADD R12, #1
            STR R5, [R2, R12, LSL #2]        ;ELEMS[J + 1] = ELEMS[J]
            SUB R12, #2                      ;J--
            B WHILE_LOOP

OUT_WHILE_LOOP
            ADD R12, #1                      
            STR R4, [R2, R12, LSL #2]        ;ELEMS[J + 1] = KEY
            ADD R3, #1                       ;I++
            B FOR_LOOP

OUT_INSSORT
            POP{R4,R5}                       ;ABI RULES
            B OUT

OUT 
            B OUT

 ENDFUNC
            END